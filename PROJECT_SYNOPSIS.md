# PROJECT SYNOPSIS

**Project Title:** HC-LEP: HyperChaos and Light-weight Encryption Protocol for Secure IoT Radar Systems  
**Team Name:** Nano Boys  
**Team Leader:** SURESHKUMAR S  

---

## 1. Abstract
The project implements a secure IoT-based radar system using an ESP32 microcontroller, an ultrasonic sensor, and a servo motor. The primary focus is on data security during transmission from the edge node (IoT device) to the central server (Cloud). We utilize a hybrid encryption approach combining **HyperChaos-based pseudo-random number generation** and **Lattice-based cryptography** concepts to establish secure session keys, followed by **AES-128 encryption** for data payloads (Distance and Angle). The system is deployed on an AWS EC2 instance, providing a real-time web interface for monitoring.

## 2. Introduction
In the era of IoT, security is paramount. Radar systems used in surveillance or industrial automation often transmit sensitive spatial data. Standard encryption can be resource-intensive for microcontrollers. HC-LEP (HyperChaos-Lattice Encryption Protocol) aims to provide a lightweight yet robust security framework for embedded devices.

## 3. Objective
- To design a sweeping ultrasonic radar system using ESP32.
- To implement a secure communication protocol using a combination of HyperChaos and AES.
- To demonstrate real-time data visualization on a cloud-based web dashboard.
- To ensure data integrity and confidentiality against eavesdropping.

## 4. Methodology
- **Sensing Phase:** The ESP32 controls a servo to rotate the ultrasonic sensor from 0° to 180°. It measures the distance to objects at each step.
- **Encryption Phase:** 
    - A session key is generated using a HyperChaos algorithm.
    - The distance and angle data are formatted as a string (e.g., `D:120,A:045`).
    - The data is encrypted using AES-128 in ECB mode.
- **Transmission:** The encrypted ciphertext is sent via TCP/IP to a Flask server hosted on AWS.
- **Processing & Visualization:** The server decrypts the data using the derived session key and updates a dynamic web interface (HTML/JS/CSS) in real-time.

## 5. System Architecture
- **IoT Node:** ESP32 + HC-SR04 + Servo + OLED.
- **Cloud Server:** AWS EC2 running a Python Flask application.
- **Encryption Engine:** Custom HyperChaos & AES-128 implementation.

## 6. Hardware Requirements
- ESP32 Microcontroller
- HC-SR04 Ultrasonic Sensor
- SG90 Servo Motor
- SSD1306 OLED (128x64)
- Jumper Wires & Breadboard

## 7. Software Requirements
- **Embedded:** Arduino IDE (C++)
- **Backend:** Python 3.x, Flask, PyCryptodome
- **Frontend:** HTML5, CSS3, JavaScript (AJAX)
- **Deployment:** AWS EC2, Linux Environment

## 8. Expected Outcome
The system will successfully map the surroundings while ensuring that any intercepted data remains undecipherable without the dynamic session keys generated via the HyperChaos protocol. The web dashboard will provide a "tactical" radar view of the environment.
