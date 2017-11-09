#include <Arduino.h>
#include <LiquidCrystal.h>
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>
#include "ascii.h"

LiquidCrystal lcd(8, 9, 10, 11, 12, 13);
Encoder myEnc(A1, A2);

struct UIState {
  long oldPosition;
  long last_turn;
  byte lastButton;
  bool button_down = false;
  long double_click_time;
  bool in_double_click = false;
}

UIState uiState;

long oldPosition  = -999;
long last_turn;
byte lastButton = 1;
bool button_down = false;
long double_click_time;

uint32_t tune_step_hz = 10000;
bool in_double_click = false;
bool in_fine_tune = false;

long freq;

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
  lcd.setCursor(0, 0);
  lcd.print("A ");
  lcd.print(out);
  lcd.print("  LSB");
}

void setupFrontPanel(long initialFrequency) {
  freq = initialFrequency;
  pinMode(A3, INPUT_PULLUP);
  lcd.begin(16, 2);
  lcd.createChar(0, s1);
  lcd.createChar(1, s4);
  lcd.createChar(2, s7);
  lcd.createChar(3, rx);
  lcd.createChar(4, tx);
  lcd.createChar(5, over);

  lcd.setCursor(0, 1);
  lcd.write((uint8_t)0);
  lcd.write((uint8_t)0);
  lcd.write((uint8_t)0);
  lcd.write((uint8_t)1);
  lcd.write((uint8_t)1);
  lcd.write((uint8_t)1);
  lcd.write((uint8_t)2);
  lcd.write((uint8_t)2);
  lcd.write((uint8_t)2);
  lcd.write((uint8_t)5);
  lcd.write((uint8_t)5);
  lcd.write((uint8_t)5);
  lcd.write((uint8_t)5);
  lcd.write((uint8_t)5);
  lcd.print(" ");
  lcd.write((uint8_t)3);
}

void pollFrontPanel() {

  long newPosition = myEnc.read();
  long posDiff = newPosition - oldPosition;
  long tune_adjust = tune_step_hz;
  if(abs(posDiff) > 3) {
    if(button_down) {
      tune_adjust = tune_step_hz / 10;
      in_fine_tune = true;
    }
    if(posDiff > 0) {
      freq += tune_adjust;
    } else {
      freq -= tune_adjust;
    }
    printFreq(freq);
    //setFrequency(VFO_A, freq);
    last_turn = millis();
    oldPosition = newPosition;
  }

  byte newButton = digitalRead(A3);
  if(lastButton == 1 && newButton == 0) {
    Serial.println("onButtonDown");
    lastButton = newButton;
    button_down = true;
    if((millis() - double_click_time) < 500) {
      Serial.println("onDoubleClick");
      in_double_click = true;
    }
    double_click_time = millis();
  } else if(lastButton == 0 && newButton == 1) {
    if(!in_double_click && !in_fine_tune) {
      Serial.println("onClick");
      tune_step_hz = tune_step_hz / 10;
      if(tune_step_hz == 10) {
        tune_step_hz = 100000;
      }
    }
    Serial.println("onButtonUp");
    lastButton = newButton;
    button_down = false;
    in_double_click = false;
    in_fine_tune = false;
  } else {
    lastButton = newButton;
  }
}
