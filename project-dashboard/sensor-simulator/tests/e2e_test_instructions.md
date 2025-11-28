# End-to-End Integration Test Instructions

## Overview

This document describes how to test the integration between the sensor simulator and Z-Monitor's `SharedMemorySensorDataSource`.

## Prerequisites

1. Both simulator and Z-Monitor must be built
2. Both should be runnable on the same machine (for shared memory to work)

## Test Procedure

### Step 1: Start the Simulator

```bash
cd project-dashboard/sensor-simulator
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
./sensor_simulator
```

**Expected Behavior:**
- Simulator UI launches
- Console shows: "Simulator: Shared memory initialized (size: X bytes, frames: 2048, frame size: 4096 bytes)"
- Console shows: "ControlServer: Listening on /tmp/z-monitor-sensor.sock"
- Vitals display updates at 60 Hz
- Waveform display updates at 250 Hz

### Step 2: Start Z-Monitor

```bash
cd project-dashboard/z-monitor
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
./z-monitor
```

**Expected Behavior:**
- Z-Monitor UI launches
- Console shows: "SharedMemorySensorDataSource: Started successfully - data transfer via shared memory (no socket I/O)"
- Vitals display shows data matching simulator
- Waveform display shows ECG data matching simulator

### Step 3: Verify Data Flow

1. **Check Console Logs:**
   - Simulator: "ControlServer: Client connected, fd: X"
   - Simulator: "ControlServer: Sent memfd (fd: X, size: Y) to client"
   - Z-Monitor: "SharedMemorySensorDataSource: Started successfully"

2. **Verify Vitals Match:**
   - Compare HR, SpO2, RR values between simulator and Z-Monitor
   - Values should match (within 1-2 seconds due to timing differences)

3. **Verify Waveform Data:**
   - ECG waveform in Z-Monitor should match simulator waveform
   - Sample rate should be 250 Hz

4. **Check Latency:**
   - Measure time from simulator write to Z-Monitor UI update
   - Should be < 16 ms

### Step 4: Test Multiple Readers

1. Start a second Z-Monitor instance
2. Verify both instances receive data
3. Check simulator console: "ControlServer: Client connected" should appear twice

### Step 5: Test Stall Detection

1. Stop the simulator (or kill the process)
2. Wait 250+ ms
3. Z-Monitor should detect stall and emit `sensorError` signal
4. Console should show: "SharedMemorySensorDataSource: Writer stalled (no heartbeat for 250ms)"

## Troubleshooting

### Handshake Fails

**Symptom:** Z-Monitor shows "Failed to receive file descriptor" or "Failed to map shared memory"

**Possible Causes:**
1. Socket path mismatch (check `/tmp/z-monitor-sensor.sock` exists)
2. Z-Monitor's `SharedMemoryControlChannel` may need to use `recvmsg()` for first receive (see `handshake_compatibility.md`)

**Fix:** Update Z-Monitor's `SharedMemoryControlChannel::onSocketDataAvailable()` to use `recvmsg()` instead of `recv()` for the first receive.

### No Data Received

**Symptom:** Z-Monitor connects but no vitals/waveforms appear

**Possible Causes:**
1. Ring buffer structure mismatch (magic number, version, field offsets)
2. Frame format mismatch (JSON structure)

**Fix:** Run `integration_test` to verify structure compatibility.

### High Latency

**Symptom:** Latency > 16 ms

**Possible Causes:**
1. System load
2. Timer precision issues
3. Frame processing overhead

**Fix:** Check timer intervals (vitals: 16.67ms, waveforms: 4ms), verify frame processing is efficient.

## Success Criteria

✅ Simulator creates memfd and starts control server  
✅ Z-Monitor connects and receives memfd via socket  
✅ Z-Monitor maps shared memory successfully  
✅ Vitals data flows at 60 Hz  
✅ Waveform data flows at 250 Hz  
✅ Data matches between simulator and Z-Monitor  
✅ Latency < 16 ms  
✅ Multiple readers can attach  
✅ Stall detection works (250ms threshold)

