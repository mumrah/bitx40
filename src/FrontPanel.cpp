#include <Arduino.h>
#include <LiquidCrystal.h>
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>
#include "ascii.h"
#include "FrontPanel.h"

LiquidCrystal lcd(8, 9, 10, 11, 12, 13);
Encoder myEnc(A1, A2);

UIState uiState;

void renderUI() {
  if(!uiState.dirty) {
    return;
  }

  if(uiState.ui_mode == UI_MODE_RUN) {
    // pretty-print the frequency, e.g. 7.123.456
    char buf[7], out[9];
    memset(buf, 0, 7);
    memset(out, 0, 9);
    ultoa(uiState.display_frequency, buf, DEC);

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

    // S-meter
    lcd.setCursor(0, 1);
    uint8_t bar;
    switch(uiState.tune_step_hz) {
      case 100000:
        bar = 4;
        break;
      case 10000:
        bar = 5;
        break;
      case 1000:
        bar = 6;
        break;
      case 100:
        bar = 8;
        break;
    }
    for(uint8_t i=0; i<9; i++) {
      uint8_t glyph = (i / 3) << 1;
      if(i == bar) {
        glyph |= 0x01;
      }
      lcd.write(glyph);
    }
  } else if(uiState.ui_mode == UI_MODE_CONFIG_MENU || uiState.ui_mode == UI_MODE_CONFIG_SELECT) {
    lcd.setCursor(0, 0);
    switch(uiState.menu_idx) {
      case 0:
        lcd.print("VFO: ");
        lcd.print(uiState.menu_vfo);
        lcd.print("          ");
        break;
      default:
      lcd.print("Menu ");
      lcd.print(uiState.menu_idx);
      lcd.print("        ");
    }
    lcd.setCursor(0, 1);
    lcd.print("                ");
  }
}

void initFrontPanel(long initialFrequency) {
  uiState.display_frequency = initialFrequency;

  pinMode(A3, INPUT_PULLUP);
  lcd.begin(16, 2);
  lcd.createChar(0, s1);
  lcd.createChar(1, s1_bar);
  lcd.createChar(2, s4);
  lcd.createChar(3, s4_bar);
  lcd.createChar(4, s7);
  lcd.createChar(5, s7_bar);
  lcd.createChar(6, rx);
  lcd.createChar(7, tx);

  // TODO placeholder s-meter
  lcd.setCursor(0, 1);
  lcd.write((uint8_t)0);
  lcd.write((uint8_t)0);
  lcd.write((uint8_t)0);
  lcd.write((uint8_t)1);
  lcd.write((uint8_t)1);
  lcd.write((uint8_t)6);
  lcd.write((uint8_t)2);
  lcd.write((uint8_t)2);
  lcd.write((uint8_t)2);
  lcd.print((char)0xFF);
  lcd.print((char)0xFF);
  lcd.print((char)0xFF);
  lcd.print((char)0xFF);
  lcd.print((char)0xFF);
  lcd.print(" ");
  lcd.write((uint8_t)6);
}

// Read from knobs and buttons, determine which UI events have occurred
UIState* pollFrontPanel() {
  long new_encoder_position = myEnc.read();
  long encoder_diff = new_encoder_position - uiState.last_encoder_position;
  if(abs(encoder_diff) > 3) {
    // gone a full turn
    if(uiState.is_button_down) {
      uiState.is_press_and_turn = true;
    }
    uint8_t turns = abs(encoder_diff) / 3;
    for(uint8_t i=1; i<turns; i++) {
      onKnobRotate(encoder_diff > 0);
    }
    onKnobRotate(encoder_diff > 0);
    uiState.last_turn_ms = millis();
    uiState.last_encoder_position = new_encoder_position;
  }

  byte new_button_state = digitalRead(A3);
  if(uiState.last_button_state == 1 && new_button_state == 0) {
    Serial.println("onButtonDown");
    uiState.last_button_state = new_button_state;
    uiState.is_button_down = true;
    if((millis() - uiState.last_button_down_ms) < 300) {
      Serial.println("onDoubleClick");
      uiState.is_double_click = true;
      onDoubleClick();
    }
    uiState.last_button_down_ms = millis();
  } else if(uiState.last_button_state == 0 && new_button_state == 1) {
    if(!uiState.is_double_click && !uiState.is_press_and_turn) {
      Serial.println("onClick");
      onClick();
    }
    Serial.println("onButtonUp");
    uiState.last_button_state = new_button_state;
    uiState.is_button_down = false;
    uiState.is_double_click = false;
    uiState.is_press_and_turn = false;
  } else {
    uiState.last_button_state = new_button_state;
  }
  return &uiState;
}

void doTune(bool is_clockwise) {
  long tune_adjust = uiState.tune_step_hz;
  if(uiState.is_press_and_turn) {
    tune_adjust = 1;
  }
  if(is_clockwise > 0) {
    uiState.display_frequency += tune_adjust;
  } else {
    uiState.display_frequency -= tune_adjust;
  }
}

void onKnobRotate(bool is_clockwise) {
  uiState.dirty = true;
  switch(uiState.ui_mode) {
    case UI_MODE_RUN:
      doTune(is_clockwise);
      break;
    case UI_MODE_CONFIG_MENU:
      if(is_clockwise) {
        if(uiState.menu_idx == 9) {
          uiState.menu_idx = 0;
        } else {
          uiState.menu_idx += 1;
        }
      } else {
        if(uiState.menu_idx == 0) {
          uiState.menu_idx = 9;
        } else {
          uiState.menu_idx -= 1;
        }
      }
      break;
    case UI_MODE_CONFIG_SELECT:
      switch(uiState.menu_idx) {
        case 0:
          // VFO
          if((uiState.menu_vfo == 'A' && is_clockwise) || (uiState.menu_vfo == 'S' && !is_clockwise)) {
            uiState.menu_vfo = 'B';
          } else if((uiState.menu_vfo == 'B' && is_clockwise) || (uiState.menu_vfo == 'A' && !is_clockwise)) {
            uiState.menu_vfo = 'S';
          } else if((uiState.menu_vfo == 'S' && is_clockwise) || (uiState.menu_vfo == 'B' && !is_clockwise)) {
            uiState.menu_vfo = 'A';
          }
          break;
      }
      break;
  }
}

void onButtonDown() {
  uiState.dirty = true;
}

void onButtonUp() {
  uiState.dirty = true;
}

void onDoubleClick() {
  uiState.dirty = true;
  switch(uiState.ui_mode) {
    case UI_MODE_RUN:
      uiState.ui_mode = UI_MODE_CONFIG_MENU;
      break;
    default:
      uiState.ui_mode = UI_MODE_RUN;
      uiState.menu_idx = 0;
      break;
  }
}

void onClick() {
  uiState.dirty = true;
  switch(uiState.ui_mode) {
    case UI_MODE_RUN:
      // Change tuning step
      uiState.tune_step_hz = uiState.tune_step_hz / 10;
      if(uiState.tune_step_hz == 10) {
        uiState.tune_step_hz = 100000;
      }
      break;
    case UI_MODE_CONFIG_MENU:
      // Select sub-menu
      uiState.ui_mode = UI_MODE_CONFIG_SELECT;
      break;
    case UI_MODE_CONFIG_SELECT:
      uiState.ui_mode = UI_MODE_CONFIG_MENU;
      break;
  }
}
