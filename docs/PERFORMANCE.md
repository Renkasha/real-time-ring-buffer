# Performance Guide

## Latency Characteristics

### Typical Push/Pop Latency

On modern x86/x64 processors:

- **Push operation:** 10-50 nanoseconds (depending on CPU, cache state)
- **Pop operation:** 10-50 nanoseconds
- **Total round-trip:** 20-100 nanoseconds

### Factors Affecting Latency

1. **CPU Cache State**
   - First access: ~40ns (L2/L3 cache hit) to 200ns (memory)
   - Subsequent accesses: ~10ns (L1 cache hit)

2. **Contention**
   - Low contention: Minimal overhead
   - High contention: Memory barriers cause pipeline stalls

3. **Buffer Capacity**
   - No direct impact (O(1) operations)
   - Modulo operation is negligible on modern CPUs

4. **Data Size**
   - Small (8-16 bytes): Native performance
   - Medium (64-256 bytes): Trivial overhead
   - Large (>1KB): Use pointers in buffer instead

## Throughput Characteristics

### Realistic Numbers

With 2 producer threads and 2 consumer threads:

- **Single-threaded pairs:** 10-50 Million operations/second
- **Contended (multiple threads):** 1-5 Million operations/second
- **Highly optimized paths:** 100+ Million operations/second

Variation depends on:
- CPU frequency and core count
- Operating system scheduling
- Memory bandwidth
- Cache locality

## Optimization Tips

### 1. Choose Appropriate Capacity

```cpp
// Too small - frequent overflows
RealTimeRingBuffer<Packet> buffer(16);

// Just right - 1ms worth of data at 1MHz rate
RealTimeRingBuffer<Packet> buffer(1000);

// Wasteful - excessive memory
RealTimeRingBuffer<Packet> buffer(1000000);
```

**Recommendation:** Size buffer to hold 1-10ms of expected data.

### 2. Avoid Large Data Types

```cpp
// ❌ Inefficient - large struct copied each operation
struct LargePacket {
    uint8_t data[4096];
};
RealTimeRingBuffer<LargePacket> buffer(256);

// ✅ Efficient - pointer to data
struct Packet {
    uint32_t id;
    uint32_t size;
    uint8_t* data;  // Allocate separately
};
RealTimeRingBuffer<Packet> buffer(256);
```

### 3. Handle Overflow Gracefully

```cpp
// ❌ Spinning wastes CPU
while (!buffer.push(data)) {
    std::this_thread::sleep_for(std::chrono::nanoseconds(1));
}

// ✅ Better - yield or drop old data
if (!buffer.push(data)) {
    // Option 1: Yield and retry
    std::this_thread::yield();
    if (!buffer.push(data)) {
        // Option 2: Drop oldest data and retry
        Packet dummy;
        if (buffer.pop(dummy)) {
            buffer.push(data);
        }
    }
}
```

### 4. Optimize Thread Affinity

```cpp
// Bind threads to specific cores for better cache locality
#include <pthread.h>

void setThreadAffinity(std::thread& t, int cpuCore) {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(cpuCore, &set);
    pthread_setaffinity_np(t.native_handle(), sizeof(set), &set);
}

// Usage:
std::thread producer([&]() { /* ... */ });
setThreadAffinity(producer, 0);  // CPU core 0

std::thread consumer([&]() { /* ... */ });
setThreadAffinity(consumer, 1);  // CPU core 1
```

### 5. Use Batch Processing

```cpp
// ❌ Inefficient - one at a time
int value;
while (buffer.pop(value)) {
    processItem(value);
}

// ✅ Efficient - batch processing
std::vector<int> batch;
batch.reserve(100);
int value;
while (buffer.pop(value)) {
    batch.push_back(value);
    if (batch.size() >= 100) {
        processBatch(batch);
        batch.clear();
    }
}
if (!batch.empty()) {
    processBatch(batch);
}
```

## Benchmarking Your Application

### Latency Measurement

```cpp
#include <chrono>
#include <numeric>

std::vector<double> latencies;
for (int i = 0; i < 10000; ++i) {
    auto start = std::chrono::high_resolution_clock::now();
    buffer.push(data);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    latencies.push_back(nanos);
}

// Calculate statistics
std::sort(latencies.begin(), latencies.end());
double p50 = latencies[latencies.size() * 0.50];
double p99 = latencies[latencies.size() * 0.99];

std::cout << "P50: " << p50 << "ns, P99: " << p99 << "ns\n";
```

### Throughput Measurement

```cpp
std::atomic<uint64_t> count(0);
auto start = std::chrono::high_resolution_clock::now();

// Run operations for fixed time
for (int i = 0; i < 1000000; ++i) {
    if (buffer.push(data)) {
        count++;
    }
}

auto end = std::chrono::high_resolution_clock::now();
auto seconds = std::chrono::duration<double>(end - start).count();
double opsPerSec = count.load() / seconds / 1e6;

std::cout << "Throughput: " << opsPerSec << " M ops/sec\n";
```

## Memory Efficiency

### Memory Usage Formula

```
Total Memory = (capacity × sizeof(T)) + 16 bytes (two atomics)
```

### Example

```cpp
struct Packet {
    uint64_t timestamp;    // 8 bytes
    uint32_t id;          // 4 bytes
    float value;          // 4 bytes
};  // Total: 16 bytes

RealTimeRingBuffer<Packet> buffer(1024);
// Memory usage: (1024 × 16) + 16 = 16,400 bytes ≈ 16 KB
```

## Scaling with Core Count

### Throughput vs. Threads

```
Throughput (M ops/sec)
    ↑
 100 │     ╭────────
     │    ╱
  50 │   ╱
     │  ╱
   1 └──────────────→ Number of Thread Pairs
     0   2    4    8
```

**Key insight:** Throughput plateaus due to contention and memory bandwidth limits. Adding more threads beyond a certain point doesn't help.

## CPU Cache Considerations

### L1/L2/L3 Cache Typical Sizes

- L1: 32-64 KB per core
- L2: 256-512 KB per core
- L3: 4-16 MB shared

### Buffer Placement

```cpp
// Aligned allocation for better cache performance
alignas(64) RealTimeRingBuffer<Packet> buffer(1024);
```

**Why 64?** Most CPU cache lines are 64 bytes. Aligning reduces false sharing.

## Production Deployment Checklist

- [ ] Measure latency P99 with expected workload
- [ ] Test with maximum expected contention
- [ ] Verify no overflow conditions under peak load
- [ ] Monitor CPU utilization (should be high but not 100%)
- [ ] Test on target hardware (latency varies by CPU)
- [ ] Enable compiler optimizations (-O3 or -Ofast)
- [ ] Use production-grade monitoring/alerting
