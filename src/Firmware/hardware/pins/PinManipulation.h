#ifndef PINMANIPULATION_H_
#define PINMANIPULATION_H_

#define setOutput(ddr, pin) ((ddr) |= (1 << (pin)))
#define setInput(ddr, pin) ((ddr) &= ~(1 << (pin)))
#define setLow(port, pin) ((port) &= ~(1 << (pin)))
#define setHigh(port, pin) ((port) |= (1 << (pin)))

#define pulseHightToLow(port, pin) do { \
    setHigh((port), (pin)); \
    _NOP(); \
    setLow((port), (pin)); \
} while (0)

#define pulseLowToHigh(port, pin) do { \
    setLow((port), (pin)); \
    _NOP(); \
    setHigh((port), (pin)); \
} while (0)

#endif