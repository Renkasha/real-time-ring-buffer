#ifndef REAL_TIME_RING_BUFFER_HPP
#define REAL_TIME_RING_BUFFER_HPP

#include <atomic>
#include <vector>
#include <cstdint>
#include <stdexcept>

/**
 * @brief Real-time, lock-free circular buffer for high-performance data streaming.
 * 
 * Thread-safe ring buffer using atomic operations and careful memory ordering.
 * Designed for microsecond-level latency applications.
 * 
 * @tparam T Data type to store in the buffer
 */
template<typename T>
class RealTimeRingBuffer {
private:
    std::vector<T> bufferMemory;
    size_t bufferCapacity;
    std::atomic<size_t> writeHead;
    std::atomic<size_t> readHead;

public:
    /**
     * @brief Initialize the ring buffer with fixed capacity.
     * @param capacity Number of elements to pre-allocate (default: 1024)
     */
    explicit RealTimeRingBuffer(size_t capacity = 1024)
        : bufferCapacity(capacity), writeHead(0), readHead(0) {
        if (capacity == 0) {
            throw std::invalid_argument("Buffer capacity must be > 0");
        }
        bufferMemory.resize(bufferCapacity);
    }

    RealTimeRingBuffer(const RealTimeRingBuffer&) = delete;
    RealTimeRingBuffer& operator=(const RealTimeRingBuffer&) = delete;

    /**
     * @brief Push a data element into the buffer.
     * @param data Element to write
     * @return true if successful, false if buffer is full
     */
    bool push(const T& data) {
        size_t currentWrite = writeHead.load(std::memory_order_relaxed);
        size_t nextWrite = (currentWrite + 1) % bufferCapacity;

        // Check for overflow (buffer full)
        if (nextWrite == readHead.load(std::memory_order_acquire)) {
            return false;
        }

        bufferMemory[currentWrite] = data;
        writeHead.store(nextWrite, std::memory_order_release);
        return true;
    }

    /**
     * @brief Pop a data element from the buffer.
     * @param output Reference to store the popped element
     * @return true if successful, false if buffer is empty
     */
    bool pop(T& output) {
        size_t currentRead = readHead.load(std::memory_order_relaxed);

        // Check for underflow (buffer empty)
        if (currentRead == writeHead.load(std::memory_order_acquire)) {
            return false;
        }

        output = bufferMemory[currentRead];
        readHead.store((currentRead + 1) % bufferCapacity, std::memory_order_release);
        return true;
    }

    /**
     * @brief Get the current number of elements in the buffer.
     * @return Approximate size (may vary due to concurrency)
     */
    size_t size() const {
        size_t write = writeHead.load(std::memory_order_acquire);
        size_t read = readHead.load(std::memory_order_acquire);
        if (write >= read) {
            return write - read;
        }
        return bufferCapacity - (read - write);
    }

    /**
     * @brief Check if the buffer is empty.
     * @return true if empty, false otherwise
     */
    bool empty() const {
        return readHead.load(std::memory_order_acquire) == writeHead.load(std::memory_order_acquire);
    }

    /**
     * @brief Check if the buffer is full.
     * @return true if full, false otherwise
     */
    bool full() const {
        size_t nextWrite = (writeHead.load(std::memory_order_acquire) + 1) % bufferCapacity;
        return nextWrite == readHead.load(std::memory_order_acquire);
    }

    /**
     * @brief Get the buffer capacity.
     * @return Maximum number of elements the buffer can hold
     */
    size_t capacity() const {
        return bufferCapacity;
    }

    /**
     * @brief Reset the buffer (move heads to zero).
     * WARNING: Not thread-safe, call only when no threads are accessing.
     */
    void reset() {
        writeHead.store(0, std::memory_order_release);
        readHead.store(0, std::memory_order_release);
    }
};

#endif // REAL_TIME_RING_BUFFER_HPP
