/**
 * @file main.cpp
 * @brief Smart Fish Vending Machine ESP32 Firmware
 *
 * Main entry point for the ESP32 firmware.
 * Handles WiFi connection, HTTP server, and hardware control.
 */

#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "web_server.h"
#include "dispenser_controller.h"

// Global objects
WiFiManager* wifiManager;
WebServer* webServer;
DispenserController* dispenser;

// System state
unsigned long lastLoopTime;
unsigned long loopCount;

/**
 * @brief Setup function - runs once on boot
 */
void setup() {
    // Initialize Serial Monitor
    Serial.begin(115200);
    delay(1000);

    DEBUG_PRINTLN("\n\n");
    DEBUG_PRINTLN("========================================");
    DEBUG_PRINTLN("  Smart Fish Vending Machine ESP32");
    DEBUG_PRINTLN("  Firmware Version: " FIRMWARE_VERSION);
    DEBUG_PRINTLN("========================================");
    DEBUG_PRINTLN();

    // Initialize dispenser hardware
    dispenser = new DispenserController();
    dispenser->begin();

    // Initialize WiFi
    wifiManager = new WiFiManager(WIFI_SSID, WIFI_PASSWORD);
    bool wifiConnected = wifiManager->connect();

    if (!wifiConnected) {
        DEBUG_PRINTLN("WARNING: Running without WiFi connection");
        DEBUG_PRINTLN("Will attempt to reconnect in loop()");
    }

    // Initialize web server
    webServer = new WebServer(HTTP_PORT, dispenser);
    webServer->setDeviceName(DEVICE_NAME);
    webServer->begin();

    // Print startup info
    DEBUG_PRINTLN("\n========== System Ready ==========");
    DEBUG_PRINT("Firmware: ");
    DEBUG_PRINTLN(FIRMWARE_VERSION);
    DEBUG_PRINT("HTTP Server: http://");
    DEBUG_PRINT(wifiManager->getIPAddress());
    DEBUG_PRINT(":");
    DEBUG_PRINTLN(HTTP_PORT);
    DEBUG_PRINT("Endpoints:");
    DEBUG_PRINTLN("  GET  /status");
    DEBUG_PRINTLN("  POST /dispense");
    DEBUG_PRINTLN("==================================\n");

    // Initialize loop tracking
    lastLoopTime = millis();
    loopCount = 0;
}

/**
 * @brief Main loop - runs continuously
 */
void loop() {
    loopCount++;

    // Maintain WiFi connection
    wifiManager->maintain();

    // Handle HTTP requests
    webServer->handleClient();

    // Print status every 30 seconds
    static unsigned long lastStatusPrint = 0;
    unsigned long now = millis();
    if (now - lastStatusPrint > 30000) {
        lastStatusPrint = now;
        DEBUG_PRINTLN("\n--- Status Update ---");
        DEBUG_PRINT("Uptime: ");
        DEBUG_PRINTLN(webServer->getUptime());
        DEBUG_PRINT("Free Heap: ");
        DEBUG_PRINT(ESP.getFreeHeap());
        DEBUG_PRINTLN(" bytes");
        DEBUG_PRINT("WiFi: ");
        DEBUG_PRINTLN(wifiManager->isConnected() ? "Connected" : "Disconnected");
        DEBUG_PRINT("IP: ");
        DEBUG_PRINTLN(wifiManager->getIPAddress());
        DEBUG_PRINT("Dispensing: ");
        DEBUG_PRINTLN(dispenser->getIsDispensing() ? "YES" : "NO");
        DEBUG_PRINTLN("-------------------\n");
    }

    // Small delay to prevent watchdog timeout
    delay(10);
}

/**
 * @brief System reset function (for emergency use)
 */
void restartSystem() {
    DEBUG_PRINTLN("Restarting system...");
    ESP.restart();
}
