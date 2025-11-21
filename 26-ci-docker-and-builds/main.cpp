#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QTabWidget>
#include <QSysInfo>
#include <QLibraryInfo>
#include <QFileInfo>
#include <QDir>
#include <QProcess>

class BuildInfoWidget : public QWidget
{
    Q_OBJECT
public:
    BuildInfoWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setupUI();
        displayInfo();
    }

private:
    void setupUI()
    {
        setWindowTitle("Lesson 26: CI, Docker, and Builds");
        resize(800, 600);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        QLabel *title = new QLabel("<h2>Build Environment Information</h2>");
        title->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(title);

        QTabWidget *tabs = new QTabWidget();

        // Build Info Tab
        QWidget *buildTab = new QWidget();
        QVBoxLayout *buildLayout = new QVBoxLayout(buildTab);
        buildInfo = new QTextEdit();
        buildInfo->setReadOnly(true);
        buildLayout->addWidget(buildInfo);
        tabs->addTab(buildTab, "Build Configuration");

        // Docker/Container Tab
        QWidget *dockerTab = new QWidget();
        QVBoxLayout *dockerLayout = new QVBoxLayout(dockerTab);
        dockerInfo = new QTextEdit();
        dockerInfo->setReadOnly(true);
        dockerLayout->addWidget(dockerInfo);
        tabs->addTab(dockerTab, "Docker/Container");

        // CI/CD Tab
        QWidget *ciTab = new QWidget();
        QVBoxLayout *ciLayout = new QVBoxLayout(ciTab);
        ciInfo = new QTextEdit();
        ciInfo->setReadOnly(true);
        ciLayout->addWidget(ciInfo);
        tabs->addTab(ciTab, "CI/CD Environment");

        // System Tab
        QWidget *sysTab = new QWidget();
        QVBoxLayout *sysLayout = new QVBoxLayout(sysTab);
        sysInfo = new QTextEdit();
        sysInfo->setReadOnly(true);
        sysLayout->addWidget(sysInfo);
        tabs->addTab(sysTab, "System Info");

        mainLayout->addWidget(tabs);

        QPushButton *refreshBtn = new QPushButton("Refresh Information");
        connect(refreshBtn, &QPushButton::clicked, this, &BuildInfoWidget::displayInfo);
        mainLayout->addWidget(refreshBtn);
    }

    void displayInfo()
    {
        showBuildInfo();
        showDockerInfo();
        showCIInfo();
        showSystemInfo();
    }

    void showBuildInfo()
    {
        buildInfo->clear();
        buildInfo->append("<b>Qt Build Information:</b><br>");

        buildInfo->append(QString("Qt Compiled Version: %1").arg(QT_VERSION_STR));
        buildInfo->append(QString("Qt Runtime Version: %1").arg(qVersion()));

#ifdef QT_DEBUG
        buildInfo->append("Build Type: <b>Debug</b>");
        buildInfo->append("Optimization: None (for debugging)");
        buildInfo->append("Debug Symbols: Yes");
#else
        buildInfo->append("Build Type: <b>Release</b>");
        buildInfo->append("Optimization: Full (-O2 or -O3)");
        buildInfo->append("Debug Symbols: Stripped");
#endif

        buildInfo->append("<br><b>Compiler Information:</b>");
#if defined(Q_CC_MSVC)
        buildInfo->append(QString("Compiler: MSVC %1").arg(_MSC_VER));
#elif defined(Q_CC_GNU)
        buildInfo->append(QString("Compiler: GCC %1.%2.%3")
                              .arg(__GNUC__)
                              .arg(__GNUC_MINOR__)
                              .arg(__GNUC_PATCHLEVEL__));
#elif defined(Q_CC_CLANG)
        buildInfo->append(QString("Compiler: Clang %1.%2.%3")
                              .arg(__clang_major__)
                              .arg(__clang_minor__)
                              .arg(__clang_patchlevel__));
#else
        buildInfo->append("Compiler: Unknown");
#endif

        buildInfo->append(QString("Build ABI: %1").arg(QSysInfo::buildAbi()));
        buildInfo->append(QString("Architecture: %1").arg(QSysInfo::currentCpuArchitecture()));

        buildInfo->append("<br><b>CMake Build Info:</b>");
        buildInfo->append(QString("Build Timestamp: %1 %2").arg(__DATE__).arg(__TIME__));

        buildInfo->append("<br><b>Qt Modules:</b>");
        buildInfo->append("• Qt6Core - Core functionality");
        buildInfo->append("• Qt6Gui - GUI foundation");
        buildInfo->append("• Qt6Widgets - Widget toolkit");
    }

    void showDockerInfo()
    {
        dockerInfo->clear();
        dockerInfo->append("<b>Container Detection:</b><br>");

        // Check multiple Docker indicators
        bool isDocker = false;
        QStringList evidence;

        // Method 1: Check /.dockerenv
        if (QFile::exists("/.dockerenv"))
        {
            isDocker = true;
            evidence << "✓ /.dockerenv file exists";
        }
        else
        {
            evidence << "✗ /.dockerenv file not found";
        }

        // Method 2: Check environment variable
        QString dockerEnv = qEnvironmentVariable("DOCKER_CONTAINER");
        if (!dockerEnv.isEmpty())
        {
            isDocker = true;
            evidence << QString("✓ DOCKER_CONTAINER=%1").arg(dockerEnv);
        }
        else
        {
            evidence << "✗ DOCKER_CONTAINER not set";
        }

        // Method 3: Check /proc/1/cgroup (Linux only)
        QFile cgroupFile("/proc/1/cgroup");
        if (cgroupFile.open(QIODevice::ReadOnly))
        {
            QString cgroup = QString::fromUtf8(cgroupFile.readAll());
            if (cgroup.contains("docker") || cgroup.contains("containerd"))
            {
                isDocker = true;
                evidence << "✓ /proc/1/cgroup contains container info";
            }
            else
            {
                evidence << "✗ /proc/1/cgroup shows no container";
            }
        }

        dockerInfo->append(QString("<b>Running in Docker:</b> %1")
                               .arg(isDocker ? "<span style='color: green;'>YES</span>" : "<span style='color: red;'>NO</span>"));

        dockerInfo->append("<br><b>Detection Evidence:</b>");
        for (const QString &ev : evidence)
        {
            dockerInfo->append(ev);
        }

        if (isDocker)
        {
            dockerInfo->append("<br><b>Container Information:</b>");
            dockerInfo->append("Running inside a Docker container");
            dockerInfo->append("This enables:");
            dockerInfo->append("• Reproducible builds");
            dockerInfo->append("• Consistent environment");
            dockerInfo->append("• Easy CI/CD integration");
            dockerInfo->append("• Isolated dependencies");
        }

        dockerInfo->append("<br><b>Multi-Stage Build Benefits:</b>");
        dockerInfo->append("• Build stage: Full Qt SDK + compilers (2-3GB)");
        dockerInfo->append("• Runtime stage: Only libraries + app (~200MB)");
        dockerInfo->append("• Size reduction: 10-20x smaller images");
        dockerInfo->append("• Security: No build tools in production");
    }

    void showCIInfo()
    {
        ciInfo->clear();
        ciInfo->append("<b>CI/CD Environment Detection:</b><br>");

        bool isCI = false;
        QString ciSystem = "None";

        // Check for various CI systems
        if (!qEnvironmentVariable("GITHUB_ACTIONS").isEmpty())
        {
            isCI = true;
            ciSystem = "GitHub Actions";
            ciInfo->append("<b>Platform:</b> GitHub Actions");
            ciInfo->append(QString("Repository: %1").arg(qEnvironmentVariable("GITHUB_REPOSITORY")));
            ciInfo->append(QString("Workflow: %1").arg(qEnvironmentVariable("GITHUB_WORKFLOW")));
            ciInfo->append(QString("Run ID: %1").arg(qEnvironmentVariable("GITHUB_RUN_ID")));
            ciInfo->append(QString("Actor: %1").arg(qEnvironmentVariable("GITHUB_ACTOR")));
            ciInfo->append(QString("Ref: %1").arg(qEnvironmentVariable("GITHUB_REF")));
        }
        else if (!qEnvironmentVariable("GITLAB_CI").isEmpty())
        {
            isCI = true;
            ciSystem = "GitLab CI";
            ciInfo->append("<b>Platform:</b> GitLab CI");
            ciInfo->append(QString("Project: %1").arg(qEnvironmentVariable("CI_PROJECT_NAME")));
            ciInfo->append(QString("Pipeline ID: %1").arg(qEnvironmentVariable("CI_PIPELINE_ID")));
            ciInfo->append(QString("Job Name: %1").arg(qEnvironmentVariable("CI_JOB_NAME")));
            ciInfo->append(QString("Commit SHA: %1").arg(qEnvironmentVariable("CI_COMMIT_SHA")));
            ciInfo->append(QString("Branch: %1").arg(qEnvironmentVariable("CI_COMMIT_BRANCH")));
        }
        else if (!qEnvironmentVariable("JENKINS_URL").isEmpty())
        {
            isCI = true;
            ciSystem = "Jenkins";
            ciInfo->append("<b>Platform:</b> Jenkins");
            ciInfo->append(QString("Job Name: %1").arg(qEnvironmentVariable("JOB_NAME")));
            ciInfo->append(QString("Build Number: %1").arg(qEnvironmentVariable("BUILD_NUMBER")));
            ciInfo->append(QString("Jenkins URL: %1").arg(qEnvironmentVariable("JENKINS_URL")));
        }
        else if (!qEnvironmentVariable("CI").isEmpty())
        {
            isCI = true;
            ciSystem = "Generic CI";
            ciInfo->append("<b>Platform:</b> Generic CI (CI=true)");
        }
        else
        {
            ciInfo->append("<b>Platform:</b> <span style='color: orange;'>Not running in CI</span>");
            ciInfo->append("This is a local build or production environment");
        }

        if (isCI)
        {
            ciInfo->append("<br><b>CI Best Practices:</b>");
            ciInfo->append("✓ Automated builds on every commit");
            ciInfo->append("✓ Consistent build environment");
            ciInfo->append("✓ Fast feedback on failures");
            ciInfo->append("✓ Artifact publishing for testing");
        }

        ciInfo->append("<br><b>Common CI Environment Variables:</b>");
        QStringList ciVars = {"CI", "GITHUB_ACTIONS", "GITLAB_CI", "JENKINS_URL",
                              "TRAVIS", "CIRCLECI", "BUILDKITE"};
        for (const QString &var : ciVars)
        {
            QString value = qEnvironmentVariable(var.toUtf8());
            if (!value.isEmpty())
            {
                ciInfo->append(QString("%1 = %2").arg(var).arg(value));
            }
        }

        ciInfo->append("<br><b>Typical CI Pipeline Stages:</b>");
        ciInfo->append("1. <b>Build</b> - Compile with cmake/make");
        ciInfo->append("2. <b>Test</b> - Run Qt Test suite (ctest)");
        ciInfo->append("3. <b>Package</b> - Create deployable artifacts");
        ciInfo->append("4. <b>Deploy</b> - Publish to staging/production");
    }

    void showSystemInfo()
    {
        sysInfo->clear();
        sysInfo->append("<b>Operating System:</b><br>");

        sysInfo->append(QString("OS: %1").arg(QSysInfo::prettyProductName()));
        sysInfo->append(QString("Kernel: %1 %2").arg(QSysInfo::kernelType(), QSysInfo::kernelVersion()));
        sysInfo->append(QString("Hostname: %1").arg(QSysInfo::machineHostName()));

        sysInfo->append("<br><b>Qt Installation:</b>");
        sysInfo->append(QString("Prefix: %1").arg(QLibraryInfo::path(QLibraryInfo::PrefixPath)));
        sysInfo->append(QString("Libraries: %1").arg(QLibraryInfo::path(QLibraryInfo::LibrariesPath)));
        sysInfo->append(QString("Plugins: %1").arg(QLibraryInfo::path(QLibraryInfo::PluginsPath)));

        sysInfo->append("<br><b>Library Search Paths:</b>");
        QStringList libPaths = QCoreApplication::libraryPaths();
        for (int i = 0; i < qMin(libPaths.size(), 5); ++i)
        {
            sysInfo->append(QString("%1. %2").arg(i + 1).arg(libPaths[i]));
        }

        sysInfo->append("<br><b>Build Tool Versions:</b>");

        // Check for CMake
        QProcess cmake;
        cmake.start("cmake", QStringList() << "--version");
        if (cmake.waitForFinished(1000))
        {
            QString output = cmake.readAllStandardOutput();
            QString version = output.split('\n').first();
            sysInfo->append(QString("CMake: %1").arg(version));
        }
        else
        {
            sysInfo->append("CMake: Not found or not accessible");
        }

        // Check for make/ninja
        QProcess make;
        make.start("make", QStringList() << "--version");
        if (make.waitForFinished(1000))
        {
            QString output = make.readAllStandardOutput();
            QString version = output.split('\n').first();
            sysInfo->append(QString("Make: %1").arg(version));
        }
        else
        {
            sysInfo->append("Make: Not found");
        }
    }

    QTextEdit *buildInfo;
    QTextEdit *dockerInfo;
    QTextEdit *ciInfo;
    QTextEdit *sysInfo;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    BuildInfoWidget window;
    window.show();

    return app.exec();
}

#include "main.moc"
