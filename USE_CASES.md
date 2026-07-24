# Real-Time Ring Buffer: Robotics Use Cases

This document outlines practical engineering scenarios in robotics where a lock-free, zero-allocation circular buffer is critical for maintaining hardware determinism.

## 1. High-Frequency IMU Sensor Data Pipeline
In robotics, inertial measurement units (IMUs) stream raw gyro and accelerometer data at very high speeds (e.g., 1000Hz). If your main navigation loop lags for even a millisecond, you drop critical orientation frames.

* **Thread 1 (Producer):** A high-priority hardware driver thread reads the raw IMU data via SPI/I2C. It instantly pushes the telemetry struct into your `RealTimeRingBuffer` using sub-microsecond atomic operations.
* **Thread 2 (Consumer):** A slower sensor-fusion thread (running at 200Hz) pops the accumulated sensor data out of the buffer to run Kalman filter math and update the robot's localization state.
* **Why this repository is needed:** Because the buffer is lock-free, the slower 200Hz math thread *never* blocks the critical 1000Hz sensor driver thread.

## 2. Motor Encoder Telemetry Logging
Tracking the exact positions, velocities, and currents of multiple robotic actuators is necessary for diagnosing failures. However, writing logs to a slow file system or sending them over a Wi-Fi network will completely stall a real-time motor control loop.

* **Thread 1 (Producer):** The core 1kHz EtherCAT or CAN bus motor control loop handles raw hardware actuation. At the end of every loop cycle, it throws a small `TelemetryPacket` (containing current position and motor temperature) into your ring buffer.
* **Thread 2 (Consumer):** A low-priority background logging thread periodically pops packets out of the buffer and writes them to a micro-SD card or flashes them out over a ROS `.bag` recording tool.
* **Why this repository is needed:** Your buffer uses zero runtime allocations (`malloc`). This guarantees that saving debug logs will never introduce heap-allocation jitter into the safety-critical motor control loop.

## 3. Camera Frame Buffer for Visual Odometry
Computer vision algorithms (like SLAM or object tracking) are computationally heavy and uneven. A camera might capture frames at a steady 30 FPS, but a complex image frame might cause the tracking thread to take slightly longer to process.

* **Thread 1 (Producer):** A video capture thread pulls incoming raw images directly from a USB3/MIPI camera driver and continuously pushes pointers to those frame buffers into your ring buffer.
* **Thread 2 (Consumer):** An object-detection neural network or visual-odometry algorithm pops the oldest frame pointer from the buffer to process it.
* **Why this repository is needed:** If the vision algorithm hits a difficult frame and slows down for a split second, your fixed-size ring buffer acts as a deterministic shock absorber, holding the incoming camera frames smoothly without dropping packets or allocating sudden spikes of system memory.
