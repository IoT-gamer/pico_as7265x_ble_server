# Pico 2W AS7265x Spectral Sensor BLE Server/Peripheral

This project transforms a **Raspberry Pi Pico 2W** into a wireless spectral sensing peripheral. It interfaces with the **ams OSRAM AS7265x Spectral Triad** via I2C and broadcasts calibrated 18-channel spectral data over Bluetooth Low Energy (BLE).

## Features
* **18-Channel Sensing:** Full spectrum coverage from 410nm to 940nm.
* **Binary BLE Notifications:** Data is transmitted as raw IEEE 754 floats across three characteristics (NIR, VIS, UV) to maximize efficiency and bypass MTU limitations.
* **Remote Control:** Command-based GATT characteristic to adjust Gain, Integration Time, and LED states.
* **Optimized for Pico 2W:** Utilizes the BTstack library and the Pico SDK's high-speed I2C implementation.

## Hardware Setup
### Wiring Diagram


| AS7265x Pin | Pico 2W Pin |GPIO| Function |
| :--- | :--- | :--- | :--- |
| 3.3V | 3V3 (Pin 36) |	- |	Power (2.7V - 3.6V) |
| GND |	GND (Pin 38) |	-        | Ground      |
| SDA |	GP4 (Pin 6)  |	GPIO 4	 | I2C0 Data   |
| SCL |	GP5 (Pin 7)  |	GPIO 5   | 	I2C0 Clock |


## BLE GATT Profile
The server exposes a custom Service (0xFF00) with the following characteristics:

**1. Data Characteristics (NOTIFY)**
Each characteristic sends 24 bytes (6 channels × 4-byte floats).
* **NIR (0xFF01):** 610nm, 680nm, 730nm, 760nm, 810nm, 860nm.
* **VIS (0xFF02):** 560nm, 585nm, 645nm, 705nm, 900nm, 940nm.
* **UV (0xFF03):** 410nm, 435nm, 460nm, 485nm, 510nm, 535nm.

**2. Control Characteristic (WRITE)**

**UUID:** `0xFF04` Accepts a 2-byte command packet: `[Command ID]` `[Value]`
* `0x01:` Set Gain (0=1x, 1=3.7x, 2=16x, 3=64x).
* `0x02:` Set Integration Time (Value × 2.8ms).
* `0x03:` Toggle NIR LED (1=On, 0=Off).
* `0x04:` Toggle VIS LED (1=On, 0=Off).
* `0x05:` Toggle UV LED (1=On, 0=Off).

## Build and Flash
1. Clone the repository.
2. Use VSCode with the official [Pico extension](https://marketplace.visualstudio.com/items?itemName=raspberry-pi.raspberry-pi-pico) for easier building and flashing.
3. `Ctlr+Shift+P` -> `CMake: Configure`
4. Click `Compile` in bottom bar.
5. Put your Pico W into BOOTSEL mode (hold the BOOTSEL button while plugging it in).
6. Click `Run` in bottom bar to flash the firmware.

## Data Format for Developers
The data is sent in Little Endian format. In Flutter or Python, you can decode the 24-byte payload using a standard float buffer.

### Example Decoding (C# / Dart-like):
```dart
// VIS Characteristic (0xFF03)
float channel7 = byteData.getFloat32(0, Endian.little); // 560nm
float channel8 = byteData.getFloat32(4, Endian.little); // 585nm
```

LICENSE
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details