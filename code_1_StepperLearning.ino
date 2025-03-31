#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <AccelStepper.h>

#define MICROSTEPPING_FACTOR 4 // Set to 4 for 1/4 microstepping, set 8 for 1/8
#define PUL_PIN 10       // TB6600 Pulse pin
#define DIR_PIN 9        // TB6600 Direction pin
#define ENA_PIN 8        // TB6600 Enable pin

LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust 0x27 to your LCD's I2C address
AccelStepper stepper(AccelStepper::DRIVER, PUL_PIN, DIR_PIN);

int rpm = 20;            // Initial RPM
int runTime = 30;        // Initial run time in seconds
bool isRunning = false;
unsigned long startTime;

void setup() {
  pinMode(ENA_PIN, OUTPUT);
  digitalWrite(ENA_PIN, LOW); // Enable motor

  lcd.init();
  lcd.backlight();
  
  stepper.setAcceleration(500); // Set acceleration

  // Start the motor immediately after setup
  startMotor();
}

void loop() {
  if (isRunning) {
    stepper.runSpeed(); // Ensure continuous motor movement

    unsigned long elapsedTime = (millis() - startTime) / 1000;
    if (elapsedTime >= runTime) { // Stop the motor when runtime is reached
      stopMotor();
    }
  }
}

void startMotor() {
  float stepsPerSecond = rpm * 200.0 / 60.0 * MICROSTEPPING_FACTOR; // 200 steps per revolution for the stepper
  stepper.setMaxSpeed(stepsPerSecond);
  stepper.setSpeed(stepsPerSecond); // Set speed for runSpeed()

  startTime = millis();
  isRunning = true;
  lcd.setCursor(0, 1);
  lcd.print("Running...     ");
}

void stopMotor() {
  isRunning = false;
  stepper.stop();
  lcd.setCursor(0, 1);
  lcd.print("Finished       ");
}
