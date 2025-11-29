/**
 * @file DiagnosticsController.h
 * @brief QML controller for diagnostics and log display UI.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

namespace zmon
{
    /**
     * @class DiagnosticsController
     * @brief QML controller for diagnostics and log display.
     *
     * @thread Main/UI Thread
     * @ingroup Interface
     */
    class DiagnosticsController : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QVariantList logEntries READ logEntries NOTIFY logEntriesChanged)
        Q_PROPERTY(QString logLevel READ logLevel WRITE setLogLevel NOTIFY logLevelChanged)
        Q_PROPERTY(QVariantList systemDiagnostics READ systemDiagnostics NOTIFY systemDiagnosticsChanged)

    public:
        explicit DiagnosticsController(QObject *parent = nullptr);
        ~DiagnosticsController() override = default;

        QVariantList logEntries() const { return m_logEntries; }
        QString logLevel() const { return m_logLevel; }
        void setLogLevel(const QString &level);
        QVariantList systemDiagnostics() const { return m_systemDiagnostics; }

        Q_INVOKABLE void refreshLogs();
        Q_INVOKABLE void clearLogs();
        Q_INVOKABLE void runDiagnostics();
        Q_INVOKABLE void exportLogs();

    signals:
        void logEntriesChanged();
        void logLevelChanged();
        void systemDiagnosticsChanged();

    private:
        QVariantList m_logEntries;
        QString m_logLevel{"info"};
        QVariantList m_systemDiagnostics;
    };
} // namespace zmon
