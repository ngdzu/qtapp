/**
 * @file tst_TrendPanelTest.qml
 * @brief Qt Quick Test for TrendPanel component
 *
 * Tests trend panel rendering and property configuration.
 *
 * @author Z Monitor Team
 * @date 2025-12-03
 */

import QtQuick
import QtTest

import "../../../resources/qml/components"

TestCase {
    id: testCase
    name: "TrendPanelTest"
    when: windowShown
    width: 600
    height: 400

    // Test component
    TrendPanel {
        id: trendPanel
        anchors.fill: parent
    }

    function init() {
        // Reset to default state before each test
        trendPanel.title = "Test Panel";
        trendPanel.accentColor = "#10b981";
        trendPanel.strokeColor = "#10b981";
        trendPanel.vitalMetricName = "";
    }

    function test_01_initialState() {
        verify(trendPanel.width > 0, "Panel has width");
        verify(trendPanel.height > 0, "Panel has height");
    }

    function test_02_titleDisplay() {
        trendPanel.title = "Heart Rate Trend";
        compare(trendPanel.title, "Heart Rate Trend", "Title set correctly");
        wait(50);
    }

    function test_03_accentColorChange() {
        trendPanel.accentColor = "#ef4444"; // Red
        compare(trendPanel.accentColor, "#ef4444", "Accent color changed");
        wait(50);
    }

    function test_04_strokeColorChange() {
        trendPanel.strokeColor = "#3b82f6"; // Blue
        compare(trendPanel.strokeColor, "#3b82f6", "Stroke color changed");
        wait(50);
    }

    function test_05_vitalMetricNameBinding() {
        trendPanel.vitalMetricName = "heartRate";
        compare(trendPanel.vitalMetricName, "heartRate", "Vital metric name set");
        wait(50);
    }

    function test_06_multipleTitles() {
        // Test different vital titles
        var titles = ["SpO2", "Heart Rate", "Respiratory Rate"];
        
        for (var i = 0; i < titles.length; i++) {
            trendPanel.title = titles[i];
            compare(trendPanel.title, titles[i], "Title updated to " + titles[i]);
            wait(20);
        }
    }

    function test_07_colorConsistency() {
        trendPanel.accentColor = "#10b981";
        trendPanel.strokeColor = "#10b981";
        
        compare(trendPanel.accentColor, trendPanel.strokeColor, "Colors match");
        wait(50);
    }

    function test_08_componentRendersWithoutCrash() {
        // Stress test: Create and destroy multiple times
        for (var i = 0; i < 3; i++) {
            var panel = Qt.createQmlObject(
                'import QtQuick; import "../../../resources/qml/components"; TrendPanel { width: 400; height: 200; title: "Test" }',
                testCase,
                "dynamicPanel"
            );
            verify(panel !== null, "Panel created successfully");
            wait(10);
            panel.destroy();
        }
    }
}
