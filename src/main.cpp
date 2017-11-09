#include <Arduino.h>
#include "VFO.h"
#include "FrontPanel.h"

void setup() {
  Serial.begin(9600);
  Serial.println("Raduino de KM4NKU");
  setupFrontPanel(DEFAULT_FREQ);
}

void loop() {
  UIState uiState = pollFrontPanel();

  byte vfo;
  if(uiState.menu_vfo == 'A' || uiState.menu_vfo == 'S') {
    vfo = VFO_A;
  } else {
    vfo = VFO_B;
  }

  setFrequency(vfo, uiState.display_frequency);
  updateVFO(vfo);
}
