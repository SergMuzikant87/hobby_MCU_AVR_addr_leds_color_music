
#ifndef _INC_WATCHDOG_
#define _INC_WATCHDOG_
    
    #include <stdint.h>

    #define WATCHDOG_SETUP_TIME_16_MS 0
    #define WATCHDOG_SETUP_TIME_32_MS 1
    #define WATCHDOG_SETUP_TIME_64_MS 2
    #define WATCHDOG_SETUP_TIME_125_MS 3
    #define WATCHDOG_SETUP_TIME_250_MS 4
    #define WATCHDOG_SETUP_TIME_500_MS 5
    
    void watchdog_init(uint8_t prescaler);
    void watchdog_reset(void);

#endif