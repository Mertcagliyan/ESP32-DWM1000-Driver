# ESP32-DWM1000-Driver & Firmware

![Language](https://img.shields.io/badge/language-C%2B%2B-blue.svg)
![Platform](https://img.shields.io/badge/platform-ESP32%20%7C%20Arduino-orange.svg)
![Hardware](https://img.shields.io/badge/hardware-Decawave%20DWM1000-green.svg)
![License](https://img.shields.io/badge/license-GPLv3-red.svg)

This repository contains the low-level firmware and drivers for a UWB (Ultra-Wideband) positioning network. It allows **ESP32** microcontrollers to communicate with **Decawave DWM1000** modules to perform high-precision Time-of-Flight (ToF) ranging.

This project is the hardware companion to the high-level positioning algorithm: [UWB-Drone-Positioning-System](https://github.com/Mertcagliyan/UWB-Drone-Positioning-System).

## 📂 Repository Structure

The codebase is organized into three main modules:

| Folder | Description |
| :--- | :--- |
| **`src/`** | **Core Firmware.** Contains the main logic for Anchors (Fixed units) and Tags (Mobile units). Handles SPI communication, TWR protocol, and outlier filtering. |
| **`calibration/`** | **Auto-Calibration Tools.** Scripts to automatically determine the unique *Antenna Delay* for each DWM1000 module to ensure centimeter-level accuracy. |
| **`experimental/`** | **ESP-NOW Network.** A dedicated mesh network implementation for telemetry data exchange between nodes, independent of UWB ranging. |

## 🔌 Hardware Pinout (Wiring)

**⚠️ CRITICAL WARNING:** The Chip Select (CS) pin configuration varies between units in this specific hardware setup. Please verify your soldering before flashing.

| DWM1000 Pin | ESP32 GPIO (Standard) |
| :--- | :--- |
| **VCC** | 3.3V |
| **GND** | GND |
| **SCK** | GPIO 18 |
| **MISO** | GPIO 19 |
| **MOSI** | GPIO 23 |
| **RST** | GPIO 27 |
| **IRQ** | GPIO 34 |

### Chip Select (CS/SS) Configuration Matrix

| Unit Name | Firmware File | **CS Pin (GPIO)** |
| :--- | :--- | :--- |
| **Anchor Unit 1** | `src/Anchor_Unit1.ino` | **GPIO 4** |
| **Anchor Unit 2** | `src/Anchor_Unit2.ino` | **GPIO 5** |
| **Anchor Unit 3** | `src/Anchor_Unit3.ino` | **GPIO 4** |
| **Tag Unit** | `src/Tag_Main.ino` | **GPIO 5** |

## 🚀 Features

* **Precision Ranging:** Uses Two-Way Ranging (TWR) algorithms.
* **Auto-Calibration:** Includes a feedback-loop script (`calibration/Anchor_Calibration.ino`) that tunes the antenna delay by comparing measured distance against a known reference.
* **Multi-Anchor Support:** Configured for a 3-Anchor setup with specific MAC addresses and delay offsets.
* **Mesh Communication:** Experimental support for ESP-NOW to broadcast ranging data across the network without Wi-Fi latency.

## 🛠️ Dependencies & Setup

To run this firmware, you need the **Arduino IDE** or **PlatformIO** with the following setup:

1.  **Board Manager:** Install `esp32` by Espressif Systems.
2.  **Required Libraries:**
    * `DW1000` (by thotro/reconfig) - *Primary driver for UWB.*
    * `SPI` - *Built-in Arduino library.*
    * `WiFi` & `esp_now` - *Built-in (for experimental/ folder).*

## ⚙️ How to Flash

1.  **Select the Unit:** Choose the appropriate file from `src/` (e.g., `Anchor_Unit1.ino` or `Tag_Main.ino`).
2.  **Verify Configuration:**
    * Check `#define ANCHOR_ADD` to ensure unique MAC addresses.
    * Check `#define DW_CS` to match the table above.
3.  **Upload:** Connect your ESP32 via USB and upload the sketch.

## 🤝 Contributing

This driver was developed as part of a comprehensive autonomous drone positioning project. Issues and Pull Requests are welcome.

## 📄 License
**GNU General Public License v3.0**
See the [LICENSE](LICENSE) file for details.