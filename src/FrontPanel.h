#include <Arduino.h>
#include "VFO.h"

#define UI_MODE_RUN 0x00
#define UI_MODE_CONFIG_MENU 0x01
#define UI_MODE_CONFIG_SELECT 0x02

#define UI_MENU_VFO 0x00
#define UI_MENU_MODE 0x01
#define UI_MENU_DRIVE 0x02
#define UI_MENU_RIT 0x03
#define UI_MENU_PPM 0x04
#define UI_MENU_SAVE 0x05


#define UI_DOUBLE_CLICK_TIMEOUT 200

struct UIState {
  bool dirty = false;

  // Input control state
  long last_encoder_position;
  long last_turn_ms;
  long last_button_down_ms;
  byte last_button_state;

  bool is_button_down = false;
  bool is_double_click = false;
  bool is_press_and_turn = false;

  // Run mode state
  byte ui_mode = UI_MODE_RUN;

  // Tuning state: 100, 1000, 10000, or 100000 Hz
  uint32_t tune_step_hz = 10000;

  // Current menu selection
  uint8_t menu_idx = 0;

  // Save VFOs was clicked
  bool requested_vfo_save = false;

  // VFO pointer
  VFOConfig* vfoConfig;

  // VFO selection
  uint8_t vfo_select = VFO_A;
};

void initFrontPanel();
UIState* pollFrontPanel(VFOConfig* vfoConfig);
void updateUI();

void onKnobRotate(bool is_clockwise);
void onClick();
void onDoubleClick();
void onButtonDown();
void onButtonUp();
