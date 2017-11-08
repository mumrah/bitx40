# Radio state

What mode are we in

```
#define RUN_NORMAL (0)      // normal operation
#define RUN_CALIBRATE (1)   // calibrate VFO frequency in LSB mode
#define RUN_DRIVELEVEL (2)  // set VFO drive level
#define RUN_TUNERANGE (3)   // set the range of the tuning pot
#define RUN_CWOFFSET (4)    // set the CW offset (=sidetone pitch)
#define RUN_SCAN (5)        // frequency scanning mode
#define RUN_SCAN_PARAMS (6) // set scan parameters
#define RUN_MONITOR (7)     // frequency scanning mode
#define RUN_FINETUNING (8)  // fine tuning mode
```

Are we TX'ing?

Various inputs: CW key, Knob turn, Function button

```
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
```

User Configuration:
* VFO A/B: frequency, mode, RIT, vfo drive, vfo scan
* CW: ...
* Split mode (RX on VFO A, TX on VFO B)









```

class VFOConfig {
  Mode mode;
  int ritHz;
  int drive;
  long scanStart;
  long scanStop;
}

class VFO {
  VFOConfig config;
  LO lo;
  void setFrequency(long hz) {
    // account for side band, correction factory, etc
    long corrected = config.correct(hz);
    lo.setFrequency(corrected);
  }  
}


```
