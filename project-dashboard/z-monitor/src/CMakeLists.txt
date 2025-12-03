# Source code build - orchestrates DDD layers and creates main executable

# Add subdirectories for each DDD layer
add_subdirectory(domain)
add_subdirectory(application)
add_subdirectory(infrastructure)
add_subdirectory(interface)

# Interface layer sources are set by interface/CMakeLists.txt via PARENT_SCOPE
# INTERFACE_SOURCES variable is now available after add_subdirectory(interface)

# QML Resources
set(QML_RESOURCES
    ${Z_MONITOR_ROOT}/resources/qml/qml.qrc
)

# Schema Resources (SQL migrations)
set(SCHEMA_RESOURCES
    ${Z_MONITOR_ROOT}/schema/schema.qrc
)

# Main executable sources
set(MAIN_SOURCES
    main.cpp
    ${INTERFACE_SOURCES}
    ${QML_RESOURCES}
    ${SCHEMA_RESOURCES}
)

# Create main executable
qt_add_executable(z-monitor
    ${MAIN_SOURCES}
)

# Link all layer libraries to main executable
# Note: Infrastructure layer transitively provides domain and application layers
# via PUBLIC linking. Only infrastructure and Qt GUI libraries need to be linked directly.
target_link_libraries(z-monitor
    PRIVATE
    z_monitor_infrastructure # Infrastructure layer (provides domain + application transitively)
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Charts
    Qt6::ChartsQml
)

# Deploy SQLite plugin to build directory for runtime loading
if(Qt6_FOUND)
    get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    execute_process(
        COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_PLUGINS
        OUTPUT_VARIABLE QT_PLUGINS_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_QML
        OUTPUT_VARIABLE QT_QML_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(SQL_PLUGIN_SOURCE "${QT_PLUGINS_DIR}/sqldrivers/libqsqlite.dylib")
    set(SQL_PLUGIN_DEST "${CMAKE_CURRENT_BINARY_DIR}/sqldrivers")
    set(CHARTS_QML_SOURCE "${QT_QML_DIR}/QtCharts")
    set(CHARTS_QML_DEST "${CMAKE_CURRENT_BINARY_DIR}/../qml/QtCharts")

    add_custom_command(TARGET z-monitor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SQL_PLUGIN_DEST}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SQL_PLUGIN_SOURCE}"
        "${SQL_PLUGIN_DEST}/libqsqlite.dylib"
        COMMENT "Deploying SQLite plugin to ${SQL_PLUGIN_DEST}"
    )

    add_custom_command(TARGET z-monitor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CHARTS_QML_DEST}
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CHARTS_QML_SOURCE}"
        "${CHARTS_QML_DEST}"
        COMMENT "Deploying QtCharts QML plugin to ${CHARTS_QML_DEST}"
    )
endif()

# Set include directories for main executable (project-relative paths)
target_include_directories(z-monitor
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Ensure proto and schema are generated before building main executable
add_dependencies(z-monitor z_monitor_proto generate_schema)
