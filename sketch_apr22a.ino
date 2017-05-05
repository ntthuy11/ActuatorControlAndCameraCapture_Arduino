#include <SoftwareSerial.h>  // used for lcd and Serial port 4
#include <Keypad.h>
//#include <PID_v1.h>

// my includes
#include "MyLCD.h"
#include "MyKeypad.h" 
#include "MyMenu.h" 
#include "MyUtil.h"


// pins
#define PWM_PIN_ACTUATOR_A         11   // orange
#define PWM_PIN_ACTUATOR_B         9    // green
#define PWM_PIN_ACTUATOR_C         2    // violet
#define PWM_PIN_ACTUATOR_D         6    // white 

#define DIRECTION_PIN_ACTUATOR_A   22   // yellow
#define DIRECTION_PIN_ACTUATOR_B   24   // blue
#define DIRECTION_PIN_ACTUATOR_C   26   // grey
#define DIRECTION_PIN_ACTUATOR_D   28   // brown

#define FEEDBACK_PIN_ACTUATOR_A    0    // analog A0
#define FEEDBACK_PIN_ACTUATOR_B    1    // analog A1
#define FEEDBACK_PIN_ACTUATOR_C    2    // analog A2
#define FEEDBACK_PIN_ACTUATOR_D    3    // analog A3

#define CAMERA_TRIGGER_PIN        53

#define RED_BUTTON_PIN            44

// actuator's setpoints
#define N_SETPOINTS                6
//int setpointActuator[N_SETPOINTS] = { 425, 460, 495, 530, 565, 600 }; // defined based on the feedback of actuator (425 -> 600)
int setpointActuator[N_SETPOINTS] = { 65, 173, 281, 389, 497, 605 };
//int setpointActuator[N_SETPOINTS] = { 70, 178, 286, 394, 502, 610 };


//
#define ACTUATOR_TO_USE            0 // 1:A , 2:B, 3:C, 4:D, 0: All
#define ACTUATOR_TO_REFER          3

#define FLUCTUATION_THR            5
#define DELAY_TO_SEE_BY_USER       1 // 500 ms

#define DELAY_TO_CAPTURE_IMG       1 // 2000 ms
#define PWM_SPEED                  255


//
int delayToCaptureImg              = DELAY_TO_CAPTURE_IMG;
int delayToCaptureImg_tmp          = 1;
int pwmSpeed                       = PWM_SPEED;
int pwmSpeed_tmp                   = 1;

boolean modeControlActuator        = true;
boolean modeSetting                = false;
boolean modeSetDelayToCaptureImg   = false;
boolean modeSetPWMSpeed            = false;

int currLCDposition                = -1;
  
  
// -------------------------------------------------------------------


void promptSetDelayToCaptureImg(int number) {
  if(currLCDposition == -1) {
    currLCDposition = LCD_LINE_3;
    delayToCaptureImg_tmp = number;
    lcdPrint(currLCDposition++, number);            
  } else {
    if(currLCDposition - LCD_LINE_3 < 4) { 
      delayToCaptureImg_tmp = delayToCaptureImg_tmp*10 + number;
      lcdPrint(currLCDposition++, number);
    }
  }  
}


void promptSetPWMSpeed(int number) {
  if(currLCDposition == -1) {
    currLCDposition = LCD_LINE_3;
    pwmSpeed_tmp = number;
    lcdPrint(currLCDposition++, number);            
  } else {
    if(currLCDposition - LCD_LINE_3 < 4) { 
      pwmSpeed_tmp = pwmSpeed_tmp*10 + number;
      lcdPrint(currLCDposition++, number);
    }
  }  
}


int findActuatorPosIdx(int pos) {
  for(int i = 0; i < N_SETPOINTS; i++) {
    if(abs(pos - setpointActuator[i]) < FLUCTUATION_THR)
      return i;
  }
  return 0; // if reach here, there is something wrong when getting feedback from the actuator
}


void moveToSetpointOneActuator(int directionPin, int directionPinValue, int pwmPin, int pwmSpeed, int feedbackPin, int expectedPosIdx) {
  digitalWrite(directionPin, directionPinValue);   
  analogWrite(pwmPin, pwmSpeed);
  
  // check when the actuator has to stop
  while(true) {
    int nextPos = analogRead(feedbackPin); // do everything first, before reading the feedback, to have the latest
    
    // for debug only
    lcdClear(); 
    lcdPrint(LCD_LINE_1, utilConvertIntToString(nextPos)); 
      
    if(setpointActuator[expectedPosIdx] == nextPos) {
      analogWrite(pwmPin, 0); // turn off PWM
      break;
    }
  }
}


void moveToSetpointAllActuators(int expectedPosIdx, int directionPinValue, int pwmSpeed) {
  digitalWrite(DIRECTION_PIN_ACTUATOR_A, directionPinValue);   
  digitalWrite(DIRECTION_PIN_ACTUATOR_B, directionPinValue);  
  digitalWrite(DIRECTION_PIN_ACTUATOR_C, directionPinValue); 
  digitalWrite(DIRECTION_PIN_ACTUATOR_D, directionPinValue); 
  analogWrite(PWM_PIN_ACTUATOR_A, pwmSpeed);
  analogWrite(PWM_PIN_ACTUATOR_B, pwmSpeed);
  analogWrite(PWM_PIN_ACTUATOR_C, pwmSpeed);
  analogWrite(PWM_PIN_ACTUATOR_D, pwmSpeed);
  
  // check when the actuator has to stop
  while(true) {  
    int nextPos = analogRead(FEEDBACK_PIN_ACTUATOR_A); // do everything first, before reading the feedback, to have the latest
    if (ACTUATOR_TO_REFER == 2)     nextPos = analogRead(FEEDBACK_PIN_ACTUATOR_B);
    else if(ACTUATOR_TO_REFER == 3) nextPos = analogRead(FEEDBACK_PIN_ACTUATOR_C);
    else if(ACTUATOR_TO_REFER == 4) nextPos = analogRead(FEEDBACK_PIN_ACTUATOR_D);
    
    // for debug only
    //lcdClear(); 
    //lcdPrint(LCD_LINE_1, "Current position:"); 
    //lcdPrint(LCD_LINE_2, utilConvertIntToString(nextPos)); 
      
    if(setpointActuator[expectedPosIdx] == nextPos) {
      analogWrite(PWM_PIN_ACTUATOR_A, 0); // turn off PWM
      analogWrite(PWM_PIN_ACTUATOR_B, 0);
      analogWrite(PWM_PIN_ACTUATOR_C, 0);
      analogWrite(PWM_PIN_ACTUATOR_D, 0);
      break;
    }
  }
}


void moveToSetpoint(int directionPinValue, int pwmSpeed, int expectedPosIdx) {
  if (ACTUATOR_TO_USE == 0) {
    moveToSetpointAllActuators(expectedPosIdx, directionPinValue, pwmSpeed);
  } else {
    if (ACTUATOR_TO_USE == 1)      moveToSetpointOneActuator(DIRECTION_PIN_ACTUATOR_A, directionPinValue, PWM_PIN_ACTUATOR_A, pwmSpeed, FEEDBACK_PIN_ACTUATOR_A, expectedPosIdx);
    else if (ACTUATOR_TO_USE == 2) moveToSetpointOneActuator(DIRECTION_PIN_ACTUATOR_B, directionPinValue, PWM_PIN_ACTUATOR_B, pwmSpeed, FEEDBACK_PIN_ACTUATOR_B, expectedPosIdx);
    else if (ACTUATOR_TO_USE == 3) moveToSetpointOneActuator(DIRECTION_PIN_ACTUATOR_C, directionPinValue, PWM_PIN_ACTUATOR_C, pwmSpeed, FEEDBACK_PIN_ACTUATOR_C, expectedPosIdx);
    else if (ACTUATOR_TO_USE == 4) moveToSetpointOneActuator(DIRECTION_PIN_ACTUATOR_D, directionPinValue, PWM_PIN_ACTUATOR_D, pwmSpeed, FEEDBACK_PIN_ACTUATOR_D, expectedPosIdx);
  }
}


void setupPinsForActuatorA() {
  pinMode(PWM_PIN_ACTUATOR_A,            OUTPUT);     
  pinMode(DIRECTION_PIN_ACTUATOR_A,      OUTPUT);    
  analogWrite(PWM_PIN_ACTUATOR_A,        LOW);     
  digitalWrite(DIRECTION_PIN_ACTUATOR_A, LOW);
}

void setupPinsForActuatorB() {
  pinMode(PWM_PIN_ACTUATOR_B,            OUTPUT);     
  pinMode(DIRECTION_PIN_ACTUATOR_B,      OUTPUT);    
  analogWrite(PWM_PIN_ACTUATOR_B,        LOW);     
  digitalWrite(DIRECTION_PIN_ACTUATOR_B, LOW);
}

void setupPinsForActuatorC() {
  pinMode(PWM_PIN_ACTUATOR_C,            OUTPUT);     
  pinMode(DIRECTION_PIN_ACTUATOR_C,      OUTPUT);    
  analogWrite(PWM_PIN_ACTUATOR_C,        LOW);     
  digitalWrite(DIRECTION_PIN_ACTUATOR_C, LOW);
}

void setupPinsForActuatorD() {
  pinMode(PWM_PIN_ACTUATOR_D,            OUTPUT);     
  pinMode(DIRECTION_PIN_ACTUATOR_D,      OUTPUT);    
  analogWrite(PWM_PIN_ACTUATOR_D,        LOW);     
  digitalWrite(DIRECTION_PIN_ACTUATOR_D, LOW);
}


void setup() {
  
  // --- setup LCD
  pinMode(LCD_PIN, OUTPUT); // setup
  lcdSerial.begin(9600); // lcdSerial is a SoftwareSerial
  lcdClear();  
  menuDisplayHome();
  
  
  // --- setup PWM and direction pins 
  if (ACTUATOR_TO_USE == 0) {
    setupPinsForActuatorA();
    setupPinsForActuatorB();
    setupPinsForActuatorC();
    setupPinsForActuatorD();
  } else {
    if (ACTUATOR_TO_USE == 1)      setupPinsForActuatorA();
    else if (ACTUATOR_TO_USE == 2) setupPinsForActuatorB();
    else if (ACTUATOR_TO_USE == 3) setupPinsForActuatorC();
    else if (ACTUATOR_TO_USE == 4) setupPinsForActuatorD();
  }
  
  pinMode(CAMERA_TRIGGER_PIN, OUTPUT); 
  
  // retract all actuators - for safety
  //moveToSetpoint(HIGH, pwmSpeed, 0);
}


void loop() {  
  int currPosIdx;
  if (ACTUATOR_TO_USE == 0) {
    if (ACTUATOR_TO_REFER == 1)      currPosIdx = findActuatorPosIdx(analogRead(FEEDBACK_PIN_ACTUATOR_A));
    else if (ACTUATOR_TO_REFER == 2) currPosIdx = findActuatorPosIdx(analogRead(FEEDBACK_PIN_ACTUATOR_B));
    else if (ACTUATOR_TO_REFER == 3) currPosIdx = findActuatorPosIdx(analogRead(FEEDBACK_PIN_ACTUATOR_C));
    else if (ACTUATOR_TO_REFER == 4) currPosIdx = findActuatorPosIdx(analogRead(FEEDBACK_PIN_ACTUATOR_D));
  } else {
    if (ACTUATOR_TO_USE == 1)      currPosIdx = findActuatorPosIdx(analogRead(FEEDBACK_PIN_ACTUATOR_A));
    else if (ACTUATOR_TO_USE == 2) currPosIdx = findActuatorPosIdx(analogRead(FEEDBACK_PIN_ACTUATOR_B));
    else if (ACTUATOR_TO_USE == 3) currPosIdx = findActuatorPosIdx(analogRead(FEEDBACK_PIN_ACTUATOR_C));
    else if (ACTUATOR_TO_USE == 4) currPosIdx = findActuatorPosIdx(analogRead(FEEDBACK_PIN_ACTUATOR_D));
  }
  
  
  // main run
  switch (keypadGetKey()) { 
    
    case '1': // ------ press button 1 ------
      if(modeSetting) {
        if(modeSetDelayToCaptureImg) {
          promptSetDelayToCaptureImg(1);       
        } else {
          if(modeSetPWMSpeed) {
            promptSetPWMSpeed(1);
          } else {
            lcdClear();
            lcdPrint(LCD_LINE_1, MSG_ENTER_PARAM_DELAY);
            lcdPrint(LCD_LINE_2 + 4, MSG_CURRENTLY);
            lcdPrint(LCD_LINE_2 + 15, delayToCaptureImg);
            lcdPrint(LCD_LINE_2 + 19, MSG_BRACKET);
            modeSetDelayToCaptureImg = true;
          }
        }
      }
      break; 
      
      
    case '2': // ------ press button 2 ------
      if(modeSetting) {
        if(modeSetDelayToCaptureImg) {
          promptSetDelayToCaptureImg(2);       
        } else {
          if(modeSetPWMSpeed) {
            promptSetPWMSpeed(2);       
          } else {
            lcdClear();
            lcdPrint(LCD_LINE_1, MSG_ENTER_PARAM_PWM_SPEED);
            lcdPrint(LCD_LINE_2 + 4, MSG_CURRENTLY);
            lcdPrint(LCD_LINE_2 + 15, pwmSpeed);
            lcdPrint(LCD_LINE_2 + 19, MSG_BRACKET);
            modeSetPWMSpeed = true;
          }
        }
      }
      break;      
      
      
    case '3': // ------ press button 3 ------
      if(modeSetting) {
        if(modeSetDelayToCaptureImg) {
          promptSetDelayToCaptureImg(3);       
        } else {
          if(modeSetPWMSpeed) {
            promptSetPWMSpeed(3);       
          }
        }
      }
      break;      
      
      
    case '4': // ------ press button 4 ------
      if(modeSetting) {
        if(modeSetDelayToCaptureImg) {
          promptSetDelayToCaptureImg(4);       
        } else {
          if(modeSetPWMSpeed) {
            promptSetPWMSpeed(4);       
          }
        }
      }
      break;  
  
  
    case '5': // ------ press button 5 ------
      if(modeSetting) {
        if(modeSetDelayToCaptureImg) {
          promptSetDelayToCaptureImg(5);       
        } else {
          if(modeSetPWMSpeed) {
            promptSetPWMSpeed(5);       
          }
        }
      }
      break;    
      
    
    case '6': // ------ press button 6 ------
      if(modeSetting) {
        if(modeSetDelayToCaptureImg) {
          promptSetDelayToCaptureImg(6);       
        } else {
          if(modeSetPWMSpeed) {
            promptSetPWMSpeed(6);       
          }
        }
      }
      break;
      
      
    case '7': // ------ press button 7 ------
      if(modeSetting) {
        if(modeSetDelayToCaptureImg) {
          promptSetDelayToCaptureImg(7);       
        } else {
          if(modeSetPWMSpeed) {
            promptSetPWMSpeed(7);       
          }
        }
      }
      break;
      
      
    case '8': // ------ press button 8 ------
      if(modeSetting) {
        if(modeSetDelayToCaptureImg) {
          promptSetDelayToCaptureImg(8);       
        } else {
          if(modeSetPWMSpeed) {
            promptSetPWMSpeed(8);       
          }
        }
      }
      break;
      
      
    case '9': // ------ press button 9 ------
      if(modeSetting) {
        if(modeSetDelayToCaptureImg) {
          promptSetDelayToCaptureImg(9);       
        } else {
          if(modeSetPWMSpeed) {
            promptSetPWMSpeed(9);       
          }
        }
      }
      break;
      
      
    case '0': // ------ press button 0 ------
      if(modeSetting) {
        if(modeSetDelayToCaptureImg) {
          promptSetDelayToCaptureImg(0);       
        } else {
          if(modeSetPWMSpeed) {
            promptSetPWMSpeed(0);       
          }
        }
      }
      break;      
      
      
    case CLEAR: // ------ press button CLEAR ------
    
      // reset settings
      modeControlActuator        = true;
      modeSetting                = false;
      modeSetDelayToCaptureImg   = false;
      modeSetPWMSpeed            = false;
      currLCDposition = -1;

      // fully retract the actuator
      lcdClear();
      menuDisplayClear();
      moveToSetpoint(HIGH, pwmSpeed, 0); // HIGH: retracting
      
      // display a msg
      lcdClear();
      menuDisplayHome();
      
      break;
      
      
    case SHIFT: // ------ press button 2ND ------
      lcdClear();
      menuDisplaySetting();
      modeControlActuator        = false;
      modeSetting                = true;
      modeSetDelayToCaptureImg   = false;
      modeSetPWMSpeed            = false;
      break;
      
     
    case HELP: // ------ press button HELP ------
      lcdClear();
      menuDisplayHelp();
      break;
      
    
    case UP: // ------ press button arrow UP ------
      if(modeControlActuator) {
        if(currPosIdx < N_SETPOINTS-1) {
          lcdClear();          lcdPrint(LCD_LINE_1, MSG_READY);    lcdPrint(LCD_LINE_2, utilConvertIntToString(currPosIdx+2));
          moveToSetpoint(LOW, pwmSpeed, currPosIdx+1); // extending
          lcdClear();          lcdPrint(LCD_LINE_1, MSG_TAKING_PICTURES);
          digitalWrite(CAMERA_TRIGGER_PIN, HIGH);                      
          delay(delayToCaptureImg); 
          digitalWrite(CAMERA_TRIGGER_PIN, LOW);          
          lcdClear();            lcdPrint(LCD_LINE_1, MSG_DONE);
        }
      }
      break;
    
    
    case DOWN: // ------ press button arrow DOWN ------
      if(modeControlActuator) {
        if(currPosIdx > 0) {      
          lcdClear();          lcdPrint(LCD_LINE_1, MSG_READY);    lcdPrint(LCD_LINE_2, utilConvertIntToString(currPosIdx));
          moveToSetpoint(HIGH, pwmSpeed, currPosIdx-1); // retracting
          lcdClear();          lcdPrint(LCD_LINE_1, MSG_TAKING_PICTURES); 
          digitalWrite(CAMERA_TRIGGER_PIN, HIGH);                  
          delay(delayToCaptureImg); 
          digitalWrite(CAMERA_TRIGGER_PIN, LOW);
          lcdClear();            lcdPrint(LCD_LINE_1, MSG_DONE);
        }
      }
      break;
      
      
    case ENTER: // ------ press button ENTER ------
      if(modeControlActuator) {
        digitalWrite(CAMERA_TRIGGER_PIN, HIGH);            
        delay(delayToCaptureImg); 
        digitalWrite(CAMERA_TRIGGER_PIN, LOW);
            
        if(currPosIdx == 0) {
          for(int i = currPosIdx+1; i <= N_SETPOINTS-1; i++) { // extending
            lcdClear();            lcdPrint(LCD_LINE_1, MSG_READY);    lcdPrint(LCD_LINE_2, utilConvertIntToString(i+1));    lcdPrint(LCD_LINE_3, MSG_OVER_6);
            moveToSetpoint(LOW, pwmSpeed, i);   
            lcdClear();            lcdPrint(LCD_LINE_1, MSG_TAKING_PICTURES);         
            digitalWrite(CAMERA_TRIGGER_PIN, HIGH);            
            delay(delayToCaptureImg); 
            digitalWrite(CAMERA_TRIGGER_PIN, LOW);
          }
        } else {
           for(int i = currPosIdx-1; i >= 0; i--) { // retracting
            lcdClear();            lcdPrint(LCD_LINE_1, MSG_READY);    lcdPrint(LCD_LINE_2, utilConvertIntToString(i+1));    lcdPrint(LCD_LINE_3, MSG_OVER_6);
            moveToSetpoint(HIGH, pwmSpeed, i);     
            lcdClear();            lcdPrint(LCD_LINE_1, MSG_TAKING_PICTURES);        
            digitalWrite(CAMERA_TRIGGER_PIN, HIGH);
            delay(delayToCaptureImg);  
            digitalWrite(CAMERA_TRIGGER_PIN, LOW);            
          } 
        }

        lcdClear();            lcdPrint(LCD_LINE_1, MSG_MOVING_BACK);
        moveToSetpoint(HIGH, pwmSpeed, 0); // HIGH: retracting
        lcdClear();            lcdPrint(LCD_LINE_1, MSG_DONE);
      } else {
        
         if( (modeSetting && modeSetDelayToCaptureImg) || (modeSetting && modeSetPWMSpeed) ) {  
           currLCDposition = -1;       
           lcdPrint(LCD_LINE_3, MSG_SAVED);
           delay(DELAY_TO_SEE_BY_USER);
           
           if (modeSetDelayToCaptureImg) {
             delayToCaptureImg        = delayToCaptureImg_tmp;
             delayToCaptureImg_tmp    = 1;
           } else {
             if (modeSetPWMSpeed) {
               pwmSpeed               = pwmSpeed_tmp;
               pwmSpeed_tmp           = 1;
             }
           }
           
           modeControlActuator        = true;
           modeSetting                = false;
           modeSetDelayToCaptureImg   = false;
      
           lcdClear();
           menuDisplayHome();
         }
      }
      break;
  }
}
