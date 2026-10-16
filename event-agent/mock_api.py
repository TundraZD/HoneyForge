#!/usr/bin/env python3
"""Мок backend для проверки event-agent по согласованному контракту.

Запуск:  python3 mock_api.py [--port 8099] [--fail-first N]
Ключ:    X-API-Key: test-key
Известные trap_ref: trap-001, http-001
"""
import argparse
import json
import uuid
from http.server import BaseHTTPRequestHandler, HTTPServer

API_KEY = "test-key"
KNOWN_TRAPS = {"trap-001": 1, "http-001": 2}
REQUIRED = ("event_id", "trap_ref", "source_ip")

seen = {}          # event_id -> id в "БД"
requests_total = 0
args = None


class Handler(BaseHTTPRequestHandler):
    def _reply(self, code, body):
        data = json.dumps(body).encode()
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_POST(self):
        global requests_total
        requests_total += 1

        if self.path != "/api/v1/events":
            return self._reply(404, {"error": "not found"})

        length = int(self.headers.get("Content-Length", "0"))
        raw = self.rfile.read(length)

        if self.headers.get("X-API-Key") != API_KEY:
            print("REQ #%d -> 401" % requests_total, flush=True)
            return self._reply(401, {"error": "unauthorized"})

        if requests_total <= args.fail_first:
            print("REQ #%d -> 503 (simulated outage)" % requests_total, flush=True)
            return self._reply(503, {"error": "unavailable"})

        try:
            event = json.loads(raw)
            if not isinstance(event, dict):
                raise ValueError
        except ValueError:
            print("REQ #%d -> 400 bad json" % requests_total, flush=True)
            return self._reply(400, {"error": "invalid json"})

        missing = [f for f in REQUIRED if not event.get(f)]
        if missing:
            print("REQ #%d -> 422 missing %s" % (requests_total, missing), flush=True)
            return self._reply(422, {"error": "missing fields", "fields": missing})

        if event["trap_ref"] not in KNOWN_TRAPS:
            print("REQ #%d -> 404 trap_ref=%s" % (requests_total, event["trap_ref"]), flush=True)
            return self._reply(404, {"error": "trap not found"})

        try:
            uuid.UUID(event["event_id"])
        except ValueError:
            return self._reply(422, {"error": "event_id must be UUID"})

        if event["event_id"] in seen:
            print("REQ #%d -> 200 duplicate %s" % (requests_total, event["event_id"]), flush=True)
            return self._reply(200, {"id": seen[event["event_id"]], "duplicate": True})

        seen[event["event_id"]] = len(seen) + 1
        print("REQ #%d -> 201 id=%d trap_ref=%s type=%s event_id=%s" % (
            requests_total, seen[event["event_id"]], event["trap_ref"],
            event.get("event_type"), event["event_id"]), flush=True)
        return self._reply(201, {"id": seen[event["event_id"]]})

    def log_message(self, *_):
        pass


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", type=int, default=8099)
    parser.add_argument("--fail-first", type=int, default=0,
                        help="первые N запросов отвечать 503")
    args = parser.parse_args()
    print("Mock API on http://127.0.0.1:%d/api/v1/events" % args.port, flush=True)
    HTTPServer(("127.0.0.1", args.port), Handler).serve_forever()
