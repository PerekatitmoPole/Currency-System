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
    ON currency_server_logs USING GIN (context);
