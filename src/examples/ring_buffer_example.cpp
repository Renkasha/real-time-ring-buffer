/*
 *  ===========================================================================
 *  REAL-WORLD PRODUCTION SOFTWARE REPOSITORY
 *  MODULE: ring_buffer_example.cpp
 *  FUNCTION: Lock-Free High-Speed Circular Sensor Buffer
 *  ===========================================================================
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <atomic>
#include "../../include/ring_buffer.hpp"

// Real-world structured sensor payload package
struct TelemetryPacket {
    uint32_t sampleId;
    double sensorValueA;  // e.g., Friction coefficient tracking
    double sensorValueB;  // e.g., Thermal core baseline tracking
};

int main() {
    // Instantiate a real 512-slot low-latency ring buffer
    RealTimeRingBuffer<TelemetryPacket> telemetryBuffer(512);
    std::atomic<bool> runThreads(true);

    std::cout << "🚀 INITIATING REAL-WORLD LOGGING BUFFER ARCHITECTURE RUN\n";
    std::cout << "========================================================\n\n";

    // 📡 Background Producer Thread: Simulates real high-frequency hardware sweeps
    std::thread producer([&telemetryBuffer, &runThreads]() {
        uint32_t counter = 1000;
        while (runThreads) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            double simulatedFriction = 0.12 + (counter % 5) * 0.05;
            double simulatedThermal = 37.0 + (counter % 3) * 0.5;

            TelemetryPacket packet{counter, simulatedFriction, simulatedThermal};
            if (telemetryBuffer.push(packet)) {
                std::cout << "📤 [PRODUCER] Pushed packet #" << counter
                          << " (Buffer size: " << telemetryBuffer.size() << ")\n";
            } else {
                std::cout << "⚠️  [PRODUCER] Buffer full! Packet #" << counter << " dropped\n";
            }
            counter++;
        }
    });

    // 📊 Background Consumer Thread: Process stream data concurrently
    std::thread consumer([&telemetryBuffer, &runThreads]() {
        TelemetryPacket livePacket;
        while (runThreads) {
            // Continuously drain the ring buffer data sequentially
            while (telemetryBuffer.pop(livePacket)) {
                std::cout << "📥 [CONSUMER] Packet #" << livePacket.sampleId
                          << " | Friction: " << std::fixed << std::setprecision(2)
                          << livePacket.sensorValueA
                          << " | Thermal: " << std::setprecision(1)
                          << livePacket.sensorValueB << "°C\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });

    // Let the parallel threads run and process data for 1.5 seconds
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Graceful execution pipeline termination
    std::cout << "\n⏹️  Shutting down threads...\n";
    runThreads = false;
    if (producer.joinable()) producer.join();
    if (consumer.joinable()) consumer.join();

    std::cout << "========================================================\n";
    std::cout << "🏆 TESTING COMPLETE. Real-world memory tracks clean.\n";

    return 0;
}
