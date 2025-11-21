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

class DeploymentInfoWidget : public QWidget
{
    Q_OBJECT
public:
    DeploymentInfoWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setupUI();
        displayInfo();
    }

private:
    void setupUI()
    {
        setWindowTitle("Lesson 25: Deployment and Packaging");
        resize(750, 550);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        QLabel *title = new QLabel("<h2>Qt Deployment Information</h2>");
        title->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(title);

        QTabWidget *tabs = new QTabWidget();

        // Executable Info Tab
        QWidget *exeTab = new QWidget();
        QVBoxLayout *exeLayout = new QVBoxLayout(exeTab);
        exeInfo = new QTextEdit();
        exeInfo->setReadOnly(true);
        exeLayout->addWidget(exeInfo);
        tabs->addTab(exeTab, "Executable Info");

        // Platform Tab
        QWidget *platformTab = new QWidget();
        QVBoxLayout *platformLayout = new QVBoxLayout(platformTab);
        platformInfo = new QTextEdit();
        platformInfo->setReadOnly(true);
        platformLayout->addWidget(platformInfo);
        tabs->addTab(platformTab, "Platform");

        // Paths Tab
        QWidget *pathsTab = new QWidget();
        QVBoxLayout *pathsLayout = new QVBoxLayout(pathsTab);
        pathsInfo = new QTextEdit();
        pathsInfo->setReadOnly(true);
        pathsLayout->addWidget(pathsInfo);
        tabs->addTab(pathsTab, "Library Paths");

        // Deployment Tab
        QWidget *deployTab = new QWidget();
        QVBoxLayout *deployLayout = new QVBoxLayout(deployTab);
        deployInfo = new QTextEdit();
        deployInfo->setReadOnly(true);
        deployLayout->addWidget(deployInfo);
        tabs->addTab(deployTab, "Deployment Guide");

        mainLayout->addWidget(tabs);

        QPushButton *refreshBtn = new QPushButton("Refresh Information");
        connect(refreshBtn, &QPushButton::clicked, this, &DeploymentInfoWidget::displayInfo);
        mainLayout->addWidget(refreshBtn);
    }

    void displayInfo()
    {
        showExecutableInfo();
        showPlatformInfo();
        showPathsInfo();
        showDeploymentGuide();
    }

    void showExecutableInfo()
    {
        exeInfo->clear();
        exeInfo->append("<b>Application Information:</b><br>");

        QString exePath = QCoreApplication::applicationFilePath();
        exeInfo->append(QString("Executable: %1").arg(exePath));

        QFileInfo fileInfo(exePath);
        exeInfo->append(QString("Directory: %1").arg(fileInfo.absolutePath()));
        exeInfo->append(QString("Size: %1 bytes").arg(fileInfo.size()));
        exeInfo->append(QString("Executable: %1").arg(fileInfo.isExecutable() ? "Yes" : "No"));

        exeInfo->append("<br><b>Qt Version Information:</b>");
        exeInfo->append(QString("Qt Runtime: %1").arg(qVersion()));
        exeInfo->append(QString("Qt Compiled: %1").arg(QT_VERSION_STR));

#ifdef QT_DEBUG
        exeInfo->append("Build Type: Debug");
#else
        exeInfo->append("Build Type: Release");
#endif

        exeInfo->append("<br><b>Qt Modules Used:</b>");
        exeInfo->append("• Qt6Core - Core functionality");
        exeInfo->append("• Qt6Gui - GUI foundation");
        exeInfo->append("• Qt6Widgets - Widget toolkit");
    }

    void showPlatformInfo()
    {
        platformInfo->clear();
        platformInfo->append("<b>System Information:</b><br>");

        platformInfo->append(QString("OS: %1").arg(QSysInfo::prettyProductName()));
        platformInfo->append(QString("Kernel: %1 %2").arg(QSysInfo::kernelType(), QSysInfo::kernelVersion()));
        platformInfo->append(QString("Architecture: %1").arg(QSysInfo::currentCpuArchitecture()));
        platformInfo->append(QString("Build ABI: %1").arg(QSysInfo::buildAbi()));

        platformInfo->append("<br><b>Platform-Specific Details:</b>");

#ifdef Q_OS_WIN
        platformInfo->append("Platform: Windows");
        platformInfo->append("Required: MSVC Runtime, Qt DLLs, platforms/qwindows.dll");
#elif defined(Q_OS_MAC)
        platformInfo->append("Platform: macOS");
        platformInfo->append("Required: .app bundle, Qt frameworks, platforms/qcocoa.dylib");
#elif defined(Q_OS_LINUX)
        platformInfo->append("Platform: Linux");
        platformInfo->append("Required: Qt libraries, platforms/qxcb.so");
#else
        platformInfo->append("Platform: Other");
#endif

        platformInfo->append("<br><b>Compiler Information:</b>");
#if defined(Q_CC_MSVC)
        platformInfo->append(QString("Compiler: MSVC %1").arg(_MSC_VER));
#elif defined(Q_CC_GNU)
        platformInfo->append(QString("Compiler: GCC %1.%2.%3").arg(__GNUC__).arg(__GNUC_MINOR__).arg(__GNUC_PATCHLEVEL__));
#elif defined(Q_CC_CLANG)
        platformInfo->append(QString("Compiler: Clang %1.%2.%3").arg(__clang_major__).arg(__clang_minor__).arg(__clang_patchlevel__));
#else
        platformInfo->append("Compiler: Unknown");
#endif
    }

    void showPathsInfo()
    {
        pathsInfo->clear();
        pathsInfo->append("<b>Qt Library Search Paths:</b><br>");

        QStringList libraryPaths = QCoreApplication::libraryPaths();
        for (int i = 0; i < libraryPaths.size(); ++i)
        {
            pathsInfo->append(QString("%1. %2").arg(i + 1).arg(libraryPaths[i]));
        }

        pathsInfo->append("<br><b>Qt Installation Paths:</b>");

        pathsInfo->append(QString("Prefix: %1").arg(QLibraryInfo::path(QLibraryInfo::PrefixPath)));
        pathsInfo->append(QString("Libraries: %1").arg(QLibraryInfo::path(QLibraryInfo::LibrariesPath)));
        pathsInfo->append(QString("Plugins: %1").arg(QLibraryInfo::path(QLibraryInfo::PluginsPath)));
        pathsInfo->append(QString("Binaries: %1").arg(QLibraryInfo::path(QLibraryInfo::BinariesPath)));

        pathsInfo->append("<br><b>Important Plugin Directories:</b>");
        pathsInfo->append("• platforms/ - Platform integration (required!)");
        pathsInfo->append("• imageformats/ - Image format plugins (PNG, JPG, etc.)");
        pathsInfo->append("• styles/ - Widget style plugins");
        pathsInfo->append("• sqldrivers/ - Database drivers");
    }

    void showDeploymentGuide()
    {
        deployInfo->clear();
        deployInfo->append("<b>Deployment Checklist:</b><br>");

#ifdef Q_OS_WIN
        deployInfo->append("<b>Windows Deployment:</b><br>");
        deployInfo->append("1. Build in Release mode");
        deployInfo->append("2. Run: windeployqt --release --no-translations MyApp.exe");
        deployInfo->append("3. Include MSVC redistributables (or install vcredist_x64.exe)");
        deployInfo->append("4. Test on clean Windows VM without Qt installed");
        deployInfo->append("5. Consider code signing for production");
        deployInfo->append("<br><b>Required Files:</b>");
        deployInfo->append("• MyApp.exe");
        deployInfo->append("• Qt6Core.dll, Qt6Gui.dll, Qt6Widgets.dll");
        deployInfo->append("• platforms/qwindows.dll");
        deployInfo->append("• MSVC runtime DLLs");

#elif defined(Q_OS_MAC)
        deployInfo->append("<b>macOS Deployment:</b><br>");
        deployInfo->append("1. Build in Release mode");
        deployInfo->append("2. Run: macdeployqt MyApp.app -dmg");
        deployInfo->append("3. Code sign: codesign --deep --sign \"Developer ID\" MyApp.app");
        deployInfo->append("4. Notarize with Apple (required for macOS 10.15+)");
        deployInfo->append("5. Test on clean Mac without Xcode/Qt");
        deployInfo->append("<br><b>Bundle Structure:</b>");
        deployInfo->append("MyApp.app/");
        deployInfo->append("  Contents/");
        deployInfo->append("    MacOS/MyApp (executable)");
        deployInfo->append("    Frameworks/ (Qt frameworks)");
        deployInfo->append("    PlugIns/platforms/qcocoa.dylib");
        deployInfo->append("    Resources/ (icons, etc.)");

#elif defined(Q_OS_LINUX)
        deployInfo->append("<b>Linux Deployment:</b><br>");
        deployInfo->append("1. Build in Release mode");
        deployInfo->append("2. Option A: Create AppImage");
        deployInfo->append("   linuxdeployqt MyApp -appimage");
        deployInfo->append("3. Option B: Create Flatpak/Snap");
        deployInfo->append("4. Option C: System packages (.deb/.rpm)");
        deployInfo->append("5. Test on different distributions");
        deployInfo->append("<br><b>AppImage Benefits:</b>");
        deployInfo->append("• Single file, runs anywhere");
        deployInfo->append("• No installation needed");
        deployInfo->append("• Bundles all dependencies");
        deployInfo->append("<br><b>Alternative: System Qt</b>");
        deployInfo->append("Rely on distribution's Qt packages (smaller but version-dependent)");
#endif

        deployInfo->append("<br><br><b>General Best Practices:</b>");
        deployInfo->append("• Always deploy release builds (smaller, faster)");
        deployInfo->append("• Test on clean systems without development tools");
        deployInfo->append("• Include README with system requirements");
        deployInfo->append("• Use deployment tools (windeployqt/macdeployqt/linuxdeployqt)");
        deployInfo->append("• Consider static linking for simple single-file deployment");
        deployInfo->append("• Document Qt version and modules used");

        deployInfo->append("<br><b>Static vs Dynamic:</b>");
        deployInfo->append("Dynamic (default): Smaller exe, needs Qt DLLs, easier updates");
        deployInfo->append("Static: Large exe (20-50MB), self-contained, licensing restrictions");
    }

    QTextEdit *exeInfo;
    QTextEdit *platformInfo;
    QTextEdit *pathsInfo;
    QTextEdit *deployInfo;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    DeploymentInfoWidget window;
    window.show();

    return app.exec();
}

#include "main.moc"
