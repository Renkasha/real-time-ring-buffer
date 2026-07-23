#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "../../include/ring_buffer.hpp"

class RingBufferTest : public ::testing::Test {
protected:
    RealTimeRingBuffer<int> buffer{16};
};

TEST_F(RingBufferTest, BufferInitialization) {
    EXPECT_EQ(buffer.capacity(), 16);
    EXPECT_TRUE(buffer.empty());
    EXPECT_FALSE(buffer.full());
    EXPECT_EQ(buffer.size(), 0);
}

TEST_F(RingBufferTest, SingleElementPushPop) {
    EXPECT_TRUE(buffer.push(42));
    EXPECT_FALSE(buffer.empty());

    int value;
    EXPECT_TRUE(buffer.pop(value));
    EXPECT_EQ(value, 42);
    EXPECT_TRUE(buffer.empty());
}

TEST_F(RingBufferTest, MultipleElementsPushPop) {
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(buffer.push(i));
    }
    EXPECT_EQ(buffer.size(), 10);

    for (int i = 0; i < 10; ++i) {
        int value;
        EXPECT_TRUE(buffer.pop(value));
        EXPECT_EQ(value, i);
    }
    EXPECT_TRUE(buffer.empty());
}

TEST_F(RingBufferTest, BufferOverflow) {
    // Fill the buffer completely (16 slots)
    for (int i = 0; i < 16; ++i) {
        EXPECT_TRUE(buffer.push(i));
    }
    EXPECT_TRUE(buffer.full());

    // Attempt to push when full
    EXPECT_FALSE(buffer.push(999));
}

TEST_F(RingBufferTest, BufferUnderflow) {
    int value;
    // Attempt to pop from empty buffer
    EXPECT_FALSE(buffer.pop(value));
}

TEST_F(RingBufferTest, CircularWraparound) {
    // Fill, partially drain, and fill again to test wraparound
    for (int i = 0; i < 10; ++i) {
        buffer.push(i);
    }

    int value;
    for (int i = 0; i < 5; ++i) {
        buffer.pop(value);
    }

    // Push more elements to wrap around
    for (int i = 10; i < 15; ++i) {
        EXPECT_TRUE(buffer.push(i));
    }

    // Verify all remaining elements
    for (int i = 5; i < 15; ++i) {
        EXPECT_TRUE(buffer.pop(value));
        EXPECT_EQ(value, i);
    }
}

TEST_F(RingBufferTest, ThreadSafeConcurrentAccess) {
    std::vector<std::thread> threads;
    const int numProducers = 4;
    const int numConsumers = 2;
    const int itemsPerProducer = 25;
    std::atomic<int> successfulPops(0);

    // Producer threads
    for (int p = 0; p < numProducers; ++p) {
        threads.emplace_back([&buffer, p, itemsPerProducer]() {
            for (int i = 0; i < itemsPerProducer; ++i) {
                int value = p * 1000 + i;
                while (!buffer.push(value)) {
                    std::this_thread::yield();
                }
            }
        });
    }

    // Consumer threads
    for (int c = 0; c < numConsumers; ++c) {
        threads.emplace_back([&buffer, &successfulPops, numProducers, itemsPerProducer]() {
            int expectedPops = numProducers * itemsPerProducer;
            int poppedCount = 0;
            int value;
            while (poppedCount < expectedPops) {
                if (buffer.pop(value)) {
                    poppedCount++;
                    successfulPops++;
                } else {
                    std::this_thread::yield();
                }
            }
        });
    }

    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(successfulPops.load(), numProducers * itemsPerProducer);
    EXPECT_TRUE(buffer.empty());
}

TEST_F(RingBufferTest, Reset) {
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    EXPECT_EQ(buffer.size(), 3);

    buffer.reset();
    EXPECT_TRUE(buffer.empty());
    EXPECT_EQ(buffer.size(), 0);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
