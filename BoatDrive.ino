#include <Servo.h>

#define ESC_NULL 1480
#define ESC_MAX 2000
#define ESC_MIN 1000

Servo escLeft;
Servo escRight;

int feedPotentimeter = A0;
int steeringPotentimeter = A1;
int autoModePin = A2;
bool autoMode = false;

int feedValue = ESC_NULL;
int steeringValue = 0;
int escValue = ESC_NULL;
int escLeftValue = ESC_NULL;
int escRightValue = ESC_NULL;

int minFeedPotentimeterValue = 0;
int maxFeedPotentimeterValue = 1023;

int minSteeringPotentimeterValue = 0;
int maxSteeringPotentimeterValue = 1023;

int feedStep = 50;
int nullOffsetFeed = 49; //max is feedStep -1!
int nullOffsetSteering = 5;

void setup() {
  escLeft.attach(8);
  escRight.attach(9);
  escLeft.writeMicroseconds(ESC_NULL);
  escRight.writeMicroseconds(ESC_NULL);
  pinMode(autoModePin, INPUT_PULLUP);
  Serial.begin(9600);
  delay(5000);
}

void loop() {
  checkAutoMode();
  
  feedValue = map(getPotentimeterValue(feedPotentimeter),
                  minFeedPotentimeterValue, 
                  maxFeedPotentimeterValue, 
                  ESC_MIN, 
                  ESC_MAX);
  steeringValue = map(getPotentimeterValue(steeringPotentimeter),
                      minSteeringPotentimeterValue,
                      maxSteeringPotentimeterValue,
                      -50,
                      50);
  
  if(steeringValue >= 0) {
    steeringValue += 1;
  }
  else {
    steeringValue -= 1;
  }
  

  //printCurrentEscValues();

  if(autoMode) {
    //at autoMode forward will encrease the speed by 150, backward will decrease speed by 150
    if(feedValue > (ESC_MAX-200)) {
      escValue += 150;
      if(escValue > ESC_MAX) {
        escValue = ESC_MAX;
      }
    }
    if(feedValue < (ESC_MIN+200)) {
      escValue -= 150;
      if(escValue < ESC_MIN) {
        escValue = ESC_MIN;
      }
    }
    //wait for NULL-Position
    while(feedValue > (ESC_MAX-200) || feedValue < (ESC_MIN+200)) {
      feedValue = map(getPotentimeterValue(feedPotentimeter),
                    minFeedPotentimeterValue, 
                    maxFeedPotentimeterValue, 
                    ESC_MIN, 
                    ESC_MAX);
      delay(100);
    }
  }
  else {
    //slow down speed changes
    if(escValue < feedValue) {
      if((escValue + feedStep) < feedValue) {
        escValue += feedStep;
      }
      else {
        escValue = feedValue;
      }
    }
    if(escValue > feedValue) {
      if((escValue - feedStep) > feedValue) {
        escValue -= feedStep;
      }
      else {
        escValue = feedValue;
      }
    }
  
    //Nullpoint tollerance
    if(escValue < (ESC_NULL + nullOffsetFeed) && escValue > (ESC_NULL - nullOffsetFeed)) {
      escValue = ESC_NULL;
    }
  }
  if(abs(steeringValue) < nullOffsetSteering) {
    steeringValue = 0;
  }

  //Serial.println(steeringValue);

  escLeftValue = double(escValue) / 100.0 * (50 + (50.0 + steeringValue));
  escRightValue = double(escValue) / 100.0 * (50 +(50.0 - steeringValue));
  if(escLeftValue > ESC_MAX) {
    escRightValue -= (escLeftValue - ESC_MAX);
    escLeftValue = ESC_MAX;
  }
  if(escRightValue > ESC_MAX) {
    escLeftValue -= (escRightValue - ESC_MAX);
    escRightValue = ESC_MAX;
  }
  //driving forward (escValue > ESC_NULL) and steering should not run a Motor backwards!
  if(escValue >= ESC_NULL) {
    //don't turn backwards...
    if(escRightValue < ESC_NULL) {
      escRightValue = ESC_NULL;
    }
    //don't turn backwards...
    if(escLeftValue < ESC_NULL) {
      escLeftValue = ESC_NULL;
    }
  }
  
  escLeft.writeMicroseconds(escLeftValue);
  escRight.writeMicroseconds(escRightValue);

  if(true) {
    Serial.print(escValue);
    Serial.print(" L: ");
    Serial.print(escLeftValue);
    Serial.print(" R: ");
    Serial.println(escRightValue);
  }
  delay(100);
}

int getPotentimeterValue(int potentimeter) {
  int cnt = 7;
  int readValue;
  int value = 0;
  int minValue=1023;
  int maxValue=0;
  
  for(int i = 0; i < cnt; i++) {
    readValue = analogRead(potentimeter);
    if(readValue > maxValue) {
      maxValue = readValue;
    }
    if(readValue < minValue) {
      minValue = readValue;
    }
    value += readValue;
  }
  value -= minValue;
  value -= maxValue;
  if(value>0) {
    value = value / (cnt-2);
  }
  if(false) {
    Serial.print("Value Potentimeter: ");
    Serial.println(value);
  }
  return value;
}

int readMicroseconds(Servo servo) {
  return map(servo.read(), 0, 180, ESC_MIN, ESC_MAX);
}

void printCurrentEscValues() {
    Serial.print("left ESC: ");
    Serial.println(readMicroseconds(escLeft));
    Serial.print("right ESC: ");
    Serial.println(readMicroseconds(escLeft));
  }

void checkAutoMode() {
  if(digitalRead(autoModePin) == 0) {
    //debounce of pushbutton
    delay(100);
    if(digitalRead(autoModePin) == 0) {
      //check after 1 second so that motor control will not be blocked for 2 seconds at once...
      delay(1000);
      if(digitalRead(autoModePin) == 0) {
        delay(1000);
        //autoMode activated by pressing button for 2 seconds...
        if(digitalRead(autoModePin) == 0) {
          autoMode = !autoMode;
          Serial.print("AutoMode switched to :");
          Serial.println(autoMode);
        }
        while(digitalRead(autoModePin) == 0) {
          delay(100);
        }
      }
    }
  }
}
