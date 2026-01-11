
#ifndef _INC_MCU_H
#define _INC_MCU_H 

    #ifdef __ASSEMBLER__

        F_CPU_MHZ = 16
        STATUS_REG_ADDR = 0x3F
        
    #endif

    #ifndef __ASSEMBLER__

        #ifndef __AVR_ATmega328P__
            #define __AVR_ATmega328P__
        #endif
        #include <avr/io.h>
        #include <avr/interrupt.h>

    #endif

#endif

