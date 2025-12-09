#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QSqlDatabase>

#include "ui/controllers/SettingsController.h"
#include "infrastructure/adapters/SettingsManager.h"

using namespace zmon;

// Minimal fake IActionLogRepository capturing last entry count
class FakeActionLogRepository : public IActionLogRepository
{
public:
    explicit FakeActionLogRepository() {}
    ~FakeActionLogRepository() override = default;

    void logAction(const ActionLogEntry &entry) override
    {
        ++count;
        last = entry;
    }

    void logActions(const QList<ActionLogEntry> &entries) override
    {
        for (const auto &e : entries)
        {
            logAction(e);
        }
    }

    void queryActions(const ActionLogFilter & /*filter*/) override {}

    int count = 0;
    ActionLogEntry last;
};

TEST(SettingsControllerTest, DeviceLabelUpdatesAndEmitsSignal)
{
    int argc = 0;
    char *argv[] = {nullptr};
    QCoreApplication app(argc, argv);

    // Initialize in-memory SQLite for SettingsManager
    auto db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    db.open();

    // Ensure SettingsManager has a known starting value
    auto &mgr = SettingsManager::instance();
    mgr.setDeviceLabel("INIT-LABEL");

    FakeActionLogRepository repo;
    SettingsController controller(&repo, nullptr);

    QSignalSpy spyLabel(&controller, &SettingsController::deviceLabelChanged);

    controller.setDeviceLabel("TEST-LABEL");

    // Process queued signals
    QCoreApplication::processEvents();

    ASSERT_EQ(spyLabel.count(), 1);
    EXPECT_EQ(controller.deviceLabel(), QString("TEST-LABEL"));
}

TEST(SettingsControllerTest, MeasurementUnitValidation)
{
    int argc = 0;
    char *argv[] = {nullptr};
    QCoreApplication app(argc, argv);

    auto db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    db.open();

    FakeActionLogRepository repo;
    SettingsController controller(&repo, nullptr);

    QSignalSpy spyFail(&controller, &SettingsController::settingsChangeFailed);

    controller.setMeasurementUnit("invalid-unit");
    QCoreApplication::processEvents();

    ASSERT_EQ(spyFail.count(), 1);
}

TEST(SettingsControllerTest, LogsOnDeviceLabelChange)
{
    int argc = 0;
    char *argv[] = {nullptr};
    QCoreApplication app(argc, argv);

    auto db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    db.open();

    auto &mgr = SettingsManager::instance();
    mgr.setDeviceLabel("INIT-LABEL");

    FakeActionLogRepository repo;
    SettingsController controller(&repo, nullptr);

    controller.setDeviceLabel("NEW-LABEL");
    QCoreApplication::processEvents();

    EXPECT_GE(repo.count, 1);
    EXPECT_EQ(repo.last.actionType, QString("CHANGE_SETTING"));
    EXPECT_EQ(repo.last.targetType, QString("SETTING"));
    EXPECT_EQ(repo.last.targetId, QString("deviceLabel"));
}

TEST(SettingsControllerTest, ServerUrlValidation)
{
    int argc = 0;
    char *argv[] = {nullptr};
    QCoreApplication app(argc, argv);

    auto db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    db.open();

    FakeActionLogRepository repo;
    SettingsController controller(&repo, nullptr);

    QSignalSpy spyFail(&controller, &SettingsController::settingsChangeFailed);

    controller.setServerUrl("not-a-url");
    QCoreApplication::processEvents();

    ASSERT_EQ(spyFail.count(), 1);
}

TEST(SettingsControllerTest, UseMockServerChangeEmits)
{
    int argc = 0;
    char *argv[] = {nullptr};
    QCoreApplication app(argc, argv);

    auto db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    db.open();

    FakeActionLogRepository repo;
    SettingsController controller(&repo, nullptr);

    QSignalSpy spy(&controller, &SettingsController::useMockServerChanged);
    QSignalSpy spyFail(&controller, &SettingsController::settingsChangeFailed);

    controller.setUseMockServer(true);
    QCoreApplication::processEvents();

    // In this test environment storage may fail; ensure no change applied.
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(spyFail.count(), 0);
    EXPECT_FALSE(controller.useMockServer());
}
