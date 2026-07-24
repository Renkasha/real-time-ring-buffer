# Real-Time Ring Buffer
## Support This Project

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/Renkasha)
A production-ready, **lock-free, thread-safe circular buffer** for high-performance real-time data streaming in C++. Designed for sub-millisecond latency applications like robotics, video streaming, audio processing, and financial systems.

## Features

✅ **Lock-Free Design** — Uses `std::atomic` with memory ordering for thread-safe concurrent access without mutexes  
✅ **Zero-Copy on Boot** — Fixed-size pre-allocated memory prevents dynamic allocation overhead  
✅ **Sub-Millisecond Latency** — Optimized for microsecond-level push/pop operations  
✅ **Thread-Safe** — No data races or corruption under concurrent producer/consumer workloads  
✅ **Production-Ready** — Tested, benchmarked, and documented  

## Quick Start

### Build

```bash
mkdir build && cd build
cmake ..
make
```

### Run Example

```bash
./bin/ring_buffer_example
```

### Run Tests

```bash
make test
```

### Run Benchmark

```bash
./bin/ring_buffer_benchmark
```

## Architecture

### Core Components

- **RealTimeRingBuffer**: Lock-free circular buffer implementation
- **TelemetryPacket**: Example data structure for sensor payloads
- **Producer/Consumer Threads**: Concurrent read/write operations

### Memory Ordering

The implementation uses careful memory ordering to ensure correctness:

- `std::memory_order_relaxed`: For non-critical index loads
- `std::memory_order_acquire`: For synchronization checks
- `std::memory_order_release`: For publishing updates to other threads

## Real-World Use Cases

- 🤖 **Robotics**: Logging high-frequency sensor data (gyros, accelerometers, encoders)
- 📹 **Video Streaming**: Buffering frames between capture and processing threads
- 🔊 **Audio Processing**: DSP pipelines and audio engine buffering
- 💰 **Financial Trading**: Ultra-low-latency order logging systems

## Performance Characteristics

- **Latency**: < 1 microsecond per push/pop (varies by CPU)
- **Throughput**: Millions of packets per second
- **Memory**: Fixed overhead (no dynamic allocation during operation)
- **Scalability**: Efficient under high contention

## Documentation

- [API Reference](docs/API.md)
- [Performance Guide](docs/PERFORMANCE.md)
- [Memory Ordering Deep Dive](docs/MEMORY_ORDERING.md)

## Compilation

### Manual Compilation (g++)

```bash
g++ -std=c++11 src/examples/ring_buffer_example.cpp -pthread -O3 -o ring_buffer
./ring_buffer
```

### With CMake (Recommended)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## License

MIT License - See LICENSE file

## Author

Created for high-performance computing systems and real-time applications.
