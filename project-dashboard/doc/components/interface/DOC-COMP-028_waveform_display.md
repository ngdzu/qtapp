---
doc_id: DOC-COMP-028
title: Waveform Display Implementation
version: 1.0
category: Components
subcategory: UI/Visualization
status: Draft
owner: UI/UX Team
reviewers: [Architecture, Performance]
last_reviewed: 2025-12-01
next_review: 2026-03-01
related_docs:
  - DOC-ARCH-007_ui_ux_guide.md
  - DOC-ARCH-011_thread_model.md
  - DOC-COMP-026_data_caching_strategy.md
  - DOC-COMP-027_sensor_integration.md
tags: [waveforms, canvas, qml, ecg, pleth, rendering]
source:
  path: project-dashboard/doc/z-monitor/architecture_and_design/41_WAVEFORM_DISPLAY_IMPLEMENTATION.md
  original_id: DESIGN-041
  last_updated: 2025-11-27
---

# Purpose
Defines real-time waveform display (ECG, plethysmogram) using Qt Quick Canvas API. Target: 60 FPS smooth scrolling, 250 Hz sample rate, < 100ms latency from sample to screen.

# Requirements

## Performance
- Frame Rate: 60 FPS minimum (< 1% frame drops)
- Sample Rate: 250 Hz (ECG), 125 Hz (Pleth)
- Display Window: 10 seconds visible
- Latency: < 100ms sample→screen
- Memory: ~10 MB for 30-second buffer

## Visual
- Smooth scrolling (right to left, newest on right edge)
- No jumping or discontinuities
- Grid background for amplitude/time reference
- Color coding: ECG (green), Pleth (red), configurable
- Baseline at center; amplitude markers

## Functional
- Auto-scroll (newest data always visible)
- Pause/Resume for inspection
- Gain adjustment (user-adjustable amplitude)
- Time scale configurable (5s, 10s, 25s)
- Signal quality indicator (GOOD, FAIR, POOR)

# Architecture

## Data Flow
```
Sensor Simulator (250 Hz)
  → SharedMemorySensorDataSource
  → MonitoringService
  → WaveformCache (30-second circular buffer)
  → WaveformController (QObject, Main/UI thread)
  → WaveformChart.qml (Canvas rendering @ 60 FPS)
  → Display
```

## Component Responsibilities

### WaveformCache (C++)
- Circular buffer: 30 seconds × 250 Hz = 7,500 samples
- Thread-safe (Real-Time thread access)
- Methods: `append()`, `getLastSeconds(int)`

### WaveformController (C++ QObject)
- Bridges C++ data to QML
- Exposes `Q_PROPERTY QVariantList samples`
- Emits `samplesUpdated()` signal
- Runs on Main/UI thread

### WaveformChart.qml (QML Canvas)
- Renders waveform using Canvas 2D API
- Updates at 60 FPS via Timer
- Handles smooth scrolling, grid, scan line effect

# Implementation Highlights

## Canvas Rendering (60 FPS)
```qml
Canvas {
    onPaint: {
        var ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)
        
        // Extract window of samples
        var slice = waveformBuffer.slice(startIndex)
        
        // Draw waveform line
        ctx.strokeStyle = waveformColor
        ctx.lineWidth = 2
        ctx.beginPath()
        for (var i = 0; i < slice.length; i++) {
            var x = (i / (slice.length - 1)) * width
            var y = height - (normalizedValue * height)
            i === 0 ? ctx.moveTo(x, y) : ctx.lineTo(x, y)
        }
        ctx.stroke()
    }
}
```

## Smooth Scrolling Timer
```qml
Timer {
    interval: 16  // ~60 FPS
    running: true
    repeat: true
    onTriggered: canvas.requestPaint()
}
```

## Grid Background
- Vertical lines every 20px (≈1 second)
- Horizontal lines every 20px (amplitude markers)
- Center baseline thicker (0.5 opacity gray)

# Waveform Snapshot Renderer
- Decodes compressed snapshot blobs for alarm review
- Reconstructs waveform samples on demand
- Delta/RLE decompression
- Stateless renderer for playback/export

# Performance Optimization
- Pre-allocated buffers (no runtime allocations)
- Window slicing (render only visible samples)
- Gain/normalization cached
- Canvas layer separation (grid static, waveform dynamic)
- Optional glow effect via shadow API

# Platform Considerations
- Qt Quick Canvas API cross-platform
- Hardware acceleration where available
- Fallback to software rendering

# Verification
- Functional: 60 FPS rendering verified; pause/resume, gain adjustment, time scale; signal quality indicator
- Code Quality: No hardcoded values; configurable colors, time windows, sample rates
- Documentation: Diagram data flow from sensor to canvas; sample code for WaveformChart.qml
- Integration: End-to-end test with SharedMemorySensorDataSource; measure latency
- Tests: Unit tests for WaveformCache, WaveformController; UI tests for smooth scrolling

# Document Metadata
| Field          | Value        |
| -------------- | ------------ |
| Original Doc   | DESIGN-041   |
| Migration Date | 2025-12-01   |
| New Doc ID     | DOC-COMP-028 |

# Revision History
- 1.0 (2025-12-01): Migrated from 41_WAVEFORM_DISPLAY_IMPLEMENTATION.md; consolidated Canvas rendering architecture.
