import argparse
import json
import os
import sys
from pathlib import Path

SCHEMA_SQL = """
CREATE TABLE IF NOT EXISTS currency_server_logs (
    id BIGSERIAL PRIMARY KEY,
    logged_at TIMESTAMPTZ NOT NULL,
    level TEXT NOT NULL,
    category TEXT NOT NULL,
    message TEXT NOT NULL,
    context JSONB NOT NULL DEFAULT '{}'::jsonb,
    raw_entry JSONB NOT NULL,
    imported_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_currency_server_logs_logged_at
    ON currency_server_logs (logged_at DESC);

CREATE INDEX IF NOT EXISTS idx_currency_server_logs_level
    ON currency_server_logs (level);

CREATE INDEX IF NOT EXISTS idx_currency_server_logs_category
    ON currency_server_logs (category);

CREATE INDEX IF NOT EXISTS idx_currency_server_logs_context
    ON currency_server_logs
    USING GIN (context);
"""


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Import CurrencySystem JSONL logs into PostgreSQL.")
    parser.add_argument(
        "--dsn",
        default=os.environ.get("POSTGRES_DSN", ""),
        help="PostgreSQL DSN. If omitted, POSTGRES_DSN is used.",
    )
    parser.add_argument(
        "--file",
        default="logs/server.jsonl",
        help="Path to JSONL log file.",
    )
    parser.add_argument(
        "--truncate",
        action="store_true",
        help="Clear the table before import.",
    )
    return parser.parse_args()


def load_entries(path: Path) -> list[dict]:
    entries: list[dict] = []

    with path.open("r", encoding="utf-8") as handle:
        for line_number, raw_line in enumerate(handle, start=1):
            line = raw_line.strip()
            if not line:
                continue

            try:
                payload = json.loads(line)
            except json.JSONDecodeError as error:
                raise ValueError(f"Invalid JSON at line {line_number}: {error}") from error

            if not isinstance(payload, dict):
                raise ValueError(f"Line {line_number} does not contain a JSON object.")

            entries.append(
                {
                    "timestamp": str(payload.get("timestamp", "")),
                    "level": str(payload.get("level", "info")),
                    "category": str(payload.get("category", "application")),
                    "message": str(payload.get("message", "")),
                    "context": payload.get("context", {}),
                    "raw_entry": payload,
                }
            )

    return entries


def main() -> int:
    args = parse_args()

    if not args.dsn:
        sys.stderr.write("DSN is required. Pass --dsn or set POSTGRES_DSN.\n")
        return 1

    log_path = Path(args.file)
    if not log_path.exists():
        sys.stderr.write(f"Log file not found: {log_path}\n")
        return 1

    try:
        import psycopg
        from psycopg.types.json import Jsonb
    except ImportError:
        sys.stderr.write("Install psycopg first: python -m pip install \"psycopg[binary]\"\n")
        return 1

    try:
        entries = load_entries(log_path)
    except ValueError as error:
        sys.stderr.write(f"{error}\n")
        return 1

    with psycopg.connect(args.dsn) as connection:
        with connection.cursor() as cursor:
            cursor.execute(SCHEMA_SQL)

            if args.truncate:
                cursor.execute("TRUNCATE TABLE currency_server_logs RESTART IDENTITY;")

            if entries:
                cursor.executemany(
                    """
                    INSERT INTO currency_server_logs (
                        logged_at,
                        level,
                        category,
                        message,
                        context,
                        raw_entry
                    ) VALUES (%s, %s, %s, %s, %s, %s)
                    """,
                    [
                        (
                            entry["timestamp"],
                            entry["level"],
                            entry["category"],
                            entry["message"],
                            Jsonb(entry["context"]),
                            Jsonb(entry["raw_entry"]),
                        )
                        for entry in entries
                    ],
                )

        connection.commit()

    print(f"Imported {len(entries)} log entries from {log_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
