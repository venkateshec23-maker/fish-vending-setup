/**
 * @file config.h
 * @brief Configuration constants for Smart Fish Vending Machine ESP32
 *
 * Hardware Pin Definitions and System Configuration
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==================== WiFi Configuration ====================
// Default values - override with environment variables or modify here
#ifndef WIFI_SSID
#define WIFI_SSID "YOUR_WIFI_SSID"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#endif

#define WIFI_TIMEOUT_MS 20000       // WiFi connection timeout (20 seconds)
#define WIFI_RECONNECT_INTERVAL_MS 5000  // Reconnect attempt interval

// ==================== HTTP Server Configuration ====================
#define HTTP_PORT 80
#define HTTP_TIMEOUT_MS 5000         // Client timeout
#define MAX_REQUEST_SIZE 1024        // Maximum HTTP request size
#define MAX_RESPONSE_SIZE 512        // Maximum HTTP response size

// ==================== Hardware Pin Configuration ====================
// Built-in LED (usually GPIO 2 on ESP32 dev boards)
#define LED_BUILTIN_PIN 2

// Stepper Motor Driver Pins (A4988 or similar)
#define STEPPER_STEP_PIN 18
#define STEPPER_DIR_PIN 19
#define STEPPER_ENABLE_PIN 5

// Servo Motor Pin (for door/compartment control)
#define SERVO_PIN 13

// Sensor Pins
#define SENSOR_IR_PIN 34            // IR sensor for fish detection
#define SENSOR_TEMP_PIN 35          // Temperature sensor
#define SENSOR_HUMIDITY_PIN 32      // Humidity sensor

// Buzzer for notifications
#define BUZZER_PIN 27

// ==================== Stepper Motor Configuration ====================
#define STEPS_PER_REVOLUTION 200     // For 1.8 degree stepper
#define MICROSTEPS 16                // Microstepping factor
#define COMPARTMENT_STEPS 50         // Steps to move between compartments
#define DISPENSE_DELAY_MS 1000       // Delay during dispensing

// ==================== JSON Configuration ====================
#define JSON_DOCUMENT_SIZE 256       // ArduinoJson document size

// ==================== Timing Configuration ====================
#define LED_BLINK_COUNT 3            // Number of LED blinks during dispensing
#define LED_BLINK_INTERVAL_MS 200    // LED on/off interval
#define DISPENSE_DURATION_MS 3000    // Total dispensing time

// ==================== MQTT Configuration (for future use) ====================
#define MQTT_BROKER "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_USERNAME "YOUR_ADAFRUIT_USERNAME"
#define MQTT_KEY "YOUR_ADAFRUIT_KEY"
#define MQTT_TOPIC "fish-vending/status"

// ==================== Version Information ====================
#define FIRMWARE_VERSION "1.0.0"
#define DEVICE_NAME "FishVendingESP32"

// ==================== Debug Configuration ====================
#define DEBUG_ENABLED 1
#if DEBUG_ENABLED
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(format, ...) Serial.printf(format, __VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(format, ...)
#endif

#endif // CONFIG_H
