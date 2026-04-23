# FallSense

Status: WORK IN PROGRESS – actively under development

FallSense is a low-power embedded system for fall detection, built on an STM32 microcontroller using FreeRTOS.

The system is being developed to detect and confirm falls using motion data from an IMU (accelerometer/gyroscope), with planned integration of pressure sensors (FSRs) for early fall prediction.

Current development focuses on:

* Interfacing and reading accelerometer data
* Basic motion analysis for fall detection
* System architecture and modular firmware design

Planned features include:

* Sensor fusion (FSR + IMU) for improved accuracy
* GPS/BLE communication for alerting
* Cloud integration for data logging (Firebase)

