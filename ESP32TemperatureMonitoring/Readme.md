---

# ESP32 Temperature Monitoring with Telegram Alerts and Web Interface

This project implements a temperature monitoring system using ESP32, DS18B20 temperature sensors, and Telegram for real-time notifications. The system includes a web interface for visualizing temperature data in charts, a `/temp` command in Telegram to request current temperatures, and automated alerts for warning and critical thresholds.

## Features

- **Multi-sensor Support**: Monitors up to 4 DS18B20 temperature sensors.
- **Web Interface**: Displays real-time temperature data in an interactive chart.
- **Telegram Integration**:
  - Sends notifications when a sensor temperature exceeds warning or critical thresholds.
  - Provides current temperature readings for all sensors via the `/temp` command.
- **Customizable Thresholds and Delays**:
  - Warning and critical temperature thresholds.
  - Delays between notifications to avoid spamming.

## How It Works

1. **Initialization**:
   - Connects to Wi-Fi.
   - Sends a "Device is active and connected to Wi-Fi" message to a Telegram chat.

2. **Web Interface**:
   - Accessible via the ESP32's IP address in a browser.
   - Displays sensor data in a live chart.

3. **Telegram Alerts**:
   - Sends a **warning** message if a sensor's temperature exceeds the warning threshold.
   - Sends a **critical** message if a sensor's temperature exceeds the critical threshold.
   - Ensures a delay between notifications (default: 3 minutes for warning, 5 minutes for critical).

4. **Temperature Requests**:
   - Users can send the `/temp` command in Telegram to get the current readings for all sensors.

## Hardware Requirements

- **ESP32 Development Board**
- **DS18B20 Temperature Sensors** (up to 4)
- **10kΩ Resistor** (pull-up resistor for the one-wire bus)
- Jumper wires and breadboard for connections.

## How to Connect DS18B20 Sensors to ESP32

### Components Needed
1. DS18B20 temperature sensor(s)
2. 10kΩ resistor
3. Breadboard and jumper wires

### Pin Connections
Each DS18B20 sensor has three pins:
1. **VCC**: Power supply (3.3V or 5V from the ESP32).
2. **GND**: Ground.
3. **DATA**: One-wire data signal.

Follow these steps to connect the sensors:
1. Connect all **VCC** pins to the **3.3V** pin on the ESP32.
2. Connect all **GND** pins to the **GND** pin on the ESP32.
3. Connect all **DATA** pins together and to a GPIO pin on the ESP32 (e.g., GPIO4, as used in the sketch).
4. Place a **10kΩ resistor** between the **DATA** line and **VCC** (pull-up resistor).

### Example Circuit
- DS18B20 pin 1 (GND) → ESP32 GND
- DS18B20 pin 2 (DATA) → ESP32 GPIO4
- DS18B20 pin 3 (VCC) → ESP32 3.3V
- 10kΩ resistor between GPIO4 (DATA) and 3.3V

### Multiple Sensors
If using multiple DS18B20 sensors, connect their **DATA** lines together to the same GPIO pin and ensure the pull-up resistor is in place.

## Software Requirements

- Arduino IDE with the following libraries:
  - [DallasTemperature](https://github.com/milesburton/Arduino-Temperature-Control-Library)
  - [OneWire](https://github.com/PaulStoffregen/OneWire)
  - [UniversalTelegramBot](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot)
  - [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)

## Setup Instructions

1. **Hardware Connection**:
   - Follow the connection guide above to wire the DS18B20 sensors to your ESP32.

2. **Code Configuration**:
   - Set your Wi-Fi credentials:
     ```cpp
     const char* ssid = "your-ssid";
     const char* password = "your-password";
     ```
   - Set your Telegram bot token and chat ID:
     ```cpp
     #define BOT_TOKEN "your-bot-token"
     #define CHAT_ID "your-chat-id"
     ```
   - Adjust thresholds and delays if needed:
     ```cpp
     float warningThreshold = 20.0; // Warning threshold in °C
     float criticalThreshold = 30.0; // Critical threshold in °C
     unsigned long warningDelay = 180000; // 3 minutes
     unsigned long criticalDelay = 300000; // 5 minutes
     ```

3. **Upload Code**:
   - Open the code in the Arduino IDE.
   - Select your ESP32 board and upload the sketch.

4. **Access Web Interface**:
   - Once the ESP32 is connected to Wi-Fi, open a browser and navigate to the device's IP address.

5. **Telegram Interaction**:
   - Add the bot to a chat (use BotFather on Telegram to set it up).
   - Send `/temp` to get sensor readings.
   - Monitor for warning or critical temperature alerts.

## Demo

### Web Interface
Displays live temperature readings with a chart.
![image](https://github.com/user-attachments/assets/0caa0952-ec07-4789-a9e4-4e8c24db189d)

### Telegram Bot
- Example of `/temp` command:
  ```
  Sensor temperatures:
  Sensor 1: 22.34 °C
  Sensor 2: 19.76 °C
  Sensor 3: 25.12 °C
  Sensor 4: 18.45 °C
  ```

- Example of warning alert:
  ```
  Warning: Sensor 2 temperature is 21.5 °C.
  ```

- Example of critical alert:
  ```
  Critical: Sensor 3 temperature is 32.0 °C.
  ```

## Circuit Diagram

*Include a simple circuit diagram here, if possible.*

## License

This project is licensed under the MIT License. Feel free to use, modify, and share.

---

**Happy monitoring!** 🎉

