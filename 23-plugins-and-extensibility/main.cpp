#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QPluginLoader>
#include <QDir>
#include <QMessageBox>
#include <QFileInfo>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Lesson 23: Plugins and Extensibility");
    window.resize(600, 400);

    QVBoxLayout *mainLayout = new QVBoxLayout(&window);

    // Title
    QLabel *titleLabel = new QLabel("<h2>Qt Plugin System Demo</h2>");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Info section
    QLabel *infoLabel = new QLabel(
        "<b>Understanding Qt Plugins:</b><br>"
        "• Plugins are shared libraries loaded at runtime<br>"
        "• They implement interfaces using Q_PLUGIN_METADATA<br>"
        "• QPluginLoader manages dynamic loading/unloading<br>"
        "• Used extensively in Qt Creator and KDE applications");
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("padding: 10px; background-color: #f0f0f0; border-radius: 5px;");
    mainLayout->addWidget(infoLabel);

    // Output area
    QTextEdit *output = new QTextEdit();
    output->setReadOnly(true);
    mainLayout->addWidget(output);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *scanBtn = new QPushButton("Scan for Plugins");
    QPushButton *infoBtn = new QPushButton("Plugin Info");
    QPushButton *clearBtn = new QPushButton("Clear");
    buttonLayout->addWidget(scanBtn);
    buttonLayout->addWidget(infoBtn);
    buttonLayout->addWidget(clearBtn);
    mainLayout->addLayout(buttonLayout);

    // Scan for plugins button
    QObject::connect(scanBtn, &QPushButton::clicked, [output]()
                     {
        output->clear();
        output->append("<b>Scanning for plugins...</b><br>");
        
        // Common plugin directories to check
        QStringList searchPaths = {
            QDir::currentPath() + "/plugins",
            QCoreApplication::applicationDirPath() + "/plugins",
            "/opt/lesson23/plugins",
            QDir::currentPath()
        };
        
        output->append(QString("Application directory: %1<br>").arg(QCoreApplication::applicationDirPath()));
        output->append(QString("Current directory: %1<br>").arg(QDir::currentPath()));
        output->append("<br><b>Search paths:</b>");
        
        int foundCount = 0;
        for (const QString &path : searchPaths) {
            QDir dir(path);
            output->append(QString("• %1: %2").arg(path, dir.exists() ? "exists" : "not found"));
            
            if (dir.exists()) {
                QStringList filters;
#ifdef Q_OS_WIN
                filters << "*.dll";
#elif defined(Q_OS_MAC)
                filters << "*.dylib" << "*.so";
#else
                filters << "*.so";
#endif
                
                QStringList files = dir.entryList(filters, QDir::Files);
                if (!files.isEmpty()) {
                    output->append(QString("  Found %1 library file(s):").arg(files.size()));
                    for (const QString &file : files) {
                        output->append(QString("    - %1").arg(file));
                        foundCount++;
                    }
                }
            }
        }
        
        output->append(QString("<br><b>Result:</b> Found %1 potential plugin file(s)").arg(foundCount));
        
        if (foundCount == 0) {
            output->append("<br><i>Note: In this demo, no actual plugins are present. "
                          "To create a plugin, you would:</i>");
            output->append("1. Define an interface with Q_DECLARE_INTERFACE");
            output->append("2. Create a plugin class with Q_PLUGIN_METADATA");
            output->append("3. Build it as a shared library");
            output->append("4. Place it in the plugins/ directory");
        } });

    // Plugin info button
    QObject::connect(infoBtn, &QPushButton::clicked, [output]()
                     {
        output->clear();
        output->append("<b>QPluginLoader API Overview:</b><br>");
        
        output->append("<b>Loading a plugin:</b>");
        output->append("  QPluginLoader loader(\"path/to/plugin.so\");");
        output->append("  if (loader.load()) {");
        output->append("      QObject *plugin = loader.instance();");
        output->append("      MyInterface *iface = qobject_cast<MyInterface*>(plugin);");
        output->append("      if (iface) { /* use plugin */ }");
        output->append("  }<br>");
        
        output->append("<b>Defining an interface:</b>");
        output->append("  class MyInterface {");
        output->append("  public:");
        output->append("      virtual ~MyInterface() {}");
        output->append("      virtual QString name() const = 0;");
        output->append("  };");
        output->append("  Q_DECLARE_INTERFACE(MyInterface, \"com.example.MyInterface/1.0\")<br>");
        
        output->append("<b>Implementing a plugin:</b>");
        output->append("  class MyPlugin : public QObject, public MyInterface {");
        output->append("      Q_OBJECT");
        output->append("      Q_PLUGIN_METADATA(IID \"com.example.MyInterface\")");
        output->append("      Q_INTERFACES(MyInterface)");
        output->append("  public:");
        output->append("      QString name() const override { return \"My Plugin\"; }");
        output->append("  };<br>");
        
        output->append("<b>Key methods:</b>");
        output->append("• loader.load() - Load the plugin");
        output->append("• loader.instance() - Get plugin instance");
        output->append("• loader.unload() - Unload the plugin");
        output->append("• loader.isLoaded() - Check load status");
        output->append("• loader.errorString() - Get error message");
        
        // Demonstrate trying to load a plugin
        output->append("<br><b>Attempting to load example plugin...</b>");
        QPluginLoader loader("/opt/lesson23/plugins/example.so");
        output->append(QString("Plugin path: %1").arg(loader.fileName()));
        output->append(QString("File exists: %1").arg(QFileInfo::exists(loader.fileName()) ? "Yes" : "No"));
        
        if (loader.load()) {
            output->append("✓ Plugin loaded successfully!");
            QObject *plugin = loader.instance();
            if (plugin) {
                output->append(QString("✓ Plugin instance obtained: %1").arg(plugin->metaObject()->className()));
            }
            loader.unload();
        } else {
            output->append(QString("✗ Failed to load: %1").arg(loader.errorString()));
            output->append("<br><i>This is expected - no actual plugin exists in this demo.</i>");
        } });

    // Clear button
    QObject::connect(clearBtn, &QPushButton::clicked, output, &QTextEdit::clear);

    // Initial message
    output->append("<b>Welcome to the Qt Plugin System Demo!</b><br>");
    output->append("Click 'Scan for Plugins' to search for plugin files");
    output->append("Click 'Plugin Info' to see QPluginLoader usage examples<br>");
    output->append("<i>Note: This is an educational demo. No actual plugins are loaded.</i>");

    window.show();

    return app.exec();
}
