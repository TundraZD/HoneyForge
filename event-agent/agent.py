#!/usr/bin/env python3
import hashlib
import json
import logging
import os
import random
import re
import sqlite3
import subprocess
import threading
import time
import urllib.error
import urllib.parse
import urllib.request
import uuid
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DB_PATH = Path(os.getenv(
    "HONEYFORGE_QUEUE_DB", str(ROOT / "data" / "event-agent.sqlite3")
))
DLQ_PATH = ROOT / "data" / "dead-letter" / "events.jsonl"
CONTAINERS = [
    item.strip()
    for item in os.getenv(
        "HONEYFORGE_CONTAINERS", "trap-001,honeyforge-http-001"
    ).split(",") if item.strip()
]
API_URL = os.getenv("HONEYFORGE_API_URL", "").strip()
API_KEY = os.getenv("HONEYFORGE_API_KEY", "").strip()
MAX_AGE = 24 * 60 * 60
STOP_SENDING = threading.Event()

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s %(levelname)s %(message)s",
)
log = logging.getLogger("honeyforge.event-agent")


def connect():
    DB_PATH.parent.mkdir(parents=True, exist_ok=True)
    db = sqlite3.connect(DB_PATH, timeout=10)
    db.execute("PRAGMA busy_timeout=10000")
    return db


def init_db():
    with connect() as db:
        db.execute("PRAGMA journal_mode=WAL")
        db.execute("""
            CREATE TABLE IF NOT EXISTS events (
                event_id TEXT PRIMARY KEY,
                source_key TEXT NOT NULL UNIQUE,
                event_json TEXT NOT NULL,
                status TEXT NOT NULL DEFAULT 'pending',
                attempts INTEGER NOT NULL DEFAULT 0,
                next_attempt REAL NOT NULL DEFAULT 0,
                created_at REAL NOT NULL,
                last_error TEXT
            )
        """)
        db.execute("""
            CREATE INDEX IF NOT EXISTS idx_pending
            ON events(status, next_attempt, created_at)
        """)
        db.execute("""
            CREATE TABLE IF NOT EXISTS collector_state (
                container TEXT PRIMARY KEY,
                cursor REAL NOT NULL
            )
        """)


def queue_event(container, raw):
    try:
        event = json.loads(raw)
    except (json.JSONDecodeError, TypeError):
        log.warning("Ignoring non-JSON log line from %s", container)
        return

    if not isinstance(event, dict):
        return

    if event.get("event_type") in ("trap_started", "trap_startup_failed"):
        return

    if not event.get("trap_ref") or not event.get("source_ip"):
        log.warning("Skipping event missing trap_ref/source_ip from %s", container)
        return

    if not event.get("event_type"):
        log.warning("Skipping event missing event_type from %s", container)
        return

    event["event_id"] = event.get("event_id") or str(uuid.uuid4())
    event["timestamp"] = event.get("timestamp") or datetime.now(
        timezone.utc
    ).isoformat().replace("+00:00", "Z")
    event.setdefault("payload", {})

    serialized = json.dumps(event, separators=(",", ":"), ensure_ascii=False)
    source_key = hashlib.sha256(
        (container + "\0" + raw).encode("utf-8", errors="replace")
    ).hexdigest()

    with connect() as db:
        cur = db.execute("""
            INSERT OR IGNORE INTO events
                (event_id, source_key, event_json, created_at)
            VALUES (?, ?, ?, ?)
        """, (event["event_id"], source_key, serialized, time.time()))

    if cur.rowcount:
        log.info(
            "Queued event_id=%s trap_ref=%s type=%s",
            event["event_id"], event["trap_ref"], event["event_type"]
        )


def collect_container(container):
    # Polling avoids relying on a long-running docker logs --follow process.
    while True:
        try:
            with connect() as db:
                row = db.execute(
                    "SELECT cursor FROM collector_state WHERE container=?",
                    (container,),
                ).fetchone()

            now = time.time()
            cursor = row[0] if row else now - 5
            # Overlap one second; SQLite source_key deduplicates repeated lines.
            since = max(0, cursor - 1)

            result = subprocess.run(
                [
                    "docker", "logs", "--timestamps",
                    "--since", str(since), container,
                ],
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                timeout=20,
                check=False,
            )

            if result.returncode != 0:
                log.warning(
                    "Cannot read logs for %s: %s",
                    container, (result.stdout or "").strip()[:300]
                )
                time.sleep(3)
                continue

            for line in (result.stdout or "").splitlines():
                # Docker --timestamps prepends an RFC3339 timestamp.
                match = re.match(r"^\S+ (.*)$", line)
                raw = match.group(1) if match else line
                if raw.strip():
                    queue_event(container, raw)

            with connect() as db:
                db.execute("""
                    INSERT INTO collector_state(container, cursor)
                    VALUES (?, ?)
                    ON CONFLICT(container) DO UPDATE SET cursor=excluded.cursor
                """, (container, now))

        except subprocess.TimeoutExpired:
            log.warning("docker logs timed out for %s", container)
        except Exception:
            log.exception("Collector error for %s", container)

        time.sleep(2)


def mark_dead(event_id, event_json, reason):
    DLQ_PATH.parent.mkdir(parents=True, exist_ok=True)
    record = {
        "dead_lettered_at": datetime.now(timezone.utc).isoformat(),
        "reason": reason,
        "event": json.loads(event_json),
    }

    with DLQ_PATH.open("a", encoding="utf-8") as f:
        f.write(json.dumps(record, ensure_ascii=False) + "\n")
        f.flush()
        os.fsync(f.fileno())

    with connect() as db:
        db.execute(
            "UPDATE events SET status='dead', last_error=? WHERE event_id=?",
            (reason, event_id),
        )

    log.error("Moved event_id=%s to dead-letter: %s", event_id, reason)


def next_event():
    with connect() as db:
        return db.execute("""
            SELECT event_id, event_json, attempts, created_at
            FROM events
            WHERE status='pending' AND next_attempt <= ?
            ORDER BY created_at LIMIT 1
        """, (time.time(),)).fetchone()


def schedule_retry(event_id, attempts, reason, retry_after=None):
    delay = min(60, 2 ** min(attempts, 6))
    delay = (
        retry_after if retry_after is not None
        else delay * random.uniform(0.8, 1.2)
    )

    with connect() as db:
        db.execute("""
            UPDATE events
            SET attempts=attempts+1, next_attempt=?, last_error=?
            WHERE event_id=?
        """, (time.time() + delay, reason[:500], event_id))

    log.warning(
        "Retry in %.1fs for event_id=%s: %s", delay, event_id, reason
    )


def send_event(event_id, event_json, attempts, created_at):
    if time.time() - created_at > MAX_AGE:
        mark_dead(event_id, event_json, "event older than 24 hours")
        return

    request = urllib.request.Request(
        API_URL,
        data=event_json.encode("utf-8"),
        headers={
            "Content-Type": "application/json",
            "X-API-Key": API_KEY,
        },
        method="POST",
    )

    status = None
    retry_after = None

    try:
        with urllib.request.urlopen(request, timeout=10) as response:
            status = response.status
    except urllib.error.HTTPError as error:
        status = error.code
        retry_after = error.headers.get("Retry-After")
    except Exception as error:
        schedule_retry(event_id, attempts, type(error).__name__)
        return

    if status in (200, 201):
        with connect() as db:
            db.execute("""
                UPDATE events SET status='sent', last_error=NULL
                WHERE event_id=?
            """, (event_id,))
        log.info("Delivered event_id=%s HTTP %s", event_id, status)

    elif status in (400, 404, 422):
        mark_dead(event_id, event_json, f"permanent HTTP error {status}")

    elif status in (401, 403):
        STOP_SENDING.set()
        log.critical(
            "API rejected credentials (HTTP %s); sending paused. "
            "Correct credentials and restart the agent.", status
        )

    else:
        try:
            retry_after = (
                max(0, min(300, int(retry_after))) if retry_after else None
            )
        except ValueError:
            retry_after = None
        schedule_retry(event_id, attempts, f"HTTP {status}", retry_after)


def sender_loop():
    while True:
        if STOP_SENDING.is_set():
            time.sleep(5)
            continue

        row = next_event()
        if row is None:
            time.sleep(1)
            continue

        send_event(*row)


def validate_config():
    if bool(API_URL) != bool(API_KEY):
        raise SystemExit(
            "Set both HONEYFORGE_API_URL and HONEYFORGE_API_KEY, or neither."
        )

    if API_URL:
        parsed = urllib.parse.urlparse(API_URL)
        local = parsed.hostname in ("localhost", "127.0.0.1", "::1")

        if parsed.scheme not in ("http", "https") or not parsed.hostname:
            raise SystemExit("Invalid HONEYFORGE_API_URL")

        if parsed.scheme != "https" and not local:
            raise SystemExit("HTTPS is required except for localhost testing")


def main():
    validate_config()
    init_db()

    log.info("Queue database: %s", DB_PATH)
    log.info("Containers: %s", ", ".join(CONTAINERS))
    log.info("API sender: %s", "enabled" if API_URL else "disabled (queue-only)")

    for container in CONTAINERS:
        threading.Thread(
            target=collect_container, args=(container,), daemon=True
        ).start()

    if API_URL:
        threading.Thread(target=sender_loop, daemon=True).start()

    try:
        while True:
            time.sleep(3600)
    except KeyboardInterrupt:
        log.info("Stopping event-agent")


if __name__ == "__main__":
    main()
