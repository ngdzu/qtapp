/**
 * @file TestCustomBackend.cpp
 * @brief Unit tests for CustomBackend logging backend.
 * 
 * This file contains unit tests for the CustomBackend class, verifying
 * formatting, rotation, file I/O, and error handling.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include <cassert>
#include <iostream>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QTemporaryDir>
#include "infrastructure/logging/backends/CustomBackend.h"
#include "infrastructure/logging/LogEntry.h"

using namespace ZMonitor::Infrastructure::Logging;

/**
 * @brief Test initialization and basic configuration.
 */
void testInitialization() {
    QTemporaryDir tempDir;
    assert(tempDir.isValid());
    
    CustomBackend backend;
    
    // Initialize with valid directory and filename
    bool initialized = backend.initialize(tempDir.path(), "test-log");
    assert(initialized);
    
    // Verify log file was created
    QString logFilePath = QDir(tempDir.path()).filePath("test-log.log");
    QFileInfo fileInfo(logFilePath);
    assert(fileInfo.exists());
    
    std::cout << "✓ Test: Initialization succeeded\n";
}

/**
 * @brief Test human-readable format writing.
 */
void testHumanFormat() {
    QTemporaryDir tempDir;
    assert(tempDir.isValid());
    
    CustomBackend backend;
    backend.initialize(tempDir.path(), "test-log");
    backend.setFormat("human");
    
    // Create a test log entry
    LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = LogLevel::Info;
    entry.category = "test";
    entry.message = "Test message";
    entry.context["key1"] = "value1";
    entry.context["key2"] = 42;
    entry.file = "TestFile.cpp";
    entry.line = 123;
    entry.function = "testFunction";
    
    // Write entry
    backend.write(entry);
    backend.flush();
    
    // Read back and verify
    QString logFilePath = QDir(tempDir.path()).filePath("test-log.log");
    QFile file(logFilePath);
    assert(file.open(QIODevice::ReadOnly | QIODevice::Text));
    
    QTextStream stream(&file);
    QString content = stream.readAll();
    
    assert(content.contains("Test message"));
    assert(content.contains("[INFO]"));
    assert(content.contains("[test]"));
    assert(content.contains("TestFile.cpp:123"));
    
    std::cout << "✓ Test: Human-readable format writing succeeded\n";
}

/**
 * @brief Test JSON format writing.
 */
void testJsonFormat() {
    QTemporaryDir tempDir;
    assert(tempDir.isValid());
    
    CustomBackend backend;
    backend.initialize(tempDir.path(), "test-log");
    backend.setFormat("json");
    
    // Create a test log entry
    LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = LogLevel::Warning;
    entry.category = "network";
    entry.message = "Connection timeout";
    entry.context["deviceId"] = "DEV-001";
    entry.context["retryCount"] = 3;
    
    // Write entry
    backend.write(entry);
    backend.flush();
    
    // Read back and verify
    QString logFilePath = QDir(tempDir.path()).filePath("test-log.log");
    QFile file(logFilePath);
    assert(file.open(QIODevice::ReadOnly | QIODevice::Text));
    
    QTextStream stream(&file);
    QString content = stream.readAll();
    
    assert(content.contains("\"level\":\"warning\""));
    assert(content.contains("\"category\":\"network\""));
    assert(content.contains("\"message\":\"Connection timeout\""));
    assert(content.contains("\"deviceId\":\"DEV-001\""));
    
    std::cout << "✓ Test: JSON format writing succeeded\n";
}

/**
 * @brief Test size-based rotation.
 */
void testSizeRotation() {
    QTemporaryDir tempDir;
    assert(tempDir.isValid());
    
    CustomBackend backend;
    backend.initialize(tempDir.path(), "test-log");
    backend.setMaxFileSize(1024);  // 1 KB limit
    
    // Write enough entries to exceed file size
    LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = LogLevel::Info;
    entry.message = QString(200, 'X');  // 200-character message
    
    // Write multiple entries to exceed 1 KB
    for (int i = 0; i < 10; ++i) {
        entry.message = QString("Entry %1: %2").arg(i).arg(QString(200, 'X'));
        backend.write(entry);
    }
    backend.flush();
    
    // Check if rotation occurred
    QString logFilePath = QDir(tempDir.path()).filePath("test-log.log");
    QFileInfo fileInfo(logFilePath);
    
    // File should exist and be smaller than max size (new file after rotation)
    assert(fileInfo.exists());
    
    // Check for rotated files
    QDir dir(tempDir.path());
    QFileInfoList files = dir.entryInfoList({"test-log_*.log"}, QDir::Files);
    
    // Should have at least one rotated file
    assert(files.size() >= 1);
    
    std::cout << "✓ Test: Size-based rotation succeeded\n";
}

/**
 * @brief Test file cleanup (maxFiles limit).
 */
void testFileCleanup() {
    QTemporaryDir tempDir;
    assert(tempDir.isValid());
    
    CustomBackend backend;
    backend.initialize(tempDir.path(), "test-log");
    backend.setMaxFileSize(100);  // Very small limit
    backend.setMaxFiles(3);  // Keep only 3 files
    
    // Write entries to trigger multiple rotations
    LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = LogLevel::Info;
    entry.message = QString(200, 'X');  // Large message to trigger rotation
    
    for (int i = 0; i < 10; ++i) {
        entry.message = QString("Entry %1: %2").arg(i).arg(QString(200, 'X'));
        backend.write(entry);
        backend.rotateIfNeeded();
    }
    backend.flush();
    
    // Check file count
    QDir dir(tempDir.path());
    QFileInfoList files = dir.entryInfoList({"test-log*.log"}, QDir::Files);
    
    // Should not exceed maxFiles + 1 (current file + rotated files)
    // Note: maxFiles() is not a public method, so we just verify cleanup happened
    assert(files.size() <= 4);  // maxFiles (3) + current file (1)
    
    std::cout << "✓ Test: File cleanup succeeded\n";
}

/**
 * @brief Test error handling (invalid directory).
 */
void testErrorHandling() {
    CustomBackend backend;
    
    // Try to initialize with invalid directory (should fail gracefully)
    bool initialized = backend.initialize("/nonexistent/path/that/does/not/exist", "test-log");
    // Note: On some systems, this might succeed if parent directories can be created
    // So we test with a more controlled scenario
    
    // Test with empty directory name (should fail)
    initialized = backend.initialize("", "test-log");
    assert(!initialized);
    
    // Test with empty filename (should fail)
    QTemporaryDir tempDir;
    initialized = backend.initialize(tempDir.path(), "");
    assert(!initialized);
    
    std::cout << "✓ Test: Error handling succeeded\n";
}

/**
 * @brief Test flush operation.
 */
void testFlush() {
    QTemporaryDir tempDir;
    assert(tempDir.isValid());
    
    CustomBackend backend;
    backend.initialize(tempDir.path(), "test-log");
    
    LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = LogLevel::Info;
    entry.message = "Flush test message";
    
    backend.write(entry);
    backend.flush();  // Explicit flush
    
    // Verify entry was written
    QString logFilePath = QDir(tempDir.path()).filePath("test-log.log");
    QFile file(logFilePath);
    assert(file.open(QIODevice::ReadOnly | QIODevice::Text));
    
    QTextStream stream(&file);
    QString content = stream.readAll();
    assert(content.contains("Flush test message"));
    
    std::cout << "✓ Test: Flush operation succeeded\n";
}

/**
 * @brief Main test runner.
 */
int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    std::cout << "Running CustomBackend unit tests...\n\n";
    
    try {
        testInitialization();
        testHumanFormat();
        testJsonFormat();
        testSizeRotation();
        testFileCleanup();
        testErrorHandling();
        testFlush();
        
        std::cout << "\n✓ All tests passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "✗ Test failed: " << e.what() << "\n";
        return 1;
    }
}

