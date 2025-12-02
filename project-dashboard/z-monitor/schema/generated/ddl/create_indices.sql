-- Auto-generated DDL for index creation
-- Generated: 2025-12-01 21:53:26 UTC
-- ⚠️ DO NOT EDIT MANUALLY - Edit schema/database.yaml and regenerate

CREATE INDEX IF NOT EXISTS idx_action_log_timestamp ON action_log (timestamp_ms);
CREATE INDEX IF NOT EXISTS idx_action_log_user ON action_log (user_id, timestamp_ms);
CREATE INDEX IF NOT EXISTS idx_action_log_action_type ON action_log (action_type, timestamp_ms);
CREATE INDEX IF NOT EXISTS idx_action_log_target ON action_log (target_type, target_id, timestamp_ms);
CREATE INDEX IF NOT EXISTS idx_action_log_device ON action_log (device_id, timestamp_ms);

CREATE INDEX IF NOT EXISTS idx_admission_events_patient ON admission_events (patient_mrn, timestamp);
CREATE INDEX IF NOT EXISTS idx_admission_events_device ON admission_events (device_label, timestamp);
CREATE INDEX IF NOT EXISTS idx_admission_events_timestamp ON admission_events (timestamp);
CREATE INDEX IF NOT EXISTS idx_admission_events_type ON admission_events (event_type, timestamp);

CREATE INDEX IF NOT EXISTS idx_alarms_patient_priority ON alarms (patient_mrn, priority, start_time);
CREATE INDEX IF NOT EXISTS idx_alarms_start_time ON alarms (start_time);

CREATE INDEX IF NOT EXISTS idx_annotations_snapshot ON annotations (snapshot_id);

CREATE INDEX IF NOT EXISTS idx_archival_jobs_status ON archival_jobs (status);

CREATE INDEX IF NOT EXISTS idx_certificates_device_status ON certificates (device_id, status);
CREATE INDEX IF NOT EXISTS idx_certificates_expires ON certificates (expires_at) WHERE status = 'active';

CREATE INDEX IF NOT EXISTS idx_device_events_timestamp ON device_events (timestamp);
CREATE INDEX IF NOT EXISTS idx_device_events_device ON device_events (device_id, timestamp);

CREATE INDEX IF NOT EXISTS idx_infusion_events_patient_time ON infusion_events (patient_id, timestamp);

CREATE INDEX IF NOT EXISTS idx_notifications_timestamp ON notifications (timestamp);

CREATE UNIQUE INDEX IF NOT EXISTS idx_patients_mrn ON patients (mrn);
CREATE INDEX IF NOT EXISTS idx_patients_admission_status ON patients (admission_status);
CREATE INDEX IF NOT EXISTS idx_patients_bed_location ON patients (bed_location) WHERE bed_location IS NOT NULL;

CREATE INDEX IF NOT EXISTS idx_predictive_scores_patient_time ON predictive_scores (patient_id, timestamp);

CREATE INDEX IF NOT EXISTS idx_security_audit_timestamp ON security_audit_log (timestamp);
CREATE INDEX IF NOT EXISTS idx_security_audit_category ON security_audit_log (event_category, timestamp);
CREATE INDEX IF NOT EXISTS idx_security_audit_device ON security_audit_log (device_id, timestamp);

CREATE UNIQUE INDEX IF NOT EXISTS idx_settings_key ON settings (key);

CREATE INDEX IF NOT EXISTS idx_snapshots_patient_time ON snapshots (patient_id, capture_time);

CREATE UNIQUE INDEX IF NOT EXISTS idx_telemetry_metrics_batch_id ON telemetry_metrics (batch_id);
CREATE INDEX IF NOT EXISTS idx_telemetry_metrics_device_id ON telemetry_metrics (device_id);
CREATE INDEX IF NOT EXISTS idx_telemetry_metrics_patient_mrn ON telemetry_metrics (patient_mrn) WHERE patient_mrn IS NOT NULL;
CREATE INDEX IF NOT EXISTS idx_telemetry_metrics_status ON telemetry_metrics (status);
CREATE INDEX IF NOT EXISTS idx_telemetry_metrics_created_at ON telemetry_metrics (created_at);
CREATE INDEX IF NOT EXISTS idx_telemetry_metrics_latency_class ON telemetry_metrics (latency_class) WHERE latency_class IS NOT NULL;

CREATE UNIQUE INDEX IF NOT EXISTS idx_users_username ON users (username);
CREATE INDEX IF NOT EXISTS idx_users_role ON users (role);

CREATE INDEX IF NOT EXISTS idx_vitals_timestamp ON vitals (timestamp);
CREATE INDEX IF NOT EXISTS idx_vitals_patient_mrn ON vitals (patient_mrn);
CREATE INDEX IF NOT EXISTS idx_vitals_patient_time ON vitals (patient_mrn, timestamp);
CREATE INDEX IF NOT EXISTS idx_vitals_unsynced ON vitals (is_synced) WHERE is_synced = 0;
CREATE INDEX IF NOT EXISTS idx_vitals_batch_id ON vitals (batch_id) WHERE batch_id IS NOT NULL;

