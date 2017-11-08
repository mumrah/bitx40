#include <Arduino.h>
#include "VFO.h"

#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>

Encoder myEnc(5, 6);

void setup() {
  pinMode(4, INPUT_PULLUP);
  Serial.begin(9600);
  Serial.println("Basic Encoder Test:");
}

long oldPosition  = -999;
long last_turn;
byte lastButton = 1;
bool button_down = false;
long double_click_time;

long freq = DEFAULT_FREQ;

void printFreq(long freq) {

  char buf[7], out[9];
  memset(buf, 0, 7);
  memset(out, 0, 9);
  ultoa(freq, buf, DEC);

  strcat(out, "");
  out[0] = buf[0];
  strcat(out, ".");
  strncat(out, &buf[1], 3);
  strcat(out, ".");
  strncat(out, &buf[4], 3);
  Serial.print(out); Serial.println(" Hz");
}

void loop() {
  long tune_freq;
  if(button_down) {
    tune_freq = 100;
  } else {
    tune_freq = 1000;
  }

  long newPosition = myEnc.read();
  long posDiff = newPosition - oldPosition;
  if(abs(posDiff) > 3) {
    if(millis() - last_turn > 100) {
      if(posDiff > 0) {
        freq += tune_freq;
      } else {
        freq -= tune_freq;
      }
      printFreq(freq);
      //setFrequency(VFO_A, freq);
      last_turn = millis();
    }
    oldPosition = newPosition;
  }

  byte newButton = digitalRead(4);
  if(lastButton == 1 && newButton == 0) {
    Serial.println("onButtonDown");
    lastButton = newButton;
    button_down = true;
    if((millis() - double_click_time) < 500) {
      Serial.println("onDoubleClick");
    }
    double_click_time = millis();
  } else if(lastButton == 0 && newButton == 1) {
    Serial.println("onButtonUp");
    lastButton = newButton;
    button_down = false;
  } else {
    lastButton = newButton;
  }
}
