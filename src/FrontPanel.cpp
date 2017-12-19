#include <Arduino.h>
#include <LiquidCrystal.h>
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>
#include "ascii.h"
#include "FrontPanel.h"

LiquidCrystal lcd(8, 9, 10, 11, 12, 13);
Encoder myEnc(A1, A2);

UIState uiState;

char LEFT_ARROW[2] = {0x7E, 0x00};

char c[17], printBuff[2][17];

/**
   Display Routine
   This display routine prints a line of characters to the upper or lower line of the 16x2 display
   linenmbr = 0 is the upper line
   linenmbr = 1 is the lower line
*/
void printLine(char line, char* message) {
  if (strcmp(message, printBuff[(uint8_t)line])) {
    lcd.setCursor(0, line);
    lcd.print(message);
    strcpy(printBuff[(uint8_t)line], message);

    // add white spaces until the end of the 16 characters line is reached
    for (byte i = strlen(message); i < 16; i++) {
      lcd.print(' ');
    }
  }
}

void printMenu(const char* menu, const char* option) {
  memset(c, 0, sizeof(c));
  strcat(c, menu);
  if(uiState.ui_mode == UI_MODE_CONFIG_SELECT) {
    strncat(c, LEFT_ARROW, 1);
  } else {
    strcat(c, ":");
  }
  strcat(c, " ");
  strcat(c, option);
  printLine(0, c);

  memset(c, 0, sizeof(c));
  strcat(c, "Selected VFO: ");
  switch(uiState.vfo_select) {
    case VFO_A:
      strcat(c, "A");
      break;
    case VFO_B:
      strcat(c, "B");
      break;
    case VFO_SPLIT:
      strcat(c, "A");
      break;
  }
  printLine(1, c);
}

long last_meter_update = 0;

void printMeter(uint16_t strength) {
  long now = millis();
  if(now - last_meter_update < 10) {
    // update the meter at most once every 10ms
    return;
  }
  // Frequency tuning step indicator
  uint8_t bar;
  switch(uiState.tune_step_hz) {
    case 100000:
      bar = 3;
      break;
    case 10000:
      bar = 4;
      break;
    case 1000:
      bar = 5;
      break;
    case 100:
      bar = 7;
      break;
  }

  // Print s-meter. We must use lcd.print and lcd.write instead of our printLine
  // method since we are using custom glyphs
  lcd.setCursor(0, 1);
  lcd.write(RX_TX_GLYPH);
  memset(c, 0, sizeof(c));
  uint8_t s_units = min(9, max(1, strength / 113));

  for(uint8_t i=1; i<9; i++) {
    if(i < s_units) {
      uint8_t glyph = (i / 3) << 1;
      if(i == bar) {
        glyph |= 0x01;
      }
      lcd.write(glyph);
    } else {
      if(i == bar) {
        lcd.write(6);
      } else {
        lcd.print(" ");
      }
    }
  }
  lcd.print("   ");

  if(uiState.vfoConfig->ritEnabled) {
    char buf[4];
    memset(buf, 0, 4);
    sprintf(buf, "%+04d", uiState.vfoConfig->ritHz);
    lcd.print(buf);
  } else {
    lcd.print("    ");
  }

  last_meter_update = now;
}

void updateUI() {
  if(!uiState.dirty) {
    return;
  }

  if(uiState.ui_mode == UI_MODE_RUN) {
    // Runtime display (frequency, side band, s-meter, etc)
    char buf[7];
    memset(buf, 0, 7);
    ultoa(uiState.vfoConfig->freq_hz, buf, DEC);

    // pretty-print the frequency, e.g. " 7.123.456"
    memset(c, 0, sizeof(c));
    strcat(c, " ");
    c[1] = buf[0];
    strcat(c, ".");
    strncat(c, &buf[1], 3);
    strcat(c, ".");
    strncat(c, &buf[4], 3);

    // VFO selection
    strcat(c, " ");
    switch(uiState.vfo_select) {
      case VFO_A:
        strcat(c, "A ");
        break;
      case VFO_B:
        strcat(c, "B ");
        break;
      case VFO_SPLIT:
        strcat(c, "S ");
        break;
    }

    // VFO mode
    switch(uiState.vfoConfig->mode) {
      case LSB:
        strcat(c, "LSB");
        break;
      case USB:
        strcat(c, "USB");
        break;
      case CWL:
        strcat(c, "CWL");
        break;
      case CWU:
        strcat(c, "CWU");
        break;
    }
    printLine(0, c);

    // Print s-meter
    printMeter(1024);

  } else if(uiState.ui_mode == UI_MODE_CONFIG_MENU || uiState.ui_mode == UI_MODE_CONFIG_SELECT) {
    // Print Config Menu
    switch(uiState.menu_idx) {
      case UI_MENU_VFO:
        switch(uiState.vfo_select) {
          case VFO_A:
            printMenu("Select VFO", "A");
            break;
          case VFO_B:
            printMenu("Select VFO", "B");;
            break;
          case VFO_SPLIT:
            printMenu("Select VFO", "Split");
            break;
        }
        break;
      case UI_MENU_MODE:
        // Side band
        switch(uiState.vfoConfig->mode) {
          case LSB:
            printMenu("Side Band", "LSB");
            break;
          case USB:
            printMenu("Side Band", "USB");
            break;
          case CWL:
            printMenu("Side Band", "CWL");
            break;
          case CWU:
            printMenu("Side Band", "CWU");
            break;
        }
        break;
      case UI_MENU_DRIVE:
        // VFO Drive
        switch(uiState.vfoConfig->drive_mA) {
          case DRIVE_2mA:
            printMenu("VFO Drive", "2mA");
            break;
          case DRIVE_4mA:
            printMenu("VFO Drive", "4mA");
            break;
          case DRIVE_6mA:
            printMenu("VFO Drive", "6mA");
            break;
          case DRIVE_8mA:
            printMenu("VFO Drive", "8mA");
            break;
        }
        break;
      case UI_MENU_RIT:
        // RIT
        if(uiState.vfoConfig->ritEnabled) {
          printMenu("RIT", "Enabled");
        } else {
          printMenu("RIT", "Disabled");
        }
        break;
      case UI_MENU_PPM:
        char buf[4];
        //memset(buf, 0, 4);
        //sprintf(buf, "%+04d", uiState.vfoConfig->ppmCorr);
        itoa(uiState.vfoConfig->ppmCorr, buf, 10);
        printMenu("Calibrate", buf);
        break;
      case UI_MENU_SAVE:
        printLine(0, "Save VFOs");
        break;
      default:
        // Placeholder
        char num[2];
        itoa(uiState.menu_idx, num, 10);
        printMenu("Menu", num);
        break;
    }
  }
}

void toggleRxTx(bool rx) {
  if(rx) {
    lcd.createChar(RX_TX_GLYPH, ASCII_RX);
  } else {
    lcd.createChar(RX_TX_GLYPH, ASCII_TX);
  }
}

void initFrontPanel() {
  pinMode(A3, INPUT_PULLUP);
  lcd.begin(16, 2);
  lcd.createChar(0, ASCII_S1);
  lcd.createChar(1, ASCII_S1_BAR);
  lcd.createChar(2, ASCII_S4);
  lcd.createChar(3, ASCII_S4_BAR);
  lcd.createChar(4, ASCII_S7);
  lcd.createChar(5, ASCII_S7_BAR);
  lcd.createChar(6, ASCII_BAR);
  toggleRxTx(true);
}

/**
 * Read from knobs and buttons, determine which UI events have occurred
 * TODO: add PTT sense in here
 */
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
  if(!is_clockwise) {
    tune_adjust = -tune_adjust;
  }

  if(uiState.is_press_and_turn) {
    tune_adjust = is_clockwise ? 1 : -1;
    if(uiState.vfoConfig->ritEnabled) {
      uiState.vfoConfig->ritHz += tune_adjust;
      return;
    }
  }

  uiState.vfoConfig->freq_hz += tune_adjust;
}

/**
 * Cycle through enums based on the direction given. Assumes enum values
 * are sequential (etc, 0x00, 0x01, 0x02)
 */
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
      uiState.menu_idx = incOrDecEnum(uiState.menu_idx, 9, is_clockwise);
      break;
    case UI_MODE_CONFIG_SELECT:
      switch(uiState.menu_idx) {
        case UI_MENU_VFO:
          // VFO
          uiState.vfo_select = incOrDecEnum(uiState.vfo_select, VFO_SPLIT, is_clockwise);
          break;
        case UI_MENU_MODE:
          // Side Band
          uiState.vfoConfig->mode = incOrDecEnum(uiState.vfoConfig->mode, CWU, is_clockwise);
          break;
        case UI_MENU_DRIVE:
          // VFO Drive
          uiState.vfoConfig->drive_mA = incOrDecEnum(uiState.vfoConfig->drive_mA, DRIVE_8mA, is_clockwise);
          break;
        case UI_MENU_RIT:
          // RIT
          uiState.vfoConfig->ritEnabled = !uiState.vfoConfig->ritEnabled;
          break;
        case UI_MENU_PPM:
          // PPM
          uiState.vfoConfig->ppmCorr += (is_clockwise ? 1 : -1);
          break;
        case UI_MENU_SAVE:
          // Save VFO
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
      uiState.menu_idx = UI_MENU_VFO;
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
      if(uiState.menu_idx == UI_MENU_SAVE) {
        // No values to select here, we just want to do some action
        uiState.requested_vfo_save = true;
      } else {
        // Enter value selection mode
        uiState.ui_mode = UI_MODE_CONFIG_SELECT;
      }
      break;
    case UI_MODE_CONFIG_SELECT:
      // Back to config menu
      uiState.ui_mode = UI_MODE_CONFIG_MENU;
      break;
  }
}
