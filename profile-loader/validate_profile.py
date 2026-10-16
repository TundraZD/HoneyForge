import argparse
from pathlib import Path
from typing import Literal

import yaml
from pydantic import BaseModel, ConfigDict, Field, ValidationError


ROOT = Path(__file__).resolve().parent.parent
DEFAULT_PROFILE = ROOT / "profiles" / "tcp-basic.yaml"


class StrictModel(BaseModel):
    model_config = ConfigDict(extra="forbid")


class Metadata(StrictModel):
    name: str = Field(min_length=1)
    version: int = Field(ge=1)
    description: str


class Network(StrictModel):
    protocol: Literal["tcp"]
    port: int = Field(ge=1, le=65535)


class Telemetry(StrictModel):
    log_connections: bool
    log_received_data: bool


class Runtime(StrictModel):
    read_only_root: bool
    memory_limit_mb: int = Field(ge=32, le=512)
    restart_policy: Literal["no", "always", "unless-stopped"]


class TcpTrap(StrictModel):
    id: str = Field(min_length=1)
    type: Literal["tcp_banner"]
    level: Literal["low"]


class TcpBehavior(StrictModel):
    banner: str = Field(min_length=1, max_length=512)
    response_mode: Literal["banner_only"]


class TcpProfile(StrictModel):
    api_version: Literal["honeyforge/v1"]
    metadata: Metadata
    trap: TcpTrap
    network: Network
    behavior: TcpBehavior
    telemetry: Telemetry
    runtime: Runtime


class HttpTrap(StrictModel):
    id: str = Field(min_length=1)
    type: Literal["http_decoy"]
    level: Literal["low"]


class HttpBehavior(StrictModel):
    server_banner: str = Field(min_length=1, max_length=128)
    page_title: str = Field(min_length=1, max_length=256)
    page_heading: str = Field(min_length=1, max_length=256)
    suspicious_status: int = Field(ge=100, le=599)
    normal_status: int = Field(ge=100, le=599)
    max_body_bytes: int = Field(ge=0, le=65536)
    read_timeout_seconds: int = Field(ge=1, le=30)


class HttpDetection(StrictModel):
    log4shell_indicators: bool


class HttpProfile(StrictModel):
    api_version: Literal["honeyforge/v1"]
    metadata: Metadata
    trap: HttpTrap
    network: Network
    behavior: HttpBehavior
    detection: HttpDetection
    telemetry: Telemetry
    runtime: Runtime


def main():
    parser = argparse.ArgumentParser(
        description="Validate a HoneyForge trap profile."
    )
    parser.add_argument(
        "profile",
        nargs="?",
        type=Path,
        default=DEFAULT_PROFILE,
        help="Path to YAML profile (default: profiles/tcp-basic.yaml)",
    )
    args = parser.parse_args()
    profile_path = args.profile.resolve()

    try:
        with profile_path.open("r", encoding="utf-8") as stream:
            raw = yaml.safe_load(stream)

        if not isinstance(raw, dict):
            raise ValueError("Profile must be a YAML mapping")

        trap_data = raw.get("trap")
        if not isinstance(trap_data, dict):
            raise ValueError("Missing or invalid 'trap' section")

        trap_type = trap_data.get("type")
        if trap_type == "tcp_banner":
            profile = TcpProfile.model_validate(raw)
        elif trap_type == "http_decoy":
            profile = HttpProfile.model_validate(raw)
        else:
            raise ValueError(f"Unsupported trap type: {trap_type!r}")

    except (OSError, yaml.YAMLError, ValidationError, ValueError) as error:
        print("PROFILE INVALID")
        print(error)
        raise SystemExit(1)

    print("PROFILE VALID")
    print(f"File: {profile_path}")
    print(f"Name: {profile.metadata.name}")
    print(f"Trap ID: {profile.trap.id}")
    print(f"Type: {profile.trap.type}")
    print(f"Level: {profile.trap.level}")
    print(f"Protocol: {profile.network.protocol}")
    print(f"Port: {profile.network.port}")
    print(f"Memory limit: {profile.runtime.memory_limit_mb} MB")


if __name__ == "__main__":
    main()
