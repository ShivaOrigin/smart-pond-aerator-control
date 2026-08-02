# Smart Pond Aerator Control System

Embedded C firmware designed for real-time water quality monitoring and dynamic aeration management in aquaculture ponds. 

Built for the **VISION-X 2K26** 36-hour national hackathon at Swarnandhra College of Engineering & Technology, in collaboration with **Aquatech Foundation Pvt. Ltd.** (Team: *Coders Desk*).

---

## Overview

Maintaining dissolved oxygen (DO) levels is critical for preventing aquatic mortality, but running high-power surface aerators continuously leads to unnecessary energy costs and motor wear. 

This firmware processes analog inputs from DO, temperature, and pH probes to trigger aerators and secondary pumps based on defined threshold logic. It includes basic moving-average filtering to prevent noise spikes from triggering relays and uses hysteresis deadbands to stop relay chatter around setpoints.

---

## Features

- **Multi-Sensor Processing:** Samples analog values for Dissolved Oxygen (mg/L), Temperature (°C), and pH.
- **Noise Reduction:** Uses a 16-sample moving average filter on analog inputs to stabilize readings.
- **Hysteresis Control:** Dual-stage activation logic with deadbands so relays don't rapidly flip ON/OFF near setpoints.
- **UART Output:** Outputs formatted JSON telemetry strings over serial for logging or IoT gateway integration.

---

## Hardware Pinout & Mapping

| Component | Pin / Channel | Type | Function |
| :--- | :--- | :--- | :--- |
| **DO Sensor** | ADC Ch 0 (A0) | Analog Input | Dissolved oxygen telemetry |
| **Temp Sensor** | ADC Ch 1 (A1) | Analog Input | Water temperature monitoring |
| **pH Sensor** | ADC Ch 2 (A2) | Analog Input | Water pH level reading |
| **Primary Aerator** | Pin 8 (Bit 0) | Digital Output | Active-LOW relay for main surface aerator |
| **Aux Aerator** | Pin 9 (Bit 1) | Digital Output | Active-LOW relay for secondary diffuser pump |
| **Alert Buzzer** | Pin 10 (Bit 2) | Digital Output | High-level alarm trigger |

---

## Logic Matrix

| Condition | Primary Aerator | Aux Pump | Alarm |
| :--- | :---: | :---: | :---: |
| **Critical DO (< 4.0 mg/L) or High Temp (> 32°C)** | **ON** | **ON** | **ACTIVE** |
| **Low DO (4.0 - 5.5 mg/L)** | **ON** | **OFF** | **OFF** |
| **Safe / Target DO (≥ 7.2 mg/L)** | **OFF** | **OFF** | **OFF** |

---

## Build & Test

The firmware is written in standard C and can be compiled using `gcc` or flashed to an AVR/ARM target.

```bash
# Navigate to firmware directory
cd firmware

# Compile
gcc smart_pond.c -o smart_pond

# Run executable
./smart_pond
