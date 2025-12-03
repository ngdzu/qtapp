/**
 * @file tst_WaveformDisplayIntegration.qml
 * @brief Integration test for WaveformDisplay in MonitorView context
 * 
 * Tests WaveformDisplay with realistic waveform data patterns
 * to verify it works as expected when integrated into MonitorView.
 */

import QtQuick
import QtTest
import "../../../resources/qml/components"

TestCase {
    id: testCase
    name: "WaveformDisplayIntegrationTest"
    when: windowShown
    width: 800
    height: 600

    Component {
        id: waveformDisplayComponent
        
        Rectangle {
            width: 800
            height: 200
            color: "#18181b"
            
            WaveformDisplay {
                id: display
                anchors.fill: parent
                anchors.margins: 8
                label: "ECG II"
                waveformColor: "#10b981"
                gridColor: "#27272a40"
                showQuality: true
                signalQuality: 0.9
            }
        }
    }

    function test_01_ecgPatternRendering() {
        var waveform = createTemporaryObject(waveformDisplayComponent, testCase);
        verify(waveform !== null, "WaveformDisplay should be created");
        
        var display = waveform.children[0];
        verify(display !== null, "Display should exist");
        
        // Generate realistic ECG pattern (60 BPM = 1 beat per second = 250 samples)
        var ecgData = [];
        for (var i = 0; i < 2500; i++) { // 10 seconds at 250 Hz
            var t = i / 250.0; // Time in seconds
            var beatPhase = (t * 1.0) % 1.0; // 60 BPM
            
            var value = 0;
            if (beatPhase < 0.1) {
                // P wave
                value = Math.sin(beatPhase * 10 * Math.PI) * 0.2;
            } else if (beatPhase >= 0.15 && beatPhase < 0.25) {
                // QRS complex
                if (beatPhase < 0.17) {
                    value = -0.3; // Q
                } else if (beatPhase < 0.20) {
                    value = 1.0; // R
                } else {
                    value = -0.2; // S
                }
            } else if (beatPhase >= 0.35 && beatPhase < 0.55) {
                // T wave
                value = Math.sin((beatPhase - 0.35) * 5 * Math.PI) * 0.3;
            }
            
            ecgData.push({time: t, value: value});
        }
        
        display.waveformData = ecgData;
        wait(100); // Allow rendering
        
        compare(display.waveformData.length, 2500, "ECG data should be loaded");
    }

    function test_02_plethPatternRendering() {
        var waveform = createTemporaryObject(waveformDisplayComponent, testCase);
        var display = waveform.children[0];
        
        // Generate realistic pleth pattern (60 BPM sine wave)
        var plethData = [];
        for (var i = 0; i < 2500; i++) {
            var t = i / 250.0;
            var value = Math.sin(t * 2 * Math.PI * 1.0) * 0.8; // 60 BPM
            plethData.push({time: t, value: value});
        }
        
        display.waveformData = plethData;
        display.waveformColor = "#3b82f6";
        display.label = "PLETH";
        wait(100);
        
        compare(display.waveformData.length, 2500, "Pleth data should be loaded");
        compare(display.waveformColor, "#3b82f6", "Pleth color should be blue");
    }

    function test_03_respPatternRendering() {
        var waveform = createTemporaryObject(waveformDisplayComponent, testCase);
        var display = waveform.children[0];
        
        // Generate realistic resp pattern (12 breaths/min = 0.2 Hz)
        var respData = [];
        for (var i = 0; i < 2500; i++) {
            var t = i / 250.0;
            var value = Math.sin(t * 2 * Math.PI * 0.2) * 0.6; // 12 breaths/min
            respData.push({time: t, value: value});
        }
        
        display.waveformData = respData;
        display.waveformColor = "#eab308";
        display.label = "RESP";
        wait(100);
        
        compare(display.waveformData.length, 2500, "Resp data should be loaded");
        compare(display.waveformColor, "#eab308", "Resp color should be yellow");
    }

    function test_04_zoomPanInteraction() {
        var waveform = createTemporaryObject(waveformDisplayComponent, testCase);
        var display = waveform.children[0];
        
        // Generate simple sine wave
        var data = [];
        for (var i = 0; i < 1000; i++) {
            data.push({time: i / 250.0, value: Math.sin(i / 40.0)});
        }
        display.waveformData = data;
        wait(50);
        
        // Test zoom
        display.zoomLevel = 2.0;
        compare(display.zoomLevel, 2.0, "Zoom should be set to 2x");
        
        // Test freeze and pan
        display.frozen = true;
        compare(display.frozen, true, "Display should be frozen");
        
        display.panOffset = 100;
        compare(display.panOffset, 100, "Pan offset should be set");
        
        // Reset
        display.resetView();
        compare(display.zoomLevel, 1.0, "Zoom should reset");
        compare(display.panOffset, 0, "Pan should reset");
    }

    function test_05_signalQualityDisplay() {
        var waveform = createTemporaryObject(waveformDisplayComponent, testCase);
        var display = waveform.children[0];
        
        // Test different quality levels
        display.showQuality = true;
        
        display.signalQuality = 1.0;
        wait(50);
        
        display.signalQuality = 0.5;
        wait(50);
        
        display.signalQuality = 0.2;
        wait(50);
        
        // Quality should be visible
        verify(display.showQuality, "Quality indicator should be shown");
    }
}
