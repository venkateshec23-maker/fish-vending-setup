/**
 * @file wifi_manager.h
 * @brief WiFi connection management for ESP32
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"

/**
 * @brief WiFi Manager Class
 * Handles WiFi connection, reconnection, and status monitoring
 */
class WiFiManager {
private:
    const char* ssid;
    const char* password;
    unsigned long lastReconnectAttempt;
    bool connected;

public:
    /**
     * @brief Constructor
     * @param ssid WiFi SSID
     * @param password WiFi password
     */
    WiFiManager(const char* ssid, const char* password);

    /**
     * @brief Initialize WiFi connection
     * @return true if connected successfully
     */
    bool connect();

    /**
     * @brief Check and maintain WiFi connection
     * Call this in loop() for auto-reconnection
     */
    void maintain();

    /**
     * @brief Check if WiFi is connected
     * @return true if connected
     */
    bool isConnected() const;

    /**
     * @brief Get current IP address
     * @return IP address string
     */
    String getIPAddress() const;

    /**
     * @brief Get connection status string
     * @return Status description
     */
    String getStatusString() const;

    /**
     * @brief Disconnect WiFi
     */
    void disconnect();

    /**
     * @brief Print connection info to Serial
     */
    void printInfo() const;
};

#endif // WIFI_MANAGER_H
