#ifndef PINMANIPULATION_H_
#define PINMANIPULATION_H_

#define setOutputMacro(ddr, pin) ((ddr) |= (1 << (pin)))
#define setInputMacro(ddr, pin) ((ddr) &= ~(1 << (pin)))
#define setLowMacro(port, pin) ((port) &= ~(1 << (pin)))
#define setHighMacro(port, pin) ((port) |= (1 << (pin)))

#define pulseHightToLow(port, pin) do { \
    setHighMacro((port), (pin)); \
    _NOP(); \
    setLowMacro((port), (pin)); \
} while (0)

#define pulseLowToHigh(port, pin) do { \
    setLowMacro((port), (pin)); \
    _NOP(); \
    setHighMacro((port), (pin)); \
} while (0)

#endif