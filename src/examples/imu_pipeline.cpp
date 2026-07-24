#include <iostream>
#include <thread>
#include <chrono>
#include "RealTimeRingBuffer.hpp" // Adjust path based on your include structure

// Define a fixed-size, lightweight telemetry packet
struct IMUPacket {
    uint64_t timestamp_ns;
    float accel[3];
    float gyro[3];
};

int main() {
    // Instantiate a lock-free buffer capable of holding 128 IMU packets.
    // Fixed size prevents dynamic allocation overhead during runtime execution.
    RealTimeRingBuffer<IMUPacket, 128> imu_buffer;
    
    std::atomic<bool> running{true};

    // --- Thread 1: High-Frequency Hardware Driver (Producer) ---
    // Simulates a 1kHz (1000Hz) hardware reading context (e.g., SPI or CAN bus interface)
    std::thread imu_hardware_thread([&]() {
        uint64_t frame_count = 0;
        while (running) {
            IMUPacket packet;
            packet.timestamp_ns = frame_count++;
            packet.accel[0] = 0.0f; packet.accel[1] = 0.0f; packet.accel[2] = 9.81f; // Simulated gravity
            packet.gyro[0]  = 0.01f; packet.gyro[1] = 0.0f;  packet.gyro[2]  = -0.02f;

            // Non-blocking push using atomic memory barriers.
            // This thread will never stall or wait for the consumer thread.
            if (!imu_buffer.push(packet)) {
                // Buffer is full (handling overflow gracefully depends on system safety requirements)
                std::cerr << "[Warning] Buffer overflow! IMU packet dropped.\n";
            }

            // Sleep to simulate ~1000Hz loop rate (1 millisecond interval)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    // --- Thread 2: State Estimation Loop (Consumer) ---
    // Simulates a slower, computationally-heavy sensor fusion loop (e.g., Extended Kalman Filter)
    std::thread sensor_fusion_thread([&]() {
        while (running) {
            IMUPacket incoming_packet;

            // Non-blocking pop. Process all accumulated packets in the queue.
            while (imu_buffer.pop(incoming_packet)) {
                // Execute real-time mathematics or state updates here
                std::cout << "[Processing] IMU Frame: " << incoming_packet.timestamp_ns 
                          << " | Accel Z: " << incoming_packet.accel[2] << " m/s^2\n";
            }

            // Sleep to simulate a slower 200Hz evaluation cycle (5 millisecond interval)
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    });

    // Let the simulation run for a brief period
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Graceful teardown
    running = false;
    imu_hardware_thread.join();
    sensor_fusion_thread.join();

    std::cout << "Pipeline terminated cleanly.\n";
    return 0;
}
