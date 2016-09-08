#ifndef PINMANIPULATION_H_
#define PINMANIPULATION_H_

#define DDR(x) (*(&x-1))
#if defined (__AVR_ATmega2560__) || defined(__AVR_ATmega32U4__)
//PINF doesn't follow standard convention of port-2 address
#define PIN(x) ( &PORTF==&(x) ? _SFR_IO8(0x0F) : (*(&x - 2)) )
#else
#define PIN(x) (*(&x-2))
#endif

#define setOutput(port, pin) ((DDR(port)) |= (1 << (pin)))
#define setInput(port, pin) ((DDR(port)) &= ~(1 << (pin)))
#define setLow(port, pin) ((port) &= ~(1 << (pin)))
#define setHigh(port, pin) ((port) |= (1 << (pin)))
#define readPin(port, pin) (((PIN(port)) >> (pin)) & 0x01)

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