---
title: "Protocol Buffers Integration Architecture"
doc_id: DOC-ARCH-014
version: 1.0
category: Architecture
phase: 6D
status: Draft
created: 2025-12-01
author: migration-bot
related:
  - DOC-ARCH-001_system_architecture.md
  - DOC-ARCH-013_dependency_injection.md
---

# Protocol Buffers Integration Architecture

This document defines the Protocol Buffers integration strategy for the Z Monitor application, including schema design, code generation, and deployment patterns.

**Key Decisions:**
- **Variant:** protobuf-lite for telemetry transmission; consider nanopb for embedded firmware
- **Schema Versioning:** Explicit `schema_version` field in all messages
- **Transport:** gRPC for bidirectional streams; REST + protobuf payloads for simple RPCs
- **Storage:** Serialize protobuf messages to SQLite BLOBs for offline queuing
- **Security:** TLS required; size limits enforced; PHI encrypted at rest

## 1. Protobuf Variant Selection

### 1.1 Option 1: Full Protocol Buffers (protoc)

**Pros:**
- ✅ Full feature set (reflection, JSON conversion, etc.)
- ✅ Extensive language support
- ✅ Official Google support

**Cons:**
- ⚠️ Large binary size (~500 KB - 1 MB overhead)
- ⚠️ Runtime overhead for reflection

**Use Case:** Desktop applications, servers, development tools

---

### 1.2 Option 2: protobuf-lite

**Pros:**
- ✅ Smaller binary size (~100-200 KB overhead)
- ✅ No reflection overhead
- ✅ Suitable for resource-constrained devices
- ✅ Compatible with full protobuf on server side

**Cons:**
- ⚠️ No reflection API
- ⚠️ No JSON conversion helpers

**Use Case:** **Z Monitor application** (Qt/C++ on embedded hardware)

**Recommendation:** ✅ **Use protobuf-lite** for telemetry transmission and data serialization.

---

### 1.3 Option 3: nanopb

**Pros:**
- ✅ Minimal footprint (~2-10 KB overhead)
- ✅ No dynamic memory allocation
- ✅ Ideal for microcontrollers
- ✅ Compatible with standard protobuf

**Cons:**
- ⚠️ Limited to C (not C++ classes)
- ⚠️ Manual buffer management
- ⚠️ Smaller community

**Use Case:** **Embedded firmware** (future sensor hardware)

**Recommendation:** ⏳ **Consider nanopb** if developing custom sensor firmware in the future.

---

## 2. Schema Design

### 2.1 Message Structure

**File:** `telemetry.proto`

```protobuf
syntax = "proto3";

package zmonitor.telemetry;

import "google/protobuf/timestamp.proto";

// Top-level message for telemetry batch
message TelemetryBatch {
    // Schema versioning for backward/forward compatibility
    int32 schema_version = 1; // Current version: 1
    
    // Device identification
    string device_id = 2; // Unique device identifier
    string device_serial = 3; // Hardware serial number
    string firmware_version = 4; // Firmware version (e.g., "1.2.3")
    
    // Patient context
    string patient_mrn = 5; // Medical Record Number (encrypted)
    google.protobuf.Timestamp admission_time = 6; // Admission timestamp
    
    // Telemetry data
    repeated VitalRecord vitals = 7; // Array of vital signs
    repeated WaveformSample waveforms = 8; // Array of waveform samples
    repeated AlarmEvent alarms = 9; // Array of alarm events
    
    // Metadata
    google.protobuf.Timestamp batch_time = 10; // Batch creation timestamp
    bytes signature = 11; // HMAC-SHA256 signature for integrity
}

// Individual vital sign measurement
message VitalRecord {
    google.protobuf.Timestamp timestamp = 1;
    float heart_rate = 2; // BPM
    float spo2 = 3; // %
    float respiratory_rate = 4; // Breaths/min
    float temperature = 5; // °C or °F (see unit field)
    MeasurementUnit unit = 6; // Metric or Imperial
}

enum MeasurementUnit {
    METRIC = 0;
    IMPERIAL = 1;
}

// Single waveform sample (ECG, SpO2 pleth, etc.)
message WaveformSample {
    google.protobuf.Timestamp timestamp = 1;
    WaveformChannel channel = 2; // ECG, SpO2, Resp
    float value = 3; // Sample value (mV, % modulation, etc.)
}

enum WaveformChannel {
    ECG = 0;
    SPO2_PLETH = 1;
    RESPIRATION = 2;
}

// Alarm event
message AlarmEvent {
    google.protobuf.Timestamp timestamp = 1;
    AlarmType type = 2; // HR_HIGH, SPO2_LOW, etc.
    AlarmPriority priority = 3; // HIGH, MEDIUM, LOW
    string message = 4; // Human-readable message
    float trigger_value = 5; // Value that triggered alarm
}

enum AlarmType {
    HR_HIGH = 0;
    HR_LOW = 1;
    SPO2_LOW = 2;
    RR_HIGH = 3;
    RR_LOW = 4;
    TEMP_HIGH = 5;
    TEMP_LOW = 6;
}

enum AlarmPriority {
    HIGH = 0;
    MEDIUM = 1;
    LOW = 2;
}
```

---

### 2.2 Schema Versioning Rules

**Rule 1:** Always include a `schema_version` field (field number 1).

**Rule 2:** Never change field numbers or delete fields (backward compatibility).

**Rule 3:** Add new fields with new field numbers (forward compatibility).

**Rule 4:** Use `reserved` to prevent accidental reuse of deleted field numbers.

**Example:**

```protobuf
message TelemetryBatch {
    int32 schema_version = 1;
    
    // Original fields
    string device_id = 2;
    string device_serial = 3;
    
    // New field added in v2 (backward compatible)
    string location = 4; // Added in schema v2
    
    // Prevent accidental reuse of deleted fields
    reserved 50 to 60; // Reserved for future extensions
    reserved "old_field_name"; // Prevent name reuse
}
```

---

## 3. Code Generation

### 3.1 CMake Integration

**File:** `CMakeLists.txt`

```cmake
find_package(Protobuf REQUIRED)

# Define proto files
set(PROTO_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/proto/telemetry.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/proto/provisioning.proto
)

# Generate C++ code from .proto files
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

# Add generated files to executable
add_executable(zmonitor
    src/main.cpp
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

# Link protobuf library
target_link_libraries(zmonitor
    PRIVATE
        protobuf::libprotobuf-lite # Use lite version
        Qt6::Core
        Qt6::Quick
)

# Include generated headers
target_include_directories(zmonitor
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR} # Generated .pb.h files
)
```

### 3.2 Build Workflow

```bash
# 1. Install protobuf compiler
sudo apt-get install protobuf-compiler libprotobuf-dev

# 2. Generate CMake build files
cmake -B build -S .

# 3. Build (protoc runs automatically during build)
cmake --build build
```

**Generated Files:**
- `build/telemetry.pb.h` - C++ header
- `build/telemetry.pb.cc` - C++ implementation

---

## 4. Usage Patterns

### 4.1 Serialization (C++ → Protobuf)

```cpp
#include "telemetry.pb.h"
#include <fstream>

void serializeTelemetry(const std::vector<VitalRecord>& vitals) {
    zmonitor::telemetry::TelemetryBatch batch;
    
    // Set metadata
    batch.set_schema_version(1);
    batch.set_device_id("DEV-12345");
    batch.set_device_serial("SN-98765");
    batch.set_firmware_version("1.2.3");
    batch.set_patient_mrn("MRN-67890");
    
    // Add vitals
    for (const auto& vital : vitals) {
        auto* record = batch.add_vitals();
        record->mutable_timestamp()->set_seconds(vital.timestamp.toSecsSinceEpoch());
        record->set_heart_rate(vital.heartRate);
        record->set_spo2(vital.spo2);
        record->set_respiratory_rate(vital.respiratoryRate);
    }
    
    // Serialize to binary
    std::string serialized;
    batch.SerializeToString(&serialized);
    
    // Write to file or send over network
    std::ofstream ofs("telemetry.bin", std::ios::binary);
    ofs.write(serialized.data(), serialized.size());
}
```

### 4.2 Deserialization (Protobuf → C++)

```cpp
void deserializeTelemetry(const std::string& data) {
    zmonitor::telemetry::TelemetryBatch batch;
    
    // Parse from binary
    if (!batch.ParseFromString(data)) {
        qWarning() << "Failed to parse telemetry batch";
        return;
    }
    
    // Extract data
    qDebug() << "Device ID:" << QString::fromStdString(batch.device_id());
    qDebug() << "Schema version:" << batch.schema_version();
    
    for (const auto& vital : batch.vitals()) {
        qDebug() << "HR:" << vital.heart_rate()
                 << "SpO2:" << vital.spo2()
                 << "RR:" << vital.respiratory_rate();
    }
}
```

### 4.3 SQLite Storage

**Schema:**

```sql
CREATE TABLE telemetry_queue (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    device_id TEXT NOT NULL,
    batch_time INTEGER NOT NULL, -- Unix timestamp
    protobuf_data BLOB NOT NULL, -- Serialized TelemetryBatch
    status TEXT DEFAULT 'pending', -- 'pending', 'sent', 'failed'
    retry_count INTEGER DEFAULT 0,
    created_at INTEGER DEFAULT (strftime('%s', 'now'))
);
```

**Insert:**

```cpp
void saveTelemetryBatch(const zmonitor::telemetry::TelemetryBatch& batch) {
    // Serialize to binary
    std::string serialized;
    batch.SerializeToString(&serialized);
    
    // Store in SQLite
    QSqlQuery query;
    query.prepare("INSERT INTO telemetry_queue (device_id, batch_time, protobuf_data) "
                  "VALUES (:device_id, :batch_time, :data)");
    query.bindValue(":device_id", QString::fromStdString(batch.device_id()));
    query.bindValue(":batch_time", batch.batch_time().seconds());
    query.bindValue(":data", QByteArray(serialized.data(), serialized.size()));
    
    if (!query.exec()) {
        qWarning() << "Failed to save telemetry batch:" << query.lastError().text();
    }
}
```

**Retrieve:**

```cpp
std::vector<zmonitor::telemetry::TelemetryBatch> getPendingBatches() {
    std::vector<zmonitor::telemetry::TelemetryBatch> batches;
    
    QSqlQuery query("SELECT protobuf_data FROM telemetry_queue WHERE status = 'pending'");
    while (query.next()) {
        QByteArray data = query.value(0).toByteArray();
        
        zmonitor::telemetry::TelemetryBatch batch;
        if (batch.ParseFromArray(data.data(), data.size())) {
            batches.push_back(std::move(batch));
        }
    }
    
    return batches;
}
```

---

## 5. Network Transmission

### 5.1 gRPC (Recommended for Bidirectional Streams)

**Proto Definition:**

```protobuf
service TelemetryService {
    // Send telemetry batch (unary RPC)
    rpc SendBatch(TelemetryBatch) returns (SendBatchResponse);
    
    // Stream telemetry (bidirectional streaming)
    rpc StreamTelemetry(stream TelemetryBatch) returns (stream ServerCommand);
}

message SendBatchResponse {
    bool success = 1;
    string message = 2;
}

message ServerCommand {
    CommandType type = 1;
    string payload = 2;
}

enum CommandType {
    UPDATE_CONFIG = 0;
    RESTART_DEVICE = 1;
    SYNC_TIME = 2;
}
```

**Client Code:**

```cpp
#include <grpcpp/grpcpp.h>
#include "telemetry.grpc.pb.h"

void sendTelemetryViaGRPC(const zmonitor::telemetry::TelemetryBatch& batch) {
    // Create gRPC channel
    auto channel = grpc::CreateChannel(
        "telemetry.hospital.com:50051",
        grpc::SslCredentials(grpc::SslCredentialsOptions()) // mTLS
    );
    
    auto stub = zmonitor::telemetry::TelemetryService::NewStub(channel);
    
    // Send batch
    grpc::ClientContext context;
    zmonitor::telemetry::SendBatchResponse response;
    grpc::Status status = stub->SendBatch(&context, batch, &response);
    
    if (status.ok()) {
        qDebug() << "Batch sent successfully";
    } else {
        qWarning() << "gRPC error:" << status.error_message().c_str();
    }
}
```

---

### 5.2 REST + Protobuf Payloads (Alternative)

**Use Case:** Simple unidirectional telemetry submission (no server commands).

**Endpoint:** `POST /api/v1/telemetry`

**Request:**
- **Content-Type:** `application/x-protobuf`
- **Body:** Serialized `TelemetryBatch` message

**Client Code:**

```cpp
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

void sendTelemetryViaHTTP(const zmonitor::telemetry::TelemetryBatch& batch) {
    // Serialize to binary
    std::string serialized;
    batch.SerializeToString(&serialized);
    
    // Create HTTP request
    QNetworkAccessManager* manager = new QNetworkAccessManager();
    QNetworkRequest request(QUrl("https://telemetry.hospital.com/api/v1/telemetry"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-protobuf");
    
    // Send POST request
    QNetworkReply* reply = manager->post(
        request,
        QByteArray(serialized.data(), serialized.size())
    );
    
    QObject::connect(reply, &QNetworkReply::finished, [reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "Telemetry sent successfully";
        } else {
            qWarning() << "HTTP error:" << reply->errorString();
        }
        reply->deleteLater();
    });
}
```

---

## 6. Security Considerations

### 6.1 Transport Security

**Requirement:** All protobuf messages transmitted over TLS (HTTPS or gRPC with SSL).

**Configuration:**
- **Certificate Pinning:** Validate server certificate against known root CA
- **Mutual TLS (mTLS):** Device presents client certificate for authentication

### 6.2 Message Integrity

**Pattern:** Include HMAC-SHA256 signature in `TelemetryBatch.signature` field.

```cpp
#include <openssl/hmac.h>

QByteArray computeSignature(const zmonitor::telemetry::TelemetryBatch& batch, const QByteArray& key) {
    // Serialize without signature field
    std::string data = batch.SerializeAsString();
    
    // Compute HMAC-SHA256
    unsigned char hash[SHA256_DIGEST_LENGTH];
    HMAC(EVP_sha256(), key.data(), key.size(),
         reinterpret_cast<const unsigned char*>(data.data()), data.size(),
         hash, nullptr);
    
    return QByteArray(reinterpret_cast<const char*>(hash), SHA256_DIGEST_LENGTH);
}
```

### 6.3 Size Limits

**Rule:** Enforce maximum message size to prevent denial-of-service.

```cpp
const size_t MAX_BATCH_SIZE = 10 * 1024 * 1024; // 10 MB

if (serialized.size() > MAX_BATCH_SIZE) {
    qWarning() << "Batch size exceeds limit:" << serialized.size();
    return;
}
```

### 6.4 PHI Encryption at Rest

**Requirement:** Encrypt `patient_mrn` field before storing in SQLite.

**Pattern:** Use SQLCipher for database-level encryption (see DOC-ARCH-017).

---

## 7. Performance Optimization

### 7.1 Message Batching

**Rule:** Batch multiple vitals/waveforms into single `TelemetryBatch` to reduce overhead.

**Target:** 1 batch per minute (60 vitals + 15,000 waveform samples).

### 7.2 Compression

**Option:** Enable gzip compression for HTTP transport.

```cpp
request.setRawHeader("Content-Encoding", "gzip");
```

**Trade-off:** CPU overhead vs. bandwidth savings (test in production environment).

---

## 8. Related Documents

- **[DOC-ARCH-001: Architecture Overview](./DOC-ARCH-001_architecture_overview.md)** - System architecture
- **[DOC-ARCH-017: Database Design](./DOC-ARCH-017_database_design.md)** - SQLite storage for protobuf messages
- **[DOC-COMP-030: Network Manager](../components/infrastructure/networking/DOC-COMP-030_network_manager.md)** - Network transmission implementation

---
**Status:** ✅ Migrated from legacy 14_PROTOCOL_BUFFERS.md
