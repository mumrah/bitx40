#include <stdio.h>
#include <Si5351.h>
#include "VFO.h"

void initVFO() {
  si5351bx_init();
}

void setMode(VFOConfig* vfoConfig, byte mode) {
  if(vfoConfig->mode & 1) { // USB
    setMode(vfoConfig, mode, USB_DRIVE);
  } else {
    setMode(vfoConfig, mode, LSB_DRIVE);
  }
}

void setMode(VFOConfig* vfoConfig, byte mode, uint8_t drive) {
  vfoConfig->mode = mode;
  vfoConfig->drive_mA = drive;
}

void setRitHz(VFOConfig* vfoConfig, int ritHz) {
  vfoConfig->ritHz = ritHz;
}

void setFineTuneHz(VFOConfig* vfoConfig, int fineHz) {
  vfoConfig->fineHz = constrain(fineHz, -1000, 1000);
}

void setScanRange(VFOConfig* vfoConfig, uint16_t scanStart, uint16_t scanStop) {
  vfoConfig->scanStart = scanStart;
  vfoConfig->scanStop = scanStop;
}


void setFrequency(VFOConfig* vfoConfig, uint32_t freq_hz) {
  vfoConfig->freq_hz = freq_hz;
}

/**
   The setFrequency is a little tricky routine, it works differently for USB and LSB
   The BITX BFO is permanently set to lower sideband, (that is, the crystal frequency
   is on the higher side slope of the crystal filter).

   LSB: The VFO frequency is subtracted from the BFO. Suppose the BFO is set to exactly 12 MHz
   and the VFO is at 5 MHz. The output will be at 12.000 - 5.000  = 7.000 MHz
   USB: The BFO is subtracted from the VFO. Makes the LSB signal of the BITX come out as USB!!
   Here is how it will work:
   Consider that you want to transmit on 14.000 MHz and you have the BFO at 12.000 MHz. We set
   the VFO to 26.000 MHz. Hence, 26.000 - 12.000 = 14.000 MHz. Now, consider you are whistling a tone
   of 1 KHz. As the BITX BFO is set to produce LSB, the output from the crystal filter will be 11.999 MHz.
   With the VFO still at 26.000, the 14 Mhz output will now be 26.000 - 11.999 = 14.001, hence, as the
   frequencies of your voice go down at the IF, the RF frequencies will go up!

   Thus, setting the VFO on either side of the BFO will flip between the USB and LSB signals.

   In addition we add some offset to USB mode so that the dial frequency is correct in both LSB and USB mode.
   The amount of offset can be set in the SETTING menu as part of the calibration procedure.

   Furthermore we add/substract the sidetone frequency only when we receive CW, to assure zero beat
   between the transmitting and receiving station (RXshift)
   The desired sidetone frequency can be set in the SETTINGS menu.
*/
long last_frequency = 0;

void updateVFO(VFOConfig* vfoConfig) {
  int RXshift = 0;
  if(vfoConfig->mode & 2) {
    RXshift = CW_SHIFT;
  }

  long new_frequency;
  if (vfoConfig->mode & 1) {   // if we are in UPPER side band mode
    new_frequency = BITX_BFO_FREQ + vfoConfig->freq_hz - RXshift + vfoConfig->ritHz + vfoConfig->fineHz - OFFSET_USB_SHIFT;

  } else {            // if we are in LOWER side band mode
    new_frequency = BITX_BFO_FREQ - vfoConfig->freq_hz - RXshift - vfoConfig->ritHz - vfoConfig->fineHz;
  }

  // Update, maybe
  if(new_frequency != last_frequency) {
    si5351bx_setfreq(2, new_frequency);
    last_frequency = new_frequency;
  }
}
