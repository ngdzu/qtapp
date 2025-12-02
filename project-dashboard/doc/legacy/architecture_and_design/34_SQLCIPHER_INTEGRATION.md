# SQLCipher Integration Plan

**Document ID:** DESIGN-034  
**Version:** 1.0  
**Status:** Draft  
**Last Updated:** 2025-01-15

---

This document outlines the plan for integrating SQLCipher encryption into the Z Monitor database system. SQLCipher provides transparent AES-256 encryption for SQLite databases, which is required for HIPAA compliance and patient data protection (REQ-SEC-ENC-001).

## 1. Overview

SQLCipher is an open-source extension to SQLite that provides transparent 256-bit AES encryption of database files. It uses the same API as SQLite, making integration straightforward.

**Current Status:**
- `DatabaseManager` already has encryption support infrastructure (`setupEncryption()` method)
- CMake option `ENABLE_SQLCIPHER` added for conditional compilation
- Encryption key handling implemented via `PRAGMA key` statements

**Remaining Work:**
- Build Qt with SQLCipher driver support (or use pre-built SQLCipher-enabled Qt)
- Key management integration with secure storage
- Migration path for existing unencrypted databases
- Testing and validation

## 2. Architecture

### 2.1 Database Encryption Flow

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

### 2.2 Key Management

**Key Storage Options:**
1. **Hardware Security Module (HSM)** - Preferred for production (future)
2. **Secure Enclave** - Platform-specific secure storage (macOS/iOS)
3. **Encrypted Configuration File** - Encrypted with device certificate (interim)
4. **Environment Variable** - Development/testing only

**Key Derivation:**
- Keys should be derived from device certificate or hardware ID
- Never store keys in plaintext
- Keys should be rotated periodically (future enhancement)

### 2.3 Integration Points

**DatabaseManager:**
- `open(dbPath, encryptionKey)` - Accepts optional encryption key
- `setupEncryption(db)` - Configures SQLCipher on connection
- `isEncrypted()` - Check if database is encrypted (future)

**Key Management Service (Future):**
- `getDatabaseKey()` - Retrieve encryption key from secure storage
- `rotateDatabaseKey()` - Rotate encryption key (requires re-encryption)
- `validateKey(key)` - Verify key works with database

## 3. Implementation Steps

### Phase 1: Build System Integration ✅

- [x] Add `ENABLE_SQLCIPHER` CMake option
- [x] Add SQLCipher detection in CMakeLists.txt
- [x] Add compile-time definitions for SQLCipher support
- [ ] Document Qt build requirements with SQLCipher driver

### Phase 2: Qt SQLCipher Driver

**Option A: Use Pre-built SQLCipher-enabled Qt (Recommended)**
- Download Qt build with SQLCipher support
- Or use third-party Qt builds (e.g., from Qt Company or community)

**Option B: Build Qt with SQLCipher Driver**
1. Install SQLCipher library:
   ```bash
   # Ubuntu/Debian
   sudo apt-get install libsqlcipher-dev
   
   # macOS
   brew install sqlcipher
   
   # Windows
   # Download from https://www.zetetic.net/sqlcipher/
   ```

2. Build Qt with SQLCipher plugin:
   ```bash
   cd qt-src
   ./configure -sql-driver-plugin sqlcipher
   make
   ```

3. Verify SQLCipher driver:
   ```bash
   qtdir/bin/qtdir/bin/qmake -query QT_PLUGINS_PATH
   # Check for libqsqlsqlcipher.so (Linux) or qsqlsqlcipher.dylib (macOS)
   ```

### Phase 3: Key Management Integration

1. **Create Key Management Interface:**
   ```cpp
   class IKeyManager {
       virtual Result<QString> getDatabaseKey() = 0;
       virtual Result<void> storeDatabaseKey(const QString& key) = 0;
       virtual Result<void> rotateDatabaseKey() = 0;
   };
   ```

2. **Implement Secure Key Storage:**
   - Platform-specific implementations (Secure Enclave, HSM, etc.)
   - Fallback to encrypted config file for development

3. **Integrate with DatabaseManager:**
   - Inject `IKeyManager` into `DatabaseManager`
   - Retrieve key automatically when opening encrypted database
   - Handle key retrieval failures gracefully

### Phase 4: Migration Path

**For Existing Unencrypted Databases:**
1. Create encrypted backup
2. Export all data to temporary location
3. Create new encrypted database
4. Import data into encrypted database
5. Verify data integrity
6. Replace old database with encrypted version

**Migration Script:**
```python
# scripts/migrate_to_encrypted.py
# - Reads unencrypted database
# - Creates encrypted database with key
# - Copies all tables and data
# - Verifies integrity
# - Replaces old database
```

### Phase 5: Testing

1. **Unit Tests:**
   - Test encryption setup
   - Test key validation
   - Test encrypted database operations
   - Test migration script

2. **Integration Tests:**
   - Test encrypted database with all repositories
   - Test key rotation
   - Test migration from unencrypted to encrypted

3. **Security Tests:**
   - Verify encrypted database cannot be read without key
   - Test key storage security
   - Test key rotation security

## 4. Configuration

### 4.1 CMake Configuration

```cmake
# Enable SQLCipher
cmake -DENABLE_SQLCIPHER=ON ..

# Disable SQLCipher (default)
cmake -DENABLE_SQLCIPHER=OFF ..
```

### 4.2 Runtime Configuration

**Encryption Key Source:**
- Environment variable: `Z_MONITOR_DB_KEY` (development only)
- Secure storage: Platform-specific secure storage (production)
- Configuration file: Encrypted config file (interim)

**Database Path:**
- Encrypted databases use same path as unencrypted
- Encryption is transparent to application code
- Key must be provided at database open time

## 5. Security Considerations

### 5.1 Key Security

- **Never log encryption keys** - Keys must never appear in logs
- **Secure key storage** - Use platform secure storage or HSM
- **Key rotation** - Implement periodic key rotation (future)
- **Key derivation** - Derive keys from device certificate or hardware ID

### 5.2 Database Security

- **Encryption at rest** - All database files encrypted
- **In-memory protection** - Consider memory encryption for sensitive data (future)
- **Backup encryption** - Encrypted backups must use same or stronger encryption
- **Access control** - Database access still requires application-level authentication

### 5.3 Compliance

- **HIPAA** - Encryption at rest required for PHI (REQ-REG-HIPAA-003)
- **Audit logging** - All encryption operations logged to security_audit_log
- **Key management** - Key access logged and audited

## 6. Performance Impact

**Expected Performance:**
- **Read operations:** ~5-10% overhead (AES-256 decryption)
- **Write operations:** ~5-10% overhead (AES-256 encryption)
- **Index operations:** Minimal impact (indices encrypted with data)
- **Query performance:** Negligible impact (encryption is transparent)

**Optimization:**
- Use connection pooling to minimize encryption overhead
- Consider page-level encryption for large databases (future)
- Monitor performance and optimize as needed

## 7. Troubleshooting

### Common Issues

**Issue: "Cannot set encryption key"**
- **Cause:** SQLCipher driver not available or Qt not built with SQLCipher
- **Solution:** Verify Qt has SQLCipher driver, check `ENABLE_SQLCIPHER` flag

**Issue: "Encryption key verification failed"**
- **Cause:** Wrong encryption key or database not encrypted
- **Solution:** Verify key is correct, check database encryption status

**Issue: "Database cannot be opened"**
- **Cause:** Database encrypted but key not provided
- **Solution:** Provide encryption key when opening database

## 8. Future Enhancements

1. **Hardware Security Module (HSM) Integration**
   - Store keys in HSM for maximum security
   - Hardware-based key operations

2. **Key Rotation**
   - Automatic key rotation
   - Zero-downtime re-encryption

3. **Memory Encryption**
   - Encrypt sensitive data in memory
   - Prevent memory dumps from revealing data

4. **Performance Optimization**
   - Page-level encryption
   - Hardware acceleration for AES operations

## 9. References

- [SQLCipher Documentation](https://www.zetetic.net/sqlcipher/)
- [Qt SQL Driver Documentation](https://doc.qt.io/qt-6/qsqldriver.html)
- [HIPAA Encryption Requirements](https://www.hhs.gov/hipaa/index.html)
- `doc/06_SECURITY.md` - Security architecture and requirements
- `doc/10_DATABASE_DESIGN.md` - Database design and schema

## 10. Related Documents

- `doc/06_SECURITY.md` - Security architecture
- `doc/10_DATABASE_DESIGN.md` - Database design
- `doc/33_SCHEMA_MANAGEMENT.md` - Schema management workflow
- `src/infrastructure/persistence/DatabaseManager.h` - DatabaseManager API

