//This Arduino code for this peristaltic pump experiment uses a microstepper in mode 4 (1/4).

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <AccelStepper.h>

#define MICROSTEPPING_FACTOR 4 // Set to 4 for 1/4 microstepping, set 8 for 1/8
#define PUL_PIN 10       // TB6600 Pulse pin
#define DIR_PIN 9       // TB6600 Direction pin
#define ENA_PIN 8       // TB6600 Enable pin

#define ENCODER_CLK 5   // Rotary Encoder CLK pin
#define ENCODER_DT 6    // Rotary Encoder DT pin
#define ENCODER_SW 7    // Rotary Encoder button pin

LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust 0x27 to your LCD's I2C address
AccelStepper stepper(AccelStepper::DRIVER, PUL_PIN, DIR_PIN);

int rpm = 30;            // Initial RPM
int runTime = 10;        // Initial run time in seconds
bool isSettingRPM = true; // Toggle between RPM and Time setting
bool isRunning = false;
unsigned long startTime;
unsigned long buttonPressTime = 0; // Track button press time
bool buttonPressed = false; // Flag to detect button press

void setup() {
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  
  pinMode(ENA_PIN, OUTPUT);
  digitalWrite(ENA_PIN, LOW); // Enable motor

  lcd.init();
  lcd.backlight();
  
  stepper.setAcceleration(500); // Set acceleration
  
  displaySettings();
}

void loop() {
  readEncoder();

  if (digitalRead(ENCODER_SW) == LOW) { // Encoder button pressed
    if (!buttonPressed) { // Only register press once
      buttonPressed = true;
      buttonPressTime = millis();
    }

    // Check if button is pressed for more than 1 second
    if (millis() - buttonPressTime > 1000 && !isRunning) {
      startMotor(); // Start motor if button is held for 1 second
    }
  } else {
    if (buttonPressed) { // Button released, reset the flag
      buttonPressed = false;
      // Short press: Toggle between RPM and Time settings if motor is not running
      if (!isRunning) {
        isSettingRPM = !isSettingRPM;
        displaySettings();
      }
    }
  }

  if (isRunning) {
    stepper.runSpeed(); // Ensure continuous motor movement

    unsigned long elapsedTime = (millis() - startTime) / 1000;
    if (elapsedTime >= runTime) { // Stop the motor when runtime is reached
      stopMotor();
    }
  }
}

void readEncoder() {
  static int lastStateCLK = HIGH;
  int stateCLK = digitalRead(ENCODER_CLK);
  
  if (stateCLK != lastStateCLK && stateCLK == LOW) {
    if (digitalRead(ENCODER_DT) != stateCLK) {
      if (isSettingRPM) {
        rpm = constrain(rpm + 1, 1, 100); // Adjust RPM, range 10-300
      } else {
        runTime = constrain(runTime + 1, 1, 600); // Adjust Time, range 1-600 sec
      }
    } else {
      if (isSettingRPM) {
        rpm = constrain(rpm - 1, 1, 100); // Adjust RPM, range 10-300
      } else {
        runTime = constrain(runTime - 1, 1, 600); // Adjust Time, range 1-600 sec
      }
    }
    displaySettings();
  }
  
  lastStateCLK = stateCLK;
}

void displaySettings() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RPM: ");
  lcd.print(rpm);
  if (isSettingRPM) lcd.print(" <"); // Highlight RPM setting mode

  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  lcd.print(runTime);
  lcd.print("s");
  if (!isSettingRPM) lcd.print(" <"); // Highlight Time setting mode
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

