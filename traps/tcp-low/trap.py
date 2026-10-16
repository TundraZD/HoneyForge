import asyncio
import json
import logging
import os
from datetime import datetime, timezone
from pathlib import Path

import yaml

PROFILE_PATH = Path(os.environ.get("HONEYFORGE_PROFILE", "/app/profile.yaml"))

logging.basicConfig(level=logging.INFO, format="%(message)s")
logger = logging.getLogger("honeyforge.trap")


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

    if trap.get("type") != "tcp_banner":
        raise ValueError("Unsupported trap type")
    if trap.get("level") != "low":
        raise ValueError("This implementation supports Low traps only")
    if network.get("protocol") != "tcp":
        raise ValueError("Only TCP protocol is supported")

    port = network.get("port")
    if not isinstance(port, int) or isinstance(port, bool) or not 1 <= port <= 65535:
        raise ValueError("Invalid TCP port")

    banner = behavior.get("banner")
    if not isinstance(banner, str) or not banner:
        raise ValueError("Profile banner must be a non-empty string")
    if "\r" in banner or "\n" in banner:
        raise ValueError("Banner must not contain newline characters")
    if behavior.get("response_mode") != "banner_only":
        raise ValueError("Unsupported response mode")

    for key in ("log_connections", "log_received_data"):
        if not isinstance(telemetry.get(key), bool):
            raise ValueError(f"telemetry.{key} must be a boolean")

    if not isinstance(profile.get("metadata", {}).get("name"), str):
        raise ValueError("metadata.name must be a string")
    if not isinstance(trap.get("id"), str):
        raise ValueError("trap.id must be a string")

    return profile


async def handle_client(reader, writer, profile):
    peer = writer.get_extra_info("peername")
    source_ip = peer[0] if isinstance(peer, tuple) and peer else None
    telemetry = profile["telemetry"]

    if telemetry["log_connections"]:
        emit_event(profile, "connection_open", source_ip)

    try:
        writer.write((profile["behavior"]["banner"] + "\r\n").encode("utf-8"))
        await writer.drain()

        if telemetry["log_received_data"]:
            try:
                data = await asyncio.wait_for(reader.read(1024), timeout=10)
                if data:
                    emit_event(
                        profile,
                        "data_received",
                        source_ip,
                        bytes_received=len(data),
                    )
            except asyncio.TimeoutError:
                emit_event(
                    profile, "read_timeout", source_ip, timeout_seconds=10
                )

    except (ConnectionError, OSError) as error:
        emit_event(
            profile,
            "connection_error",
            source_ip,
            error_type=type(error).__name__,
        )

    finally:
        writer.close()
        try:
            await writer.wait_closed()
        except (ConnectionError, OSError):
            pass

        if telemetry["log_connections"]:
            emit_event(profile, "connection_close", source_ip)


async def main():
    profile = load_profile()
    port = profile["network"]["port"]

    server = await asyncio.start_server(
        lambda reader, writer: handle_client(reader, writer, profile),
        host="0.0.0.0",
        port=port,
    )

    emit_event(profile, "trap_started")

    async with server:
        await server.serve_forever()


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except (OSError, ValueError, KeyError, yaml.YAMLError) as error:
        logger.error(json.dumps({
            "event_type": "trap_startup_failed",
            "error_type": type(error).__name__,
        }))
        raise SystemExit(1)
