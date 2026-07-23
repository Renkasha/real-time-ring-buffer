#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <iomanip>
#include "../../include/ring_buffer.hpp"

struct BenchmarkData {
    uint64_t value1;
    uint64_t value2;
    double floatValue;
};

void benchmarkLatency() {
    std::cout << "\n=== LATENCY BENCHMARK ===";
    std::cout << "\nMeasuring single push/pop cycle latency...\n";

    RealTimeRingBuffer<BenchmarkData> buffer(1024);
    const int iterations = 1000000;

    BenchmarkData data{12345, 67890, 3.14159};
    BenchmarkData output;

    // Warmup
    for (int i = 0; i < 1000; ++i) {
        buffer.push(data);
        buffer.pop(output);
    }

    // Measure push latency
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        buffer.push(data);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto pushTime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    double pushLatency = static_cast<double>(pushTime.count()) / iterations;

    // Clear buffer
    while (buffer.pop(output)) {}

    // Measure pop latency
    for (int i = 0; i < iterations; ++i) {
        buffer.push(data);
    }

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        buffer.pop(output);
    }
    end = std::chrono::high_resolution_clock::now();
    auto popTime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    double popLatency = static_cast<double>(popTime.count()) / iterations;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Average push latency:  " << pushLatency << " ns\n";
    std::cout << "Average pop latency:   " << popLatency << " ns\n";
    std::cout << "Average cycle latency: " << (pushLatency + popLatency) << " ns\n";
}

void benchmarkThroughput() {
    std::cout << "\n=== THROUGHPUT BENCHMARK ===";
    std::cout << "\nMeasuring concurrent producer/consumer throughput...\n";

    RealTimeRingBuffer<uint64_t> buffer(4096);
    std::atomic<uint64_t> totalPushed(0);
    std::atomic<uint64_t> totalPopped(0);
    std::atomic<bool> running(true);

    const int numProducers = 2;
    const int numConsumers = 2;
    const auto benchmarkDuration = std::chrono::seconds(5);

    std::vector<std::thread> threads;

    // Producer threads
    for (int p = 0; p < numProducers; ++p) {
        threads.emplace_back([&]() {
            uint64_t count = 0;
            while (running) {
                if (buffer.push(count)) {
                    count++;
                    totalPushed++;
                }
            }
        });
    }

    // Consumer threads
    for (int c = 0; c < numConsumers; ++c) {
        threads.emplace_back([&]() {
            uint64_t value;
            while (running) {
                if (buffer.pop(value)) {
                    totalPopped++;
                }
            }
        });
    }

    // Run benchmark
    std::this_thread::sleep_for(benchmarkDuration);
    running = false;

    // Wait for threads
    for (auto& t : threads) {
        t.join();
    }

    std::cout << std::fixed << std::setprecision(0);
    std::cout << "Total pushed:  " << totalPushed.load() << " elements\n";
    std::cout << "Total popped:  " << totalPopped.load() << " elements\n";
    std::cout << "Push rate:     " << (totalPushed.load() / 5.0 / 1e6) << " M ops/sec\n";
    std::cout << "Pop rate:      " << (totalPopped.load() / 5.0 / 1e6) << " M ops/sec\n";
}

int main() {
    std::cout << "\n╔════════════════════════════════════════╗";
    std::cout << "\n║   Ring Buffer Performance Benchmark    ║";
    std::cout << "\n╚════════════════════════════════════════╝\n";

    benchmarkLatency();
    benchmarkThroughput();

    std::cout << "\n✅ Benchmark complete!\n\n";
    return 0;
}
