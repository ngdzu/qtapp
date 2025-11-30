ALTER TABLE telemetry_metrics ADD COLUMN created_at INTEGER;
ALTER TABLE telemetry_metrics ADD COLUMN latency_class TEXT;
ALTER TABLE telemetry_metrics ADD COLUMN record_count INTEGER;
ALTER TABLE telemetry_metrics ADD COLUMN batch_size_bytes INTEGER;
ALTER TABLE telemetry_metrics ADD COLUMN updated_at INTEGER;
ALTER TABLE telemetry_metrics ADD COLUMN retry_count INTEGER;
ALTER TABLE alarms ADD COLUMN raw_value REAL;
ALTER TABLE alarms ADD COLUMN acknowledged_by TEXT;
ALTER TABLE alarms ADD COLUMN acknowledged_time INTEGER;
ALTER TABLE alarms ADD COLUMN threshold_value REAL;
ALTER TABLE action_log ADD COLUMN user_role TEXT;
ALTER TABLE action_log ADD COLUMN details TEXT;
ALTER TABLE action_log ADD COLUMN error_code TEXT;
ALTER TABLE action_log ADD COLUMN error_message TEXT;
ALTER TABLE action_log ADD COLUMN session_token_hash TEXT;
ALTER TABLE action_log ADD COLUMN ip_address TEXT;
ALTER TABLE action_log ADD COLUMN previous_hash TEXT;
ALTER TABLE admission_events ADD COLUMN details TEXT;

CREATE TABLE IF NOT EXISTS certificates (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    expires_at INTEGER,
    status TEXT
);
