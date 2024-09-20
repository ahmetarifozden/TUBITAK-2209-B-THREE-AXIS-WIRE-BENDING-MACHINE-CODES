#include <AccelStepper.h>
#include <Servo.h>

#define LIMIT_SWITCH_PIN 11
#define SERVO_PIN 2
#define FEEDER_STEP_PIN 5
#define FEEDER_DIR_PIN 6
#define Z_AXIS_STEP_PIN 7
#define Z_AXIS_DIR_PIN 8
#define BENDER_STEP_PIN 9
#define BENDER_DIR_PIN 10

// Define the stepper motors and the pins they will use
AccelStepper feederStepper(1, FEEDER_STEP_PIN, FEEDER_DIR_PIN);
AccelStepper zAxisStepper(1, Z_AXIS_STEP_PIN, Z_AXIS_DIR_PIN);
AccelStepper benderStepper(1, BENDER_STEP_PIN, BENDER_DIR_PIN);

Servo servo01;

int count = 0;
int angleConst = 18;  // angle constant

void setup() {
  Serial.begin(9600);
  pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);
  
  // Initialize servo and stepper motors
  servo01.attach(SERVO_PIN);
  servo01.write(40);  // Initial position, bending pin up
  
  setupStepper(feederStepper, 2000);
  setupStepper(zAxisStepper, 2000);
  setupStepper(benderStepper, 2000);
  
  homing(benderStepper);
}

void loop() {
  String mode = Serial.readString();
  
  if (mode.startsWith("manual")) {
    manualMode();
  } else if (mode.startsWith("star")) {
    createShape("star");
  } else if (mode.startsWith("cube")) {
    createShape("cube");
  } else if (mode.startsWith("stand")) {
    createShape("stand");
  }
}

// Setup stepper motors
void setupStepper(AccelStepper& stepper, int maxSpeed) {
  stepper.setMaxSpeed(maxSpeed);
}

// Homing procedure
void homing(AccelStepper& benderStepper) {
  while (digitalRead(LIMIT_SWITCH_PIN) != LOW) {
    benderStepper.setSpeed(1200);
    benderStepper.runSpeed();
  }
  benderStepper.setCurrentPosition(0);
  moveStepperToPosition(benderStepper, -1400, 1200);
}

// General function to move stepper to position
void moveStepperToPosition(AccelStepper& stepper, int position, int speed) {
  stepper.setSpeed((position > 0) ? speed : -speed);
  while (stepper.currentPosition() != position) {
    stepper.run();
  }
  stepper.setCurrentPosition(0);
}

// Function for feeding wire
void feedWire(AccelStepper& stepper, int distance) {
  int feedSteps = distance * 48;  // convert distance to steps
  moveStepperToPosition(stepper, feedSteps, 1200);
}

// Function for bending wire
void bendWire(AccelStepper& benderStepper, Servo& servo, int angle, bool bendUp = true) {
  servo.write(bendUp ? 40 : 130);
  delay(200);
  moveStepperToPosition(benderStepper, angle * angleConst, bendUp ? -700 : 700);
  delay(100);
}

// Function to create different shapes
void createShape(const String& shape) {
  if (shape == "star") {
    while (count != 5) {
      feedWire(feederStepper, 38);
      bendWire(benderStepper, servo01, 52, true);
      feedWire(feederStepper, 38);
      bendWire(benderStepper, servo01, 42, false);
      count++;
    }
  } else if (shape == "cube") {
    // Simplified and reused steps for cube shape
    for (int i = 0; i < 3; i++) {
      feedWire(feederStepper, 40);
      bendWire(benderStepper, servo01, 90, true);
      bendWire(benderStepper, servo01, 90, false);
    }
    zAxisStepper.setSpeed(500);
    moveStepperToPosition(zAxisStepper, 88 * angleConst, 500);
  } else if (shape == "stand") {
    feedWire(feederStepper, 20);
    bendWire(benderStepper, servo01, 90, true);
    feedWire(feederStepper, 80);
    bendWire(benderStepper, servo01, 42, false);
  }
  count = 0;  // Reset the counter for the next shape
}

// Manual control mode
void manualMode() {
  Serial.println("  // MANUAL MODE //");
  String command = "";
  
  while (!command.startsWith("end")) {
    command = Serial.readString();
    if (command.startsWith("f")) {
      int distance = command.substring(1).toInt();
      Serial.print("Feed ");
      Serial.print(distance);
      Serial.println("mm wire.");
      feedWire(feederStepper, distance);
    }
    else if (command.startsWith("b")) {
      int angle = command.substring(1).toInt();
      Serial.print("Bend ");
      Serial.print(angle);
      Serial.println(" degrees.");
      bendWire(benderStepper, servo01, angle);
    }
  }
}
