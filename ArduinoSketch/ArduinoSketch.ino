/**********************************************************************************************************************
Grace Digital Timeout Buster

This sketch was written to work with the Grace Digital GDI-IRBM20 streaming radio. The purpouse of this sketch and the 
acompanying hardware is to cycle through selected presets on the GD player to bypass the inactivity timer of the streaming
servive. The GD player supposrt various streaming services but I've only tested with the Sirius service.

For more information see - http://adambyers.com/2013/11/mailbox-notifier/ or ping adam@adambyers.com

Requires Nick Gammon's LED flasher library - http://www.gammon.com.au/forum/?id=11411
Requires Ken Shirriff's IR Remote library - http://www.righto.com/2009/08/multi-protocol-infrared-remote-library.html

**********************************************************************************************************************/

/*
Preset 1-5 IR Codes for Grade Digital GDI-IRBM20 Music Player 
P1 - 0x1FE807F
P2 - 0x1FE40BF
P3 - 0x1FEC03F
P4 - 0x1FE20DF
P5 - 0x1FEA05F
 */

#include <IRremote.h>
#include <LedFlasher.h> 

IRsend irsend;

//States
#define S_Boot 1
#define S_Test 2
#define S_PresetProgram 3
#define S_TimeProgram 4
#define S_Run 5

//PINs 
const int SelectSwitch = 10;
const int EnterSwitch = 11;

const int LED1 = 4; // Preset 1/1 hour indicator
const int LED2 = 5; // Preset 2/2 hour indicator
const int LED3 = 6; // Preset 3/3/hour indicator
const int LED4 = 7; // Preset 4/4 hour indicator
const int LED5 = 8; // Preset 5/5 hour indicator
const int LEDRun = 13; // LED to indicate when in test mode

LedFlasher LEDPreProg (2, 100, 200); // pin 2, off for 200 mS, on for 300 mS
LedFlasher LEDTimeProg (9, 100, 200); // pin 9, off for 200 mS, on for 300 mS

//Vars
int PresetCount = 0; // We need a minimum of 2 presets to flip between so we default to 2
int ButtonPressCount = 0;

unsigned long PresetDelay[6] = {0, 3600000, 7200000, 10800000, 14400000, 18000000}; // 1 hour, 2 hours, 3 hours, 4 hours, 5 hours
int PresetDelayIndex = 0;

unsigned long CurrentTime;

char RemoteCodes[6] = {0, 0x1FE807F, 0x1FE40BF, 0x1FEC03F, 0x1FE20DF, 0x1FEA05F};
int CodeIndex = 0;

bool SendP1 = true;
bool SendP2 = true;
bool SendP3 = true;
bool SendP4 = true;
bool SendP5 = true;

// Button debounce vars
int buttonState;
int lastButtonState = HIGH;
long lastDebounceTime = 0;
long debounceDelay = 10;

// Default start up state
int state = S_Boot;

void setup() { 
  
  pinMode(SelectSwitch, INPUT);
  pinMode(EnterSwitch, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(LED5, OUTPUT);
  pinMode(LEDRun, OUTPUT);
  
  digitalWrite(SelectSwitch, HIGH);
  digitalWrite(EnterSwitch, HIGH);
  
  LEDPreProg.begin();
  LEDTimeProg.begin();
  
}

void loop() {
  
  switch(state) {
    
    case S_Boot:
      F_Boot();
    break;
    
    case S_Test:
      F_Test();
    break;
    
    case S_PresetProgram:
      F_PresetProgram();
    break;
    
    case S_TimeProgram:
      F_TimeProgram();
    break;
    
    case S_Run:
      F_Run();
    break;
    
  }
}

//Functions

void F_Boot() {
  
  CurrentTime = millis();
  state = S_Test;
  
}

// In test mode we can use the Select Switch to cycle through the presets to make sure the IR is working
void F_Test() {
  
  // If we sit in test for longer than 5min we set some defualts and run 
  if (millis() - CurrentTime > 300000) {
    PresetCount = 2;
    PresetDelayIndex = 6;
    LEDPreProg.off(); // Stop blinking the LEDs
    LEDTimeProg.off(); // Stop blinking the LEDs
    
    // Found that I have to run "LEDpin.on();" before switching to another state so we can blink the LEDs in the next state 
    // LEDs don't blink when this is run, it just enables them to blink when "LEDpin.update();" is called in another state 
    LEDPreProg.on();
    LEDTimeProg.on();
    
    state = S_Run;  
  }
  
  LEDPreProg.update(); // Blink the Preset and Program LED to indicate we are in test mode 
  LEDTimeProg.update(); // Blink the Preset and Program LED to indicate we are in test mode
  
  // The momentary toggle switches used for Select and Enter are very bouncy and have to be debounced
  // Just debouncing and counting the number of times the Select Switch is clicked
  int reading = digitalRead(SelectSwitch);
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
        if (buttonState == LOW) { // Logic is inverted since we're using the internal resistors. When button is pressed the PIN goes LOW, when not pressed it's HIGH.
          ButtonPressCount++;
        }
     }
   }
  
  lastButtonState = reading;
  // Done debouncing Select Switch
  
  if (ButtonPressCount == 1) {
    digitalWrite(LED1, HIGH); // Turn on LED one to indicate Preset One should be on

    if (SendP1 == true) { // The  bool statments are used to toggle the irsend function on/off. Otherwise it just keeps transmiting the same code over and over
      irsend.sendNEC(0x1FE807F, 32);
      SendP1 = false;
    }
  }
  
  if (ButtonPressCount == 2) {
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, HIGH);
          
    if (SendP2 == true) {
      irsend.sendNEC(0x1FE40BF, 32);
      SendP2 = false;
    }
  }
        
  if (ButtonPressCount == 3) {
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, HIGH);
          
    if (SendP3 == true) {
      irsend.sendNEC(0x1FEC03F, 32);
      SendP3 = false;
    }
  }
        
  if (ButtonPressCount == 4) {
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, HIGH);
          
    if (SendP4 == true) {
      irsend.sendNEC(0x1FE20DF, 32);
      SendP4 = false;
    }
  }
        
  if (ButtonPressCount == 5) {
    digitalWrite(LED4, LOW);
    digitalWrite(LED5, HIGH);
          
    if (SendP5 == true) {
      irsend.sendNEC(0x1FEA05F, 32);
      SendP5 = false;
    }
  }
        
  if (ButtonPressCount == 6) {
    digitalWrite(LED5, LOW);
    ButtonPressCount = 0;
    SendP1 = true;
    SendP2 = true;
    SendP3 = true;
    SendP4 = true;
    SendP5 = true;
  }
  
  if (digitalRead(EnterSwitch) == LOW) {
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, LOW);
    digitalWrite(LED5, LOW);
    LEDPreProg.off(); // Stop blinking the Preset and Program LEDs
    LEDTimeProg.off(); // Stop blinking the Preset and Program LEDs
    ButtonPressCount = 0;
    delay(1500); // Crude debounce for the Enter Switch
    LEDPreProg.on();
    LEDTimeProg.on();    
    state = S_PresetProgram;
  }
}

/////////////////////////////////////////////////////////////////////////////////
  
void F_PresetProgram() {
  
  LEDPreProg.update(); // Blink to indicate we are in Preset Programing mode
  
   // Debouncing the and counting the Select switch
  int reading = digitalRead(SelectSwitch);
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
        if (buttonState == LOW) { // Logic is inverted since we're using the internal resistors. When button is pressed the PIN goes LOW, when not pressed it's HIGH.
          ButtonPressCount++;
          PresetCount++;
      }
    }
  }
  
  lastButtonState = reading;
  // Done debouncing
  
  if (PresetCount == 1) {
    digitalWrite(LED1, HIGH);
  }
        
  if (PresetCount == 2) {
    digitalWrite(LED2, HIGH);
  }
        
  if (PresetCount == 3) {
    digitalWrite(LED3, HIGH);
  }
        
  if (PresetCount == 4) {
    digitalWrite(LED4, HIGH);
  }
        
  if (PresetCount == 5) {
    digitalWrite(LED5, HIGH);
  }
  
  if (PresetCount == 6) {
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, LOW);
    digitalWrite(LED5, LOW);
    ButtonPressCount = 0;
    PresetCount = 0;
  }
  
  if (digitalRead(EnterSwitch) == LOW) {
    ButtonPressCount = 0;
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, LOW);
    digitalWrite(LED5, LOW);
    LEDPreProg.off();
    delay(1500);   
    LEDPreProg.on();
    state = S_TimeProgram;
  }
}

/////////////////////////////////////////////////////////////////////////////////

void F_TimeProgram() {
  
  LEDTimeProg.update();
  
  // Debouncing the and counting the Select switch
  int reading = digitalRead(SelectSwitch);
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
        if (buttonState == LOW) { // Logic is inverted since we're using the internal resistors. When button is pressed the PIN goes LOW, when not pressed it's HIGH.
          ButtonPressCount++;
          PresetDelayIndex++;
        }
      }
    }
  
  lastButtonState = reading;
  // Done debouncing
  
  if (PresetDelayIndex == 1) {
    digitalWrite(LED1, HIGH);
  }
  
  if (PresetDelayIndex == 2) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
  }
  
  if (PresetDelayIndex == 3) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, HIGH);
  }
  
  if (PresetDelayIndex == 4) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, HIGH);
    digitalWrite(LED4, HIGH);
  }
  
  if (PresetDelayIndex == 5) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, HIGH);
    digitalWrite(LED4, HIGH);
    digitalWrite(LED5, HIGH);
  }
  
  if (PresetDelayIndex == 6) {
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, LOW);
    digitalWrite(LED5, LOW);
    ButtonPressCount = 0;
    PresetDelayIndex = 0;
  }
  
  if (digitalRead(EnterSwitch) == LOW) {
    ButtonPressCount = 0;
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, LOW);
    digitalWrite(LED5, LOW);
    LEDTimeProg.off();
    delay(1500);
    LEDTimeProg.on();
    state = S_Run;
  } 
}

///////////////////////////////////////////////////////////

void F_Run() {
  
  digitalWrite(LEDRun, HIGH);
  
  if (PresetCount == 2) {
    irsend.sendNEC(0x1FE807F, 32); // P1
    digitalWrite(LED2, LOW);
    digitalWrite(LED1, HIGH);
    delay(PresetDelay[PresetDelayIndex]);
    irsend.sendNEC(0x1FE40BF, 32); // P2
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, HIGH);
    delay(PresetDelay[PresetDelayIndex]);
  }
  
  if (PresetCount == 3) {
    irsend.sendNEC(0x1FE807F, 32); // P1
    digitalWrite(LED3, LOW);
    digitalWrite(LED1, HIGH);
    delay(PresetDelay[PresetDelayIndex]);
    irsend.sendNEC(0x1FE40BF, 32); // P2
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, HIGH);
    delay(PresetDelay[PresetDelayIndex]);
    irsend.sendNEC(0x1FEC03F, 32); // P3
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, HIGH);
    delay(PresetDelay[PresetDelayIndex]);
  }
  
  if (PresetCount == 4) {
    irsend.sendNEC(0x1FE807F, 32); // P1
    digitalWrite(LED4, LOW);
    digitalWrite(LED1, HIGH);
    delay(PresetDelay[PresetDelayIndex]);
    irsend.sendNEC(0x1FE40BF, 32); // P2
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, HIGH);
    delay(PresetDelay[PresetDelayIndex]);
    irsend.sendNEC(0x1FEC03F, 32); // P3
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, HIGH);
    delay(PresetDelay[PresetDelayIndex]);
    irsend.sendNEC(0x1FE20DF, 32); // P4
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, HIGH);
    delay(PresetDelay[PresetDelayIndex]);
  }
  
  if (PresetCount == 5) {
    irsend.sendNEC(0x1FE807F, 32); // P1
    digitalWrite(LED5, LOW);
    digitalWrite(LED1, HIGH);
    delay(PresetDelay[PresetDelayIndex]);
    irsend.sendNEC(0x1FE40BF, 32); // P2
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, HIGH);
    delay(PresetDelay[PresetDelayIndex]);
    irsend.sendNEC(0x1FEC03F, 32); // P3
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, HIGH);
    delay(PresetDelay[PresetDelayIndex]);
    irsend.sendNEC(0x1FE20DF, 32); // P4
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, HIGH);
    delay(PresetDelay[PresetDelayIndex]);
    irsend.sendNEC(0x1FEA05F, 32); // P5
    digitalWrite(LED4, LOW);
    digitalWrite(LED5, HIGH);
    delay(PresetDelay[PresetDelayIndex]);
  }
}
