<p align="center">
  <!-- Badges for the English version -->
  <a href="https://github.com/artuppp/Riego-automatico"><img src="https://img.shields.io/badge/ESP32_Smart_Irrigation-v1.0-blueviolet"/></a>
  <a href="https://github.com/artuppp/Riego-automatico"><img src="https://img.shields.io/badge/code-Source-yellowgreen"/></a>
  <a href="#-license"><img src="https://img.shields.io/badge/license-MIT-green"/></a>
  <a href="https://github.com/artuppp/Riego-automatico/commits/main"><img src="https://img.shields.io/github/last-commit/artuppp/Riego-Automatico"/></a>
  <br>
  <a href="https://github.com/artuppp/Riego-automatico/stargazers"><img src="https://img.shields.io/github/stars/artuppp/Riego-Automatico?style=social"/></a>
  <a href="https://github.com/artuppp/Riego-automatico/network/members"><img src="https://img.shields.io/github/forks/artuppp/Riego-Automatico?style=social"/></a>
  <a href="https://github.com/artuppp/Riego-automatico/watchers"><img src="https://img.shields.io/github/watchers/artuppp/Riego-Automatico?style=social"/></a>
  <a href="https://github.com/artuppp/Riego-automatico/issues"><img src="https://img.shields.io/github/issues/artuppp/Riego-Automatico"/></a>
</p>

<p align="center">
  <img src="https://imgur.com/a/dQFuA4z" alt="Web Interface Demo" width="700"/>
</p>

<h1 align="center">🌿 ESP32 Smart Irrigation System <sub>v1.0</sub></h1>

<p align="center">
  <i>Control and monitor your plants with a complete web interface, historical graphs, and flexible watering logic.</i>
</p>
<hr>

A complete and robust automatic irrigation system based on the ESP32 board. Control and monitor your plants through an intuitive **web interface**, accessible from any device on your local network. Ideal for urban gardens, potted plants, or small flowerbeds.

## ✨ Key Features

*   **Complete Web Interface:** A modern and responsive control panel (mobile-friendly) built with Bootstrap and Chart.js.
*   **Real-Time Monitoring:** Instantly view temperature, ambient humidity, and soil moisture levels.
*   **Historical Graphs:** Interactive charts display the trends of all sensors over the last 24 hours, updated every minute.
*   **Flexible Watering Control:**
    *   **Manual Mode:** Turn the watering on or off with a single click.
    *   **Humidity-Based Automatic Mode:** Set a soil moisture threshold; the system will water automatically when needed.
    *   **Schedule-Based Automatic Mode:** Program up to two daily watering cycles at your preferred times.
*   **Persistent Configuration:** All your settings (threshold, duration, schedules) are saved to the ESP32's non-volatile memory and are retained after a reboot.
*   **Efficient Code:** The main `loop()` is non-blocking, allowing the web server and control logic to run smoothly and simultaneously.
*   **Time Synchronization:** Obtains the exact time from the internet (NTP) for precise watering schedules.

---

## 🛠️ Required Components

| Component                | Description                                                   |
| :------------------------ | :------------------------------------------------------------ |
| **Development Board**     | ESP32 (any variant with WiFi)                                 |
| **Temp/Hum Sensor**       | DHT22 (or DHT11, by adjusting `DHTTYPE` in the code)          |
| **Moisture Sensor**       | Capacitive soil moisture sensor (recommended over resistive)  |
| **Actuator**              | 1-Channel Relay Module (5V or 3.3V)                           |
| **Pump / Solenoid Valve** | A small 5V/12V water pump or a solenoid valve                 |
| **Power Supply**          | External supply for the pump/solenoid valve.                  |
| **Jumper Wires & Breadboard**| For making the connections.                               |

---

## 🔌 Connection Diagram

Make sure to connect the components to the correct ESP32 pins as defined in the code.

| Component             | Component Pin      | ESP32 Pin     |
| :-------------------- | :----------------- | :------------ |
| **DHT22 Sensor**      | DATA               | `GPIO 4`      |
| **Relay Module**      | IN                 | `GPIO 12`     |
| **Soil Sensor**       | AOUT (Analog)      | `GPIO 34`     |

**Important Notes:**
*   Connect the `VCC` and `GND` of the sensors and the relay to the `3.3V` (or `5V`, depending on the module) and `GND` pins of the ESP32, respectively.
*   **Never power the pump or solenoid valve directly from the ESP32!** Use the relay as a switch and an external power supply for the load.

---

## 🚀 Getting Started

1.  **Set Up Your Environment:**
    *   Open the project in the [Arduino IDE](https://www.arduino.cc/en/software) or [PlatformIO](https://platformio.org/).
    *   Ensure you have the ESP32 board support package installed.
    *   Install the **"DHT sensor library" by Adafruit** from the library manager. Other libraries (`WiFi`, `WebServer`, etc.) are included with the ESP32 core.

2.  **Modify WiFi Credentials:**
    *   Open the code file and modify the following lines with your WiFi network's name and password:
    ```cpp
    // --- NETWORK CONFIGURATION ---
    #define WIFI_SSID "YOUR_SSID"
    #define WIFI_PASSWORD "YOUR_PASSWORD"
    ```

3.  **Compile and Upload the Code:**
    *   Select your ESP32 board and the correct COM port.
    *   Click the upload button.

4.  **Find the IP Address:**
    *   Open the **Serial Monitor** (baud rate: 115200).
    *   Once connected to WiFi, the ESP32 will print its IP address.
    ```
    Connecting to WiFi...
    ...
    WiFi connected. IP: 192.168.1.123
    ```

5.  **Take Control!**
    *   Open a web browser on your computer or phone (connected to the same WiFi network) and navigate to the IP address you obtained. You are now in the control panel!

---

## ⚙️ How the Interface Works

The web interface is divided into several clear sections:

*   **24h-Graphs:** Three charts show the evolution of temperature, ambient humidity, and soil moisture. They update automatically every minute.
*   **Current Status:** Displays the most recent sensor readings and the relay's status (watering `ACTIVE` or `OFF`). If watering is active, it will show a countdown timer.
*   **Manual Control:** Buttons to force watering on (for the configured duration) or turn it off immediately.
*   **Configuration:**
    *   **Humidity Threshold:** Sets the soil sensor value below which automatic watering will be triggered.
    *   **Watering Duration:** Defines how many minutes each watering cycle (manual or automatic) will last.
*   **Schedule Programming:** Configure the two times of day when you want a watering cycle to be activated.

All configuration changes are automatically saved to the ESP32's memory.

---

## 📄 License

This project is distributed under the **MIT License**. Feel free to use, modify, and distribute it.

---

<p align="center">
  Made with ❤️ for happier plants.
</p>