#include <Arduino.h>

struct UIState {
  long last_encoder_position;
  long last_turn_ms;
  long last_button_down_ms;
  byte last_button_state;

  bool is_button_down = false;
  bool in_double_click = false;
};

void setupFrontPanel(long initialFrequency);
void pollFrontPanel();

void onClick(UIState uiState);
void onDoubleClick(UIState uiState);
