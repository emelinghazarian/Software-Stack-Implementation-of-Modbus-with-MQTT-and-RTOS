# Modbus and MQTT-Based Master-Slave Control System with RTOS

## Overview
This project integrates Modbus and MQTT protocols into an embedded control system using an RTOS for efficient task scheduling. The system includes:
- **Master Device**: Manages communication with multiple slave devices via Modbus.
- **Controller (Slave)**: Controls the relay states.
- **Actuator (Slave)**: Reads sensor data and shares it with the Master.
- **MQTT Integration**: Allows communication with a PC or smartphone for real-time monitoring and control of relays.

The RTOS is used to ensure deterministic control of processes, handling Modbus communication, MQTT tasks, and other control operations seamlessly.

## Requirements
- **Programming Language**: C/C++ for embedded development.
- **Hardware**: Devices supporting RS-232/RS-485, microcontroller with RTOS capability.
- **Software**: MQTT Broker, RTOS (FreeRTOS or similar)

## Project Steps

### System Architecture
- The system uses **RTOS** to manage tasks including Modbus communication, MQTT data transmission, and device control.
- Modbus protocol is used to handle communication between the Master, Controller, and Actuator using **RS-232/RS-485**.
- **MQTT** is used to communicate with external devices like a PC or smartphone for remote monitoring and control.

### Devices Implemented

#### Master Device
The Master device handles:
- Sending commands to read and write data to connected slaves using Modbus.
- Handling MQTT messages to receive instructions from a PC or smartphone.
- Monitoring relay statuses using MQTT for real-time feedback.

#### Controller (Slave)
The Controller device handles:
- Managing relays using **Write Single Coil** and **Write Single Register** commands from the Master.
- Relays can also be controlled remotely via MQTT messages, providing flexibility.

#### Actuator (Slave)
The Actuator device is responsible for:
- Handling **Read Input Register** commands to collect sensor data.
- Reading temperature using an **LM75 sensor** and sending it to the Master.
- Transmitting temperature data to a PC or smartphone using MQTT.

### Communication Flow
- The **Master** communicates with the **Controller** and **Actuator** through **RS-485** to issue commands and collect data.
- **MQTT** allows relay control via an external PC or mobile phone, providing an easy interface to change the relay states remotely.
- The **RTOS** ensures that Modbus and MQTT communication tasks do not interfere, providing reliable timing and process control.

### RTOS Implementation
- **Task Scheduling**: Separate tasks are used for Modbus communication, MQTT transmission, and relay state control, ensuring real-time and conflict-free operation.
- **Priority Assignment**: Tasks are prioritized based on their criticality, with MQTT and Modbus communication having higher priorities to ensure timely data transmission.

