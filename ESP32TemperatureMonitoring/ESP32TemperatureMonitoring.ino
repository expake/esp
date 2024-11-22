#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>

// Wi-Fi credentials
const char* ssid = "xxxxxxx";
const char* password = "xxxxxxxxxxx";

// Telegram
#define BOT_TOKEN "xxxxxx:xxxxxxxxxxx"
#define CHAT_ID "xxxxxxxxx"

// Secure client setup for Telegram
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// Temperature sensor setup
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Thresholds and notification delays
float warningThreshold = 60.0;      // Warning temperature threshold
float criticalThreshold = 90.0;     // Critical temperature threshold
unsigned long warningDelay = 180000; // Delay between warning messages (1 minute)
unsigned long criticalDelay = 300000; // Delay between critical messages (1 minute)

// Array to store the last notification time for each sensor
unsigned long lastWarningTime[4] = {0, 0, 0, 0};
unsigned long lastCriticalTime[4] = {0, 0, 0, 0};

// Temperature data storage
const int maxDataPoints = 60;
float temperatureData[maxDataPoints][4];  // Array for 4 sensors
unsigned long timestamps[maxDataPoints];
int currentIndex = 0;

// Array for sensor addresses
DeviceAddress sensorAddresses[4];

// Web server setup
AsyncWebServer server(80);

// Function to send Telegram messages
void sendTelegramMessage(String message) {
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  if (secured_client.connect("api.telegram.org", 443)) {
    Serial.println("Connected to Telegram!");

    if (bot.sendMessage(CHAT_ID, message, "")) {
      Serial.println("Message sent to Telegram");
    } else {
      Serial.println("Failed to send message to Telegram");
    }
  } else {
    Serial.println("Telegram connection error!");
  }
}

// Function to check temperature and send alerts
void checkTemperatureAlerts() {
  sensors.requestTemperatures();
  for (int i = 0; i < 4; i++) {
    float temperature = sensors.getTempCByIndex(i);

    // Check for warning
    if (temperature > warningThreshold && temperature < criticalThreshold) {
      unsigned long currentTime = millis();
      if (currentTime - lastWarningTime[i] > warningDelay) {
        String message = "Warning: Sensor " + String(i + 1) + " temperature is " + String(temperature, 2) + " °C.";
        sendTelegramMessage(message);
        lastWarningTime[i] = currentTime;
      }
    }

    // Check for critical
    if (temperature >= criticalThreshold) {
      unsigned long currentTime = millis();
      if (currentTime - lastCriticalTime[i] > criticalDelay) {
        String message = "Critical: Sensor " + String(i + 1) + " temperature is " + String(temperature, 2) + " °C.";
        sendTelegramMessage(message);
        lastCriticalTime[i] = currentTime;
      }
    }
  }
}

// Function to handle Telegram messages
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    Serial.println("Received message: " + text);

    if (text == "/temp") {
      sensors.requestTemperatures();
      String temperatureMessage = "Sensor temperatures:\n";
      for (int i = 0; i < 4; i++) {
        float temperature = sensors.getTempCByIndex(i);
        temperatureMessage += "Sensor " + String(i + 1) + ": " + String(temperature, 2) + " °C\n";
      }
      bot.sendMessage(chat_id, temperatureMessage, "");
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Send notification about device activation
  sendTelegramMessage("Device is active and connected to Wi-Fi.");

  // Initialize temperature sensors
  sensors.begin();

  // Read unique sensor addresses and save them to EEPROM
  for (int i = 0; i < 4; i++) {
    if (sensors.getAddress(sensorAddresses[i], i)) {
      Serial.print("Sensor ");
      Serial.print(i + 1);
      Serial.print(" address: ");
      for (int j = 0; j < 8; j++) {
        Serial.print(sensorAddresses[i][j], HEX);
        if (j < 7) Serial.print(":");
      }
      Serial.println();

      EEPROM.begin(512);  // Initialize EEPROM
      for (int j = 0; j < 8; j++) {
        EEPROM.write(i * 8 + j, sensorAddresses[i][j]);
      }
      EEPROM.commit();
    }
  }

  // Initialize temperature data array
  for (int i = 0; i < maxDataPoints; i++) {
    for (int j = 0; j < 4; j++) {
      temperatureData[i][j] = -127;  // Initialize with invalid temperature
    }
    timestamps[i] = 0;
  }

  // Handle GET requests for the main page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    sensors.requestTemperatures();
    unsigned long currentTime = millis();

    // Collect temperature data for the chart
    String jsonData = "[";
    String temperatureString = "<h2>Temperature Readings:</h2>";
    for (int i = 0; i < 4; i++) {  // 4 sensors
      float temperature = sensors.getTempCByIndex(i);
      temperatureData[currentIndex][i] = temperature;

      // Generate string for each sensor
      temperatureString += "<p>Sensor " + String(i + 1) + " Temperature: " + String(temperature, 2) + " &#8451;</p>";

      jsonData += "{";
      jsonData += "\"sensor\": " + String(i + 1) + ",";
      jsonData += "\"temperature\": " + String(temperature, 2);
      jsonData += "}";
      if (i < 3) jsonData += ",";
    }
    jsonData += "]";

    timestamps[currentIndex] = currentTime;
    currentIndex = (currentIndex + 1) % maxDataPoints;

    // Generate HTML page with the chart
    String html = "<html><head>";
    html += "<script src=\"https://cdn.jsdelivr.net/npm/chart.js\"></script>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; }";
    html += "h1 { font-size: 24px; }";
    html += "h2 { font-size: 20px; }";
    html += "canvas { max-width: 100%; height: auto; }";
    html += "p { font-size: 16px; margin: 5px 0; }";
    html += "</style>";
    html += "</head><body>";
    html += "<h1>Temperature Data</h1>";
    html += temperatureString;  // Add temperature readings for each sensor
    html += "<canvas id=\"temperatureChart\" width=\"400\" height=\"200\"></canvas>";
    html += "<script>";
    html += "var ctx = document.getElementById('temperatureChart').getContext('2d');";
    html += "var temperatureChart = new Chart(ctx, {";
    html += "    type: 'line',";
    html += "    data: {";
    html += "        labels: [";  // Timestamps
    for (int i = 0; i < maxDataPoints; i++) {
      html += "\"" + String(timestamps[i]) + "\"";
      if (i < maxDataPoints - 1) html += ",";
    }
    html += "],";
    html += "        datasets: [";
    for (int i = 0; i < 4; i++) {  // For 4 sensors
      html += "{";
      html += "label: 'Sensor " + String(i + 1) + "',";
      html += "data: [";
      for (int j = 0; j < maxDataPoints; j++) {
        html += String(temperatureData[j][i], 2);
        if (j < maxDataPoints - 1) html += ",";
      }
      html += "],";
      html += "borderColor: 'rgba(" + String(50 + i * 50) + ", 150, 255, 1)',";
      html += "fill: false";
      html += "}";
      if (i < 3) html += ",";
    }
    html += "]";
    html += "    },";
    html += "    options: {";
    html += "        scales: {";
    html += "            y: {";
    html += "                min: 0,";
    html += "                max: 120";
    html += "            },";
    html += "            x: {";
    html += "                type: 'linear',";
    html += "                position: 'bottom'";
    html += "            }";
    html += "        }";
    html += "    }";
    html += "});";
    html += "</script>";
    html += "</body></html>";

    // Send HTML page
    request->send(200, "text/html", html);
  });

  // Start web server
  server.begin();
}

void loop() {
  // Check for new Telegram messages
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  if (numNewMessages) {
    handleNewMessages(numNewMessages);
  }

  // Check temperatures and send alerts
  checkTemperatureAlerts();
}
