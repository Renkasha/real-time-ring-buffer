# API Reference

## RealTimeRingBuffer Template Class

A generic, lock-free circular buffer for thread-safe concurrent data access.

### Template Parameter

```cpp
template<typename T>
class RealTimeRingBuffer { ... };
```

- `T`: The data type to store in the buffer (e.g., `int`, `struct`, class object)

### Constructor

```cpp
explicit RealTimeRingBuffer(size_t capacity = 1024);
```

- **Parameters:**
  - `capacity`: Number of elements to pre-allocate (must be > 0)
- **Throws:**
  - `std::invalid_argument` if capacity is 0

**Example:**
```cpp
RealTimeRingBuffer<int> buffer(512);  // 512-element buffer
```

### Methods

#### push()

```cpp
bool push(const T& data);
```

- **Description:** Atomically insert an element into the buffer
- **Parameters:**
  - `data`: Element to insert
- **Returns:** `true` if successful, `false` if buffer is full
- **Thread-safe:** Yes
- **Complexity:** O(1)

**Example:**
```cpp
int value = 42;
if (buffer.push(value)) {
    std::cout << "Pushed successfully\n";
} else {
    std::cout << "Buffer full\n";
}
```

#### pop()

```cpp
bool pop(T& output);
```

- **Description:** Atomically extract an element from the buffer
- **Parameters:**
  - `output`: Reference to store the extracted element
- **Returns:** `true` if successful, `false` if buffer is empty
- **Thread-safe:** Yes
- **Complexity:** O(1)

**Example:**
```cpp
int value;
if (buffer.pop(value)) {
    std::cout << "Popped: " << value << "\n";
} else {
    std::cout << "Buffer empty\n";
}
```

#### size()

```cpp
size_t size() const;
```

- **Description:** Return the approximate number of elements in the buffer
- **Returns:** Number of elements (may vary slightly due to concurrent access)
- **Thread-safe:** Yes (approximate)
- **Complexity:** O(1)

**Example:**
```cpp
std::cout << "Buffer contains " << buffer.size() << " elements\n";
```

#### empty()

```cpp
bool empty() const;
```

- **Description:** Check if the buffer is empty
- **Returns:** `true` if empty, `false` otherwise
- **Thread-safe:** Yes
- **Complexity:** O(1)

**Example:**
```cpp
while (!buffer.empty()) {
    int value;
    buffer.pop(value);
}
```

#### full()

```cpp
bool full() const;
```

- **Description:** Check if the buffer is full
- **Returns:** `true` if full, `false` otherwise
- **Thread-safe:** Yes
- **Complexity:** O(1)

**Example:**
```cpp
if (!buffer.full()) {
    buffer.push(newValue);
}
```

#### capacity()

```cpp
size_t capacity() const;
```

- **Description:** Get the maximum capacity of the buffer
- **Returns:** Buffer capacity (number of elements)
- **Thread-safe:** Yes
- **Complexity:** O(1)

**Example:**
```cpp
std::cout << "Max capacity: " << buffer.capacity() << " elements\n";
```

#### reset()

```cpp
void reset();
```

- **Description:** Reset the buffer to empty state
- **Returns:** void
- **Thread-safe:** No (only call when no threads are accessing)
- **Complexity:** O(1)
- **Warning:** This is NOT thread-safe and should only be called when you have exclusive access

**Example:**
```cpp
buffer.reset();  // Only when no other threads are using the buffer
```

### Deleted Methods

The following operations are explicitly deleted to prevent unsafe copying:

```cpp
RealTimeRingBuffer(const RealTimeRingBuffer&) = delete;
RealTimeRingBuffer& operator=(const RealTimeRingBuffer&) = delete;
```

Buffers should be shared via pointer or reference.

## Usage Examples

### Basic Single-Threaded Usage

```cpp
#include "ring_buffer.hpp"
#include <iostream>

int main() {
    RealTimeRingBuffer<int> buffer(10);
    
    // Push elements
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    
    // Pop elements
    int value;
    while (buffer.pop(value)) {
        std::cout << "Popped: " << value << "\n";
    }
    
    return 0;
}
```

### Multi-Threaded Producer/Consumer

```cpp
#include "ring_buffer.hpp"
#include <thread>
#include <atomic>

int main() {
    RealTimeRingBuffer<int> buffer(1024);
    std::atomic<bool> running(true);
    
    // Producer thread
    std::thread producer([&]() {
        for (int i = 0; i < 1000; ++i) {
            while (!buffer.push(i)) {
                std::this_thread::yield();
            }
        }
    });
    
    // Consumer thread
    std::thread consumer([&]() {
        int value;
        int count = 0;
        while (count < 1000) {
            if (buffer.pop(value)) {
                count++;
            }
        }
    });
    
    producer.join();
    consumer.join();
    
    return 0;
}
```

### Custom Data Type

```cpp
struct Packet {
    uint32_t id;
    double timestamp;
    int8_t data[64];
};

int main() {
    RealTimeRingBuffer<Packet> buffer(256);
    
    Packet pkt{1, 12345.6, {0}};
    if (buffer.push(pkt)) {
        std::cout << "Packet pushed\n";
    }
    
    if (buffer.pop(pkt)) {
        std::cout << "Packet ID: " << pkt.id << "\n";
    }
    
    return 0;
}
```
