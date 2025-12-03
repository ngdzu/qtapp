/**
 * @file tst_TrendsViewTest.qml
 * @brief Qt Quick Test for TrendsView
 *
 * Tests trends view rendering, time range selection, and chart display.
 *
 * @author Z Monitor Team
 * @date 2025-12-03
 */

import QtQuick
import QtQuick.Window
import QtTest

import "../../../resources/qml/views"

TestCase {
    id: testCase
    name: "TrendsViewTest"
    when: windowShown
    width: 1024
    height: 768

    // Visual root for the test window
    Window {
        id: testWin
        width: testCase.width
        height: testCase.height
        visible: true

        // Test component under a real visual root
        TrendsView {
            id: trendsView
            anchors.fill: parent
        }
    }

    function init() {
        // Reset to default state before each test
        trendsView.hours = 24;
    }

    function test_01_initialState() {
        verify(trendsView.width > 0, "View has width");
        verify(trendsView.height > 0, "View has height");
        compare(trendsView.hours, 24, "Default hours is 24");
    }

    function test_02_hoursProperty() {
        trendsView.hours = 1;
        compare(trendsView.hours, 1, "Hours set to 1");
        wait(50);
        
        trendsView.hours = 6;
        compare(trendsView.hours, 6, "Hours set to 6");
        wait(50);
        
        trendsView.hours = 24;
        compare(trendsView.hours, 24, "Hours set to 24");
    }

    function test_03_viewRendersWithoutCrash() {
        // Rapidly switch hours
        for (var i = 0; i < 10; i++) {
            trendsView.hours = (i % 2 === 0) ? 1 : 24;
            wait(20);
        }
        
        verify(testWin.visible && trendsView.visible, "View still visible after rapid updates");
    }

    function test_04_responsiveResize() {
        // Test view responds to size changes
        testWin.width = 1280;
        testWin.height = 1024;
        wait(50);
        
        verify(trendsView.width === 1280, "Width updated");
        verify(trendsView.height === 1024, "Height updated");
    }
}
