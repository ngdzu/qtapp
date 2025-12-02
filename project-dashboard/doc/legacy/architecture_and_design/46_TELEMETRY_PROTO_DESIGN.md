# Telemetry Protocol Buffers Design

**Document ID:** DESIGN-TELEMETRY-PROTO  
**Version:** 1.0.0  
**Status:** Approved  
**Last Updated:** 2025-01-15

---

## Overview

This document describes the Protocol Buffers (protobuf) schema and OpenAPI specification for Z Monitor telemetry data transmission. The schema defines canonical data structures for all telemetry data transmitted from Z Monitor device to central server, ensuring type safety, interoperability, and HIPAA compliance.

## Purpose

The telemetry schema serves multiple purposes:

- **Type Safety:** Compile-time validation of telemetry data structures
- **Code Generation:** Auto-generate C++ classes from proto definitions for type-safe serialization/deserialization
- **Documentation:** OpenAPI spec provides human-readable API documentation
- **Testing:** Mock servers and simulators can use same schema for consistent test data
- **Interoperability:** Protobuf provides efficient binary serialization while OpenAPI enables JSON/REST compatibility
- **Schema Evolution:** Versioning support allows schema changes without breaking existing deployments
- **HIPAA Compliance:** Patient MRN association ensures proper patient data tracking
- **Security:** Digital signature and encryption metadata fields support secure transmission requirements

## Schema Files

- **Protocol Buffers:** `proto/telemetry.proto` - Binary serialization format
- **OpenAPI Specification:** `openapi/telemetry.yaml` - JSON/REST API format
- **Generated C++ Code:** `src/infrastructure/telemetry/generated/` (auto-generated from proto)

## Message Structure

### Core Message Types

1. **VitalsRecord** - Individual vital sign measurement
2. **TelemetryBatch** - Container for batched vital signs data
3. **AlarmEvent** - Alarm occurrence and lifecycle metadata
4. **DeviceStatus** - Device health metrics and status information
5. **Heartbeat** - Periodic heartbeat message for connection monitoring
6. **RegistrationRequest/Response** - Device registration payloads
7. **BatchContainer** - Top-level container for all telemetry messages
8. **ErrorResponse** - Server error response structure
9. **Acknowledgment** - Server acknowledgment for received telemetry

### Message Hierarchy

```
BatchContainer (top-level)
├── TelemetryBatch
│   └── VitalsRecord[] (array of vital sign records)
├── AlarmEvent
├── DeviceStatus
├── Heartbeat
└── RegistrationRequest
```

## Patient MRN Association (HIPAA Compliance)

**Critical Requirement:** All patient-related telemetry data MUST include `patient_mrn` field.

### Patient-Related Messages (MRN Required)

- **VitalsRecord:** `patient_mrn` field is REQUIRED (non-nullable)
- **TelemetryBatch:** `patient_mrn` field is REQUIRED (non-nullable) if batch contains patient data
- **AlarmEvent:** `patient_mrn` field is REQUIRED (non-nullable) for patient alarms

### Device-Only Messages (MRN Optional)

- **DeviceStatus:** `patient_mrn` field is nullable (NULL if device in STANDBY state)
- **Heartbeat:** No patient MRN (device-level message)
- **RegistrationRequest:** No patient MRN (device-level message)

### STANDBY State Handling

When device is in STANDBY state (no patient admitted):

- Device should NOT send patient-related telemetry (VitalsRecord, patient-related AlarmEvent)
- Device MAY send DeviceStatus with `patient_mrn` set to null
- Device MAY send Heartbeat messages
- All patient-related messages MUST have `patient_mrn` populated when patient is admitted

## Message Details

### VitalsRecord

Represents a single vital sign measurement with timestamp, patient association, value, unit, and quality indicators.

**Required Fields:**
- `timestamp` (int64): Unix milliseconds timestamp
- `patient_mrn` (string): Patient Medical Record Number (REQUIRED)
- `metric_name` (string): Metric name (e.g., "heart_rate", "spo2")
- `value` (double): Measured value
- `unit` (string): Unit of measurement (e.g., "bpm", "%", "°C")

**Optional Fields:**
- `quality` (QualityIndicator): Signal quality indicator
- `sensor_id` (string): Sensor identifier
- `device_id` (string): Device serial number
- `device_label` (string): Device asset tag

**Example:**
```json
{
  "timestamp": 1705312800000,
  "patient_mrn": "12345",
  "metric_name": "heart_rate",
  "value": 72.5,
  "unit": "bpm",
  "quality": "QUALITY_GOOD",
  "sensor_id": "ECG-001",
  "device_id": "ZM-001",
  "device_label": "ICU-MON-04"
}
```

### TelemetryBatch

Container for batched vital signs data. Groups multiple VitalsRecord entries into a single batch for efficient transmission.

**Required Fields:**
- `batch_id` (string, UUID): Unique batch identifier
- `device_id` (string): Device serial number
- `patient_mrn` (string): Patient MRN (REQUIRED if batch contains patient data)
- `timestamp_start` (int64): Timestamp of oldest record
- `timestamp_end` (int64): Timestamp of newest record
- `record_count` (int32): Number of records in batch
- `records` (VitalsRecord[]): Array of vital sign records

**Optional Fields:**
- `device_label` (string): Device asset tag
- `digital_signature` (string): Digital signature of batch
- `signed_at` (int64): Timestamp when batch was signed

**Batching Strategy:**
- Batch size: 10-100 records per batch (configurable)
- Batch interval: 5-60 seconds (configurable)
- Batch creation: Triggered by time interval or record count threshold

### AlarmEvent

Represents an alarm event with type, priority, status, acknowledgment information, and context data.

**Required Fields:**
- `alarm_id` (string, UUID): Unique alarm identifier
- `patient_mrn` (string): Patient MRN (REQUIRED for patient alarms)
- `alarm_type` (string): Alarm type (e.g., "HR_HIGH", "SPO2_LOW")
- `priority` (AlarmPriority): Alarm priority level
- `status` (AlarmStatus): Current alarm status
- `start_timestamp` (int64): Timestamp when alarm started

**Optional Fields:**
- `acknowledged_by` (string): User ID who acknowledged alarm
- `acknowledged_at` (int64): Timestamp when acknowledged
- `context_data` (string, JSON): Alarm context (vital values, thresholds)
- `related_vitals_snapshot_id` (string): Reference to vitals snapshot
- `device_id` (string): Device serial number
- `device_label` (string): Device asset tag

**Alarm Types:**
- `HR_HIGH`: Heart rate above threshold
- `HR_LOW`: Heart rate below threshold
- `SPO2_LOW`: SpO2 below threshold
- `RESPIRATION_HIGH`: Respiration rate above threshold
- `RESPIRATION_LOW`: Respiration rate below threshold
- `TEMPERATURE_HIGH`: Temperature above threshold
- `TEMPERATURE_LOW`: Temperature below threshold
- `SYSTEM_ERROR`: System error alarm

### DeviceStatus

Contains device health metrics including battery, CPU temperature, memory usage, network latency, connection state, firmware version, and device capabilities.

**Required Fields:**
- `device_id` (string): Device serial number
- `timestamp` (int64): Unix milliseconds timestamp

**Optional Fields:**
- `device_label` (string): Device asset tag
- `battery_percent` (double): Battery level (0-100%)
- `cpu_temp_c` (double): CPU temperature in Celsius
- `memory_percent` (double): Memory usage (0-100%)
- `network_latency_ms` (int32): Network latency to server
- `connection_state` (ConnectionState): Current connection state
- `firmware_version` (string): Firmware version string
- `capabilities` (string[]): Array of device capabilities
- `patient_mrn` (string, nullable): Patient MRN (NULL if device in STANDBY)

### Heartbeat

Periodic heartbeat message for connection monitoring. Sent every 30-60 seconds to indicate device is alive.

**Required Fields:**
- `device_id` (string): Device serial number
- `timestamp` (int64): Unix milliseconds timestamp

**Optional Fields:**
- `device_label` (string): Device asset tag
- `connection_quality` (int32): Connection quality score (0-100)
- `last_successful_transmission` (int64): Timestamp of last successful transmission

### RegistrationRequest/Response

Device registration payloads for establishing device identity and capabilities with central server.

**RegistrationRequest Required Fields:**
- `device_id` (string): Device serial number
- `firmware_version` (string): Firmware version string
- `certificate_fingerprint` (string): SHA-256 fingerprint of device certificate
- `timestamp` (int64): Unix milliseconds timestamp

**RegistrationResponse Required Fields:**
- `session_id` (string, UUID): Session identifier
- `server_timestamp` (int64): Server timestamp
- `success` (bool): Registration success status

### BatchContainer

Top-level container for all telemetry messages. Wraps individual telemetry messages with version information, message type, device identification, timestamp, digital signature, and nonce.

**Required Fields:**
- `schema_version` (int32): Schema version (for backward compatibility)
- `message_type` (MessageType): Type of message in payload
- `device_id` (string): Device serial number
- `timestamp` (int64): Unix milliseconds timestamp
- `nonce` (string, UUID): Nonce for replay attack prevention
- `signature` (string): Digital signature (ECDSA or RSA)

**Payload (oneof):**
- `batch` (TelemetryBatch): Batched vital signs data
- `alarm` (AlarmEvent): Alarm event
- `device_status` (DeviceStatus): Device health metrics
- `heartbeat` (Heartbeat): Heartbeat message
- `registration_request` (RegistrationRequest): Registration request

## Security Metadata

### Digital Signatures

Each `BatchContainer` includes a digital signature computed over:
```
signature = Sign(device_id + timestamp + nonce + SHA256(payload))
```

**Signature Algorithm:**
- ECDSA (P-256) or RSA (2048-bit minimum)
- Signature stored in `signature` field as base64-encoded string

### Replay Prevention

- **Nonce:** Each message includes a unique nonce (UUID) to prevent replay attacks
- **Timestamp Validation:** Server validates timestamp to prevent replay (clock skew tolerance: ±5 minutes)
- **Replay Window:** 1 minute (messages older than 1 minute are rejected)

### Encryption Metadata

For field-level encryption (optional):
- Encryption algorithm identifier
- Key version/identifier
- Initialization vector (IV)

## Schema Versioning

### Version Field

- **Field:** `BatchContainer.schema_version` (int32)
- **Current Version:** 1
- **Purpose:** Support schema evolution without breaking existing deployments

### Versioning Strategy

1. **Backward Compatible Changes:**
   - Adding optional fields
   - Adding new enum values
   - Adding new message types

2. **Breaking Changes:**
   - Removing fields
   - Changing field types
   - Changing required fields to optional (or vice versa)

3. **Version Increment:**
   - Increment `schema_version` for breaking changes
   - Maintain backward compatibility for at least 2 versions
   - Document migration path in release notes

## Serialization Formats

### Protocol Buffers (Binary)

- **Format:** Binary protobuf encoding
- **Content-Type:** `application/x-protobuf`
- **Advantages:**
  - Efficient binary serialization (smaller payload size)
  - Fast serialization/deserialization
  - Type-safe C++ code generation
  - Cross-language support

### JSON (OpenAPI)

- **Format:** JSON encoding
- **Content-Type:** `application/json`
- **Advantages:**
  - Human-readable
  - Easy debugging
  - REST API compatibility
  - Web browser support

### Format Selection

- **Production:** Use Protocol Buffers for efficiency
- **Development/Testing:** Use JSON for debugging
- **Server Support:** Server must support both formats

## Usage Examples

### Example 1: Sending Vitals Batch

```cpp
// Create vitals records
VitalsRecord hr_record;
hr_record.set_timestamp(1705312800000);
hr_record.set_patient_mrn("12345");
hr_record.set_metric_name("heart_rate");
hr_record.set_value(72.5);
hr_record.set_unit("bpm");
hr_record.set_quality(QualityIndicator::QUALITY_GOOD);

VitalsRecord spo2_record;
spo2_record.set_timestamp(1705312800000);
spo2_record.set_patient_mrn("12345");
spo2_record.set_metric_name("spo2");
spo2_record.set_value(98.0);
spo2_record.set_unit("%");
spo2_record.set_quality(QualityIndicator::QUALITY_EXCELLENT);

// Create batch
TelemetryBatch batch;
batch.set_batch_id("550e8400-e29b-41d4-a716-446655440000");
batch.set_device_id("ZM-001");
batch.set_device_label("ICU-MON-04");
batch.set_patient_mrn("12345");
batch.set_timestamp_start(1705312800000);
batch.set_timestamp_end(1705312800000);
batch.set_record_count(2);
batch.add_records()->CopyFrom(hr_record);
batch.add_records()->CopyFrom(spo2_record);

// Create batch container
BatchContainer container;
container.set_schema_version(1);
container.set_message_type(MessageType::MESSAGE_TYPE_BATCH);
container.set_device_id("ZM-001");
container.set_timestamp(1705312800000);
container.set_nonce("660e8400-e29b-41d4-a716-446655440001");
container.mutable_batch()->CopyFrom(batch);

// Serialize to protobuf
std::string serialized;
container.SerializeToString(&serialized);
```

### Example 2: Sending Alarm Event

```cpp
AlarmEvent alarm;
alarm.set_alarm_id("770e8400-e29b-41d4-a716-446655440002");
alarm.set_patient_mrn("12345");
alarm.set_alarm_type("HR_HIGH");
alarm.set_priority(AlarmPriority::PRIORITY_CRITICAL);
alarm.set_status(AlarmStatus::ALARM_STATUS_ACTIVE);
alarm.set_start_timestamp(1705312800000);
alarm.set_context_data(R"({"heart_rate": 120, "threshold": 100})");

BatchContainer container;
container.set_schema_version(1);
container.set_message_type(MessageType::MESSAGE_TYPE_ALARM);
container.set_device_id("ZM-001");
container.set_timestamp(1705312800000);
container.set_nonce("880e8400-e29b-41d4-a716-446655440003");
container.mutable_alarm()->CopyFrom(alarm);
```

### Example 3: Sending Device Status

```cpp
DeviceStatus status;
status.set_device_id("ZM-001");
status.set_device_label("ICU-MON-04");
status.set_timestamp(1705312800000);
status.set_battery_percent(85.5);
status.set_cpu_temp_c(45.2);
status.set_memory_percent(62.3);
status.set_network_latency_ms(25);
status.set_connection_state(ConnectionState::CONNECTION_STATE_CONNECTED);
status.set_firmware_version("1.0.0");
status.add_capabilities("ECG");
status.add_capabilities("SpO2");
status.add_capabilities("NIBP");
// patient_mrn is null (device in STANDBY)

BatchContainer container;
container.set_schema_version(1);
container.set_message_type(MessageType::MESSAGE_TYPE_DEVICE_STATUS);
container.set_device_id("ZM-001");
container.set_timestamp(1705312800000);
container.set_nonce("990e8400-e29b-41d4-a716-446655440004");
container.mutable_device_status()->CopyFrom(status);
```

## Database Schema Alignment

The telemetry proto schema aligns with the database schema defined in `schema/database.yaml`:

- **VitalsRecord** ↔ `vitals` table
- **AlarmEvent** ↔ `alarms` table
- **DeviceStatus** ↔ `device_events` table
- **TelemetryBatch** ↔ `telemetry_metrics` table

**Key Mappings:**
- `patient_mrn` field maps to `patients.mrn` (foreign key)
- `batch_id` field maps to `telemetry_metrics.batch_id`
- `alarm_id` field maps to `alarms.alarm_id`
- Timestamps use Unix milliseconds (consistent with database)

## Code Generation

### C++ Code Generation

Generate C++ classes from proto using `protoc`:

```bash
protoc --cpp_out=src/infrastructure/telemetry/generated \
       --proto_path=proto \
       proto/telemetry.proto
```

This generates:
- `telemetry.pb.h` - C++ header file
- `telemetry.pb.cc` - C++ implementation file

### CMake Integration

CMake automatically generates proto C++ classes during build:

```cmake
find_package(Protobuf REQUIRED)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS proto/telemetry.proto)
```

## Validation

### Proto Schema Validation

Validate proto schema syntax:

```bash
protoc --proto_path=proto --descriptor_set_out=/dev/null proto/telemetry.proto
```

### OpenAPI Schema Validation

Validate OpenAPI specification:

```bash
swagger-cli validate openapi/telemetry.yaml
```

### Consistency Validation

Validate that proto and OpenAPI schemas are consistent:

```bash
scripts/validate_proto_openapi.sh
```

## Security Considerations

### Transport Security

- **Protocol:** HTTPS (TLS 1.2 minimum, TLS 1.3 preferred)
- **Authentication:** Mutual TLS (mTLS) with client certificates
- **Cipher Suites:** Strong cipher suites only (ECDHE-RSA-AES256-GCM-SHA384 or better)

### Data Integrity

- **Digital Signatures:** Each message digitally signed (ECDSA or RSA)
- **Replay Prevention:** Nonce + timestamp validation
- **Clock Skew Tolerance:** ±5 minutes
- **Replay Window:** 1 minute

### Patient Data Protection

- **Patient MRN Association:** All patient-related messages include `patient_mrn`
- **HIPAA Compliance:** Patient data properly associated and tracked
- **Field-Level Encryption:** Optional field-level encryption for sensitive PHI

## Related Documentation

- **Interface Documentation:** `45_ITELEMETRY_SERVER.md` - Telemetry transmission interface
- **Security Documentation:** `06_SECURITY.md` - Security requirements and digital signatures
- **Database Design:** `10_DATABASE_DESIGN.md` - Database schema alignment
- **Simulator Documentation:** `doc/simulator/DEVICE_SIMULATOR.md` - Simulator message format requirements

## Version History

- **1.0.0** (2025-01-15): Initial schema definition with all message types, patient MRN association, digital signatures, and schema versioning

