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
