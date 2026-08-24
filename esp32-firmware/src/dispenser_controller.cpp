/**
 * @file dispenser_controller.cpp
 * @brief Hardware control implementation
 */

#include "dispenser_controller.h"

DispenserController::DispenserController()
    : ledPin(LED_BUILTIN_PIN),
      ledState(false),
      stepperStepPin(STEPPER_STEP_PIN),
      stepperDirPin(STEPPER_DIR_PIN),
      stepperEnablePin(STEPPER_ENABLE_PIN),
      currentCompartment(1),
      stepperEnabled(false),
      sensorIRPin(SENSOR_IR_PIN),
      lastDispenseTime(0),
      isDispensing(false) {
}

void DispenserController::begin() {
    DEBUG_PRINTLN("Initializing Dispenser Controller...");

    // Initialize LED
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);
    ledState = false;

    // Initialize stepper motor pins
    pinMode(stepperStepPin, OUTPUT);
    pinMode(stepperDirPin, OUTPUT);
    pinMode(stepperEnablePin, OUTPUT);

    // Disable stepper by default
    digitalWrite(stepperEnablePin, HIGH);  // HIGH = disabled for A4988
    stepperEnabled = false;

    // Initialize sensors
    pinMode(sensorIRPin, INPUT);

    // Initialize buzzer
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    DEBUG_PRINTLN("Dispenser Controller initialized");
    DEBUG_PRINT("  LED Pin: ");
    DEBUG_PRINTLN(ledPin);
    DEBUG_PRINT("  Stepper Step Pin: ");
    DEBUG_PRINTLN(stepperStepPin);
    DEBUG_PRINT("  Stepper Dir Pin: ");
    DEBUG_PRINTLN(stepperDirPin);
    DEBUG_PRINT("  Stepper Enable Pin: ");
    DEBUG_PRINTLN(stepperEnablePin);
}

void DispenserController::blinkLED(int count) {
    DEBUG_PRINT("Blinking LED ");
    DEBUG_PRINT(count);
    DEBUG_PRINTLN(" times...");

    for (int i = 0; i < count; i++) {
        digitalWrite(ledPin, HIGH);
        delay(LED_BLINK_INTERVAL_MS);
        digitalWrite(ledPin, LOW);
        delay(LED_BLINK_INTERVAL_MS);
    }

    ledState = false;
    DEBUG_PRINTLN("LED blink complete");
}

void DispenserController::moveStepper(int steps, bool direction) {
    if (!stepperEnabled) {
        digitalWrite(stepperEnablePin, LOW);  // Enable stepper
        stepperEnabled = true;
        delayMicroseconds(100);  // Wait for driver to enable
    }

    digitalWrite(stepperDirPin, direction ? HIGH : LOW);

    DEBUG_PRINT("Moving stepper ");
    DEBUG_PRINT(steps);
    DEBUG_PRINT(" steps ");
    DEBUG_PRINTLN(direction ? "forward" : "backward");

    for (int i = 0; i < steps; i++) {
        stepMotor();
        stepDelay();
    }

    // Disable stepper after movement
    digitalWrite(stepperEnablePin, HIGH);
    stepperEnabled = false;
}

void DispenserController::stepMotor() {
    digitalWrite(stepperStepPin, HIGH);
    delayMicroseconds(2);
    digitalWrite(stepperStepPin, LOW);
    delayMicroseconds(2);
}

void DispenserController::stepDelay() const {
    delayMicroseconds(1000);  // Adjust for speed
}

void DispenserController::moveToCompartment(int compartment) {
    if (compartment < 1 || compartment > 4) {
        DEBUG_PRINTLN("Invalid compartment number");
        return;
    }

    DEBUG_PRINT("Moving to compartment ");
    DEBUG_PRINTLN(compartment);

    int targetSteps = (compartment - 1) * COMPARTMENT_STEPS;
    int currentSteps = (currentCompartment - 1) * COMPARTMENT_STEPS;
    int stepsToMove = abs(targetSteps - currentSteps);

    bool direction = targetSteps > currentSteps;
    moveStepper(stepsToMove, direction);

    currentCompartment = compartment;
    DEBUG_PRINT("Now at compartment ");
    DEBUG_PRINTLN(currentCompartment);
}

bool DispenserController::dispense(int compartment, int quantity) {
    if (isDispensing) {
        DEBUG_PRINTLN("Already dispensing! Please wait.");
        return false;
    }

    if (compartment < 1 || compartment > 4) {
        DEBUG_PRINTLN("Invalid compartment");
        return false;
    }

    if (quantity <= 0 || quantity > 10) {
        DEBUG_PRINTLN("Invalid quantity");
        return false;
    }

    DEBUG_PRINT("Starting dispense: Compartment ");
    DEBUG_PRINT(compartment);
    DEBUG_PRINT(", Quantity ");
    DEBUG_PRINTLN(quantity);

    isDispensing = true;
    lastDispenseTime = millis();

    // Move to correct compartment
    moveToCompartment(compartment);

    // Blink LED to indicate dispensing
    blinkLED(LED_BLINK_COUNT);

    // Play buzzer
    playBuzzer(300);

    // Simulate dispensing delay
    delay(DISPENSE_DURATION_MS);

    // Verify fish was dispensed using IR sensor
    bool fishDetected = readIRSensor();
    DEBUG_PRINT("Fish detected: ");
    DEBUG_PRINTLN(fishDetected ? "YES" : "NO");

    isDispensing = false;
    DEBUG_PRINTLN("Dispensing complete!");

    return true;
}

bool DispenserController::readIRSensor() const {
    int sensorValue = digitalRead(sensorIRPin);
    DEBUG_PRINT("IR Sensor Value: ");
    DEBUG_PRINTLN(sensorValue);
    return sensorValue == LOW;  // Assuming LOW = detected
}

float DispenserController::readTemperature() const {
    // Placeholder for actual temperature reading
    // Use proper temperature sensor library in production
    int sensorValue = analogRead(SENSOR_TEMP_PIN);
    float voltage = sensorValue * (3.3 / 4095.0);
    float temperature = voltage * 100.0;  // Placeholder conversion
    return temperature;
}

float DispenserController::readHumidity() const {
    // Placeholder for actual humidity reading
    // Use proper humidity sensor library in production
    int sensorValue = analogRead(SENSOR_HUMIDITY_PIN);
    float humidity = map(sensorValue, 0, 4095, 0, 100);
    return humidity;
}

void DispenserController::playBuzzer(unsigned long durationMs) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(durationMs);
    digitalWrite(BUZZER_PIN, LOW);
    DEBUG_PRINT("Buzzer played for ");
    DEBUG_PRINT(durationMs);
    DEBUG_PRINTLN("ms");
}

bool DispenserController::getIsDispensing() const {
    return isDispensing;
}

int DispenserController::getCurrentCompartment() const {
    return currentCompartment;
}

void DispenserController::emergencyStop() {
    DEBUG_PRINTLN("EMERGENCY STOP!");

    // Disable stepper
    digitalWrite(stepperEnablePin, HIGH);
    stepperEnabled = false;

    // Turn off buzzer
    digitalWrite(BUZZER_PIN, LOW);

    // Blink LED rapidly
    for (int i = 0; i < 10; i++) {
        digitalWrite(ledPin, HIGH);
        delay(50);
        digitalWrite(ledPin, LOW);
        delay(50);
    }

    isDispensing = false;
    DEBUG_PRINTLN("Emergency stop complete");
}

String DispenserController::getStatus() const {
    String status = "{";
    status += "\"is_dispensing\":" + String(isDispensing ? "true" : "false") + ",";
    status += "\"compartment\":" + String(currentCompartment) + ",";
    status += "\"led_state\":" + String(ledState ? "true" : "false") + ",";
    status += "\"stepper_enabled\":" + String(stepperEnabled ? "true" : "false") + ",";
    status += "\"ir_sensor\":" + String(readIRSensor() ? "true" : "false");
    status += "}";
    return status;
}
