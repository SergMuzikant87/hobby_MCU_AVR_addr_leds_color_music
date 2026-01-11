
#include "watchdog.h"
#include "mcu.h"

void watchdog_init(uint8_t prescaler)
{
    uint8_t SREG_copy = SREG;
    cli();
        WDTCSR = (0 << WDIF) | (0 << WDIE) | (((prescaler >> 3) & 0x01) << WDP3) | (1 << WDCE) | (1 <<WDE) | (prescaler & ((1 << WDP0) | (1 << WDP1) | (1 << WDP2)));
        asm("wdr"); 
    SREG = SREG_copy;
}

void watchdog_reset(void)
{
    uint8_t SREG_copy = SREG;
    cli();
        asm("wdr");
    SREG = SREG_copy;
}