import json
import logging
import os
import re
from datetime import datetime, timezone
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlsplit

import yaml

PROFILE_PATH = Path(os.environ.get("HONEYFORGE_PROFILE", "/app/profile.yaml"))

logging.basicConfig(level=logging.INFO, format="%(message)s")
logger = logging.getLogger("honeyforge.http")

INDICATORS = (
    ("jndi_expression", re.compile(r"\$\{[^}\r\n]{0,300}jndi\s*:", re.I)),
    ("nested_expression", re.compile(r"\$\{\$\{::?-", re.I)),
    ("naming_uri", re.compile(r"\b(?:ldap|ldaps|rmi|dns)://", re.I)),
)


def emit_event(profile, event_type, source_ip=None, **payload):
    event = {
        "timestamp": datetime.now(timezone.utc).isoformat().replace("+00:00", "Z"),
        "trap_ref": profile["trap"]["id"],
        "profile": profile["metadata"]["name"],
        "event_type": event_type,
        "source_ip": source_ip,
        "dest_port": profile["network"]["port"],
        "protocol": profile["network"]["protocol"],
        "payload": payload,
    }
    logger.info(json.dumps(event, separators=(",", ":")))


def load_profile():
    with PROFILE_PATH.open("r", encoding="utf-8") as file:
        profile = yaml.safe_load(file)

    if not isinstance(profile, dict):
        raise ValueError("Profile must be a YAML mapping")
    if profile.get("api_version") != "honeyforge/v1":
        raise ValueError("Unsupported profile API version")

    trap = profile.get("trap", {})
    network = profile.get("network", {})
    behavior = profile.get("behavior", {})
    telemetry = profile.get("telemetry", {})
    detection = profile.get("detection", {})

    if trap.get("type") != "http_decoy":
        raise ValueError("Unsupported trap type")
    if trap.get("level") != "low":
        raise ValueError("This implementation supports Low traps only")
    if network.get("protocol") != "tcp":
        raise ValueError("Only TCP transport is supported")

    port = network.get("port")
    if not isinstance(port, int) or isinstance(port, bool) or not 1 <= port <= 65535:
        raise ValueError("Invalid TCP port")

    for key in ("server_banner", "page_title", "page_heading"):
        if not isinstance(behavior.get(key), str) or not behavior[key]:
            raise ValueError(f"behavior.{key} must be a non-empty string")

    for key in ("normal_status", "suspicious_status"):
        status = behavior.get(key)
        if not isinstance(status, int) or isinstance(status, bool) or not 100 <= status <= 599:
            raise ValueError(f"behavior.{key} must be an HTTP status code")

    max_body = behavior.get("max_body_bytes")
    timeout = behavior.get("read_timeout_seconds")
    if not isinstance(max_body, int) or isinstance(max_body, bool) or not 0 <= max_body <= 1048576:
        raise ValueError("behavior.max_body_bytes must be between 0 and 1048576")
    if not isinstance(timeout, (int, float)) or isinstance(timeout, bool) or timeout <= 0:
        raise ValueError("behavior.read_timeout_seconds must be positive")

    if not isinstance(detection.get("log4shell_indicators"), bool):
        raise ValueError("detection.log4shell_indicators must be a boolean")
    for key in ("log_connections", "log_received_data"):
        if not isinstance(telemetry.get(key), bool):
            raise ValueError(f"telemetry.{key} must be a boolean")

    if not isinstance(trap.get("id"), str) or not isinstance(profile.get("metadata", {}).get("name"), str):
        raise ValueError("Profile must contain string trap.id and metadata.name")

    return profile


class DecoyHandler(BaseHTTPRequestHandler):
    server_version = "Apache"
    sys_version = ""

    def version_string(self):
        return self.server.profile["behavior"]["server_banner"]

    def setup(self):
        super().setup()
        self.connection.settimeout(
            self.server.profile["behavior"]["read_timeout_seconds"]
        )

    def log_message(self, fmt, *args):
        # Avoid the default unstructured access log.
        pass

    def do_GET(self):
        self.handle_request()

    def do_POST(self):
        self.handle_request()

    def do_PUT(self):
        self.handle_request()

    def do_PATCH(self):
        self.handle_request()

    def do_DELETE(self):
        self.handle_request()

    def do_HEAD(self):
        self.handle_request(head_only=True)

    def handle_request(self, head_only=False):
        profile = self.server.profile
        behavior = profile["behavior"]
        telemetry = profile["telemetry"]
        detection = profile["detection"]
        source_ip = self.client_address[0]

        try:
            content_length = int(self.headers.get("Content-Length", "0"))
            if content_length < 0:
                raise ValueError
        except ValueError:
            self.send_error(400)
            return

        max_body = behavior["max_body_bytes"]
        body = self.rfile.read(min(content_length, max_body)) if content_length else b""

        # If a request exceeds the inspection limit, close rather than parse
        # remaining bytes as another request on the same connection.
        if content_length > max_body:
            self.close_connection = True

        request_path = urlsplit(self.path).path[:256]
        indicators = []

        if detection["log4shell_indicators"]:
            inspected = [self.path[:2048]]
            inspected.extend(value[:2048] for _, value in self.headers.items())
            inspected.append(body.decode("latin-1", errors="replace"))

            for name, pattern in INDICATORS:
                if any(pattern.search(part) for part in inspected):
                    indicators.append(name)

        suspicious = bool(indicators)
        status = behavior["suspicious_status"] if suspicious else behavior["normal_status"]

        page = (
            "<!doctype html><html><head><title>"
            + behavior["page_title"]
            + "</title></head><body><h1>"
            + behavior["page_heading"]
            + "</h1></body></html>"
        ).encode("utf-8")

        self.send_response(status)
        self.send_header("Content-Type", "text/html; charset=utf-8")
        self.send_header("Content-Length", str(len(page)))
        self.send_header("Connection", "close")
        self.end_headers()

        if not head_only:
            try:
                self.wfile.write(page)
            except (BrokenPipeError, ConnectionResetError):
                pass

        if telemetry["log_connections"] or telemetry["log_received_data"]:
            emit_event(
                profile,
                "http_request",
                source_ip,
                method=self.command,
                path=request_path,
                body_bytes=len(body),
                suspicious=suspicious,
                indicators=indicators,
                status_code=status,
                body_truncated=content_length > max_body,
            )


def main():
    profile = load_profile()
    port = profile["network"]["port"]

    class ConfiguredServer(ThreadingHTTPServer):
        daemon_threads = True
        allow_reuse_address = True

    server = ConfiguredServer(("0.0.0.0", port), DecoyHandler)
    server.profile = profile

    emit_event(profile, "trap_started")
    try:
        server.serve_forever()
    finally:
        server.server_close()


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, KeyError, yaml.YAMLError) as error:
        logger.error(json.dumps({
            "event_type": "trap_startup_failed",
            "error_type": type(error).__name__,
        }))
        raise SystemExit(1)
