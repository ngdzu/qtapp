/**
 * @file WaveformRenderingBenchmark.cpp
 * @brief Performance benchmarks for waveform data processing operations.
 * 
 * Simulates realistic waveform processing workloads (data generation, decimation,
 * mathematical transformations) to measure performance without domain dependencies.
 */

#include <benchmark/benchmark.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>

// Simulate single frame processing (250 ECG samples + 60 pleth samples)
static void BM_SingleFrameProcessing(benchmark::State &state)
{
    for (auto _ : state)
    {
        std::vector<double> ecg_data(250);
        std::vector<double> pleth_data(60);
        
        // Generate realistic waveform data
        for (size_t i = 0; i < ecg_data.size(); ++i) {
            double t = i / 250.0;
            ecg_data[i] = std::sin(2 * M_PI * 1.2 * t) + 0.3 * std::sin(2 * M_PI * 60 * t);
        }
        for (size_t i = 0; i < pleth_data.size(); ++i) {
            double t = i / 60.0;
            pleth_data[i] = 0.5 + 0.4 * std::sin(2 * M_PI * 1.2 * t);
        }
        
        benchmark::DoNotOptimize(ecg_data);
        benchmark::DoNotOptimize(pleth_data);
    }
}
BENCHMARK(BM_SingleFrameProcessing)->Unit(benchmark::kMillisecond);

// Simulate sustained 60 FPS (60 frames in one benchmark iteration)
static void BM_Sustained60FPS(benchmark::State &state)
{
    for (auto _ : state)
    {
        for (int frame = 0; frame < 60; ++frame) {
            std::vector<double> data(250);
            for (size_t i = 0; i < data.size(); ++i) {
                data[i] = std::sin(2 * M_PI * (frame + i / 250.0));
            }
            benchmark::DoNotOptimize(data);
        }
    }
}
BENCHMARK(BM_Sustained60FPS)->Unit(benchmark::kMillisecond);

// Simulate ring buffer operations (add 250 samples, remove old ones)
static void BM_RingBufferOperations(benchmark::State &state)
{
    std::vector<double> buffer;
    buffer.reserve(2500); // 10 seconds of data
    
    for (auto _ : state)
    {
        // Add new samples
        for (int i = 0; i < 250; ++i) {
            buffer.push_back(std::sin(i * 0.01));
        }
        
        // Remove old samples if buffer too large
        if (buffer.size() > 2500) {
            buffer.erase(buffer.begin(), buffer.begin() + 250);
        }
        
        benchmark::DoNotOptimize(buffer);
    }
}
BENCHMARK(BM_RingBufferOperations)->Unit(benchmark::kMillisecond);

// Simulate decimation (reduce 2500 samples to 250 for zoomed view)
static void BM_DecimationLargeDataset(benchmark::State &state)
{
    // Generate 10 seconds of data at 250 Hz
    std::vector<double> full_data(2500);
    for (size_t i = 0; i < full_data.size(); ++i) {
        full_data[i] = std::sin(2 * M_PI * i / 250.0);
    }
    
    for (auto _ : state)
    {
        std::vector<double> decimated;
        decimated.reserve(250);
        
        // Decimate by taking every 10th sample
        for (size_t i = 0; i < full_data.size(); i += 10) {
            decimated.push_back(full_data[i]);
        }
        
        benchmark::DoNotOptimize(decimated);
    }
}
BENCHMARK(BM_DecimationLargeDataset)->Unit(benchmark::kMillisecond);

// Simulate multi-waveform concurrent update (3 waveforms)
static void BM_MultiWaveformUpdate(benchmark::State &state)
{
    for (auto _ : state)
    {
        std::vector<double> ecg(250), pleth(60), resp(25);
        
        // Generate ECG
        for (size_t i = 0; i < ecg.size(); ++i) {
            ecg[i] = std::sin(2 * M_PI * i / 250.0);
        }
        
        // Generate Pleth
        for (size_t i = 0; i < pleth.size(); ++i) {
            pleth[i] = std::sin(2 * M_PI * i / 60.0);
        }
        
        // Generate Respiration
        for (size_t i = 0; i < resp.size(); ++i) {
            resp[i] = std::sin(2 * M_PI * i / 25.0);
        }
        
        benchmark::DoNotOptimize(ecg);
        benchmark::DoNotOptimize(pleth);
        benchmark::DoNotOptimize(resp);
    }
}
BENCHMARK(BM_MultiWaveformUpdate)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
