#include "wiring_private.h"
#include "pins_arduino.h"

unsigned long hf_pulseIn(int pin, unsigned long timeout)
{

    int bit = CORE_PIN9_BITMASK;//TODO digitialPinToBitMask doesn't seem to work here
    uint8_t port = digitalPinToPort(pin);
    uint8_t stateMask = (HIGH ? bit : 0);
    unsigned long width = 0; // keep initialization out of time critical area

    // convert the timeout from microseconds to a number of times through
    // the initial loop; it takes 16 clock cycles per iteration.
    unsigned long numloops = 0;
    unsigned long maxloops = microsecondsToClockCycles(timeout) / 16;

    // wait for any previous pulse to end
    while ((*portInputRegister(port) & bit) == stateMask)
        if (numloops++ == maxloops)
            return 1;

    // wait for the pulse to start
    while ((*portInputRegister(port) & bit) != stateMask)
        if (numloops++ == maxloops)
            return 2;

    // wait for the pulse to stop
    while ((*portInputRegister(port) & bit) == stateMask) {
        if (numloops++ == maxloops)
            return 3;
        width++;
    }
    
    return width; 
}