// Pin connections



#include <Wire.h>
#include <Adafruit_VL53L0X.h>


// Create two sensor objects
Adafruit_VL53L0X sensor1 = Adafruit_VL53L0X();
Adafruit_VL53L0X sensor2 = Adafruit_VL53L0X();

int xpos = 0;
int ypos = 0;

int closeTime = 115;
int openTime = 150;


const int M2EnablePin = 25;

int switchTime = 1000;

const int XSHUT1 = 32;  
const int XSHUT2 = 33; 


#define ADDR1 0x30
#define ADDR2 0x31

const int dirPin = 5;  // Direction pin
const int stepPin = 4; // Step pin

int moveConstAmount = 100;
int calibrationConstant = 80;
int calibrationConstant1 = 0;
int moveAmount = 10;
int adjustmentFactor = 400;

const int stepDelay = 700;    // Delay between steps in microseconds

void processCommand(const String &cmdLine);

void setDistance(int pos);
void setHeight (int yPosition);
void checkHeight();
void equalizeHeight();
void disableM2();
void EnableM2();
void setCalibrationConstant();

void scoopAtPosition(int xpos, int ypos);

void slideClose();
void slideOpen();

void verticalDown(int miliseconds);
void verticalUp(int miliseconds);

void allowCommands();
void conveyorForward(long steps);
void horizontalOut(int steps);

void horizontalIn(int steps);
void equalizeHeights();
int enablePin = 14;
int enablePinConveyor = 27;

int motorAPin = 17; // Pin for Motor A
int motorBPin = 16; // Pin for Motor B

int motorCPin = 26
; // Pin for Motor A
int motorDPin = 13; // Pin for Motor B

String inputLine = "";



void setup() {
  pinMode(XSHUT1, OUTPUT);
pinMode(XSHUT2, OUTPUT);
digitalWrite(XSHUT1, LOW);
digitalWrite(XSHUT2, LOW);
delay(10); // sensors held in reset
    Serial.begin(115200);

   
    Serial.println("Setup started");

   Wire.begin();  
   Wire.setClock(50000);  // 50 kHz

  digitalWrite(XSHUT1, HIGH);
  delay(100);
  if (!sensor1.begin(0x29, &Wire)) {
    Serial.println("Failed to boot sensor 1");
    while (1);
  }
  sensor1.setAddress(ADDR1);
  delay(10);

  // Turn on sensor2 and set new address
  digitalWrite(XSHUT2, HIGH);
  delay(100);
  if (!sensor2.begin(0x29, &Wire)) {
    Serial.println("Failed to boot sensor 2");
    while (1);
  }
  sensor2.setAddress(ADDR2);
  delay(10);

  Serial.println("Both sensors initialized.");


  pinMode(M2EnablePin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
    pinMode(enablePinConveyor, OUTPUT);

  pinMode(motorAPin, OUTPUT); // Set motor A pin as output
  pinMode(motorBPin, OUTPUT); // Set motor B pin as output
  pinMode(motorCPin, OUTPUT); // Set motor A pin as output
  pinMode(motorDPin, OUTPUT); // Set motor B pin as output


  Serial.println(F("Ready. Commands:"));
  Serial.println(F("  verticalUp <ms>"));
  Serial.println(F("  verticalDown <ms>"));
  Serial.println(F("  slideOpen"));
  Serial.println(F("  slideClose"));
  Serial.println(F("  horizontalOut <ms>"));
  Serial.println();
  digitalWrite(enablePin, HIGH);
  digitalWrite(enablePinConveyor, HIGH);
  digitalWrite(M2EnablePin, LOW);
  delay(1000);
  
}
  void loop(){
allowCommands();

}

void setCalibrationConstant(int newValue) {
  calibrationConstant = newValue;
  Serial.print("Calibration constant updated to: ");
  Serial.println(calibrationConstant);
}

void disableM2(){
  digitalWrite(M2EnablePin,LOW);
  
  }

  void enableM2(){
  digitalWrite(M2EnablePin,HIGH);
  
  }
 
void allowCommands() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == '\r') return;  // Ignore carriage return
    if (c == '\n') {
      processCommand(inputLine);
      inputLine = "";
    } else {
      inputLine += c;
    }
  }
}

void checkHeight() {
  long totalA = 0;
  long totalB = 0;
  int countA = 0;
  int countB = 0;

  for (int i = 0; i < 10; i++) {
    VL53L0X_RangingMeasurementData_t measure;

    // Sensor 1
    sensor1.rangingTest(&measure, false);
  
    if (measure.RangeStatus != 4) {
 
      totalA += measure.RangeMilliMeter;
      countA++;
    } else {
     
    }

    // Sensor 2
    sensor2.rangingTest(&measure, false);
   
    if (measure.RangeStatus != 4) {
      
      totalB += measure.RangeMilliMeter;
      countB++;
    } else {
      
    }

    delay(100);
  }

  if (countA > 0) {
    int averageA = totalA / countA;
    Serial.print("Average Sensor 1: ");
    Serial.print(averageA);
    Serial.println(" mm");
  } else {
    Serial.println("Sensor 1: All measurements out of range");
  }

  if (countB > 0) {
    int averageB = totalB / countB;
    Serial.print("Average Sensor 2: ");
    Serial.print(averageB);
    Serial.println(" mm");
  } else {
    Serial.println("Sensor 2: All measurements out of range");
  }
}
void setHeight(int yPosition){
  int toggleSliderNeeded = 0;
  
  const int maxDuration = 29000;
  const int checkInterval = 100;
  const int maxSamples = 4;

  int readings1[maxSamples] = {0}; // Sensor 1
  int readings2[maxSamples] = {0}; // Sensor 2
  int readingIndex = 0;
  int validCount = 0;

  unsigned long startTime = millis();

  Serial.print("Starting dual real-time height adjustment to: ");
  Serial.println(yPosition);

  bool done1 = false;
  bool done2 = false;

  while (millis() - startTime < maxDuration) {
    VL53L0X_RangingMeasurementData_t measure;
    int s1 = -1;
    int s2 = -1;

    /// Sensor 1 with retry
bool success1 = false;
for (int i = 0; i < 3; i++) {
  sensor1.rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    s1 = measure.RangeMilliMeter + calibrationConstant1;
    success1 = true;
    break;
  }
  delay(10);
}

// Sensor 2 with retry
bool success2 = false;
for (int i = 0; i < 3; i++) {
  sensor2.rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    s2 = measure.RangeMilliMeter + calibrationConstant;
    success2 = true;
    break;
  }
  delay(10);
}

if (!success1 || !success2) {
  Serial.println("Sensor retry failed. Skipping this loop.");
  delay(checkInterval);
  continue;
}

    if (s1 == -1 || s2 == -1) {
      Serial.println("One or both sensors failed. Skipping this loop.");
      delay(checkInterval);
      continue;
    }

    readings1[readingIndex] = s1;
    readings2[readingIndex] = s2;
    readingIndex = (readingIndex + 1) % maxSamples;
    if (validCount < maxSamples) validCount++;

    // Compute moving averages
    long sum1 = 0, sum2 = 0;
    for (int i = 0; i < validCount; i++) {
      sum1 += readings1[i];
      sum2 += readings2[i];
    }
    int avg1 = sum1 / validCount;
    int avg2 = sum2 / validCount;

    int diff1 = yPosition - avg1;
    int diff2 = yPosition - avg2;

    Serial.print("Avg Sensor 1: "); Serial.print(avg1);
    Serial.print(" | Avg Sensor 2: "); Serial.print(avg2);
    Serial.print(" | Diff1: "); Serial.print(diff1);
    Serial.print(" | Diff2: "); Serial.println(diff2);

    // Direction-aware pause logic
    bool pause1 = false;
    bool pause2 = false;
    int imbalance = avg1 - avg2;

    if (abs(imbalance) > 10) {
      if (diff1 > 0 && diff2 > 0) { // both going up
        if (imbalance < 0) pause2 = true; // sensor2 is ahead
        else pause1 = true;               // sensor1 is ahead
      } else if (diff1 < 0 && diff2 < 0) { // both going down
        if (imbalance < 0) pause1 = true; // sensor1 is lower (ahead)
        else pause2 = true;               // sensor2 is lower (ahead)
      }
    }

    // Sensor 1 control
    if (!done1 && !pause1) {
      if (diff1 > 0) {
        digitalWrite(motorCPin, LOW);
        digitalWrite(motorDPin, HIGH);
        if (avg1 >= yPosition - 1) {
          done1 = true;
          digitalWrite(motorCPin, LOW);
          digitalWrite(motorDPin, LOW);
          Serial.println("Sensor 1 reached target (up).");
        }
      } else {
        digitalWrite(motorCPin, HIGH);
        digitalWrite(motorDPin, LOW);
        if (avg1 <= yPosition + 1) {
          done1 = true;
          digitalWrite(motorCPin, LOW);
          digitalWrite(motorDPin, LOW);
          Serial.println("Sensor 1 reached target (down).");
        }
      }
    } else if (pause1) {
      digitalWrite(motorCPin, LOW);
      digitalWrite(motorDPin, LOW);
    }

    // Sensor 2 control
    if (!done2 && !pause2) {
      if (diff2 > 0) {
        digitalWrite(motorAPin, LOW);
        digitalWrite(motorBPin, HIGH);
          if (toggleSliderNeeded) {
    Serial.println("Toggling M2 for 0.5s...");
    enableM2();       // assumed to activate motor A/B
    delay(switchTime);
    disableM2();      // deactivates motor A/B
    toggleSliderNeeded = 0;
  }

        if (avg2 >= yPosition - 1) {
          done2 = true;
          digitalWrite(motorAPin, LOW);
          digitalWrite(motorBPin, LOW);
          Serial.println("Sensor 2 reached target (up).");
        }
      } else {
        digitalWrite(motorAPin, HIGH);
        digitalWrite(motorBPin, LOW);
           if (toggleSliderNeeded) {
    Serial.println("Toggling M2 for 0.5s...");
    enableM2();       // assumed to activate motor A/B
    delay(switchTime*3);
    disableM2();      // deactivates motor A/B
    toggleSliderNeeded = 0;
  }
        if (avg2 <= yPosition + 1) {
          done2 = true;
          digitalWrite(motorAPin, LOW);
          digitalWrite(motorBPin, LOW);
          Serial.println("Sensor 2 reached target (down).");
        }
      }
    } else if (pause2) {
      digitalWrite(motorAPin, LOW);
      digitalWrite(motorBPin, LOW);
    }

    if (done1 && done2) {
      Serial.println("Both heights reached target. Motors stopped.");
      break;
    }

    delay(checkInterval);
  }

  // Final motor shutdown
  digitalWrite(motorAPin, LOW);
  digitalWrite(motorBPin, LOW);
  digitalWrite(motorCPin, LOW);
  digitalWrite(motorDPin, LOW);

  Serial.println("Height adjustment complete.");
}





void equalizeHeights() {
 
}

void conveyorForward(long steps) {
   digitalWrite(dirPin, LOW);
  digitalWrite(enablePinConveyor, LOW);   // enable (active-low on most drivers)
  for (long i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay*5);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay*5);
  }
  digitalWrite(enablePinConveyor, HIGH);  // disable
}

void conveyorBackward(long steps) {
   digitalWrite(dirPin, HIGH);
  digitalWrite(enablePinConveyor, LOW);
  for (long i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay*5);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay*5);
  }
  digitalWrite(enablePinConveyor, HIGH);
}



void scoopAtPosition(int xpos, int ypos){
  
  setDistance(xpos+adjustmentFactor);
  slideOpen();
  setHeight(ypos);
  setDistance(xpos);
  slideClose();
  setHeight(ypos + 70);
  
  }


void setDistance(int pos) {
  int delta = pos - xpos;

  Serial.print("Current X Position: ");
  Serial.println(xpos);
  Serial.print("Target X Position: ");
  Serial.println(pos);
  Serial.print("Delta: ");
  Serial.println(delta);

  if (delta == 0) {
    Serial.println("Already at target position. No movement needed.");
    return;
  }

  if (delta > 0) {
    Serial.print("Moving Out by ");
    Serial.print(delta);
    Serial.println(" steps.");
    horizontalOut(delta);
  } else {
    Serial.print("Moving In by ");
    Serial.print(-delta);
    Serial.println(" steps.");
    horizontalIn(-delta);
  }

  xpos = pos;  // Update global position
}



void processCommand(const String &cmdLine) {
  if (cmdLine.length() == 0) return;

  Serial.print(F("> "));
  Serial.println(cmdLine);

  // Split the command and arguments
  int spaceIdx = cmdLine.indexOf(' ');
  String cmd = (spaceIdx < 0) ? cmdLine : cmdLine.substring(0, spaceIdx);
  String args = (spaceIdx < 0) ? "" : cmdLine.substring(spaceIdx + 1);

  // Split first and second argument
  int secondSpaceIdx = args.indexOf(' ');
  String arg1 = (secondSpaceIdx < 0) ? args : args.substring(0, secondSpaceIdx);
  String arg2 = (secondSpaceIdx < 0) ? "" : args.substring(secondSpaceIdx + 1);

  unsigned long t = arg1.toInt();        // First argument: number
  int b = arg2.toInt();            // Second argument: boolean (non-zero = true)

  // Command handler
  if (cmd.equalsIgnoreCase("verticalUp")) {
    verticalUp(t);
  }
  else if (cmd.equalsIgnoreCase("verticalDown")) {
    verticalDown(t);
  }
  else if (cmd.equalsIgnoreCase("equalizeHeights")) {
    equalizeHeights();
  }
  else if (cmd.equalsIgnoreCase("enableM2")) {
    enableM2();
  }
  else if (cmd.equalsIgnoreCase("disableM2")) {
    disableM2();
  }
  else if (cmd.equalsIgnoreCase("slideOpen")) {
    enableM2();
  
    delay(2000);
   
    slideOpen();
    
  }
  else if (cmd.equalsIgnoreCase("setHeight")) {
    setHeight(t);  // Now supports second argument
  }
  else if (cmd.equalsIgnoreCase("setSliderTiming")) {
    openTime = t;
    closeTime = b;
  }
  else if (cmd.equalsIgnoreCase("setAdjustmentFactor")) {
   adjustmentFactor = t;
  }
  else if (cmd.equalsIgnoreCase("setDistance")) {
    setDistance(t);
  }
  else if (cmd.equalsIgnoreCase("setCalibrationConstant")) {
    setCalibrationConstant(t);
  }
  else if (cmd.equalsIgnoreCase("setCalibrationConstant1")) {
    calibrationConstant1 = t;
  }
  else if (cmd.equalsIgnoreCase("slideClose")) {
    enableM2();
    delay(500);
    slideClose();
  } else if (cmd.equalsIgnoreCase("fullSequence")) {
    scoopAtPosition(70,275);
    
    dropOff(330);
    setHeight(320);
    scoopAtPosition(5600,275);
    dropOff(340);
    setHeight(320);
    setDistance(0);
    setHeight(320);
    conveyorForward(1000);
    scoopAtPosition(70,275);
        setHeight(320);
    dropOff(340);
    scoopAtPosition(5600,275);
    dropOff(340);
    setDistance(0);
    setHeight(320);
    
  } else if (cmd.equalsIgnoreCase("scoopAtPosition")) {
    scoopAtPosition(t,b);
  }
  else if (cmd.equalsIgnoreCase("horizontalOut")) {
    horizontalOut(t);
  }
    else if (cmd.equalsIgnoreCase("conveyorBackward")) {
    conveyorBackward(t);
  }
    else if (cmd.equalsIgnoreCase("conveyorForward")) {
    conveyorForward(t);
  }
  else if (cmd.equalsIgnoreCase("horizontalIn")) {
    horizontalIn(t);
  }
  else if (cmd.equalsIgnoreCase("dropOff")) {
    dropOff(t);
  }
  else {
    Serial.print(F("Unknown command: "));
    Serial.println(cmd);
  }
}

   
  void horizontalOut(int steps){
    xpos = xpos + steps;
    digitalWrite(enablePin, LOW);
    digitalWrite(dirPin, LOW);
    for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
    
  
  }
    
     digitalWrite(enablePin, HIGH);  // Disable motor after motion
}
void dropOff(int ypos){
  setDistance(8500);
    setHeight(ypos);
    setDistance(10550);
    delay(200);
    slideOpen();
    delay(200);
   
    
    verticalUp(4000);
   
    slideClose();
    setDistance(8000);
    setHeight(ypos);
  
  }
 void horizontalIn(int steps) {
  xpos = xpos - steps;
  digitalWrite(enablePin, LOW);   // Enable motor
  digitalWrite(dirPin, HIGH);     // Set direction

  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
  }

  digitalWrite(enablePin, HIGH);  // Disable motor after motion
}

void verticalDown(int miliseconds){
 
        
        checkHeight();
        digitalWrite(motorAPin, HIGH);
        digitalWrite(motorBPin, LOW);
         digitalWrite(motorCPin, HIGH);
        digitalWrite(motorDPin, LOW);
        delay(miliseconds);
        digitalWrite(motorAPin, LOW);
        digitalWrite(motorBPin, LOW);
         digitalWrite(motorCPin, LOW);
        digitalWrite(motorDPin, LOW);
        checkHeight();}



void verticalDownPartial(int miliseconds){
 
        
        checkHeight();
        digitalWrite(motorAPin, HIGH);
        digitalWrite(motorBPin, LOW);
       
        delay(miliseconds);
        digitalWrite(motorAPin, LOW);
        digitalWrite(motorBPin, LOW);
        
        checkHeight();}
        
void verticalUpPartial(int miliseconds){
 
        
        checkHeight();
        digitalWrite(motorAPin, LOW);
        digitalWrite(motorBPin, HIGH);
        delay(miliseconds);
        digitalWrite(motorAPin, LOW);
        digitalWrite(motorBPin, LOW);
        checkHeight();
        }

void verticalUp(int miliseconds){
 
        
        checkHeight();
        digitalWrite(motorAPin, LOW);
        digitalWrite(motorBPin, HIGH);
        digitalWrite(motorCPin, LOW);
        digitalWrite(motorDPin, HIGH);
        delay(miliseconds);
        digitalWrite(motorAPin, LOW);
        digitalWrite(motorBPin, LOW);
        digitalWrite(motorCPin, LOW);
        digitalWrite(motorDPin, LOW);
        checkHeight();
        }

  void slideClose(){
        enableM2();
        // Activate the motor for 0.5 seconds
       
        digitalWrite(motorBPin, HIGH);
        digitalWrite(motorAPin, LOW);
        delay(closeTime); // Wait for 0.5 seconds

        // Stop the motor
        digitalWrite(motorBPin, LOW);
        digitalWrite(motorAPin, LOW);
        disableM2();
        digitalWrite(motorBPin, LOW);
        digitalWrite(motorAPin, HIGH);
        delay(closeTime); // Wait for 0.5 seconds

        // Stop the motor
        digitalWrite(motorBPin, LOW);
        digitalWrite(motorAPin, LOW);
        }
        
     void slideOpen(){
        enableM2();
        // Activate the motor for 0.5 seconds
        digitalWrite(motorBPin, LOW);
        digitalWrite(motorAPin, HIGH);
        delay(openTime); // Wait for 0.5 seconds for

        // Stop the motor
        digitalWrite(motorBPin, LOW);
        digitalWrite(motorAPin, LOW);
        disableM2();
         digitalWrite(motorBPin, HIGH);
        digitalWrite(motorAPin, LOW);
        delay(openTime); // Wait for 0.5 seconds

        // Stop the motor
        digitalWrite(motorBPin, LOW);
        digitalWrite(motorAPin, LOW);

}
