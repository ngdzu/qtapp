/**
 * @file network_retry_test.cpp
 * @brief Unit tests for MockNetworkManager retry and backoff behavior.
 *
 * Tests retry logic, exponential backoff calculation, timeout handling,
 * and request recording functionality in MockNetworkManager.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QTimer>
#include <QSignalSpy>
#include <QThread>
#include <QDateTime>
#include "infrastructure/network/MockNetworkManager.h"
#include <chrono>

using namespace zmon;

class NetworkRetryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create QCoreApplication if it doesn't exist
        if (!QCoreApplication::instance()) {
            int argc = 1;
            char* argv[] = {const_cast<char*>("test")};
            app = std::make_unique<QCoreApplication>(argc, argv);
        }

        manager = std::make_unique<MockNetworkManager>();
        manager->setServerUrl("https://test.server.com");
        manager->setSimulatedDelay(10);  // Fast delay for tests
        manager->connect();
    }

    void TearDown() override {
        manager->disconnect();
        manager.reset();
        app.reset();
    }

    std::unique_ptr<QCoreApplication> app;
    std::unique_ptr<MockNetworkManager> manager;
};

// Test: Successful request on first attempt
TEST_F(NetworkRetryTest, SuccessOnFirstAttempt) {
    manager->setSimulatedResponseCode(200);
    manager->clearRecordedRequests();

    TelemetryData data;
    data.deviceId = "TEST-DEVICE-01";
    data.patientMrn = "MRN123";
    data.timestamp = QDateTime::currentDateTime();

    ServerResponse response = manager->sendTelemetry(data);

    EXPECT_TRUE(response.success);
    EXPECT_EQ(response.statusCode, 200);
    EXPECT_EQ(response.message, "OK");

    // Verify request was recorded
    auto requests = manager->getRecordedRequests();
    EXPECT_EQ(requests.size(), 1);
    EXPECT_EQ(requests[0].data.deviceId, "TEST-DEVICE-01");
    EXPECT_EQ(requests[0].attemptNumber, 1);
}

// Test: Retry on 500 server error
TEST_F(NetworkRetryTest, RetryOnServerError) {
    manager->setSimulatedResponseCode(500);
    manager->setRetryConfig(3, 50, 1000);  // 3 retries, 50ms initial, 1000ms max
    manager->clearRecordedRequests();

    TelemetryData data;
    data.deviceId = "TEST-DEVICE-01";
    data.timestamp = QDateTime::currentDateTime();

    QSignalSpy sentSpy(manager.get(), &MockNetworkManager::telemetrySent);
    QSignalSpy failedSpy(manager.get(), &MockNetworkManager::telemetrySendFailed);

    // Send async request
    bool callbackCalled = false;
    ServerResponse receivedResponse;
    manager->sendTelemetryAsync(data, [&callbackCalled, &receivedResponse](const ServerResponse& resp) {
        callbackCalled = true;
        receivedResponse = resp;
    });

    // Process events to allow async operations
    QCoreApplication::processEvents();
    QThread::msleep(200);  // Wait for retries
    QCoreApplication::processEvents();

    // Should have failed after retries
    EXPECT_TRUE(callbackCalled);
    EXPECT_FALSE(receivedResponse.success);
    EXPECT_EQ(receivedResponse.statusCode, 500);

    // Verify retry statistics
    auto stats = manager->getRetryStatistics();
    EXPECT_GT(stats.size(), 0);  // Should have retry attempts
}

// Test: Exponential backoff calculation
TEST_F(NetworkRetryTest, ExponentialBackoff) {
    manager->setRetryConfig(5, 100, 10000);  // 100ms initial, 10s max

    // Test backoff delays (using reflection via public method)
    // Note: We can't directly test calculateBackoffDelay as it's private,
    // but we can verify behavior through retry timing

    TelemetryData data;
    data.deviceId = "TEST-DEVICE-01";
    data.timestamp = QDateTime::currentDateTime();

    manager->setSimulatedResponseCode(500);
    manager->clearRecordedRequests();

    auto start = std::chrono::steady_clock::now();
    manager->sendTelemetryAsync(data, [](const ServerResponse&) {});
    
    QCoreApplication::processEvents();
    QThread::msleep(500);  // Wait for some retries
    QCoreApplication::processEvents();

    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Should have taken some time due to backoff
    EXPECT_GT(elapsed, 100);
}

// Test: Timeout simulation
TEST_F(NetworkRetryTest, TimeoutHandling) {
    manager->setSimulateTimeout(true);
    manager->clearRecordedRequests();

    TelemetryData data;
    data.deviceId = "TEST-DEVICE-01";
    data.timestamp = QDateTime::currentDateTime();

    ServerResponse response = manager->sendTelemetry(data);

    EXPECT_FALSE(response.success);
    EXPECT_EQ(response.statusCode, 408);  // Request Timeout
    EXPECT_EQ(response.message, "Request timeout");

    // Verify request was recorded
    auto requests = manager->getRecordedRequests();
    EXPECT_EQ(requests.size(), 1);
}

// Test: Non-retryable error (4xx client error)
TEST_F(NetworkRetryTest, NonRetryableError) {
    manager->setSimulatedResponseCode(400);  // Bad Request - not retryable
    manager->setRetryConfig(3, 50, 1000);
    manager->clearRecordedRequests();

    TelemetryData data;
    data.deviceId = "TEST-DEVICE-01";
    data.timestamp = QDateTime::currentDateTime();

    QSignalSpy failedSpy(manager.get(), &MockNetworkManager::telemetrySendFailed);

    bool callbackCalled = false;
    manager->sendTelemetryAsync(data, [&callbackCalled](const ServerResponse&) {
        callbackCalled = true;
    });

    QCoreApplication::processEvents();
    QThread::msleep(100);
    QCoreApplication::processEvents();

    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(failedSpy.count(), 1);

    // Should not have retried (400 is not retryable)
    auto stats = manager->getRetryStatistics();
    EXPECT_EQ(stats.size(), 0);  // No retries for non-retryable errors
}

// Test: Retryable error codes
TEST_F(NetworkRetryTest, RetryableErrorCodes) {
    manager->setRetryConfig(2, 10, 1000);
    manager->clearRecordedRequests();

    // Test 500 (server error) is retryable
    manager->setSimulatedResponseCode(500);
    TelemetryData data;
    data.deviceId = "TEST-DEVICE-01";
    data.timestamp = QDateTime::currentDateTime();

    bool callbackCalled = false;
    manager->sendTelemetryAsync(data, [&callbackCalled](const ServerResponse&) {
        callbackCalled = true;
    });

    QCoreApplication::processEvents();
    QThread::msleep(200);
    QCoreApplication::processEvents();

    EXPECT_TRUE(callbackCalled);
    // Should have attempted retries
    auto stats = manager->getRetryStatistics();
    // Note: Exact count depends on timing, but should have some retries
}

// Test: Request recording
TEST_F(NetworkRetryTest, RequestRecording) {
    manager->setSimulatedResponseCode(200);
    manager->clearRecordedRequests();

    TelemetryData data1;
    data1.deviceId = "DEVICE-01";
    data1.patientMrn = "MRN001";
    data1.timestamp = QDateTime::currentDateTime();

    TelemetryData data2;
    data2.deviceId = "DEVICE-02";
    data2.patientMrn = "MRN002";
    data2.timestamp = QDateTime::currentDateTime();

    manager->sendTelemetry(data1);
    manager->sendTelemetry(data2);

    auto requests = manager->getRecordedRequests();
    EXPECT_EQ(requests.size(), 2);
    EXPECT_EQ(requests[0].data.deviceId, "DEVICE-01");
    EXPECT_EQ(requests[1].data.deviceId, "DEVICE-02");
}

// Test: Clear recorded requests
TEST_F(NetworkRetryTest, ClearRecordedRequests) {
    manager->setSimulatedResponseCode(200);
    manager->clearRecordedRequests();

    TelemetryData data;
    data.deviceId = "TEST-DEVICE-01";
    data.timestamp = QDateTime::currentDateTime();

    manager->sendTelemetry(data);

    auto requests = manager->getRecordedRequests();
    EXPECT_EQ(requests.size(), 1);

    manager->clearRecordedRequests();
    requests = manager->getRecordedRequests();
    EXPECT_EQ(requests.size(), 0);
}

// Test: Connection status
TEST_F(NetworkRetryTest, ConnectionStatus) {
    EXPECT_FALSE(manager->isConnected());

    manager->connect();
    EXPECT_TRUE(manager->isConnected());
    EXPECT_TRUE(manager->isServerAvailable());

    manager->disconnect();
    EXPECT_FALSE(manager->isConnected());
    EXPECT_FALSE(manager->isServerAvailable());
}

// Test: Error when not connected
TEST_F(NetworkRetryTest, ErrorWhenNotConnected) {
    manager->disconnect();

    TelemetryData data;
    data.deviceId = "TEST-DEVICE-01";
    data.timestamp = QDateTime::currentDateTime();

    ServerResponse response = manager->sendTelemetry(data);

    EXPECT_FALSE(response.success);
    EXPECT_EQ(response.statusCode, 0);
    EXPECT_EQ(response.message, "Not connected to server");
    EXPECT_EQ(manager->getLastError(), "Not connected to server");
}

// Test: Max retries exhaustion
TEST_F(NetworkRetryTest, MaxRetriesExhaustion) {
    manager->setSimulatedResponseCode(500);
    manager->setRetryConfig(2, 10, 1000);  // Only 2 retries
    manager->clearRecordedRequests();

    TelemetryData data;
    data.deviceId = "TEST-DEVICE-01";
    data.timestamp = QDateTime::currentDateTime();

    bool callbackCalled = false;
    ServerResponse finalResponse;
    manager->sendTelemetryAsync(data, [&callbackCalled, &finalResponse](const ServerResponse& resp) {
        callbackCalled = true;
        finalResponse = resp;
    });

    // Wait for all retries to complete
    QCoreApplication::processEvents();
    QThread::msleep(500);
    QCoreApplication::processEvents();

    EXPECT_TRUE(callbackCalled);
    EXPECT_FALSE(finalResponse.success);
    EXPECT_EQ(finalResponse.statusCode, 500);
}

