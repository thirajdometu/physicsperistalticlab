#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <AccelStepper.h>

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2); // Initialize the LCD with address 0x27, 16 columns, and 2 rows

// Pin definitions
const int dirPin = 9;                // Pin for controlling the direction of the stepper motor
const int stepPin = 10;              // Pin for sending step pulses to the stepper motor
const int enablePin = 8;             // Pin for enabling or disabling the stepper motor driver

// Motor setup
const int stepsPerRevolution = 400;  // Number of steps per full revolution (for 1/8 microstepping)
const int rpm = 20;                  // Speed of the stepper motor in Revolutions Per Minute
const int durationSeconds =35;      // Duration for which the motor will run, in seconds

// Create an AccelStepper object. The DRIVER mode is used because we're controlling the stepper with a driver.
AccelStepper stepper(AccelStepper::DRIVER, stepPin, dirPin);

// Variables for timing and control
unsigned long startTime; // To store the start time of motor run
bool isRunning = false;  // To check if the motor is currently running

// Function to run the stepper motor for a specific duration
void runStepperWithTimer(int durationSeconds) {
  // Set the maximum speed and acceleration for the stepper motor.
  stepper.setMaxSpeed(rpm * stepsPerRevolution / 60.0);
  stepper.setAcceleration(rpm * stepsPerRevolution / 60.0 / 2.0);

  // Enable the motor driver (typically, LOW enables it).
  digitalWrite(enablePin, LOW);

  // Calculate the total number of steps to move the motor for the given duration.
  long totalSteps = stepsPerRevolution * durationSeconds;

  // Set the current position as the starting point.
  stepper.setCurrentPosition(0);

  // Set the target position for the stepper motor.
  stepper.moveTo(totalSteps);

  // Start the timer (in milliseconds).
  startTime = millis();
  isRunning = true;

  // Loop to keep the motor running for the specified duration or until the motor reaches the target position.
  while (isRunning) {
    stepper.run();  // Continuously update the motor's position

    // Check elapsed time
    unsigned long elapsedTime = (millis() - startTime) / 1000; // Get elapsed time in seconds

    // Update the remaining time on the LCD every second
    if (elapsedTime <= durationSeconds) {
      lcd.setCursor(0, 1);
      int remainingTime = durationSeconds - elapsedTime;
      lcd.print("Timer: ");
      lcd.print(remainingTime);
      lcd.print("s ");
    } else {
      // If the timer has finished, stop the motor and exit the loop
      stepper.stop();
      isRunning = false;
    }
  }

  // Disable the motor driver (typically, HIGH disables it).
  digitalWrite(enablePin, HIGH);

  // Clear the LCD display and show the process is done.
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Process Done");
}

void setup() {
  // Initialize pin modes
  pinMode(dirPin, OUTPUT);    // Set direction pin as output
  pinMode(stepPin, OUTPUT);   // Set step pin as output
  pinMode(enablePin, OUTPUT); // Set enable pin as output

  // Initialize the LCD display
  lcd.begin(16, 2);  // Set up a 16x2 LCD
  lcd.backlight();   // Turn on the backlight of the LCD

  // Display RPM and Timer on the LCD
  lcd.setCursor(0, 0); // Set cursor to the first row
  lcd.print("RPM: ");
  lcd.print(rpm); // Display the RPM
  lcd.setCursor(0, 1); // Set cursor to the second row
  lcd.print("Timer: ");
  lcd.print(durationSeconds); // Display the timer seconds
  lcd.print("s");

  delay(2000); // Wait for 2 seconds to let the user see the RPM and Timer information

  // Set the initial position of the stepper motor to zero
  stepper.setCurrentPosition(0);

  // Change direction here by manually toggling the direction pin:
  digitalWrite(dirPin, HIGH);  // Set to HIGH for one direction (use LOW for the opposite direction)

  // Call the function to run the stepper motor for the specified duration
  runStepperWithTimer(durationSeconds);
}

void loop() {
  // The loop function is empty because we're only running the motor once in the setup function
}
