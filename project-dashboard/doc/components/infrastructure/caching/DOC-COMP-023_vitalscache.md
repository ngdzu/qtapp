---
doc_id: DOC-COMP-023
title: VitalsCache
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
tags: [infrastructure, caching, vitals, memory, thread-safe]
diagram_files: []
---

# DOC-COMP-023: VitalsCache

## 1. Overview

**Purpose:** Thread-safe in-memory cache for vital signs with 3-day capacity (~39 MB). Provides fast access for UI display and batch persistence to database.

**Responsibilities:**
- Cache vital records in memory (std::deque for efficient front/back operations)
- Provide range queries by timestamp
- Track persistence status (getUnpersistedVitals, markAsPersisted)
- Auto-evict oldest 10% when at capacity
- Thread-safe read/write operations

**Layer:** Infrastructure Layer (Caching)

**Module:** `z-monitor/src/infrastructure/caching/VitalsCache.h`

**Thread Affinity:** Any thread (thread-safe via QReadWriteLock)

**Capacity:**
- **Max records:** 259,200 (3 days × 24 hours × 60 minutes × 60 Hz)
- **Memory:** ~39 MB (150 bytes per VitalRecord × 259,200)

## 2. Public API

```cpp
class VitalsCache {
public:
    explicit VitalsCache(size_t maxCapacity = 259200);

    void append(const VitalRecord &vital);
    std::vector<VitalRecord> getRange(int64_t startMs, int64_t endMs) const;
    std::vector<VitalRecord> getUnpersistedVitals() const;
    void markAsPersisted(int64_t upToTimestampMs);
    size_t size() const;
    void clear();
    VitalRecord getLatest(const std::string &vitalType, bool &found) const;

private:
    mutable QReadWriteLock m_lock;
    std::deque<VitalRecord> m_vitals;
    int64_t m_lastPersistedTimestampMs;
    size_t m_maxCapacity;
};
```

## 3. Key Features

**Auto-Eviction:** When at capacity, removes oldest 10% before appending new vital

**Persistence Tracking:**
- `getUnpersistedVitals()`: Returns vitals not yet written to database
- `markAsPersisted()`: Updates last persisted timestamp

**Thread Safety:** QReadWriteLock for concurrent reads, exclusive writes

## 4. Usage Example

```cpp
VitalsCache cache(259200); // 3-day capacity

// Append vital
VitalRecord vital("HR", 75, timestamp);
cache.append(vital);

// Get range
auto vitals = cache.getRange(startMs, endMs);

// Persistence
auto unpersisted = cache.getUnpersistedVitals();
saveToDatabase(unpersisted);
cache.markAsPersisted(lastTimestamp);

// Latest value
bool found;
VitalRecord latest = cache.getLatest("HR", found);
```

## 5. Related Documentation

- DOC-COMP-010: MonitoringService (uses VitalsCache)
- DOC-ARCH-005: Data Flow and Caching

## 6. Changelog

| Version | Date       | Author      | Changes                                  |
| ------- | ---------- | ----------- | ---------------------------------------- |
| v1.0    | 2025-12-01 | Dustin Wind | Initial documentation from VitalsCache.h |
