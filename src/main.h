#include <Arduino.h>

void shiftBase();
void setFrequency();
void save_frequency();
void set_CWparams();
void set_tune_range();
void set_drive_level(byte level);
void calibrate_touch_pads();
void SetSideBand(byte drivelevel);
void VFOdrive();
void scan_params();
void toggleRIT();
void toggleMode();
void toggleSPLIT();
void resetVFOs();
void swapVFOs();
int knob_position();

void si5351bx_init();
void si5351bx_setfreq(uint8_t clknum, uint32_t fout);
void i2cWrite(uint8_t reg, uint8_t val);
void i2cWriten(uint8_t reg, uint8_t *vals, uint8_t vcnt);
