#include <Arduino.h>
#include "VFO.h"
#include "FrontPanel.h"

VFOConfig A;
VFOConfig B;
VFOConfig* selectedVFO;
bool vfo_split = false;

void setup() {
  Serial.begin(9600);
  Serial.println("Raduino de KM4NKU");
  initVFO();
  initFrontPanel();
  selectedVFO = &A;
}

void loop() {
  // Read the state of the controls
  UIState* uiState = pollFrontPanel(selectedVFO);

  // Switch VFOs, maybe
  switch(uiState->vfo_select) {
    case VFO_A:
      selectedVFO = &A;
      vfo_split = false;
      break;
    case VFO_B:
      selectedVFO = &B;
      vfo_split = false;
      break;
    case VFO_SPLIT:
     selectedVFO = &A;
     vfo_split = true;
     break;
  }
  uiState->vfoConfig = selectedVFO;

  // Update VFO
  updateVFO(selectedVFO);
  renderUI();
}
