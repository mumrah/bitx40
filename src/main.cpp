#include <Arduino.h>
#include "VFO.h"
#include "FrontPanel.h"

void setup() {
  Serial.begin(9600);
  Serial.println("Raduino de KM4NKU");
  initFrontPanel(DEFAULT_FREQ);
  initVFO();
  delay(1000);
}

void loop() {
  // Read the state of the controls
  UIState* uiState = pollFrontPanel();

  byte vfo;
  if(uiState->menu_vfo == 'A') {
    vfo = VFO_A;
  } else {
    vfo = VFO_B;
  }

  // Update VFO
  setFrequency(vfo, uiState->display_frequency);
  updateVFO(vfo);

  renderUI();

}
