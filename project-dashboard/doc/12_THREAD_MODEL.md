"""markdown
# Thread Model and Low-Latency Architecture

This document specifies the threading model, communication channels, latency targets, and implementation constraints for the Z Monitor. It explains how components should be partitioned across threads, which synchronization primitives to use, and measurable performance requirements to achieve a low-latency, safe, and predictable system.

Purpose
- Provide an unambiguous thread-affinity architecture for core subsystems (data acquisition, signal processing, alarms, DB persistence, networking, UI).
- Define low-latency requirements and measurable targets.
- Explain implementation patterns (QThread, lock-free queues, atomic synchronization, event loops) and platform considerations.

Audience
- Backend and firmware engineers implementing `DatabaseManager`, `SignalProcessor`, `AlarmManager`, `NetworkManager`.
- QA/performance engineers who will validate latency, jitter and throughput.

Overview (logical components)
- Sensor / Device I/O: reads raw telemetry (waveforms, samples) from serial, USB, BLE or IPC.
- Signal Processor: filters, derivatives, beat detection, simple analytics, predictive-score preprocessor.
- Alarm Manager: evaluates alarm rules, escalation, debouncing and acknowledgement handling.
- DB Writer (DatabaseManager): batched persistence of events, vitals and snapshots.
- Network Sync (NetworkManager): secure mTLS synchronization, retries and conflict handling.
- UI (Qt GUI main thread): rendering, user interaction, patient context and alarm overlay.
- Archiver / Background: on-demand or scheduled archival and restore tasks.
- Logger / Audit: persistent audit logging and secure local logs.

Are boxes threads?
 - Short answer: No — boxes in the diagram are logical services/components, not necessarily 1:1 OS threads.
 - Rationale: the diagram shows component boundaries and data flows. Each box represents a service with a well-defined responsibility and API. Whether a service runs in its own thread, shares a thread with other services, or is implemented as a thread-pool worker depends on platform resources, expected load (sampling rates, number of patients), and real-time guarantees.

When to give a service its own thread
 - High-frequency I/O or processing: services that handle continuous streams (waveforms at hundreds of Hz) should have a dedicated thread or a bounded set of threads to avoid contention and jitter.
 - Blocking I/O or long syscalls: network sync, file export/archival, or blocking device reads should not execute on the UI thread and are good candidates for separate threads.
 - Serialized single-writer resources: SQLite writes are easiest when funneled through a single DB-writer thread to avoid write-lock contention.

When to colocate services on the same thread
 - Low-throughput or infrequent tasks: logger flushers, occasional archiver callbacks, or a low-rate network health check can be placed in the same background thread.
 - Tight memory/CPU budgets: on very constrained hardware, you may combine Alarm Manager and Signal Processor into a single real-time thread if budgeted carefully (but measure latency).

Recommended thread-service mappings (examples)
 - Minimal / very constrained (1 CPU core, embedded)
   - Main UI Thread (Qt event loop)
   - RT Processing Thread: Sensor I/O + Signal Processor + Alarm Manager (single real-time thread)
   - DB/Network/Logger Background Thread (batching duties)
   - Watchdog (lightweight timer on UI or background)
   - Notes: combine services but carefully pre-allocate buffers and test for P99 latency under load.

 - Typical embedded/mobile (2 cores)
   - Main UI Thread
   - Sensor I/O + Signal Processor (dedicated real-time thread)
   - DB Writer (single) + Logger (same background thread)
   - Network Sync (separate thread)
   - Watchdog / Archiver (low-priority background)

 - Multi-core / desktop (4+ cores)
   - Main UI Thread
   - One or more Signal Processor threads (scale with sensors/patients)
   - Sensor I/O threads (one per fast device or grouped by bus)
   - Alarm Manager (separate thread)
   - DB Writer (single)
   - Network Sync (separate)
   - Archiver and Logger (separate or pooled background threads)

Sizing guidance & decision heuristics
 - Estimate per-sample processing time: measure the time to process one sample (microseconds). Multiply by expected sample rate to compute CPU load.
 - Budgeting: reserve 10-20% CPU headroom for OS, GUI, and unexpected spikes.
 - If sample processing consumes > 40-50% of a CPU core, give it a dedicated core/thread.
 - For each thread, estimate worst-case latency and backlog: if queues can grow beyond safe memory limits, split the service across threads or add worker pool.

Thread count sanity check
 - Ten logical boxes does not mandate ten OS threads. For a small device you typically run 3–5 threads (UI, RT processing, DB writer, network, background). For larger systems you may map each service to its own thread if CPU/cores and memory allow.

Example mapping table (summary)
 - UI: Main thread (must be main) — priority default
 - Sensor I/O: dedicated or grouped thread(s) — high priority
 - Signal Processor: dedicated thread(s) — real-time/high priority
 - Alarm Manager: colocate with Signal Processor (if light) or separate — high priority
 - DB Writer: single background thread — I/O priority
 - Network Sync: background thread — normal priority
 - Logger: background thread (or co-located with DB writer) — low priority
 - Archiver: background thread — low priority
 - Watchdog: lightweight timer on UI or background — low priority

Implementation checklist (practical steps)
 - Start with a conservative mapping: UI, RT processing (I/O+processing), DB writer, network/logger background.
 - Measure CPU, latency, and queue depth under representative loads.
 - If processing or I/O saturates a core, split services into separate threads or add worker threads.
 - Use per-thread priorities and CPU affinity on multi-core devices to isolate real-time threads.
 - Add watchdogs and queue-depth alarms early to detect backpressure.

Decision flow (quick):
 1. Is the service handling high-frequency data (>50 Hz) or real-time constraints? If yes → prefer dedicated thread.
 2. Does it perform blocking I/O or long-running CPU work? If yes → separate thread.
 3. If neither, consider co-locating with a low-priority background thread.

Decision Rationale (why some services are co-located)
-----------------------------------------------
This section explains specific placement choices shown in the diagram (for example, why `Network` and `Archiver` were co-located in earlier diagrams), and provides clear trade-offs for other threads.

- Network + Archiver (co-located rationale):
  - Work type: Both are largely background I/O tasks that perform non-real-time work (uploads, downloads, file moves, compression, and scheduled exports).
  - Latency sensitivity: Neither operation is on the real-time alarm path. They tolerate batching, backoff, and retries without affecting alarm handling or UI responsiveness.
  - Resource sharing: They often share TLS configuration, HTTP client state, retry/backoff logic and may access the same DB/archive files. Co-locating avoids frequent cross-thread handoffs for small jobs.
  - Constrained systems: On devices with very limited cores, combining them reduces thread count and synchronization complexity which simplifies scheduling and lowers context-switch overhead.
  - Trade-offs: Co-location is only advisable when archive tasks are lightweight (small files or infrequent), and when network activity won't block long-running CPU work (e.g., heavy compression). If archiving compresses/encrypts large blobs or network sends are very frequent/large, they should be separated.

- Database Writer (separate by default):
  - Rationale: SQLite (even with WAL) benefits from a single writer to avoid contention and simplify transactional semantics. Keeping DB writes on a dedicated thread prevents unpredictable pauses in real-time processing caused by synchronous disk flushes.
  - Trade-offs: If DB writes are extremely light and device is single-threaded, DB writer can be co-located with a background thread, but be cautious about commit latency for critical alarms.

- Signal Processor & Sensor I/O (dedicated / co-located depending on load):
  - Rationale: These are on the hot path. They must minimize jitter and allocations. In many cases Sensor I/O (device reads) feeds a dedicated Signal Processor thread via an SPSC ring buffer for deterministic handoff.
  - Trade-offs: On very constrained hardware you may colocate Sensor I/O + Signal Processor in the same real-time thread to reduce inter-thread signaling latency; however, this increases the risk that long-running processing will delay new reads — so measure carefully.

- Alarm Manager (co-locate or separate):
  - Rationale: If alarm rule evaluation is lightweight, it can live alongside the Signal Processor (to keep evaluation deterministic and avoid queueing latency). If rules reference heavier analytics, keep Alarm Manager separate and have it consume pre-computed scores from the analytics pool.
  - Trade-offs: Co-location reduces messaging overhead but may increase worst-case latency during bursts of processing.

- Logger / Audit (background, often co-located with DB writer):
  - Rationale: Audit logs are append-only and low priority. Co-locating with the DB writer enables transactional writes consistent with events and reduces concurrency complexity.
  - Trade-offs: For extremely high-volume trace logs, a separate logger or a batching compression worker is preferable.

- Watchdog and Observability (lightweight, can be co-located):
  - Rationale: Watchdog can run as a lightweight timer on the UI or a background thread; it only needs periodic heartbeats and should not be high priority.

Guidelines to decide whether to split a co-located pair
- If either service starts consuming > 25–40% of the CPU budget on that thread, split it out.
- If either service performs heavy CPU work (compression, encryption, large serialization), offload that work to a worker pool or separate thread.
- If failure of one service should not impact the other (isolation requirement), put them on separate threads/processes.
- If you need different thread priorities or QoS classes (e.g., Network should be low-priority, but Archiver must immediately persist data), separate them.

Applying this to the diagram
- The diagram groups services by recommended QoS and typical placement: UI and RT processing are high-priority, DB writer is dedicated, Network/Archiver/Logger are low-priority background services that may be co-located on constrained devices.
- If you plan to deploy to multi-core systems, prefer splitting Network and Archiver so you can tune each independently. On small embedded targets, start with co-location and monitor metrics (queue depth, CPU utilization, latency) to determine if splitting is necessary.



Thread Topology (recommended)

ASCII Diagram

Main/UI Thread (Qt)
  │
  ├─(queued signals / polling)→ UI updates, user actions
  │
Sensor I/O Thread(s) [one per fast I/O source]
  ├─(lock-free ring buffer)→ Signal Processor Thread(s)

Signal Processor Thread(s) [real-time priority]
  ├─(lock-free queue)→ Alarm Manager Thread
  ├─(lock-free batch queue)→ DB Writer Thread
  └─(optional)→ Predictive Analytics Worker Pool (lower priority)

Alarm Manager Thread
  ├─(queued signals)→ Main/UI Thread (Qt::QueuedConnection)
  ├─(enqueue)→ notifications table via DB Writer Thread

DB Writer Thread (single writer)
  ├─(batched writes)→ SQLite (SQLCipher) I/O

Network Sync Thread
  ├─(TLS handshake & upload)→ remote server

Archiver Thread (background)

Logger Thread (append-only)

Principles and rationale
- Clear ownership: each subsystem owns data structures it mutates. Pass immutable copies or pre-allocated buffers to consumers.
- Minimize shared mutable state across threads. Prefer message passing (queues) over locks in the hot path.
- Use one dedicated DB writer thread to avoid SQLite concurrency pitfalls and to batch writes for IOPS efficiency.
- Keep the UI (Qt main thread) free of blocking operations — use queued signals to update UI state.
- Prefer single-producer single-consumer (SPSC) lock-free ring buffers for waveform/sample streaming.
- Use worker pools for heavy CPU work (analytics) with tunable concurrency to avoid starving real-time processing threads.

Thread responsibilities and constraints

Main / UI Thread
- Responsibilities: render UI, handle user gestures, show alarms/notifications, manage QObject lifetimes for UI controllers.
- Constraints: do not perform blocking I/O or heavy processing. Keep frame render budget under 16ms (for 60Hz) or 33ms (for 30Hz). Use `Qt::QueuedConnection` for cross-thread signals to avoid re-entrancy.

Sensor I/O Thread(s)
- Responsibilities: read device packets, deserialize frames, push raw samples into ring buffers.
- Constraints: must have minimal jitter. Avoid heap allocations in the read loop; reuse buffers. If I/O blocking is expected, use OS-level blocking reads but isolate in thread.
- Implementation notes: use non-blocking/interrupt-driven I/O where available; set thread priority higher than default.

Signal Processor Thread(s)
- Responsibilities: apply digital filters, compute heart rate, detect beats, compute derived metrics, short-term aggregates, and produce alarm-candidates.
- Constraints: soft real-time behavior. Use SPSC queues to accept raw samples. Avoid locks during inner loops. Use pre-allocated FIR/IIR buffers. If SIMD is available, utilize it.
- Real-time priority: on Linux embedded devices prefer SCHED_FIFO with careful CPU budgeting. On macOS, real-time scheduling is limited — prefer QoS or thread priority APIs. Always test for starvation.

Alarm Manager Thread
- Responsibilities: evaluate alarm rules, deduplicate and debounce alarms, produce UI events and persistent alarm records.
- Constraints: maintain deterministic rule evaluation order. Avoid expensive computations here; offload heavy scoring to analytics pool.

DB Writer Thread
- Responsibilities: collect batched records (vitals, alarms, snapshots, infusion_events, notifications) and perform transactional writes to the encrypted SQLite DB.
- Constraints: single writer affinity to avoid SQLite write locks. Use prepared statements and transactions; batch commits (e.g., every 100 records or 200ms) to minimize I/O overhead.
- Durability: ensure audit_log and critical alarm events flushed with higher priority (option to force commit for critical events).

Network Sync Thread
- Responsibilities: manage TLS connections, perform uploads of batched telemetry, download server ack lists, handle retries and exponential backoff.
- Constraints: network operations may block; keep them off real-time threads. Perform TLS handshake once and reuse connections if possible. Use `TCP_NODELAY` for low-latency small packets.

Archiver / Background Thread(s)
- Responsibilities: move old data to archive, run DB compaction, perform long-running export tasks.
- Constraints: low priority background tasks. Throttle resource usage to avoid impacting real-time threads.

Logger Thread
- Responsibilities: append-only logging for audit and trace events. Flush critical entries periodically or on demand.
- Constraints: use pre-allocated buffers for log messages in hot path and a background flusher to disk.

Communication patterns and primitives
- Lock-free SPSC ring buffers: waveform samples and high-frequency telemetry (avoid mutex in hot path).
- MPSC queues or lock-free queues: multiple producers (processing workers) can push to DB writer or logger.
- Atomics and sequence counters: use atomic indices and sequence counters to detect overwrites in ring buffers.
- Queued signals (Qt::QueuedConnection): for safe cross-thread UI updates between worker threads and main thread.
- Direct connections (Qt::DirectConnection) should only be used when the caller guarantees same-thread context.
- Mutexes only for low-frequency control paths, not for per-sample processing.

Memory & allocation rules
- Pre-allocate all real-time buffers at startup.
- Avoid heap allocations in Sensor I/O and Signal Processor threads; use object pools / slab allocators when required.
- Use small fixed-size message structs for queue passing and avoid JSON serialization on hot paths. Serialize to compact binary or pre-allocated buffers.

Latency and jitter targets (suggested, tuneable)
- Sensor read -> sample enqueued: < 1 ms (depends on hardware)
- Sample enqueued -> processed (derived metrics available): < 5 ms
- Alarm detection to UI visible: < 50 ms (most cases); critical alarms should be prioritized
- DB write latency (per-batch): < 200 ms for background batched writes; immediate flush for critical alarm writes should complete < 100 ms
- Network upload latency: depends on network; keep local ack semantics so UI does not wait on network.

Real-time safety and watchdogs
- Add a watchdog thread that checks heartbeat timestamps from each critical thread (Sensor I/O, Signal Processor, Alarm Manager, DB Writer). If heartbeat missing for configurable threshold, escalate (log, show UI error, attempt restart of the thread or subsystem).
- Monitor queue depths for ring buffers and alert when thresholds are crossed to avoid unbounded backlog.

Performance measurement and observability
- Instrument with timestamped events for:
  - sample read
  - processing start/end
  - alarm detected
  - DB enqueue and DB commit
  - network enqueue and send
- Collect histograms for latency and jitter; fail CI if P99 exceeds thresholds.
- Use platform tracing tools: Instruments (macOS), perf or LTTng (Linux), or custom binary trace logs.

Platform-specific notes
- Linux (embedded): use pthread APIs to set `SCHED_FIFO` for processing threads when necessary. Set CPU affinity on high-frequency threads to dedicated cores where possible.
- macOS: real-time scheduling is constrained; use thread QoS classes (e.g., QOS_CLASS_USER_INTERACTIVE) and test for priority inversion. Avoid SCHED_FIFO unless platform allows and entitlements are set.
- Windows: use `SetThreadPriority()` and Multimedia Class Scheduler Service (MMCSS) for audio-like real-time threads.

Concurrency pitfalls and mitigations
- Priority inversion: avoid long-held locks in low-priority threads. Use lock-free primitives in hot paths.
- Starvation: don't set all background threads to real-time priority. Reserve CPUs for processing threads.
- Deadlocks: prohibit cross-thread blocking waits that can lead to deadlock; prefer asynchronous callbacks.

Code-level guidance
- QThread usage: create a QObject worker, moveToThread(thread), start the thread's event loop, and communicate with signals/slots (queued) for control messages.
- Ring buffer libraries: consider `folly::ProducerConsumerQueue`, `boost::lockfree::spsc_queue`, or small custom SPSC ring buffer tuned for cache-line size.
- For SQLite writes: use WAL mode (`PRAGMA journal_mode=WAL`) and tuned `PRAGMA synchronous = NORMAL` (or FULL for strict durability) depending on regulatory constraints.

Acceptance criteria and tests
- Unit tests for thread-safety (stress tests with high data rates).
- Integration tests that feed waveform at max expected sampling rates for extended periods while measuring CPU, memory, latency and queue backpressure.
- Performance gate: P99 sample->alarm latency < 100ms under nominal load; user to approve final thresholds.

Checklist for implementers
- [ ] Define exact sampling budgets and expected max sample rates per device.
- [ ] Implement SPSC ring buffer for waveform streaming.
- [ ] Implement single DB writer with batched transactions.
- [ ] Add watchdog monitoring and queue-depth alerts.
- [ ] Add tracing hooks for latency measurement.
- [ ] Document thread affinity and set thread priorities in startup config.

This document is intended to be a precise reference for developers and QA to implement and validate the low-latency, multi-threaded architecture of the Z Monitor.

If you want, I can also:
- produce a small reference implementation (C++/Qt) with a `SensorSimulator` thread, `SignalProcessor` thread, and `DBWriter` thread connected by an SPSC ring buffer, plus a simple latency measurement harness; OR
- generate a PNG sequence or Mermaid diagram for integration into the docs.

---
Generated: 2025-11-23

## Diagram

A Mermaid version of the thread model diagram is included in the repository: `doc/12_THREAD_MODEL.mmd`.

Inline Mermaid (renderers that support Mermaid will show the diagram):

```mermaid
flowchart LR
  subgraph UIThread[Main / UI Thread (Qt)]
    UI[UI Renderer & Controllers]
  end

  subgraph SensorIO[Sensor I/O Threads]
    S1[Sensor I/O (fast)]
    S2[Sensor I/O (slow/aux)]
  end

  subgraph SignalProc[Signal Processor(s) (RT)]
    SP1[Signal Processor]
    SPpool[Predictive Analytics Pool]
  end

  subgraph AlarmMgr[Alarm Manager]
    AM[Alarm Manager]
  end

  subgraph DBWriter[DB Writer]
    DB[Database Writer (single)]
  end

  subgraph Network[Network Sync]
    NET[Network Manager (mTLS)]
  end

  subgraph Archiver[Archiver / Background]
    ARC[Archiver]
  end

  subgraph Logger[Logger]
    LOG[Audit Logger]
  end

  S1 -->|SPSC ring buffer| SP1
  S2 -->|SPSC ring buffer| SP1
  SP1 -->|lock-free queue (alarms)| AM
  SP1 -->|batch queue (vitals)| DB
  SP1 -->|work items| SPpool
  AM -->|queued signals (Qt::QueuedConnection)| UI
  AM -->|enqueue| DB
  DB -->|WAL writes / transactions| LOG
  NET -->|ack lists| DB
  ARC -->|archive jobs| DB
  UI -->|user actions| AM
  UI -->|config / control| NET

  SP1 ---|heartbeat| WD[Watchdog]
  DB ---|heartbeat| WD
  AM ---|heartbeat| WD
  NET ---|heartbeat| WD
```

