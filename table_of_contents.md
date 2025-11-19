Table of Contents — Qt Learning Course

This file lists lesson folders, difficulty and a short description of each lesson. Each lesson should become a folder named with a two-digit prefix and a short slug (e.g., `01-qt-setup`).

1. 01-qt-setup — Beginner
- Overview: Installing Qt, setting up toolchain, CMake basics, building a minimal Qt Widgets "Hello" app.
- Modules: Qt6::Widgets
- Notes: Includes a `Dockerfile` that installs Qt and CMake.

2. 02-cpp-prereqs — Beginner
- Overview: Short refresher on modern C++ features used in Qt examples (RAII, smart pointers, lambdas, move semantics).
- Modules: none (standard C++)

3. 03-widgets-basics — Beginner
- Overview: Basic QWidget, QPushButton, QLabel; event loop and main window creation.
- Modules: Qt6::Widgets

4. 04-layouts-and-containers — Beginner
- Overview: QHBoxLayout, QVBoxLayout, QGridLayout; resizing behavior and size policies.
- Modules: Qt6::Widgets

5. 05-signals-and-slots — Beginner
- Overview: QObject, signals/slots (new syntax), connecting lambdas, queued vs direct connections.
- Modules: Qt6::Core, Qt6::Widgets

6. 06-events-and-handling — Beginner
- Overview: Event system, overriding `event()` and widget-specific events (mouse, key), event filters.
- Modules: Qt6::Core, Qt6::Widgets

7. 07-dialogs-and-widgets — Beginner
- Overview: QFileDialog, QMessageBox, custom dialogs, modal vs modeless.
- Modules: Qt6::Widgets

8. 08-model-view-intro — Intermediate
- Overview: Model/View architecture, QAbstractItemModel vs convenience models, QListView/QTableView.
- Modules: Qt6::Core, Qt6::Widgets

9. 09-implementing-models — Intermediate
- Overview: Implementing a custom `QAbstractTableModel` and connecting to `QTableView`.
- Modules: Qt6::Core, Qt6::Widgets

10. 10-item-delegates — Intermediate
- Overview: QStyledItemDelegate, custom editors and delegates for complex cell rendering.
- Modules: Qt6::Widgets

11. 11-resources-and-translations — Intermediate
- Overview: Resource system (`.qrc`), embedding assets, basic i18n using `tr()` and `lupdate`/`lrelease`.
- Modules: Qt6::Core, Qt6::Widgets

12. 12-qtquick-qml-intro — Intermediate
- Overview: QML vs Widgets, Qt Quick scene, basic QML components and integrating with C++ backend.
- Modules: Qt6::Quick, Qt6::Qml
- Notes: Include Dockerfile steps for Qt Quick.

13. 13-qtquick-controls — Intermediate
- Overview: Qt Quick Controls 2, styling, basic animations in QML.
- Modules: Qt6::Quick, Qt6::QuickControls2

14. 14-graphicsview — Intermediate
- Overview: QGraphicsScene/View, items, coordinates, transformations and performance tips.
- Modules: Qt6::Widgets, Qt6::Gui

15. 15-multimedia — Intermediate
- Overview: Playing audio/video, QMediaPlayer, capturing devices basics.
- Modules: Qt6::Multimedia

16. 16-networking — Intermediate
- Overview: QNetworkAccessManager for HTTP, sockets (QTcpSocket/QTcpServer) basics.
- Modules: Qt6::Network

17. 17-threading-and-concurrency — Intermediate
- Overview: QThread vs worker objects, QtConcurrent basics, signals across threads.
- Modules: Qt6::Core

18. 18-sql-and-models — Intermediate
- Overview: Qt SQL drivers, QSqlDatabase, integrating SQL with models/views.
- Modules: Qt6::Sql
- Notes: Dockerfile must install database client (e.g., sqlite3) where appropriate.

19. 19-serialization-and-settings — Intermediate
- Overview: QSettings, QVariant, JSON serialization with QJsonDocument.
- Modules: Qt6::Core

20. 20-styles-themes-and-palette — Intermediate
- Overview: Styling widgets, QStyle, QPalette, custom styling using QSS and QML themes.
- Modules: Qt6::Widgets, Qt6::Quick

21. 21-animation-and-transitions — Intermediate
- Overview: QVariantAnimation, QPropertyAnimation, QML animations.
- Modules: Qt6::Core, Qt6::Widgets, Qt6::Quick

22. 22-opengl-and-3d — Advanced
- Overview: QOpenGLWidget, integrating raw OpenGL rendering and examples.
- Modules: Qt6::OpenGL, Qt6::Gui

23. 23-plugins-and-extensibility — Advanced
- Overview: Plugin architecture with `QPluginLoader`, writing a minimal plugin and loading at runtime.
- Modules: Qt6::Core, Qt6::Widgets

24. 24-testing-and-automation — Advanced
- Overview: Unit testing with QTest, GUI testing basics, CI integration tips.
- Modules: Qt6::Test

25. 25-deployment-and-packaging — Advanced
- Overview: Deploying Qt apps (linux/mac/windows), using `windeployqt`, `macdeployqt`, `linux` packaging tips, containerized delivery.
- Modules: varies

26. 26-ci-docker-and-builds — Advanced
- Overview: Example GitHub Actions / GitLab CI to build lessons in container, caching Qt packages, reproducible builds.
- Modules: tools & shell scripts

27. 27-performance-and-profiling — Advanced
- Overview: Profiling Qt apps, memory/CPU bottlenecks, GPU profiling tips.
- Modules: none (tools)

28. 28-accessibility-and-internationalization — Advanced
- Overview: Accessibility best practices, role and name properties in QML/Widgets, advanced i18n.
- Modules: QtAccessibility, Qt6::Core

Appendix: Extra lessons (optional)
- CI: cross-compilation for embedded targets
- Advanced QML: custom QML types in C++
- Embedded & IoT: Qt for MCUs (if applicable)

Each lesson folder should follow the scaffolding rules from `prompt.md` and include a `README.md` with quick build/run instructions and a `Dockerfile` when non-default Qt modules or system libs are required.
