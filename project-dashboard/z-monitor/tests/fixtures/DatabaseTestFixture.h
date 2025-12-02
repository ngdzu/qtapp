#pragma once

/**
 * @file DatabaseTestFixture.h
 * @brief Base GoogleTest fixture that provisions an isolated in-memory SQLite database
 *        with full Z Monitor schema applied via migrations.
 *
 * Uses robust SQL statement splitting (parentheses + string literal aware) to
 * execute multi-statement migration files safely. Each test gets a unique
 * connection name and an empty in-memory database. Foreign keys are enforced.
 *
 * Responsibilities:
 *  - Create QCoreApplication if not already present (required by some Qt types)
 *  - Open unique in-memory QSQLITE connection
 *  - Apply all migrations from schema/migrations (ordered by zero-padded prefix)
 *  - Provide accessor for the QSqlDatabase
 *  - Cleanly tear down and remove connection to avoid leaked connection warnings
 *
 * @note This fixture intentionally bypasses DatabaseManager::executeMigrations() because
 *       that implementation still references legacy embedded resource paths and uses
 *       naive semicolon splitting. The improved parser here matches the logic validated
 *       in integration migration tests (TASK-DB-001).
 */

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QUuid>
#include <QDebug>
#include "infrastructure/persistence/DatabaseManager.h"

namespace zmon::test
{

    class DatabaseTestFixture : public ::testing::Test
    {
    protected:
        void SetUp() override;
        void TearDown() override;

        /**
         * @return Reference to the write-capable test database connection.
         */
        QSqlDatabase &db();

        /**
         * @return Pointer to DatabaseManager managing test connections.
         */
        DatabaseManager *databaseManager();

    private:
        void applyMigrations();
        QStringList discoverMigrationFiles(const QString &dirPath) const;
        QStringList splitSqlStatements(const QString &sql) const;

        std::unique_ptr<DatabaseManager> m_dbManager;
        std::unique_ptr<QCoreApplication> m_app; // Only created if none exists
    };

} // namespace zmon::test
