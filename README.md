# Gesture Controlled Bot

A versatile robotics project that enables controlling a robot through hand gestures using two distinct methods: **Computer Vision** (OpenCV + MediaPipe) and **Hardware-based IMU** (MPU6050 + NRF24L01).

## 🚀 Overview

This project explores human-robot interaction by translating physical hand movements into robotic actions. It features:
1.  **Vision Control:** A Python application that uses a webcam to track hand gestures and send commands to an ESP32-based robot over WiFi (UDP).
2.  **IMU Control:** A pair of Arduino-compatible controllers that use an Inertial Measurement Unit (IMU) to transmit tilt data wirelessly via RF.

---

## 🛠️ Hardware Components

### For the Robot (Receiver)
-   **Microcontroller:** ESP32 (MicroPython) or Arduino (Nano/Uno)
-   **Motor Driver:** L298N or TB6612FNG
-   **Motors:** 2x DC Motors with Chassis
-   **Communication:** 
    -   Onboard WiFi (for ESP32)
    -   NRF24L01+ Wireless Module (for Arduino)

### For the Controllers
-   **Vision Mode:** PC with Webcam
-   **IMU Mode:** Arduino + MPU6050 (6-Axis Gyro/Accel) + NRF24L01+

---

## 💻 Software & Libraries

### Python (Vision Control)
-   `opencv-python`: For image processing.
-   `mediapipe`: For high-fidelity hand landmark tracking.
-   `socket`: For UDP communication.

### MicroPython (ESP32)
-   `machine`: For GPIO and PWM control.
-   `network`: For WiFi connectivity.

### Arduino (IMU/RF)
-   `RF24`: For NRF24L01 communication.
-   `MPU6050_6Axis_MotionApps20`: For processing IMU data.

---

## 📂 Project Structure

```text
.
├── gesture.py             # Python script for CV hand tracking
├── hardware.py            # MicroPython code for ESP32 robot
├── ip_fetch.py            # Utility to find ESP32 IP
└── hand_gestured_bot/      # Arduino-based IMU/RF implementation
    ├── transmit/          # Code for the hand-worn transmitter
    └── recieve/           # Code for the robot receiver
```

---

## ⚙️ How It Works

### 1. Computer Vision Mode (`gesture.py` + `hardware.py`)
The Python script captures video, identifies hand landmarks, and counts extended fingers. This count is sent as a UDP packet to the ESP32.

| Fingers Extended | Action |
| :--- | :--- |
| 0 | Stop |
| 1 | Move Forward |
| 2 | Move Backward |
| 3 | Turn Left |
| 4 | Turn Right |

### 2. IMU / Gesture Glove Mode (`transmit.ino` + `recieve.ino`)
The transmitter reads the **Pitch** and **Roll** from the MPU6050 sensor. These values are mapped and sent over a 2.4GHz RF link to the robot, which uses differential drive logic to move according to the tilt of your hand.

---

## 🔧 Setup

1.  **Vision Mode:**
    -   Install dependencies: `pip install opencv-python mediapipe`
    -   Flash `hardware.py` to your ESP32 using Thonny or ampy.
    -   Update `ESP32_IP` in `gesture.py` with your robot's IP address.
    -   Run `python gesture.py`.

2.  **IMU Mode:**
    -   Upload `transmit.ino` to the transmitter Arduino.
    -   Upload `recieve.ino` to the robot Arduino.
    -   Ensure the `pipeOut`/`pipeIn` addresses match in both files.

---

## 📝 License
This project is open-source and available for educational purposes.
