#include <Arduino.h>
#include <EEPROM.h>

#include "VFO.h"
#include "FrontPanel.h"

VFOConfig A;
VFOConfig B;
VFOConfig* selectedVFO;
bool vfo_split = false;

const byte VERSION = 2;

void setup() {
  Serial.begin(9600);
  Serial.println("Raduino de K4DBZ");

  byte version;
  EEPROM.get(0, version);
  if(version != VERSION) {
    EEPROM.put(0, A);
    EEPROM.put(sizeof(A), B);
  } else {
    EEPROM.get(0, A);
    EEPROM.get(sizeof(A), B);
  }

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

  // Save the VFOs to EEPROM
  if(uiState->requested_vfo_save) {
    Serial.println("Save VFOs!");
    EEPROM.put(0, A);
    EEPROM.put(sizeof(A), B);
    uiState->requested_vfo_save = false;
  }
  int corr = selectedVFO->ppmCorr;
  A.ppmCorr = corr;
  B.ppmCorr = corr;

  // Update VFO
  updateVFO(selectedVFO);

  // Update the UI
  updateUI();
}
