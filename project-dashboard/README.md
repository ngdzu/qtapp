# Project Dashboard

This repository contains the Z Monitor project - a comprehensive patient monitoring system with supporting tools and simulators.

## Project Structure

- **[`z-monitor/`](./z-monitor/)** - The Z Monitor application (main medical device application)
  - Patient monitoring system that reads sensor data and displays vital signs
  - See [`z-monitor/README.md`](./z-monitor/README.md) for build and run instructions

- **[`sensor-simulator/`](./sensor-simulator/)** - WebSocket-based sensor simulator
  - Provides simulated sensor data for development and testing
  - See [`sensor-simulator/README.md`](./sensor-simulator/README.md) for details

- **[`doc/`](./doc/)** - Comprehensive project documentation
  - Architecture, design, security, and implementation guides

- **[`scripts/`](./scripts/)** - Utility scripts
  - Screenshot capture, Mermaid diagram generation, etc.

## What is Z Monitor?

**Z Monitor** is a patient monitoring device application that:
- Reads real-time data from medical sensors (ECG, SpO2, respiration, infusion pumps)
- Displays vital signs and waveforms on an embedded touch-screen interface
- Manages multi-level alarms and notifications for clinical staff
- Securely transmits telemetry data to a central monitoring server
- Stores encrypted patient data locally with archival capabilities

The Z Monitor is designed for embedded touch-screen devices (8-inch, 1280x800 resolution) and provides continuous patient monitoring with modern UI/UX, comprehensive alarm management, and enterprise-grade security.

![Z Monitor Screenshot](./z-monitor/media/screenshot.png)

## Overview

This project demonstrates:
- **Modern QML UI:** Dark theme, custom components, and real-time visualizations.
- **C++ Backend:** Data simulation and business logic separated from UI.
- **Architecture:** Model-View-Controller (MVC) pattern with `DashboardController`.
- **Testing:** Mock objects for data services.

![Z Monitor Architecture](./doc/z-monitor/architecture_and_design/02_ARCHITECTURE.svg)

## Quick Start

### Building Z Monitor

See `z-monitor/README.md` for detailed build and run instructions.

### Building Sensor Simulator

See `sensor-simulator/README.md` for detailed build and run instructions.

