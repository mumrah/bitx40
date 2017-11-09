#include <Arduino.h>
#include <stdio.h>

#define DEFAULT_FREQ 7125000UL
#define MIN_FREQ 7000000UL        // absolute minimum tuning frequency in Hz
#define MAX_FREQ 7300000UL        // absolute maximum tuning frequency in Hz

#define BITX_BFO_FREQ (11998000UL)

#define LSB 0x00
#define CWL 0x01
#define USB 0x02
#define CWU 0x03

#define OFFSET_USB_SHIFT 1500
#define CW_SHIFT 800              // RX shift in CW mode in Hz, equal to sidetone pitch [accepted range 200-1200 Hz]

#define DRIVE_2mA 0x2
#define DRIVE_4mA 0x4
#define DRIVE_6mA 0x6
#define DRIVE_8mA 0x8

#define LSB_DRIVE 0x4
#define USB_DRIVE 0x8

// frequency scanning parameters
#define SCAN_START 7100           // Scan start frequency in kHz [accepted range MIN_FREQ - MAX_FREQ, see above]
#define SCAN_STOP 7150            // Scan stop frequency in kHz [accepted range SCAN_START - MAX_FREQ, see above]
#define SCAN_STEP 1000            // Scan step size in Hz [accepted range 50Hz to 10000Hz]
#define SCAN_STEP_DELAY 500       // Scan step delay in ms [accepted range 0-2000 ms]

#define VFO_A 0x0
#define VFO_B 0x1

struct VFOConfig {
  uint32_t freq_hz = DEFAULT_FREQ;
  byte mode = LSB;
  int ritHz = 0;
  int fineHz = 0;
  uint8_t drive_mA = LSB_DRIVE;
  uint32_t scanStart = MIN_FREQ;
  uint32_t scanStop = MAX_FREQ;
};

void vfoInit();
void setFrequency(byte vfo, uint32_t freq_hz);
void setMode(byte vfo, byte mode);
void setMode(byte vfo, byte mode, uint8_t drive);
void setRitHz(byte vfo, int ritHz);
void setFineTuneHz(byte vfo, int fineHz);
void setScanRange(byte vfo, uint16_t scanStart, uint16_t scanStop);
void updateVFO(byte vfo);
