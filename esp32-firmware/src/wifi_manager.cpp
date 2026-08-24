/**
 * @file wifi_manager.cpp
 * @brief WiFi manager implementation
 */

#include "wifi_manager.h"

WiFiManager::WiFiManager(const char* ssid, const char* password)
    : ssid(ssid), password(password), lastReconnectAttempt(0), connected(false) {
}

bool WiFiManager::connect() {
    DEBUG_PRINTLN("\n========================================");
    DEBUG_PRINTLN("Connecting to WiFi...");
    DEBUG_PRINT("SSID: ");
    DEBUG_PRINTLN(ssid);

    // Disconnect if already connected
    if (WiFi.status() == WL_CONNECTED) {
        WiFi.disconnect();
        delay(100);
    }

    // Start connection
    WiFi.begin(ssid, password);
    DEBUG_PRINT("Connecting");

    // Wait for connection with timeout
    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        DEBUG_PRINT(".");

        if (millis() - startAttempt > WIFI_TIMEOUT_MS) {
            DEBUG_PRINTLN("\nWiFi connection TIMEOUT!");
            DEBUG_PRINT("Status: ");
            DEBUG_PRINTLN(getStatusString());
            return false;
        }
    }

    connected = true;
    DEBUG_PRINTLN("\nWiFi connected successfully!");
    printInfo();
    return true;
}

void WiFiManager::maintain() {
    if (WiFi.status() != WL_CONNECTED) {
        if (connected) {
            DEBUG_PRINTLN("\nWiFi disconnected! Attempting to reconnect...");
            connected = false;
        }

        unsigned long now = millis();
        if (now - lastReconnectAttempt > WIFI_RECONNECT_INTERVAL_MS) {
            lastReconnectAttempt = now;
            DEBUG_PRINTLN("Reconnecting to WiFi...");
            connect();
        }
    } else if (!connected) {
        connected = true;
        DEBUG_PRINTLN("WiFi reconnected!");
        printInfo();
    }
}

bool WiFiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::getIPAddress() const {
    if (isConnected()) {
        return WiFi.localIP().toString();
    }
    return "0.0.0.0";
}

String WiFiManager::getStatusString() const {
    wl_status_t status = WiFi.status();
    switch (status) {
        case WL_CONNECTED: return "Connected";
        case WL_NO_SHIELD: return "No WiFi Shield";
        case WL_IDLE_STATUS: return "Idle";
        case WL_NO_SSID_AVAIL: return "SSID Not Found";
        case WL_SCAN_COMPLETED: return "Scan Completed";
        case WL_CONNECT_FAILED: return "Connection Failed";
        case WL_CONNECTION_LOST: return "Connection Lost";
        case WL_DISCONNECTED: return "Disconnected";
        default: return "Unknown";
    }
}

void WiFiManager::disconnect() {
    WiFi.disconnect();
    connected = false;
    DEBUG_PRINTLN("WiFi disconnected");
}

void WiFiManager::printInfo() const {
    DEBUG_PRINTLN("\n========== WiFi Info ==========");
    DEBUG_PRINT("SSID: ");
    DEBUG_PRINTLN(ssid);
    DEBUG_PRINT("Status: ");
    DEBUG_PRINTLN(getStatusString());
    DEBUG_PRINT("IP Address: ");
    DEBUG_PRINTLN(getIPAddress());
    DEBUG_PRINT("Signal Strength (RSSI): ");
    DEBUG_PRINT(WiFi.RSSI());
    DEBUG_PRINTLN(" dBm");
    DEBUG_PRINT("MAC Address: ");
    DEBUG_PRINTLN(WiFi.macAddress());
    DEBUG_PRINTLN("================================\n");
}
