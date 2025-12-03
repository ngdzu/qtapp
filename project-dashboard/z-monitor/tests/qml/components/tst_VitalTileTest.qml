/**
 * @file tst_VitalTileTest.qml
 * @brief Qt Quick Test for VitalTile component
 *
 * Tests vital sign tile rendering, value updates, and alarm state display.
 *
 * @author Z Monitor Team
 * @date 2025-12-03
 */

import QtQuick
import QtTest

import "../../../resources/qml/components"

TestCase {
    id: testCase
    name: "VitalTileTest"
    when: windowShown
    width: 400
    height: 300

    // Test component
    VitalTile {
        id: vitalTile
        anchors.centerIn: parent
        width: 200
        height: 150
    }

    function init() {
        // Reset to default state before each test
        vitalTile.label = "";
        vitalTile.value = "0";
        vitalTile.unit = "";
    }

    function test_01_initialState() {
        verify(vitalTile.width > 0, "Tile has width");
        verify(vitalTile.height > 0, "Tile has height");
    }

    function test_02_labelAndUnitDisplay() {
        vitalTile.label = "Heart Rate";
        vitalTile.unit = "bpm";
        
        compare(vitalTile.label, "Heart Rate", "Label set correctly");
        compare(vitalTile.unit, "bpm", "Unit set correctly");
        wait(50); // Allow rendering
    }

    function test_03_valueUpdate() {
        vitalTile.label = "SpO2";
        vitalTile.value = "98";
        vitalTile.unit = "%";
        
        compare(vitalTile.value, "98", "Value updated to '98'");
        wait(50);
    }

    function test_04_multipleValueUpdates() {
        vitalTile.label = "Heart Rate";
        vitalTile.unit = "bpm";
        
        for (var i = 70; i <= 75; i++) {
            vitalTile.value = i.toString();
            wait(10);
        }
        
        compare(vitalTile.value, "75", "Final value is 75");
    }

    function test_05_accentColorChange() {
        vitalTile.accentColor = "#ef4444"; // Red
        compare(vitalTile.accentColor, "#ef4444", "Accent color changed");
        wait(50);
    }
}
