#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <AccelStepper.h>

#define MICROSTEPPING_FACTOR 4 // Set to 4 for 1/4 microstepping, set 8 for 1/8
#define PUL_PIN 10       // TB6600 Pulse pin
#define DIR_PIN 9        // TB6600 Direction pin
#define ENA_PIN 8        // TB6600 Enable pin

#define ENCODER_CLK 5    // Rotary Encoder CLK pin
#define ENCODER_DT 6     // Rotary Encoder DT pin
#define ENCODER_SW 7     // Rotary Encoder button pin

LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust 0x27 to your LCD's I2C address
AccelStepper stepper(AccelStepper::DRIVER, PUL_PIN, DIR_PIN);

float flowRate = 10.0;           // Initial flow rate in arbitrary units
int runTime = 10;                // Initial run time in seconds
bool isSettingFlowRate = true;   // Toggle between Flow rate and Time setting
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
      // Short press: Toggle between Flow Rate and Time settings if motor is not running
      if (!isRunning) {
        isSettingFlowRate = !isSettingFlowRate;
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
      if (isSettingFlowRate) {
        flowRate = constrain(flowRate + 1.0, 1.0, 100.0); // Adjust flow rate, range 1-100
      } else {
        runTime = constrain(runTime + 1, 1, 600); // Adjust Time, range 1-600 sec
      }
    } else {
      if (isSettingFlowRate) {
        flowRate = constrain(flowRate - 1.0, 1.0, 100.0); // Adjust flow rate, range 1-100
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
  lcd.print("FlowRate: ");
  lcd.print(flowRate);
  if (isSettingFlowRate) lcd.print(" <"); // Highlight Flow rate setting mode

  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  lcd.print(runTime);
  lcd.print("s");
  if (!isSettingFlowRate) lcd.print(" <"); // Highlight Time setting mode
}

void startMotor() {
  // Use the calibration equation to calculate RPM based on flow rate
  float rpm = 1.0484 * flowRate - 1.153;

  // Convert RPM to steps per second
  float stepsPerSecond = rpm * 200.0 / 60.0 * MICROSTEPPING_FACTOR;
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

