---
doc_id: DOC-COMP-024
title: WaveformCache
version: v1.0
category: Component
subcategory: Infrastructure Layer / Caching
status: Draft
owner: Infrastructure Team
reviewers: [Architecture Team]
last_reviewed: 2025-12-01
next_review: 2026-12-01
related_docs: [DOC-COMP-010, DOC-ARCH-005]
related_tasks: [TASK-3C-001]
tags: [infrastructure, caching, waveform, memory, circular-buffer]
diagram_files: []
---

# DOC-COMP-024: WaveformCache

## 1. Overview

**Purpose:** Thread-safe circular buffer for 30 seconds of waveform data (~0.1 MB). Display-only cache, not persisted to database.

**Responsibilities:**
- Cache waveform samples in circular buffer
- Provide last N seconds of samples
- Filter samples by channel (ECG, Pleth, Respiration)
- Auto-overwrite oldest samples (circular buffer behavior)
- Thread-safe read/write operations

**Layer:** Infrastructure Layer (Caching)

**Module:** `z-monitor/src/infrastructure/caching/WaveformCache.h`

**Thread Affinity:** Any thread (thread-safe via QReadWriteLock)

**Capacity:**
- **Channels:** ECG, Pleth, Respiration
- **Sample rate:** 250 Hz per channel
- **Duration:** 30 seconds
- **Total samples:** 22,500 (30 sec × 250 Hz × 3 channels)
- **Memory:** ~0.1 MB

## 2. Public API

```cpp
class WaveformCache {
public:
    explicit WaveformCache(size_t capacity = 22500);

    void append(const WaveformSample &sample);
    std::vector<WaveformSample> getLastSeconds(int seconds) const;
    std::vector<WaveformSample> getChannelSamples(const QString &channel, int seconds) const;
    void clear();
    size_t size() const;

private:
    mutable QReadWriteLock m_lock;
    std::deque<WaveformSample> m_samples;
    size_t m_capacity;
};
```

## 3. Key Features

**Circular Buffer:** Overwrites oldest sample when at capacity (FIFO)

**Channel Filtering:** `getChannelSamples("ecg", 10)` returns last 10 seconds of ECG samples

**Display-Only:** Waveforms are not persisted to database (high volume, low retention value)

## 4. Usage Example

```cpp
WaveformCache cache(22500); // 30-second capacity

// Append sample
WaveformSample sample("ecg", value, timestamp);
cache.append(sample);

// Get last 10 seconds
auto samples = cache.getLastSeconds(10);

// Get channel-specific samples
auto ecgSamples = cache.getChannelSamples("ecg", 10);
auto plethSamples = cache.getChannelSamples("pleth", 5);
```

## 5. Related Documentation

- DOC-COMP-010: MonitoringService (uses WaveformCache)
- DOC-ARCH-005: Data Flow and Caching

## 6. Changelog

| Version | Date       | Author      | Changes                                    |
| ------- | ---------- | ----------- | ------------------------------------------ |
| v1.0    | 2025-12-01 | Dustin Wind | Initial documentation from WaveformCache.h |
