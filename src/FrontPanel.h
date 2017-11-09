#include <Arduino.h>

#define UI_MODE_RUN 0x00
#define UI_MODE_CONFIG_MENU 0x01
#define UI_MODE_CONFIG_SELECT 0x02


struct UIState {
  long last_encoder_position;
  long last_turn_ms;
  long last_button_down_ms;
  byte last_button_state;

  bool is_button_down = false;
  bool is_double_click = false;
  bool is_press_and_turn = false;

  byte ui_mode = UI_MODE_RUN;
  // Run mode state
  long display_frequency;
  uint32_t tune_step_hz = 10000;

  // Config mode state
  uint8_t menu_idx = 0;
  char menu_vfo = 'A';
};

void setupFrontPanel(long initialFrequency);
void pollFrontPanel();

void onKnobRotate(bool is_clockwise);
void onClick();
void onDoubleClick();
void onButtonDown();
void onButtonUp();
