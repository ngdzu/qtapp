/**
 * @file MockDatabaseManager.h
 * @brief Mock implementation of IDatabaseManager for unit testing.
 *
 * Provides a lightweight mock that simulates database operations without
 * requiring actual SQLite connections. Used for repository unit tests.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#pragma once

#include "infrastructure/persistence/IDatabaseManager.h"
#include <gmock/gmock.h>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMap>

namespace zmon
{
    namespace testing
    {

        /**
         * @class MockDatabaseManager
         * @brief Mock implementation of IDatabaseManager for unit testing.
         *
         * This mock allows repository tests to run without a real database,
         * focusing on testing the repository logic itself rather than
         * database integration.
         *
         * Key features:
         * - Google Mock integration for behavior verification
         * - Simulated query preparation without SQL execution
         * - No actual database connection required
         * - Fast, isolated unit tests
         */
        class MockDatabaseManager : public IDatabaseManager
        {
        public:
            MockDatabaseManager() = default;
            virtual ~MockDatabaseManager() = default;

            // IDatabaseManager interface
            MOCK_METHOD(Result<void>, open, (const QString &dbPath, const QString &encryptionKey), (override));
            MOCK_METHOD(void, close, (), (override));
            MOCK_METHOD(bool, isOpen, (), (const, override));

            // Note: Skipping methods that return references since gmock doesn't handle them well
            // For unit tests, we only need isOpen(), registerPreparedQuery(), and hasQuery()

            MOCK_METHOD(Result<void>, beginTransaction, (), (override));
            MOCK_METHOD(Result<void>, commit, (), (override));
            MOCK_METHOD(Result<void>, rollback, (), (override));
            MOCK_METHOD(Result<void>, registerPreparedQuery, (const QString &queryId, const QString &sql), (override));
            MOCK_METHOD(bool, hasQuery, (const QString &queryId), (const, override));

            // Provide stub implementations for methods returning references
            QSqlDatabase &getConnection()
            {
                static QSqlDatabase db;
                return db;
            }
            QSqlDatabase &getReadConnection()
            {
                static QSqlDatabase db;
                return db;
            }
            QSqlDatabase &getWriteConnection()
            {
                static QSqlDatabase db;
                return db;
            }
            QSqlQuery getPreparedQuery(const QString &) { return QSqlQuery(); }
            QSqlQuery getPreparedQueryForRead(const QString &) { return QSqlQuery(); }
        };

    } // namespace testing
} // namespace zmon
