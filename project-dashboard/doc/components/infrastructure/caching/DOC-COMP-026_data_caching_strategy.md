---
doc_id: DOC-COMP-026
title: Data Caching Strategy
version: 1.0
category: Components
subcategory: Infrastructure
status: Draft
owner: Infrastructure Team
reviewers: [Architecture, Performance]
last_reviewed: 2025-12-01
next_review: 2026-03-01
related_docs:
  - DOC-ARCH-001_software_architecture.md
  - DOC-ARCH-005_data_flow_and_caching.md
  - DOC-ARCH-011_thread_model.md
  - DOC-COMP-027_sensor_integration.md
tags: [caching, vitals, waveforms, persistence, memory]
source:
  path: project-dashboard/doc/z-monitor/architecture_and_design/36_DATA_CACHING_STRATEGY.md
  original_id: DESIGN-036
  last_updated: 2025-11-27
---

# Purpose
Defines in-memory caching strategy separating critical (real-time monitoring/alarms) from non-critical (persistence) paths. Includes VitalsCache (3-day capacity), WaveformCache (30-second circular buffer), periodic persistence, and 7-day retention.

# Critical Architecture Decisions

## ✅ Strengths
- In-memory critical path (low latency for alarms < 50ms)
- Periodic persistence (non-blocking, every 10 min)
- 3-day in-memory capacity (buffer for network/DB outages)
- 7-day local storage (compliance, trend analysis)

## ⚠️ Addressed Issues
- **ISensorDataSource interface:** Abstracts simulator vs real sensors (DIP)
- **In-memory cache structure:** `std::deque<VitalRecord>`, FIFO eviction, `QReadWriteLock`
- **Persistence timing:** Every 10 min OR 10k records OR 80% capacity OR immediate flush for P1 alarms
- **Cleanup schedule:** Daily at 3 AM local time, rolling 7-day window
- **Waveform separation:** Dedicated `WaveformCache` (30s circular buffer), no 7-day retention

# Component Design

## VitalsCache
- **Capacity:** 2.6M records ≈ 3 days @ 10 Hz (~372 MB)
- **Structure:** `std::deque<VitalRecord>`, protected by `QReadWriteLock`
- **Eviction:** FIFO when > maxRecords
- **Thread:** Real-Time Processing thread
- **Methods:** `append()`, `getRange()`, `getUnpersistedVitals()`, `markAsPersisted()`

## WaveformCache
- **Capacity:** 15,000 samples (30s @ 500 Hz)
- **Retention:** Display-only, no persistence
- **Snapshots:** Optional 10-second compressed blobs for critical alarms
- **Structure:** Circular buffer
- **Thread:** Real-Time Processing thread

## PersistenceScheduler
- **Triggers:** Timer (10 min) OR threshold (10k unpersisted OR 80% capacity)
- **Batch Size:** 10k records per transaction
- **Thread:** Database I/O thread
- **Failure:** Exponential backoff, retry until success

## DataCleanupService
- **Schedule:** Daily at 3 AM local time
- **Retention:** Rolling 7-day window (UTC-based cutoff)
- **Batch Delete:** 10k records per transaction
- **Vacuum:** Quarterly (not daily)

# Data Flow Priority Levels
- **P1 Critical (< 50ms):** Sensor → VitalsCache → Alarm Detection → UI
- **P2 High (every 10s):** VitalsCache → Telemetry Batch → Network
- **P3 Medium (every 10 min):** VitalsCache → Database Persistence
- **P4 Low (daily 3 AM):** Database Cleanup (delete > 7 days)

# Missing Interfaces (Created)
- `ISensorDataSource`: Abstract sensor data source (DIP)
- `IVitalsRepository`: Abstract vitals persistence
- `IAlarmRepository`: Abstract alarm persistence

# Memory Budget
- VitalsCache: 372 MB (3-day vitals)
- WaveformCache: ~10 MB (30-second waveforms)
- Object pools: Pre-allocated at startup
- RT thread: Zero heap allocations

# Verification
- Functional: Simulate 3-day vitals accumulation; verify eviction, persistence, cleanup
- Code Quality: No hardcoded values; use configuration for intervals/capacities
- Documentation: Diagrams for data flow priorities and component interactions
- Integration: End-to-end test with mock sensor, verify latencies
- Tests: Unit tests for cache logic, persistence scheduler, cleanup service

# Document Metadata
| Field          | Value        |
| -------------- | ------------ |
| Original Doc   | DESIGN-036   |
| Migration Date | 2025-12-01   |
| New Doc ID     | DOC-COMP-026 |

# Revision History
- 1.0 (2025-12-01): Migrated from 36_DATA_CACHING_STRATEGY.md; consolidated caching architecture.
