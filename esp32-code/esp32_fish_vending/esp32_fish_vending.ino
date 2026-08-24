/**
 * Smart Fish Vending Machine - ESP32 Production Sketch
 * For Arduino IDE
 *
 * Hardware:
 * - DHT11 Temperature/Humidity Sensor
 * - 4x MG995 Servo Motors (compartments 1-4)
 * - LDR Sensor with Laser Beam (fish drop confirmation)
 * - Buzzer
 * - Built-in LED
 * - I2C LCD Display (16x2)
 *
 * Features:
 * - WiFi connection with auto-reconnect
 * - HTTP server on port 80
 * - POST /dispense - dispense fish from compartment
 * - GET /status - machine status
 * - POST /sensor-data - update backend with DHT11 readings every 30s
 * - Servo control: 0° = closed, 180° = open
 * - LDR + laser beam drop confirmation
 * - Buzzer patterns for order placed and completion
 * - LCD display shows machine status and order information
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

// ==================== CONFIGURATION ====================
const char* WIFI_SSID = "Abhi12";
const char* WIFI_PASSWORD = "11223344";

const char* BACKEND_HOST = "10.11.196.173";  // 10.221.72.173.   10.11.196.173
const int BACKEND_PORT = 5050;
const char* BACKEND_ORDER_COMPLETE = "/api/order/";
const char* BACKEND_SENSOR_DATA = "/api/esp32/sensor-data";

// Hardware pins
const int LED_PIN = 2;
const int DHT_PIN = 4;
const int BUZZER_PIN = 27;
const int LDR_PIN = 34;

// I2C LCD pins (ESP32 default I2C)
const int LCD_SDA = 21;
const int LCD_SCL = 22;

// Servo pins for 4 compartments
const int SERVO_PINS[4] = {18, 19, 14, 16};

// Per-compartment calibration angles (adjust these for your hardware)
const int SERVO_OPEN_ANGLE[4] = {180, 150, 0, 0};    // Open position for each compartment
const int SERVO_CLOSE_ANGLE[4] = {0, 0, 180, 180}; // Close position for each compartment
const int SERVO_MOVE_DELAY = 4000; // Delay for servo to reach position (ms)

// DHT11 sensor
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// I2C LCD (16x2, address 0x27 or 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Servo objects
Servo servos[4];

// ==================== GLOBAL VARIABLES ====================
WebServer server(80);
String currentOrderId = "";
int currentCompartment = 0;
bool isDispensing = false;
unsigned long lastDispenseTime = 0;
unsigned long lastSensorUpdate = 0;
const unsigned long SENSOR_UPDATE_INTERVAL = 30000; // 30 seconds

// ==================== LCD FUNCTIONS ====================
void initLCD() {
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Fish Vending");
  lcd.setCursor(0, 1);
  lcd.print("Machine Ready");
  Serial.println("LCD initialized");
}

void clearLCD() {
  lcd.clear();
}

void displayIdle() {
  clearLCD();
  lcd.setCursor(0, 0);
  lcd.print("Fish Vending");
  lcd.setCursor(0, 1);
  lcd.print("Machine Ready");
  Serial.println("[LCD] Fish Vending / Machine Ready");
}

void displayOrderReceived(String orderId, String fishName) {
  clearLCD();
  lcd.setCursor(0, 0);
  int nameLen = fishName.length();
  if (nameLen > 12) nameLen = 12;
  lcd.print("Order:");
  lcd.setCursor(6, 0);
  lcd.print(fishName.substring(0, nameLen));
  lcd.setCursor(0, 1);
  lcd.print("ID:");
  lcd.setCursor(4, 1);
  lcd.print(orderId);
  Serial.print("[LCD] Order Received / Fish: ");
  Serial.print(fishName);
  Serial.print(" / ID: ");
  Serial.println(orderId);
}

void displayDispensingItem(int current, int total, String fishName) {
  clearLCD();
  lcd.setCursor(0, 0);
  lcd.print("Dispensing...");
  int nameLen = fishName.length();
  if (nameLen > 9) nameLen = 9;
  lcd.setCursor(0, 1);
  lcd.print(fishName.substring(0, nameLen));
  lcd.setCursor(10, 1);
  lcd.print(current);
  lcd.print("/");
  lcd.print(total);
  Serial.print("[LCD] Dispensing item ");
  Serial.print(current);
  Serial.print("/");
  Serial.print(total);
  Serial.print(" - ");
  Serial.println(fishName);
}

void displayDispensing() {
  clearLCD();
  lcd.setCursor(0, 0);
  lcd.print("Dispensing...");
  lcd.setCursor(0, 1);
  lcd.print("Please Wait");
  Serial.println("[LCD] Dispensing... / Please Wait");
}

void displayDispensed() {
  clearLCD();
  lcd.setCursor(0, 0);
  lcd.print("Dispensed!");
  lcd.setCursor(0, 1);
  lcd.print("Thank You :)");
  Serial.println("[LCD] Dispensed! / Thank You :)");
}

void displayError(String message) {
  clearLCD();
  lcd.setCursor(0, 0);
  lcd.print("Error:");
  lcd.setCursor(0, 1);
  lcd.print(message.substring(0, 16));
  Serial.print("[LCD] Error: ");
  Serial.println(message);
}

void displaySensorData(float temp, float hum) {
  clearLCD();
  lcd.setCursor(0, 0);
  lcd.print("Temp:");
  lcd.setCursor(6, 0);
  lcd.print(temp, 1);
  lcd.print((char)223); // Degree symbol
  lcd.setCursor(11, 0);
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Hum:");
  lcd.setCursor(5, 1);
  lcd.print(hum, 0);
  lcd.print("%");
  Serial.print("[LCD] Temp: ");
  Serial.print(temp);
  Serial.print(" C / Hum: ");
  Serial.print(hum);
  Serial.println("%");
}

// ==================== WiFi FUNCTIONS ====================
void connectToWiFi() {
  Serial.println("\n========================================");
  Serial.println("Connecting to WiFi...");
  Serial.print("SSID: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected successfully!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("\nWiFi connection FAILED!");
  }
  Serial.println("========================================");
}

void maintainWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWiFi disconnected! Reconnecting...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(1000);
  }
}

// ==================== SENSOR FUNCTIONS ====================
float readTemperature() {
  for (int i = 0; i < 5; i++) {
    float temp = dht.readTemperature();
    if (!isnan(temp) && temp > -40 && temp < 80) {
      return temp;
    }
    delay(300);
  }
  Serial.println("Failed to read temperature from DHT11!");
  return 0.0;
}

float readHumidity() {
  for (int i = 0; i < 5; i++) {
    float hum = dht.readHumidity();
    if (!isnan(hum) && hum >= 0 && hum <= 100) {
      return hum;
    }
    delay(300);
  }
  Serial.println("Failed to read humidity from DHT11!");
  return 0.0;
}

bool readLDR() {
  int ldrValue = analogRead(LDR_PIN);
  // Assuming HIGH = laser beam detected (fish not blocking), LOW = fish blocking
  bool beamBroken = (ldrValue < 500); // Adjust threshold based on your LDR
  Serial.print("LDR Value: ");
  Serial.print(ldrValue);
  Serial.print(" - Beam: ");
  Serial.println(beamBroken ? "BROKEN (fish detected)" : "INTACT");
  return beamBroken;
}

// ==================== SERVO FUNCTIONS ====================
void openCompartment(int compartment) {
  if (compartment < 1 || compartment > 4) return;

  int servoIndex = compartment - 1;
  Serial.print("Opening compartment ");
  Serial.println(compartment);
  Serial.print("  Angle: ");
  Serial.println(SERVO_OPEN_ANGLE[servoIndex]);

  servos[servoIndex].write(SERVO_OPEN_ANGLE[servoIndex]);
  delay(SERVO_MOVE_DELAY); // Wait for servo to reach position
}

void closeCompartment(int compartment) {
  if (compartment < 1 || compartment > 4) return;

  int servoIndex = compartment - 1;
  Serial.print("Closing compartment ");
  Serial.println(compartment);
  Serial.print("  Angle: ");
  Serial.println(SERVO_CLOSE_ANGLE[servoIndex]);

  servos[servoIndex].write(SERVO_CLOSE_ANGLE[servoIndex]);
  delay(SERVO_MOVE_DELAY); // Wait for servo to reach position
}

void initServos() {
  for (int i = 0; i < 4; i++) {
    servos[i].attach(SERVO_PINS[i]);
    servos[i].write(SERVO_CLOSE_ANGLE[i]); // Start in closed position
    Serial.print("Servo ");
    Serial.print(i + 1);
    Serial.print(" initialized on pin ");
    Serial.print(SERVO_PINS[i]);
    Serial.print(" | Close: ");
    Serial.print(SERVO_CLOSE_ANGLE[i]);
    Serial.print(" | Open: ");
    Serial.println(SERVO_OPEN_ANGLE[i]);
  }
  delay(2000); // Allow servos to initialize and settle
}

// ==================== BUZZER FUNCTIONS ====================
void beepBuzzer(int times, int intervalMs = 200) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(intervalMs);
    digitalWrite(BUZZER_PIN, LOW);
    delay(intervalMs);
  }
}

void playOrderPlacedSound() {
  Serial.println("Playing order placed sound...");
  beepBuzzer(2, 150); // 2 quick beeps
  delay(200);
}

void playCompletionSound() {
  Serial.println("Playing completion sound...");
  // Simple 3-second melody
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN, LOW);
  delay(100);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN, LOW);
  delay(100);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(800);
  digitalWrite(BUZZER_PIN, LOW);
  delay(100);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("Completion sound done");
}

void playErrorSound() {
  Serial.println("Playing error sound...");
  beepBuzzer(3, 100); // 3 rapid beeps
}

// ==================== LED FUNCTIONS ====================
void blinkLED(int times, int intervalMs = 200) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(intervalMs);
    digitalWrite(LED_PIN, LOW);
    delay(intervalMs);
  }
}

// ==================== BACKEND COMMUNICATION ====================
void sendSensorData() {
  bool wifiOnline = (WiFi.status() == WL_CONNECTED);
  float temp = 0.0;
  float hum = 0.0;

  if (wifiOnline) {
    temp = readTemperature();
    hum = readHumidity();
  }

  Serial.println("\n========== Sending Sensor Data ==========");
  Serial.print("WiFi Online: ");
  Serial.println(wifiOnline ? "YES" : "NO");
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println(" C");
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.println(" %");

  WiFiClient client;
  String url = String(BACKEND_SENSOR_DATA);

  if (!client.connect(BACKEND_HOST, BACKEND_PORT)) {
    Serial.println("ERROR: Could not connect to backend for sensor data!");
    return;
  }

  String json = "{";
  json += "\"online\":" + String(wifiOnline ? "true" : "false") + ",";
  json += "\"temperature\":" + String(temp, 1) + ",";
  json += "\"humidity\":" + String(hum, 1);
  json += "}";

  client.print(String("POST ") + url + " HTTP/1.1\r\n");
  client.print(String("Host: ") + BACKEND_HOST + "\r\n");
  client.print("Content-Type: application/json\r\n");
  client.print("Connection: close\r\n");
  client.print("Content-Length: ");
  client.print(json.length());
  client.print("\r\n");
  client.print("\r\n");
  client.print(json);

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println("Sensor data upload timeout!");
      client.stop();
      return;
    }
    delay(10);
  }

  String response = "";
  while (client.available()) {
    response += (char)client.read();
  }

  if (response.indexOf("success") != -1) {
    Serial.println("Sensor data uploaded successfully!");
  } else {
    Serial.println("WARNING: Sensor data upload may have failed");
    Serial.print("Response: ");
    Serial.println(response.substring(0, 200));
  }

  client.stop();
}

void notifyBackendOrderComplete(String orderId) {
  Serial.print("Notifying backend order complete: ");
  Serial.println(orderId);

  WiFiClient client;
  String url = String(BACKEND_ORDER_COMPLETE) + orderId + "/complete";

  if (!client.connect(BACKEND_HOST, BACKEND_PORT)) {
    Serial.println("ERROR: Could not connect to backend!");
    return;
  }

  client.print(String("POST ") + url + " HTTP/1.1\r\n");
  client.print(String("Host: ") + BACKEND_HOST + "\r\n");
  client.print("Content-Type: application/json\r\n");
  client.print("Connection: close\r\n");
  client.print("\r\n");

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println("Backend response timeout!");
      client.stop();
      return;
    }
    delay(10);
  }

  String response = "";
  while (client.available()) {
    response += (char)client.read();
  }

  if (response.indexOf("success") != -1) {
    Serial.println("Backend confirmed order completion!");
  } else {
    Serial.println("WARNING: Backend may not have processed completion");
  }

  client.stop();
}

// ==================== DISPENSE HANDLER ====================
void handleDispense() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No JSON body\"}");
    return;
  }

  String body = server.arg("plain");
  Serial.println("\n========== Received Dispense Request ==========");
  Serial.println("Raw JSON: " + body);

  // Parse JSON
  String orderId = "";
  int compartment = 1;
  String fishName = "";
  int qty = 1;

  int idx = body.indexOf("\"orderId\"");
  if (idx != -1) {
    int start = body.indexOf("\"", idx + 9);
    int end = body.indexOf("\"", start + 1);
    if (start != -1 && end != -1) orderId = body.substring(start + 1, end);
  }

  idx = body.indexOf("\"compartment\"");
  if (idx != -1) {
    int colon = body.indexOf(":", idx + 13);
    if (colon != -1) compartment = body.substring(colon + 1).toInt();
  }

  idx = body.indexOf("\"fishName\"");
  if (idx != -1) {
    int start = body.indexOf("\"", idx + 11);
    int end = body.indexOf("\"", start + 1);
    if (start != -1 && end != -1) fishName = body.substring(start + 1, end);
  }

  idx = body.indexOf("\"qty\"");
  if (idx != -1) {
    int colon = body.indexOf(":", idx + 5);
    if (colon != -1) qty = body.substring(colon + 1).toInt();
  }

  Serial.println("Parsed Request:");
  Serial.print("  Order ID: ");
  Serial.println(orderId);
  Serial.print("  Compartment: ");
  Serial.println(compartment);
  Serial.print("  Fish Name: ");
  Serial.println(fishName);
  Serial.print("  Quantity: ");
  Serial.println(qty);

  currentOrderId = orderId;
  currentCompartment = compartment;
  isDispensing = true;

  // Send response immediately so backend doesn't timeout
  String response = "{\"status\":\"success\",\"message\":\"Request received\",\"orderId\":\"" + orderId + "\",\"compartment\":" + String(compartment) + ",\"fishName\":\"" + fishName + "\",\"qty\":" + String(qty) + "}";
  server.send(200, "application/json", response);

  Serial.println("Response sent to client");
  Serial.println("========================================\n");

  // Now do the physical dispensing work AFTER response is sent
  displayOrderReceived(orderId, fishName);
  playOrderPlacedSound();
  blinkLED(3, 200);

  // Dispense each fish one at a time (qty may be > 1)
  for (int i = 0; i < qty; i++) {
    displayDispensingItem(i + 1, qty, fishName);
    openCompartment(compartment);

    // Wait for fish to drop and confirm with LDR
    bool dropConfirmed = false;
    int attempts = 0;
    while (!dropConfirmed && attempts < 20) {
      if (readLDR()) {
        dropConfirmed = true;
        Serial.println("Fish drop confirmed by LDR!");
        break;
      }
      delay(100);
      attempts++;
    }

    if (!dropConfirmed) {
      Serial.println("WARNING: Fish drop not confirmed by LDR!");
    }

    delay(500);
    closeCompartment(compartment);

    if (i < qty - 1) {
      delay(500);
    }
  }

  displayDispensed();
  playCompletionSound();

  // Wait so user can see thank you message
  delay(3000);

  displayIdle();
  isDispensing = false;

  // Notify backend that dispensing is complete
  notifyBackendOrderComplete(orderId);
}

// ==================== STATUS HANDLER ====================
void handleStatus() {
  float temp = readTemperature();
  float hum = readHumidity();

  String response = "{";
  response += "\"machine\":\"online\",";
  response += "\"firmware\":\"1.0.0\",";
  response += "\"uptime\":\"" + String(millis() / 1000) + "s\",";
  response += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  response += "\"rssi\":" + String(WiFi.RSSI()) + ",";
  response += "\"free_heap\":" + String(ESP.getFreeHeap()) + ",";
  response += "\"temperature\":" + String(temp, 1) + ",";
  response += "\"humidity\":" + String(hum, 1) + ",";
  response += "\"is_dispensing\":" + String(isDispensing ? "true" : "false");
  response += "}";

  server.send(200, "application/json", response);
}

// ==================== 404 HANDLER ====================
void handleNotFound() {
  String message = "404 Not Found\n\n";
  message += "Available endpoints:\n";
  message += "  POST /dispense\n";
  message += "  GET  /status\n";
  message += "  POST /sensor-data\n";
  server.send(404, "text/plain", message);
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n");
  Serial.println("========================================");
  Serial.println("  Smart Fish Vending Machine ESP32");
  Serial.println("  Production Firmware v1.0.0");
  Serial.println("========================================");

  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Initialize LDR
  pinMode(LDR_PIN, INPUT);

  // Initialize DHT11
  dht.begin();
  Serial.println("DHT11 sensor initialized on GPIO " + String(DHT_PIN));
  Serial.println("Waiting for DHT11 to stabilize...");
  delay(2000); // DHT11 needs 1-2 seconds warmup after power-on

  // Initialize LCD
  initLCD();

  // Initialize servos
  initServos();

  // Connect to WiFi
  connectToWiFi();

  // Setup HTTP server routes
  server.on("/dispense", HTTP_POST, handleDispense);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/sensor-data", HTTP_POST, handleStatus); // Reuse status handler for now
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP Server started on port 80");
  Serial.println("Endpoints:");
  Serial.println("  POST /dispense");
  Serial.println("  GET  /status");
  Serial.println("  POST /sensor-data");
  Serial.println("========================================");
  Serial.println("Waiting for orders...\n");
}

// ==================== MAIN LOOP ====================
void loop() {
  // Maintain WiFi connection
  maintainWiFi();

  // Handle HTTP clients
  server.handleClient();

  // Send sensor data every 30 seconds
  unsigned long now = millis();
  if (now - lastSensorUpdate > SENSOR_UPDATE_INTERVAL) {
    lastSensorUpdate = now;
    sendSensorData();
  }

  // Small delay to prevent watchdog timeout
  delay(10);
}
