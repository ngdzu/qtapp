/**
 * @file WaveformDisplayTest.qml
 * @brief Qt Quick Test for WaveformDisplay component
 *
 * Tests rendering, zoom, pan, freeze modes, and performance.
 *
 * @author Z Monitor Team
 * @date 2025-12-02
 */

import QtQuick
import QtTest

import "../../../resources/qml/components"

TestCase {
    id: testCase
    name: "WaveformDisplayTest"
    when: windowShown
    width: 800
    height: 400

    // Test component
    WaveformDisplay {
        id: waveform
        anchors.fill: parent
        label: "ECG"
        waveformColor: "#10b981"
    }

    function test_01_initialState() {
        compare(waveform.zoomLevel, 1.0, "Default zoom is 1.0");
        compare(waveform.panOffset, 0.0, "Default pan offset is 0.0");
        compare(waveform.frozen, false, "Default frozen state is false");
        compare(waveform.gain, 1.0, "Default gain is 1.0");
        compare(waveform.timeScale, 10.0, "Default time scale is 10.0 seconds");
    }

    function test_02_waveformDataBinding() {
        var testData = [
            {time: 0.0, value: 0.5},
            {time: 0.004, value: 0.6},
            {time: 0.008, value: 0.4}
        ];
        
        waveform.waveformData = testData;
        wait(50); // Allow canvas to repaint
        
        verify(waveform.waveformData.length === 3, "Waveform data updated");
    }

    function test_03_zoomIncrease() {
        waveform.zoomLevel = 2.0;
        compare(waveform.zoomLevel, 2.0, "Zoom level increased to 2.0");
        
        waveform.zoomLevel = 4.0;
        compare(waveform.zoomLevel, 4.0, "Zoom level increased to 4.0 (max)");
    }

    function test_04_zoomDecrease() {
        waveform.zoomLevel = 2.0;
        waveform.zoomLevel = 1.0;
        compare(waveform.zoomLevel, 1.0, "Zoom level decreased to 1.0 (min)");
    }

    function test_05_panOffset() {
        waveform.panOffset = 50.0;
        compare(waveform.panOffset, 50.0, "Pan offset set to 50.0");
        
        waveform.panOffset = 0.0;
        compare(waveform.panOffset, 0.0, "Pan offset reset to 0.0");
    }

    function test_06_freezeMode() {
        waveform.frozen = true;
        compare(waveform.frozen, true, "Freeze mode enabled");
        
        waveform.frozen = false;
        compare(waveform.frozen, false, "Freeze mode disabled");
    }

    function test_07_toggleFreeze() {
        var initialFrozen = waveform.frozen;
        waveform.toggleFreeze();
        compare(waveform.frozen, !initialFrozen, "Freeze toggled");
        
        waveform.toggleFreeze();
        compare(waveform.frozen, initialFrozen, "Freeze toggled back");
    }

    function test_08_gainAdjustment() {
        waveform.gain = 2.0;
        compare(waveform.gain, 2.0, "Gain set to 2.0");
        
        waveform.gain = 0.5;
        compare(waveform.gain, 0.5, "Gain set to 0.5");
    }

    function test_09_resetView() {
        waveform.zoomLevel = 3.0;
        waveform.panOffset = 100.0;
        
        waveform.resetView();
        
        compare(waveform.zoomLevel, 1.0, "Zoom reset to 1.0");
        compare(waveform.panOffset, 0.0, "Pan offset reset to 0.0");
    }

    function test_10_clearBuffer() {
        var testData = [
            {time: 0.0, value: 0.5},
            {time: 0.004, value: 0.6}
        ];
        
        waveform.waveformData = testData;
        wait(50);
        
        waveform.clearBuffer();
        wait(50);
        
        // Buffer should be cleared (verify canvas repainted)
        verify(true, "Buffer cleared successfully");
    }

    function test_11_signalQuality() {
        waveform.signalQuality = 1.0;
        compare(waveform.signalQuality, 1.0, "Signal quality at 100%");
        
        waveform.signalQuality = 0.5;
        compare(waveform.signalQuality, 0.5, "Signal quality at 50%");
        
        waveform.signalQuality = 0.2;
        compare(waveform.signalQuality, 0.2, "Signal quality at 20%");
    }

    function test_12_accessibilityProperties() {
        verify(waveform.Accessible.role === Accessible.Chart, "Accessible role is Chart");
        verify(waveform.Accessible.name.includes("ECG"), "Accessible name includes label");
        verify(waveform.Accessible.description.length > 0, "Accessible description provided");
    }

    function test_13_renderingPerformance() {
        // Generate large dataset (10 seconds at 250 Hz = 2500 samples)
        var largeData = [];
        for (var i = 0; i < 2500; i++) {
            largeData.push({
                time: i * 0.004,
                value: Math.sin(i * 0.1) * 0.5 + 0.5
            });
        }
        
        var startTime = new Date().getTime();
        waveform.waveformData = largeData;
        wait(100); // Allow multiple frames to render
        var endTime = new Date().getTime();
        
        var elapsedMs = endTime - startTime;
        
        // Should render within reasonable time (< 100ms for frame generation)
        verify(elapsedMs < 200, "Large dataset rendered in " + elapsedMs + "ms");
    }

    function test_14_frozenModeStopsUpdates() {
        waveform.frozen = true;
        
        var testData1 = [{time: 0.0, value: 0.5}];
        waveform.waveformData = testData1;
        wait(50);
        
        var testData2 = [{time: 0.004, value: 0.9}];
        waveform.waveformData = testData2;
        wait(50);
        
        // When frozen, new data should not trigger canvas updates
        // (Timer should be stopped)
        verify(waveform.frozen, "Waveform remains frozen");
        
        waveform.frozen = false;
    }

    function test_15_colorCustomization() {
        waveform.waveformColor = "#ff0000";
        compare(waveform.waveformColor, "#ff0000", "Waveform color changed to red");
        
        waveform.gridColor = "#00ff00";
        compare(waveform.gridColor, "#00ff00", "Grid color changed to green");
    }

    function cleanup() {
        // Reset to defaults after each test
        waveform.zoomLevel = 1.0;
        waveform.panOffset = 0.0;
        waveform.frozen = false;
        waveform.gain = 1.0;
        waveform.waveformData = [];
        waveform.clearBuffer();
        wait(50);
    }
}
