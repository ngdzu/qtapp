# Sensor Simulator Tests

## Integration Test

The `integration_test` executable verifies structure compatibility between the simulator's shared memory structures and Z-Monitor's expected format.

### Running the Test

```bash
cd project-dashboard/sensor-simulator
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
./integration_test
```

### What It Tests

- Structure size compatibility (RingBufferHeader, SensorFrame)
- Field offset compatibility
- Magic number match (0x534D5242 "SMRB")
- Version match (1)
- Frame type enum values match

### Future Tests

- End-to-end test with Z-Monitor's SharedMemorySensorDataSource
- Performance test (verify < 16ms latency)
- Multiple reader test (multiple Z-Monitor instances)
- Stall detection test

