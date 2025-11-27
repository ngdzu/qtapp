# API Versioning & Compatibility Strategy

This document defines the API versioning approach, backward compatibility policies, and migration strategies for the Z Monitor application's external interfaces.

## 1. Guiding Principles

- **Backward Compatibility:** Maintain compatibility with previous API versions when possible
- **Explicit Versioning:** All APIs must be versioned
- **Deprecation Policy:** Clear deprecation and removal timeline
- **Migration Support:** Provide migration tools and documentation
- **Breaking Changes:** Minimize breaking changes, document thoroughly when necessary

## 2. API Versioning Schemes

### 2.1. Telemetry API Versioning

**URL-based versioning:**

```
POST /api/v1/telemetry
POST /api/v2/telemetry
```

**Header-based versioning:**

```
X-API-Version: 1
X-API-Version: 2
```

**Decision:** Use URL-based versioning for clarity and explicit version selection.

### 2.2. Version Format

- **Major Version:** Breaking changes (v1, v2, v3)
- **Minor Version:** Backward-compatible additions (v1.1, v1.2)
- **Patch Version:** Bug fixes (not exposed in API)

**Example:**
- `v1` - Initial API
- `v2` - Breaking change (e.g., patient MRN required)
- `v1.1` - New optional fields added

## 3. Telemetry API Versions

### 3.1. API v1 (Initial)

**Endpoint:** `POST /api/v1/telemetry`

**Request:**
```json
{
  "deviceId": "ZM-001",
  "timestamp": "2025-01-15T14:30:00Z",
  "vitals": [
    {
      "id": 101,
      "timestamp": "2025-01-15T14:29:58Z",
      "heart_rate": 78,
      "spo2": 98.5
    }
  ]
}
```

**Response:**
```json
{
  "status": "success",
  "processed_ids": [101]
}
```

### 3.2. API v2 (Current - Patient Association Required)

**Endpoint:** `POST /api/v2/telemetry`

**Request:**
```json
{
  "deviceId": "ZM-001",
  "deviceLabel": "ICU-MON-04",
  "patientMrn": "12345",
  "patientName": "John Doe",
  "bedLocation": "ICU-4B",
  "timestamp": "2025-01-15T14:30:00Z",
  "vitals": [
    {
      "id": 101,
      "timestamp": "2025-01-15T14:29:58Z",
      "patientMrn": "12345",
      "heart_rate": 78,
      "spo2": 98.5
    }
  ],
  "alarms": [
    {
      "id": 201,
      "timestamp": "2025-01-15T14:30:00Z",
      "patientMrn": "12345",
      "alarmType": "high_heart_rate",
      "priority": "high"
    }
  ]
}
```

**Breaking Changes:**
- `patientMrn` is now **required** (was optional in v1)
- `deviceLabel` added (new field)
- `patientName` and `bedLocation` added (new fields)
- Each data record must include `patientMrn`

**Response:**
```json
{
  "status": "success",
  "processed_ids": [101, 201],
  "api_version": "2.0"
}
```

## 4. Backward Compatibility Policy

### 4.1. Compatibility Rules

- **Major Version:** Breaking changes allowed
- **Minor Version:** Only additive changes (new optional fields)
- **Deprecation:** Minimum 6 months notice before removal
- **Support:** Support last 2 major versions

### 4.2. Version Negotiation

Device negotiates API version with server:

```cpp
class NetworkManager {
    int m_apiVersion = 2;  // Preferred version
    
    Result<ServerCapabilities> negotiateApiVersion() {
        // Try v2 first
        auto result = tryConnect("/api/v2/telemetry");
        if (result.isSuccess()) {
            m_apiVersion = 2;
            return result;
        }
        
        // Fallback to v1
        result = tryConnect("/api/v1/telemetry");
        if (result.isSuccess()) {
            m_apiVersion = 1;
            return result;
        }
        
        return Result<ServerCapabilities>::error("No supported API version");
    }
};
```

## 5. Deprecation Process

### 5.1. Deprecation Timeline

1. **Announcement:** Document deprecation in API docs (6 months notice)
2. **Warning Period:** Log warnings when deprecated API is used (3 months)
3. **Removal:** Remove deprecated API after notice period

### 5.2. Deprecation Example

```cpp
// API v1 is deprecated
class NetworkManager {
    void sendTelemetryV1(const TelemetryData& data) {
        LogService::warning("Using deprecated API v1", {
            {"endpoint", "/api/v1/telemetry"},
            {"deprecationDate", "2025-07-15"},
            {"removalDate", "2026-01-15"}
        });
        // Continue to support for compatibility
    }
};
```

## 6. Migration Strategy

### 6.1. Client-Side Migration

Device automatically migrates to new API version:

```cpp
class NetworkManager {
    void migrateToV2() {
        // Update telemetry payload format
        // Add required patient MRN
        // Add new fields (deviceLabel, patientName, bedLocation)
        
        LogService::info("Migrated to API v2", {
            {"previousVersion", "1"},
            {"newVersion", "2"}
        });
    }
};
```

### 6.2. Server-Side Migration

Server supports multiple API versions simultaneously:

```python
# Flask example
@app.route('/api/v1/telemetry', methods=['POST'])
def telemetry_v1():
    # Handle v1 format
    # Convert to internal format
    pass

@app.route('/api/v2/telemetry', methods=['POST'])
def telemetry_v2():
    # Handle v2 format
    # Validate patient MRN requirement
    pass
```

## 7. Version Detection

### 7.1. Client Version Detection

Device reports its API version capability:

```json
{
  "deviceId": "ZM-001",
  "firmwareVersion": "1.2.3",
  "apiVersions": ["1", "2"],
  "preferredApiVersion": "2"
}
```

### 7.2. Server Version Detection

Server reports supported API versions:

```json
{
  "supportedVersions": ["1", "2"],
  "defaultVersion": "2",
  "deprecatedVersions": ["1"],
  "deprecationDate": "2025-07-15"
}
```

## 8. Breaking Changes Policy

### 8.1. When Breaking Changes Are Allowed

- **Security:** Security fixes may require breaking changes
- **Compliance:** Regulatory requirements may require breaking changes
- **Major Feature:** Significant new features may require breaking changes

### 8.2. Breaking Change Process

1. **Proposal:** Document proposed breaking change
2. **Review:** Review impact and migration path
3. **Announcement:** Announce 6 months in advance
4. **Implementation:** Implement in new major version
5. **Migration Guide:** Provide detailed migration guide
6. **Support Period:** Support old version for 12 months after new version release

## 9. Compatibility Testing

### 9.1. Version Compatibility Matrix

| Device Version | Server v1 | Server v2 | Notes |
|---------------|-----------|-----------|-------|
| Device v1.0 | ✅ | ❌ | Device doesn't send patient MRN |
| Device v1.1 | ✅ | ⚠️ | Device sends patient MRN if available |
| Device v2.0 | ⚠️ | ✅ | Device requires patient MRN |

### 9.2. Compatibility Tests

```cpp
TEST(TelemetryApiCompatibility, V1ToV2Migration) {
    // Test device can send v1 format
    auto v1Result = sendTelemetryV1(data);
    EXPECT_TRUE(v1Result.isSuccess());
    
    // Test device can send v2 format
    auto v2Result = sendTelemetryV2(data);
    EXPECT_TRUE(v2Result.isSuccess());
    
    // Test server accepts both
    EXPECT_TRUE(serverSupportsV1());
    EXPECT_TRUE(serverSupportsV2());
}
```

## 10. API Documentation

### 10.1. Version Documentation

Each API version must have:
- **Request Format:** JSON schema
- **Response Format:** JSON schema
- **Error Codes:** Error code reference
- **Migration Guide:** How to migrate from previous version
- **Deprecation Notice:** If deprecated

### 10.2. OpenAPI Specification

Maintain OpenAPI specs for each version:

```yaml
openapi: 3.0.0
info:
  title: Z Monitor Telemetry API
  version: 2.0
paths:
  /api/v2/telemetry:
    post:
      summary: Send telemetry data
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: '#/components/schemas/TelemetryDataV2'
```

## 11. Best Practices

### 11.1. Do's

- ✅ Version all APIs explicitly
- ✅ Maintain backward compatibility when possible
- ✅ Document breaking changes thoroughly
- ✅ Provide migration guides
- ✅ Support multiple versions during transition
- ✅ Test version compatibility

### 11.2. Don'ts

- ❌ Don't remove APIs without deprecation notice
- ❌ Don't make breaking changes in minor versions
- ❌ Don't ignore version negotiation
- ❌ Don't assume all clients are on latest version

## 12. Related Documents

- `doc/interfaces/ITelemetryServer.md` - Telemetry API interface
- [19_ADT_WORKFLOW.md](./19_ADT_WORKFLOW.md) - Patient association requirements
- [20_ERROR_HANDLING_STRATEGY.md](./20_ERROR_HANDLING_STRATEGY.md) - Error codes and handling

