#!/bin/bash
# Script to run QML component demos using Qt's qml tool

QML_TOOL="/Users/dustinwind/Qt/6.9.2/macos/bin/qml"
DEMO_DIR="/Users/dustinwind/Development/Qt/qtapp/project-dashboard/z-monitor/resources/qml/demos"

if [ $# -eq 0 ]; then
    echo "Usage: $0 <demo-name>"
    echo ""
    echo "Available demos:"
    echo "  topbar      - TopBar component demo"
    echo "  alarmpanel  - AlarmPanel component demo"
    echo ""
    echo "Example: $0 topbar"
    exit 1
fi

case "$1" in
    topbar)
        "$QML_TOOL" "$DEMO_DIR/TopBarDemo.qml"
        ;;
    alarmpanel)
        "$QML_TOOL" "$DEMO_DIR/AlarmPanelDemo.qml"
        ;;
    *)
        echo "Unknown demo: $1"
        echo "Available demos: topbar, alarmpanel"
        exit 1
        ;;
esac
