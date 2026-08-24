/**
 * @file dispenser_controller.h
 * @brief Hardware control for fish vending machine dispenser
 */

#ifndef DISPENSER_CONTROLLER_H
#define DISPENSER_CONTROLLER_H

#include <Arduino.h>
#include "config.h"

/**
 * @brief Dispenser Controller Class
 * Controls stepper motor, servo, LEDs, and sensors
 */
class DispenserController {
private:
    // LED control state
    int ledPin;
    bool ledState;

    // Stepper motor state
    int stepperStepPin;
    int stepperDirPin;
    int stepperEnablePin;
    int currentCompartment;
    bool stepperEnabled;

    // Sensor state
    int sensorIRPin;

    // Timing
    unsigned long lastDispenseTime;
    bool isDispensing;

public:
    /**
     * @brief Constructor
     */
    DispenserController();

    /**
     * @brief Initialize all hardware components
     */
    void begin();

    /**
     * @brief Blink LED N times
     * @param count Number of blinks
     */
    void blinkLED(int count = LED_BLINK_COUNT);

    /**
     * @brief Move stepper motor
     * @param steps Number of steps to move
     * @param direction Direction (true = forward, false = backward)
     */
    void moveStepper(int steps, bool direction);

    /**
     * @brief Move to specific compartment
     * @param compartment Compartment number (1-4)
     */
    void moveToCompartment(int compartment);

    /**
     * @brief Dispense fish from compartment
     * @param compartment Compartment number
     * @param quantity Quantity to dispense
     * @return true if dispensing started successfully
     */
    bool dispense(int compartment, int quantity);

    /**
     * @brief Read IR sensor (fish detection)
     * @return true if fish detected
     */
    bool readIRSensor() const;

    /**
     * @brief Read temperature
     * @return Temperature in Celsius
     */
    float readTemperature() const;

    /**
     * @brief Read humidity
     * @return Humidity percentage
     */
    float readHumidity() const;

    /**
     * @brief Play buzzer notification
     * @param durationMs Duration in milliseconds
     */
    void playBuzzer(unsigned long durationMs = 500);

    /**
     * @brief Check if currently dispensing
     * @return true if dispensing
     */
    bool getIsDispensing() const;

    /**
     * @brief Get current compartment
     * @return Current compartment number
     */
    int getCurrentCompartment() const;

    /**
     * @brief Emergency stop
     */
    void emergencyStop();

    /**
     * @brief Get system status
     * @return Status string
     */
    String getStatus() const;

private:
    /**
     * @brief Single step for stepper motor
     */
    void stepMotor();

    /**
     * @brief Delay between steps
     */
    void stepDelay() const;
};

#endif // DISPENSER_CONTROLLER_H
