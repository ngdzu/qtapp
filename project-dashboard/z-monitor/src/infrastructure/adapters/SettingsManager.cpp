/**
 * @file SettingsManager.cpp
 * @brief Implementation of SettingsManager for device configuration.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "SettingsManager.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>

namespace zmon {
// Singleton instance
SettingsManager* SettingsManager::s_instance = nullptr;

SettingsManager& SettingsManager::instance()
{
    if (!s_instance) {
        s_instance = new SettingsManager();
    }
    return *s_instance;
}

SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent)
{
    // Initialize settings table
    if (!initializeSettingsTable()) {
        qWarning() << "SettingsManager: Failed to initialize settings table";
    }
}

QVariant SettingsManager::getValue(const QString& key, const QVariant& defaultValue) const
{
    QString connectionName = getDatabaseConnection();
    if (connectionName.isEmpty()) {
        qWarning() << "SettingsManager::getValue: No database connection available";
        return defaultValue;
    }

    QSqlDatabase db = QSqlDatabase::database(connectionName);
    QSqlQuery query(db);

    query.prepare("SELECT value FROM settings WHERE key = :key");
    query.bindValue(":key", key);

    if (!query.exec()) {
        qWarning() << "SettingsManager::getValue: Query failed:" << query.lastError().text();
        return defaultValue;
    }

    if (query.next()) {
        return query.value(0);
    }

    return defaultValue;
}

bool SettingsManager::setValue(const QString& key, const QVariant& value, const QString& userId)
{
    QString connectionName = getDatabaseConnection();
    if (connectionName.isEmpty()) {
        qWarning() << "SettingsManager::setValue: No database connection available";
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database(connectionName);
    QSqlQuery query(db);

    // Use INSERT OR REPLACE to handle both insert and update
    query.prepare(
        "INSERT OR REPLACE INTO settings (key, value, updated_at, updated_by) "
        "VALUES (:key, :value, :updated_at, :updated_by)"
    );
    query.bindValue(":key", key);
    query.bindValue(":value", value.toString());
    query.bindValue(":updated_at", QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
    query.bindValue(":updated_by", userId);

    if (!query.exec()) {
        qWarning() << "SettingsManager::setValue: Query failed:" << query.lastError().text();
        return false;
    }

    emit settingChanged(key, value);
    return true;
}

bool SettingsManager::removeValue(const QString& key)
{
    QString connectionName = getDatabaseConnection();
    if (connectionName.isEmpty()) {
        qWarning() << "SettingsManager::removeValue: No database connection available";
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database(connectionName);
    QSqlQuery query(db);

    query.prepare("DELETE FROM settings WHERE key = :key");
    query.bindValue(":key", key);

    if (!query.exec()) {
        qWarning() << "SettingsManager::removeValue: Query failed:" << query.lastError().text();
        return false;
    }

    emit settingChanged(key, QVariant());
    return true;
}

bool SettingsManager::contains(const QString& key) const
{
    QString connectionName = getDatabaseConnection();
    if (connectionName.isEmpty()) {
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database(connectionName);
    QSqlQuery query(db);

    query.prepare("SELECT 1 FROM settings WHERE key = :key LIMIT 1");
    query.bindValue(":key", key);

    if (!query.exec()) {
        qWarning() << "SettingsManager::contains: Query failed:" << query.lastError().text();
        return false;
    }

    return query.next();
}

QString SettingsManager::deviceId() const
{
    return getValue("deviceId", "ZM-001").toString();
}

bool SettingsManager::setDeviceId(const QString& deviceId)
{
    return setValue("deviceId", deviceId);
}

QString SettingsManager::deviceLabel() const
{
    return getValue("deviceLabel", "ICU-MON-04").toString();
}

bool SettingsManager::setDeviceLabel(const QString& deviceLabel)
{
    return setValue("deviceLabel", deviceLabel);
}

QString SettingsManager::measurementUnit() const
{
    return getValue("measurementUnit", "metric").toString();
}

bool SettingsManager::setMeasurementUnit(const QString& unit)
{
    return setValue("measurementUnit", unit);
}

QString SettingsManager::serverUrl() const
{
    return getValue("serverUrl", "https://localhost:8443").toString();
}

bool SettingsManager::setServerUrl(const QString& url)
{
    return setValue("serverUrl", url);
}

bool SettingsManager::useMockServer() const
{
    return getValue("useMockServer", false).toBool();
}

bool SettingsManager::setUseMockServer(bool useMock)
{
    return setValue("useMockServer", useMock);
}

bool SettingsManager::initializeSettingsTable()
{
    QString connectionName = getDatabaseConnection();
    if (connectionName.isEmpty()) {
        qWarning() << "SettingsManager::initializeSettingsTable: No database connection available";
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database(connectionName);
    QSqlQuery query(db);

    // Create settings table if it doesn't exist
    QString createTableSql = R"(
        CREATE TABLE IF NOT EXISTS settings (
            key TEXT PRIMARY KEY,
            value TEXT NOT NULL,
            updated_at INTEGER NOT NULL,
            updated_by TEXT NULL
        )
    )";

    if (!query.exec(createTableSql)) {
        qWarning() << "SettingsManager::initializeSettingsTable: Failed to create table:"
                   << query.lastError().text();
        return false;
    }

    // Remove bedId if it exists (migration from old schema)
    query.prepare("DELETE FROM settings WHERE key = 'bedId'");
    query.exec();  // Ignore errors if bedId doesn't exist

    // Set default deviceLabel if not present
    if (!contains("deviceLabel")) {
        setDeviceLabel("ICU-MON-04");
    }

    return true;
}

QString SettingsManager::getDatabaseConnection() const
{
    // TODO: Get database connection from DatabaseManager
    // For now, return default connection name
    // This will be updated when DatabaseManager is implemented
    return QSqlDatabase::defaultConnection;
}

} // namespace zmon
} // namespace zmon