#include <Arduino.h>
#include <LiquidCrystal.h>
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>
#include "ascii.h"
#include "FrontPanel.h"

LiquidCrystal lcd(8, 9, 10, 11, 12, 13);
Encoder myEnc(A1, A2);

UIState uiState;

void printMenu(const char* menu) {
  lcd.print(menu);
  if(uiState.ui_mode == UI_MODE_CONFIG_SELECT) {
    lcd.print((char)0x7E);
  } else {
    lcd.print(":");
  }
  lcd.print(" ");
}

void renderUI() {
  if(!uiState.dirty) {
    return;
  }

  if(uiState.ui_mode == UI_MODE_RUN) {
    // pretty-print the frequency, e.g. 7.123.456
    char buf[7], out[9];
    memset(buf, 0, 7);
    memset(out, 0, 9);
    ultoa(uiState.vfoConfig->freq_hz, buf, DEC);

    strcat(out, "");
    out[0] = buf[0];
    strcat(out, ".");
    strncat(out, &buf[1], 3);
    strcat(out, ".");
    strncat(out, &buf[4], 3);
    lcd.setCursor(0, 0);
    switch(uiState.vfo_select) {
      case VFO_A:
        lcd.print("A ");
        break;
      case VFO_B:
        lcd.print("B ");
        break;
      case VFO_SPLIT:
        lcd.print("S ");
        break;
    }
    lcd.print(out);

    switch(uiState.vfoConfig->mode) {
      case LSB:
        lcd.print("  LSB");
        break;
      case USB:
        lcd.print("  USB");
        break;
      case CWL:
        lcd.print("  CWL");
        break;
      case CWU:
        lcd.print("  CWU");
        break;
    }

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
        printMenu("VFO");
        switch(uiState.vfo_select) {
          case VFO_A:
            lcd.print("A          ");
            break;
          case VFO_B:
            lcd.print("B          ");
            break;
          case VFO_SPLIT:
            lcd.print("Split      ");
            break;
        }
        break;
      case 1:
        // Side band
        lcd.setCursor(0, 0);
        printMenu("Side Band");
        switch(uiState.vfoConfig->mode) {
          case LSB:
            lcd.print("LSB  ");
            break;
          case USB:
            lcd.print("USB  ");
            break;
          case CWL:
            lcd.print("CWL  ");
            break;
          case CWU:
            lcd.print("CWU  ");
            break;
        }
        break;
      default:
        // Placeholder
        printMenu("Menu");
        lcd.print(uiState.menu_idx);
        lcd.print("        ");
        lcd.setCursor(0, 1);
        lcd.print("                ");
        break;
    }
  }
}

void initFrontPanel() {
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

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Raduino KM4NKU  ");

  // TODO placeholder s-meter
  lcd.setCursor(0, 1);
  lcd.write((uint8_t)0);
  lcd.write((uint8_t)0);
  lcd.write((uint8_t)0);
  lcd.write((uint8_t)2);
  lcd.write((uint8_t)2);
  lcd.write((uint8_t)2);
  lcd.write((uint8_t)4);
  lcd.write((uint8_t)4);
  lcd.write((uint8_t)4);
  lcd.print((char)0xFF);
  lcd.print((char)0xFF);
  lcd.print((char)0xFF);
  lcd.print((char)0xFF);
  lcd.print((char)0xFF);
  lcd.print(" ");
  lcd.write((uint8_t)6);

  /*
  for(int i=0; i<3; i++) {
    lcd.setCursor(15, 1);
    lcd.write((uint8_t)7);
    delay(500);
    lcd.setCursor(15, 1);
    lcd.write((uint8_t)6);
    delay(500);
  }
  */
}

// Read from knobs and buttons, determine which UI events have occurred
UIState* pollFrontPanel(VFOConfig* vfoConfig) {
  uiState.vfoConfig = vfoConfig;

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
    if((millis() - uiState.last_button_down_ms) < UI_DOUBLE_CLICK_TIMEOUT) {
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
    uiState.vfoConfig->freq_hz += tune_adjust;
  } else {
    uiState.vfoConfig->freq_hz -= tune_adjust;
  }
}

uint8_t incOrDecEnum(uint8_t current_value, uint8_t max_value, bool is_clockwise) {
  if(is_clockwise) {
    if(current_value == max_value) {
      current_value = 0;
    } else {
      current_value += 1;
    }
  } else {
    if(current_value == 0) {
      current_value = max_value;
    } else {
      current_value -= 1;
    }
  }
  return current_value;
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
          uiState.vfo_select = incOrDecEnum(uiState.vfo_select, VFO_SPLIT, is_clockwise);
          break;
        case 1:
          // Side Band
          uiState.vfoConfig->mode = incOrDecEnum(uiState.vfoConfig->mode, CWU, is_clockwise);
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
