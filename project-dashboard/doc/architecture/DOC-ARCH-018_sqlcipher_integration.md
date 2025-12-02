---
doc_id: DOC-ARCH-018
title: SQLCipher Integration and Database Encryption
version: 1.0
category: Architecture
status: Approved
related_docs:
  - DOC-ARCH-017  # Database Design
  - DOC-ARCH-010  # Security Architecture
  - DOC-GUIDE-014 # Database Access Strategy
  - DOC-PROC-009  # Schema Management
tags:
  - security
  - encryption
  - sqlcipher
  - hipaa
  - database
  - aes-256
source:
  original_id: DESIGN-034
  file: 34_SQLCIPHER_INTEGRATION.md
  migrated_date: 2025-12-01
---

# SQLCipher Integration and Database Encryption

## Purpose

This document outlines the plan for integrating SQLCipher encryption into the Z Monitor database system. SQLCipher provides transparent AES-256 encryption for SQLite databases, which is required for HIPAA compliance and patient data protection (REQ-SEC-ENC-001).

SQLCipher is an open-source extension to SQLite that provides transparent 256-bit AES encryption of database files. It uses the same API as SQLite, making integration straightforward while maintaining strong security guarantees for Protected Health Information (PHI).

## Current Implementation Status

**Completed:**
- ✅ `DatabaseManager` has encryption support infrastructure (`setupEncryption()` method)
- ✅ CMake option `ENABLE_SQLCIPHER` added for conditional compilation
- ✅ Encryption key handling implemented via `PRAGMA key` statements

**Remaining Work:**
- ⏳ Build Qt with SQLCipher driver support (or use pre-built SQLCipher-enabled Qt)
- ⏳ Key management integration with secure storage
- ⏳ Migration path for existing unencrypted databases
- ⏳ Testing and validation

## Architecture

### Database Encryption Flow

```
┌─────────────────┐
│ DatabaseManager │
│   ::open()      │
└────────┬────────┘
         │
         ├─→ Check ENABLE_SQLCIPHER flag
         │
         ├─→ If enabled:
         │   ├─→ Load encryption key from secure storage
         │   ├─→ Open database connection
         │   ├─→ Execute PRAGMA key = '<key>'
         │   └─→ Verify encryption (SELECT COUNT(*) FROM sqlite_master)
         │
         └─→ If disabled:
             └─→ Open standard SQLite connection (no encryption)
```

**Key Points:**
1. **Transparent Encryption:** Application code unchanged - encryption handled at database layer
2. **Key Management:** Encryption key provided at connection time via PRAGMA
3. **Verification:** After setting key, verify database can be read (encryption test)
4. **Fallback:** If SQLCipher disabled, falls back to standard SQLite (development only)

### Key Management

**Key Storage Options (Priority Order):**

1. **Hardware Security Module (HSM)** - Preferred for production (future enhancement)
   - Tamper-resistant hardware storage
   - Key operations performed in hardware
   - Highest security level

2. **Secure Enclave** - Platform-specific secure storage (macOS/iOS)
   - OS-provided secure storage
   - Hardware-backed encryption
   - Good balance of security and availability

3. **Encrypted Configuration File** - Encrypted with device certificate (interim solution)
   - Device certificate encrypts configuration file
   - Configuration file contains database key
   - Reasonable security for interim deployment

4. **Environment Variable** - Development/testing only (NOT for production)
   - `Z_MONITOR_DB_KEY` environment variable
   - Convenient for testing
   - ⚠️ **NEVER USE IN PRODUCTION**

**Key Derivation:**
- Keys should be derived from device certificate or hardware ID
- Never store keys in plaintext
- Keys should be rotated periodically (future enhancement)
- Key length: 256 bits (32 bytes) for AES-256

**Key Lifecycle:**
1. **Generation:** Generate strong random key during device provisioning
2. **Storage:** Store in secure storage (HSM, Secure Enclave, encrypted config)
3. **Retrieval:** Load key when opening database connection
4. **Rotation:** Periodic key rotation (future) requires database re-encryption
5. **Revocation:** Key revocation on device decommissioning

### Integration Points

**DatabaseManager Interface:**

```cpp
class DatabaseManager : public QObject {
public:
    // Open database with optional encryption key
    bool open(const QString& dbPath, const QString& encryptionKey = QString());
    
    // Configure SQLCipher on connection
    bool setupEncryption(QSqlDatabase& db);
    
    // Check if database is encrypted (future)
    bool isEncrypted();
    
private:
    QString m_encryptionKey;
    bool m_encryptionEnabled;
};
```

**Key Management Service (Future):**

```cpp
class IKeyManager {
public:
    // Retrieve encryption key from secure storage
    virtual Result<QString, Error> getDatabaseKey() = 0;
    
    // Store encryption key in secure storage
    virtual Result<void, Error> storeDatabaseKey(const QString& key) = 0;
    
    // Rotate encryption key (requires re-encryption)
    virtual Result<void, Error> rotateDatabaseKey() = 0;
};
```

## Implementation Steps

### Phase 1: Build System Integration ✅

**Completed:**
- [x] Add `ENABLE_SQLCIPHER` CMake option
- [x] Add SQLCipher detection in CMakeLists.txt
- [x] Add compile-time definitions for SQLCipher support

**Pending:**
- [ ] Document Qt build requirements with SQLCipher driver
- [ ] Add SQLCipher library detection and linking

### Phase 2: Qt SQLCipher Driver

**Option A: Use Pre-built SQLCipher-enabled Qt (Recommended)**

Use Qt build with SQLCipher support already compiled:
- Download from Qt Company or community builds
- Verify SQLCipher plugin available: `libqsqlsqlcipher.so` (Linux) or `qsqlsqlcipher.dylib` (macOS)
- Update deployment scripts to use SQLCipher-enabled Qt

**Option B: Build Qt with SQLCipher Driver**

1. **Install SQLCipher Library:**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install libsqlcipher-dev
   
   # macOS
   brew install sqlcipher
   
   # Windows
   # Download from https://www.zetetic.net/sqlcipher/
   ```

2. **Build Qt with SQLCipher Plugin:**
   ```bash
   cd qt-src
   ./configure -sql-driver-plugin sqlcipher
   make
   ```

3. **Verify SQLCipher Driver:**
   ```bash
   qtdir/bin/qmake -query QT_PLUGINS_PATH
   # Check for libqsqlsqlcipher.so (Linux) or qsqlsqlcipher.dylib (macOS)
   ```

### Phase 3: Key Management Integration

**1. Create Key Management Interface:**

```cpp
// IKeyManager.h - Domain interface
class IKeyManager {
public:
    virtual ~IKeyManager() = default;
    
    virtual Result<QString, Error> getDatabaseKey() = 0;
    virtual Result<void, Error> storeDatabaseKey(const QString& key) = 0;
    virtual Result<void, Error> rotateDatabaseKey() = 0;
};
```

**2. Implement Secure Key Storage:**

Platform-specific implementations:
- **SecureEnclaveKeyManager** (macOS/iOS) - Uses system Keychain
- **HSMKeyManager** (production) - Uses Hardware Security Module
- **EncryptedConfigKeyManager** (interim) - Encrypted config file
- **EnvironmentKeyManager** (development) - Environment variable

```cpp
// SecureEnclaveKeyManager.cpp (macOS example)
Result<QString, Error> SecureEnclaveKeyManager::getDatabaseKey() {
    // Load key from macOS Keychain
    CFDataRef keyData = nullptr;
    OSStatus status = SecItemCopyMatching(query, (CFTypeRef*)&keyData);
    
    if (status == errSecSuccess) {
        // Convert CFData to QString
        return Result<QString, Error>::ok(keyString);
    }
    
    return Result<QString, Error>::error(
        Error::create(ErrorCode::KeyNotFound, "Database key not found in Keychain")
    );
}
```

**3. Integrate with DatabaseManager:**

```cpp
// DatabaseManager.cpp
DatabaseManager::DatabaseManager(IKeyManager* keyManager, LogService* logService)
    : m_keyManager(keyManager), m_logService(logService) {
}

bool DatabaseManager::open(const QString& dbPath) {
#ifdef ENABLE_SQLCIPHER
    // Retrieve encryption key from secure storage
    auto keyResult = m_keyManager->getDatabaseKey();
    
    if (keyResult.isError()) {
        m_logService->error("Failed to retrieve database encryption key", {
            {"error", keyResult.error().message()}
        });
        return false;
    }
    
    return open(dbPath, keyResult.value());
#else
    return open(dbPath, QString());  // No encryption (development)
#endif
}
```

### Phase 4: Migration Path

**For Existing Unencrypted Databases:**

1. **Create Encrypted Backup:**
   - Copy unencrypted database to temporary location
   - Create new encrypted database with same schema

2. **Export All Data:**
   - Read all tables from unencrypted database
   - Preserve data types, nulls, foreign keys

3. **Import into Encrypted Database:**
   - Insert data into new encrypted database
   - Maintain referential integrity

4. **Verify Data Integrity:**
   - Compare row counts
   - Sample random records
   - Verify foreign keys

5. **Replace Old Database:**
   - Rename encrypted database to production filename
   - Delete unencrypted database (secure deletion)

**Migration Script:**

```python
# scripts/migrate_to_encrypted.py
import sqlite3
import sqlcipher3  # SQLCipher Python bindings

def migrate_to_encrypted(src_path, dst_path, encryption_key):
    """
    Migrate unencrypted SQLite database to encrypted SQLCipher database.
    
    Args:
        src_path: Path to unencrypted source database
        dst_path: Path to encrypted destination database
        encryption_key: Encryption key (256-bit hex string)
    """
    # Open source (unencrypted)
    src_conn = sqlite3.connect(src_path)
    
    # Open destination (encrypted)
    dst_conn = sqlcipher3.connect(dst_path)
    dst_conn.execute(f"PRAGMA key = '{encryption_key}'")
    
    # Get all table names
    tables = src_conn.execute(
        "SELECT name FROM sqlite_master WHERE type='table'"
    ).fetchall()
    
    for (table_name,) in tables:
        # Get table schema
        schema = src_conn.execute(
            f"SELECT sql FROM sqlite_master WHERE name='{table_name}'"
        ).fetchone()[0]
        
        # Create table in destination
        dst_conn.execute(schema)
        
        # Copy all rows
        rows = src_conn.execute(f"SELECT * FROM {table_name}").fetchall()
        
        if rows:
            placeholders = ','.join(['?' for _ in rows[0]])
            dst_conn.executemany(
                f"INSERT INTO {table_name} VALUES ({placeholders})",
                rows
            )
    
    # Verify migration
    for (table_name,) in tables:
        src_count = src_conn.execute(f"SELECT COUNT(*) FROM {table_name}").fetchone()[0]
        dst_count = dst_conn.execute(f"SELECT COUNT(*) FROM {table_name}").fetchone()[0]
        
        assert src_count == dst_count, f"Row count mismatch for {table_name}"
    
    dst_conn.commit()
    dst_conn.close()
    src_conn.close()
```

### Phase 5: Testing

**Unit Tests:**

```cpp
// Test encryption setup
TEST_F(DatabaseManagerTest, TestEncryptionSetup) {
    DatabaseManager dbManager;
    QString testKey = "test_encryption_key_256_bits_hex";
    
    ASSERT_TRUE(dbManager.open(":memory:", testKey));
    ASSERT_TRUE(dbManager.isEncrypted());
}

// Test key validation
TEST_F(DatabaseManagerTest, TestWrongKey) {
    DatabaseManager dbManager;
    QString correctKey = "correct_key";
    QString wrongKey = "wrong_key";
    
    // Create encrypted database with correct key
    ASSERT_TRUE(dbManager.open("test.db", correctKey));
    dbManager.close();
    
    // Try to open with wrong key (should fail)
    ASSERT_FALSE(dbManager.open("test.db", wrongKey));
}

// Test encrypted database operations
TEST_F(DatabaseManagerTest, TestEncryptedOperations) {
    DatabaseManager dbManager;
    QString key = "test_key";
    
    ASSERT_TRUE(dbManager.open(":memory:", key));
    
    // Create table, insert data, query data
    // All operations should work normally with encryption
}
```

**Integration Tests:**

```cpp
// Test encrypted database with repositories
TEST_F(RepositoryIntegrationTest, TestEncryptedPatientRepository) {
    DatabaseManager dbManager;
    QString key = "integration_test_key";
    
    ASSERT_TRUE(dbManager.open(":memory:", key));
    
    SQLitePatientRepository repo(&dbManager, m_logService);
    
    // Test CRUD operations with encryption
    PatientAggregate patient(...);
    ASSERT_TRUE(repo.save(patient).isSuccess());
    
    auto loaded = repo.findByMrn(patient.getMrn());
    ASSERT_TRUE(loaded.has_value());
}
```

**Security Tests:**

```cpp
// Verify encrypted database cannot be read without key
TEST_F(SecurityTest, TestEncryptedFileUnreadable) {
    // Create encrypted database
    DatabaseManager dbManager;
    QString key = "secure_key";
    ASSERT_TRUE(dbManager.open("secure.db", key));
    dbManager.close();
    
    // Try to read file with standard SQLite (should fail)
    QSqlDatabase plainDb = QSqlDatabase::addDatabase("QSQLITE", "plain");
    plainDb.setDatabaseName("secure.db");
    ASSERT_FALSE(plainDb.open());  // Cannot open encrypted DB without key
}

// Test key rotation security
TEST_F(SecurityTest, TestKeyRotation) {
    // Future test for key rotation feature
}
```

## Configuration

### CMake Configuration

```cmake
# Enable SQLCipher
cmake -DENABLE_SQLCIPHER=ON ..

# Disable SQLCipher (default)
cmake -DENABLE_SQLCIPHER=OFF ..
```

### Runtime Configuration

**Encryption Key Source:**

```cpp
// Priority order (first available used):
// 1. Secure storage (Keychain, HSM)
// 2. Encrypted configuration file
// 3. Environment variable (development only)

#ifdef ENABLE_SQLCIPHER
    QString key;
    
    // Try secure storage first
    if (m_keyManager) {
        auto result = m_keyManager->getDatabaseKey();
        if (result.isSuccess()) {
            key = result.value();
        }
    }
    
    // Fallback to environment variable (development)
    if (key.isEmpty()) {
        key = qgetenv("Z_MONITOR_DB_KEY");
    }
#endif
```

**Database Path:**
- Encrypted databases use same path as unencrypted
- Encryption transparent to application code
- Key must be provided at database open time

## Security Considerations

### Key Security

**Critical Rules:**
1. **Never log encryption keys** - Keys must NEVER appear in logs (even debug logs)
2. **Secure key storage** - Use platform secure storage or HSM (never plaintext files)
3. **Key rotation** - Implement periodic key rotation (future enhancement)
4. **Key derivation** - Derive keys from device certificate or hardware ID (not user passwords)

**Example - Secure Logging:**

```cpp
// ❌ BAD: Logs encryption key
m_logService->debug("Opening database", {
    {"path", dbPath},
    {"key", encryptionKey}  // NEVER DO THIS!
});

// ✅ GOOD: Logs that encryption is enabled, not the key
m_logService->debug("Opening encrypted database", {
    {"path", dbPath},
    {"encryption_enabled", true}
});
```

### Database Security

1. **Encryption at Rest:** All database files encrypted with AES-256
2. **In-Memory Protection:** Consider memory encryption for sensitive data (future)
3. **Backup Encryption:** Encrypted backups must use same or stronger encryption
4. **Access Control:** Database access requires application-level authentication (PIN, certificate)

### Compliance

**HIPAA Requirements:**
- **Encryption at Rest:** AES-256 encryption for PHI (REQ-REG-HIPAA-003) ✅
- **Audit Logging:** All encryption operations logged to `security_audit_log`
- **Key Management:** Key access logged and audited
- **Tamper Detection:** Hash chains in audit logs detect unauthorized modifications

**Audit Log Example:**

```cpp
// Log encryption setup
m_logService->security("Database encryption configured", {
    {"event_type", "encryption_setup"},
    {"encryption_algorithm", "AES-256"},
    {"database_path", dbPath},
    {"success", true}
});

// Log key retrieval
m_logService->security("Encryption key retrieved", {
    {"event_type", "key_retrieval"},
    {"key_source", "secure_enclave"},
    {"success", true}
});
```

## Performance Impact

**Expected Performance:**

| Operation         | Overhead   | Notes                                   |
| ----------------- | ---------- | --------------------------------------- |
| Read operations   | ~5-10%     | AES-256 decryption overhead             |
| Write operations  | ~5-10%     | AES-256 encryption overhead             |
| Index operations  | Minimal    | Indices encrypted with data             |
| Query performance | Negligible | Encryption transparent to query planner |

**Optimization Strategies:**
1. **Connection Pooling:** Minimize encryption setup overhead (key only set once per connection)
2. **Page-Level Encryption:** SQLCipher encrypts at page level (future optimization)
3. **Hardware Acceleration:** Use AES-NI CPU instructions (automatic on supported CPUs)
4. **Monitoring:** Monitor performance and optimize as needed

**Benchmark Example:**

```cpp
// Benchmark encrypted vs. unencrypted writes
void benchmarkEncryption() {
    const int NUM_RECORDS = 10000;
    
    // Unencrypted baseline
    auto start = std::chrono::high_resolution_clock::now();
    // ... write NUM_RECORDS to unencrypted database
    auto unencryptedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start
    ).count();
    
    // Encrypted test
    start = std::chrono::high_resolution_clock::now();
    // ... write NUM_RECORDS to encrypted database
    auto encryptedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start
    ).count();
    
    double overhead = ((encryptedTime - unencryptedTime) / (double)unencryptedTime) * 100;
    
    m_logService->info("Encryption performance impact", {
        {"unencrypted_ms", QString::number(unencryptedTime)},
        {"encrypted_ms", QString::number(encryptedTime)},
        {"overhead_percent", QString::number(overhead, 'f', 2)}
    });
}
```

## Troubleshooting

### Common Issues

**Issue: "Cannot set encryption key"**
- **Cause:** SQLCipher driver not available or Qt not built with SQLCipher
- **Solution:** Verify Qt has SQLCipher driver, check `ENABLE_SQLCIPHER` CMake flag
- **Debug:** Check `QSqlDatabase::drivers()` for "QSQLCIPHER" driver

**Issue: "Encryption key verification failed"**
- **Cause:** Wrong encryption key or database not encrypted
- **Solution:** Verify key is correct, check database encryption status
- **Debug:** Try opening with empty key (if unencrypted database)

**Issue: "Database cannot be opened"**
- **Cause:** Database encrypted but key not provided
- **Solution:** Provide encryption key when opening database
- **Debug:** Check if `PRAGMA key` executed successfully

**Issue: "Database file is not a database"**
- **Cause:** Database encrypted but opened without key, or corrupted database
- **Solution:** Provide correct encryption key or restore from backup
- **Debug:** Verify database file is SQLCipher-encrypted (not standard SQLite)

## Future Enhancements

### Hardware Security Module (HSM) Integration

**Goal:** Store keys in HSM for maximum security

**Benefits:**
- Tamper-resistant hardware storage
- Hardware-based key operations (no key ever in software)
- Regulatory compliance (PCI-DSS, FIPS 140-2)

**Implementation:**
```cpp
class HSMKeyManager : public IKeyManager {
public:
    Result<QString, Error> getDatabaseKey() override {
        // Request key from HSM (key never leaves HSM)
        // HSM decrypts database key and returns it
    }
};
```

### Key Rotation

**Goal:** Automatic periodic key rotation

**Benefits:**
- Reduce risk of key compromise over time
- Compliance with key rotation policies
- Zero-downtime re-encryption

**Implementation:**
- Generate new encryption key
- Re-encrypt database with new key (PRAGMA rekey)
- Update secure storage with new key
- Log key rotation event

### Memory Encryption

**Goal:** Encrypt sensitive data in memory

**Benefits:**
- Prevent memory dumps from revealing data
- Protect against cold boot attacks
- Additional layer of defense

**Implementation:**
- Use secure memory allocators
- Encrypt sensitive structures in memory
- Clear memory after use (secure erase)

### Performance Optimization

**Goal:** Hardware acceleration for AES operations

**Benefits:**
- Reduce encryption overhead
- Faster database operations
- Better user experience

**Implementation:**
- Verify AES-NI CPU support
- Configure SQLCipher to use hardware acceleration
- Benchmark and validate performance improvements

## Verification Guidelines

### Encryption Verification

1. **Verify encryption enabled:** Check `ENABLE_SQLCIPHER` CMake flag
2. **Verify driver available:** Check `QSqlDatabase::drivers()` for "QSQLCIPHER"
3. **Verify key set:** Verify `PRAGMA key` executed successfully
4. **Verify database readable:** Verify `SELECT COUNT(*) FROM sqlite_master` succeeds

### Security Verification

1. **Key storage security:** Verify keys stored in secure storage (not plaintext)
2. **Key logging prevention:** Verify keys never appear in logs
3. **Database unreadable without key:** Verify encrypted database cannot be opened without key
4. **Audit logging:** Verify all encryption operations logged

### Performance Verification

1. **Measure overhead:** Benchmark encrypted vs. unencrypted operations
2. **Verify acceptable performance:** Verify < 10% overhead on write operations
3. **Monitor production:** Monitor database performance in production
4. **Optimize if needed:** Optimize queries, indices, connection pooling

## Document Metadata

**Original Document ID:** DESIGN-034  
**Migration Date:** 2025-12-01  
**New Document ID:** DOC-ARCH-018  
**Implementation Status:** Partial (build system complete, Qt driver and key management pending)

## Revision History

| Version | Date       | Changes                                                                                                                                                                               |
| ------- | ---------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 1.0     | 2025-12-01 | Initial migration from DESIGN-034 to DOC-ARCH-018. Complete SQLCipher integration plan with architecture, key management, migration path, testing, security, and future enhancements. |
