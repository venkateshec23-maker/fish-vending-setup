/**
 * @file web_server.h
 * @brief HTTP web server for ESP32 API endpoints
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "dispenser_controller.h"

/**
 * @brief Web Server Class
 * Handles HTTP requests and API endpoints
 */
class WebServer {
private:
    WiFiServer server;
    DispenserController* dispenser;
    unsigned long uptimeStart;
    String deviceName;

    // HTTP response helpers
    void sendOK(WiFiClient& client, const String& contentType, const String& body);
    void sendNotFound(WiFiClient& client);
    void sendBadRequest(WiFiClient& client, const String& message);
    void sendInternalError(WiFiClient& client, const String& message);
    void sendJSON(WiFiClient& client, const String& json);

    // Request handlers
    void handleRoot(WiFiClient& client);
    void handleDispense(WiFiClient& client, const String& body);
    void handleStatus(WiFiClient& client);
    void handleNotFound(WiFiClient& client);

    // JSON parsing
    bool parseJSON(const String& json, String& orderId, int& compartment,
                   String& fishName, int& qty);

    // Uptime calculation
    String getUptime() const;

public:
    /**
     * @brief Constructor
     * @param port HTTP port number
     * @param dispenser Pointer to dispenser controller
     */
    WebServer(int port, DispenserController* dispenser);

    /**
     * @brief Initialize web server
     */
    void begin();

    /**
     * @brief Handle incoming client connections
     * Call this in loop()
     */
    void handleClient();

    /**
     * @brief Get server port
     * @return Port number
     */
    int getPort() const;

    /**
     * @brief Set device name
     * @param name Device name string
     */
    void setDeviceName(const String& name);

    /**
     * @brief Get device name
     * @return Device name string
     */
    String getDeviceName() const;
};

#endif // WEB_SERVER_H
