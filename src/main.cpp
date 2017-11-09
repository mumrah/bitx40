#include <Arduino.h>
#include "VFO.h"
#include "FrontPanel.h"

void setup() {
  Serial.begin(9600);
  Serial.println("Basic Encoder Test:");
  setupFrontPanel(DEFAULT_FREQ);
}

void loop() {
  pollFrontPanel();
  
}
