/**
 * @file web_server.cpp
 * @brief HTTP web server implementation
 */

#include "web_server.h"

WebServer::WebServer(int port, DispenserController* dispenser)
    : server(port), dispenser(dispenser), uptimeStart(millis()), deviceName(DEVICE_NAME) {
}

void WebServer::begin() {
    server.begin();
    DEBUG_PRINT("HTTP Server started on port ");
    DEBUG_PRINTLN(getPort());
    DEBUG_PRINT("Device name: ");
    DEBUG_PRINTLN(getDeviceName());
}

void WebServer::handleClient() {
    WiFiClient client = server.available();

    if (!client) {
        return;
    }

    DEBUG_PRINTLN("\n========== New HTTP Request ==========");

    // Wait for client to send data
    unsigned long timeout = millis();
    while (!client.available() && (millis() - timeout < HTTP_TIMEOUT_MS)) {
        delay(10);
    }

    if (!client.available()) {
        DEBUG_PRINTLN("Client timeout");
        client.stop();
        return;
    }

    // Read HTTP request
    String request = "";
    while (client.available()) {
        char c = client.read();
        request += c;

        // Prevent buffer overflow
        if (request.length() >= MAX_REQUEST_SIZE) {
            DEBUG_PRINTLN("Request too large!");
            break;
        }
    }

    DEBUG_PRINTLN("Request:");
    DEBUG_PRINTLN(request);

    // Parse request line
    String method, path;
    int firstSpace = request.indexOf(' ');
    int secondSpace = request.indexOf(' ', firstSpace + 1);

    if (firstSpace == -1 || secondSpace == -1) {
        DEBUG_PRINTLN("Invalid HTTP request");
        sendBadRequest(client, "Invalid HTTP request");
        client.stop();
        return;
    }

    method = request.substring(0, firstSpace);
    path = request.substring(firstSpace + 1, secondSpace);

    DEBUG_PRINT("Method: ");
    DEBUG_PRINTLN(method);
    DEBUG_PRINT("Path: ");
    DEBUG_PRINTLN(path);

    // Route request
    if (path == "/" || path == "/index.html") {
        handleRoot(client);
    } else if (path.equalsIgnoreCase("/dispense") && method == "POST") {
        // Extract JSON body
        int bodyStart = request.indexOf("\r\n\r\n");
        String body = "";
        if (bodyStart != -1) {
            body = request.substring(bodyStart + 4);
            body.trim();
        }
        handleDispense(client, body);
    } else if (path.equalsIgnoreCase("/status") && method == "GET") {
        handleStatus(client);
    } else {
        handleNotFound(client);
    }

    // Close connection
    delay(10);
    client.stop();
    DEBUG_PRINTLN("========== Request Complete ==========\n");
}

void WebServer::handleRoot(WiFiClient& client) {
    DEBUG_PRINTLN("Handling GET /");

    String html = "<!DOCTYPE html><html><head><title>";
    html += getDeviceName();
    html += "</title></head><body>";
    html += "<h1>";
    html += getDeviceName();
    html += "</h1>";
    html += "<p>Firmware Version: ";
    html += FIRMWARE_VERSION;
    html += "</p>";
    html += "<p>Status: ONLINE</p>";
    html += "<p>Uptime: ";
    html += getUptime();
    html += "</p>";
    html += "<p>IP: ";
    html += WiFi.localIP().toString();
    html += "</p>";
    html += "<p>Compartment: ";
    html += String(dispenser->getCurrentCompartment());
    html += "</p>";
    html += "</body></html>";

    sendOK(client, "text/html", html);
}

void WebServer::handleDispense(WiFiClient& client, const String& body) {
    DEBUG_PRINTLN("Handling POST /dispense");

    // Parse JSON
    String orderId, fishName;
    int compartment, qty;

    if (!parseJSON(body, orderId, compartment, fishName, qty)) {
        DEBUG_PRINTLN("Failed to parse JSON");
        sendBadRequest(client, "Invalid JSON format");
        return;
    }

    // Print received fields
    DEBUG_PRINTLN("Received dispense request:");
    DEBUG_PRINT("  Order ID: ");
    DEBUG_PRINTLN(orderId);
    DEBUG_PRINT("  Compartment: ");
    DEBUG_PRINTLN(compartment);
    DEBUG_PRINT("  Fish Name: ");
    DEBUG_PRINTLN(fishName);
    DEBUG_PRINT("  Quantity: ");
    DEBUG_PRINTLN(qty);

    // Execute dispensing
    bool success = dispenser->dispense(compartment, qty);

    // Send response
    String response = "{";
    response += "\"status\":\"success\",";
    response += "\"message\":\"Request received\",";
    response += "\"orderId\":\"" + orderId + "\",";
    response += "\"compartment\":" + String(compartment) + ",";
    response += "\"fishName\":\"" + fishName + "\",";
    response += "\"quantity\":" + String(qty) + ",";
    response += "\"dispensed\":" + String(success ? "true" : "false");
    response += "}";

    sendJSON(client, response);

    DEBUG_PRINTLN("Dispense response sent");
}

void WebServer::handleStatus(WiFiClient& client) {
    DEBUG_PRINTLN("Handling GET /status");

    String response = "{";
    response += "\"machine\":\"online\",";
    response += "\"firmware\":\"" FIRMWARE_VERSION "\",";
    response += "\"uptime\":\"" + getUptime() + "\",";
    response += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
    response += "\"rssi\":" + String(WiFi.RSSI()) + ",";
    response += "\"compartment\":" + String(dispenser->getCurrentCompartment()) + ",";
    response += "\"is_dispensing\":" + String(dispenser->getIsDispensing() ? "true" : "false") + ",";
    response += "\"free_heap\":" + String(ESP.getFreeHeap()) + ",";
    response += "\"uptime_ms\":" + String(millis());
    response += "}";

    sendJSON(client, response);
    DEBUG_PRINTLN("Status response sent");
}

void WebServer::handleNotFound(WiFiClient& client) {
    DEBUG_PRINTLN("Handling 404 Not Found");
    sendNotFound(client);
}

bool WebServer::parseJSON(const String& json, String& orderId, int& compartment,
                          String& fishName, int& qty) {
    // Simple JSON parsing without ArduinoJson library dependency
    // In production, use ArduinoJson for robust parsing

    DEBUG_PRINT("Parsing JSON: ");
    DEBUG_PRINTLN(json);

    // Extract orderId
    int orderIdStart = json.indexOf("\"orderId\"");
    if (orderIdStart == -1) return false;
    int orderIdValueStart = json.indexOf('"', orderIdStart + 9);
    int orderIdValueEnd = json.indexOf('"', orderIdValueStart + 1);
    if (orderIdValueStart == -1 || orderIdValueEnd == -1) return false;
    orderId = json.substring(orderIdValueStart + 1, orderIdValueEnd);

    // Extract compartment
    int compartmentStart = json.indexOf("\"compartment\"");
    if (compartmentStart == -1) return false;
    int compartmentValueStart = json.indexOf(':', compartmentStart + 13);
    if (compartmentValueStart == -1) return false;
    compartment = json.substring(compartmentValueStart + 1).toInt();

    // Extract fishName
    int fishNameStart = json.indexOf("\"fishName\"");
    if (fishNameStart == -1) return false;
    int fishNameValueStart = json.indexOf('"', fishNameStart + 11);
    int fishNameValueEnd = json.indexOf('"', fishNameValueStart + 1);
    if (fishNameValueStart == -1 || fishNameValueEnd == -1) return false;
    fishName = json.substring(fishNameValueStart + 1, fishNameValueEnd);

    // Extract qty
    int qtyStart = json.indexOf("\"qty\"");
    if (qtyStart == -1) return false;
    int qtyValueStart = json.indexOf(':', qtyStart + 6);
    if (qtyValueStart == -1) return false;
    qty = json.substring(qtyValueStart + 1).toInt();

    DEBUG_PRINT("Parsed -> orderId: ");
    DEBUG_PRINT(orderId);
    DEBUG_PRINT(", compartment: ");
    DEBUG_PRINT(compartment);
    DEBUG_PRINT(", fishName: ");
    DEBUG_PRINT(fishName);
    DEBUG_PRINT(", qty: ");
    DEBUG_PRINTLN(qty);

    return true;
}

String WebServer::getUptime() const {
    unsigned long uptimeMs = millis() - uptimeStart;

    unsigned long seconds = uptimeMs / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    unsigned long days = hours / 24;

    String uptime = "";
    if (days > 0) {
        uptime += String(days) + "d ";
    }
    uptime += String(hours % 24) + "h ";
    uptime += String(minutes % 60) + "m ";
    uptime += String(seconds % 60) + "s";

    return uptime;
}

void WebServer::sendOK(WiFiClient& client, const String& contentType, const String& body) {
    client.println("HTTP/1.1 200 OK");
    client.print("Content-Type: ");
    client.println(contentType);
    client.println("Connection: close");
    client.println("Access-Control-Allow-Origin: *");
    client.println();
    client.println(body);
}

void WebServer::sendNotFound(WiFiClient& client) {
    String body = "404 Not Found";
    client.println("HTTP/1.1 404 Not Found");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println(body);
}

void WebServer::sendBadRequest(WiFiClient& client, const String& message) {
    String body = "400 Bad Request: " + message;
    client.println("HTTP/1.1 400 Bad Request");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println(body);
}

void WebServer::sendInternalError(WiFiClient& client, const String& message) {
    String body = "500 Internal Server Error: " + message;
    client.println("HTTP/1.1 500 Internal Server Error");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println(body);
}

void WebServer::sendJSON(WiFiClient& client, const String& json) {
    sendOK(client, "application/json", json);
}

void WebServer::setDeviceName(const String& name) {
    deviceName = name;
}

String WebServer::getDeviceName() const {
    return deviceName;
}

int WebServer::getPort() const {
    return HTTP_PORT;
}
