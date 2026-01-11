////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Заголовочный файл модуля обработки аналоговых сигналов.
	разработал Николаев Сергей (г. Кулебаки).
	Все права защищены.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _INC_ANALOG_H_
#define _INC_ANALOG_H_

    #include <stdint.h>

    #define ADC_CHANELS_COUNT (2)
    #define SAMPLES_COUNT (16)
    #define FREQ_BANDS_COUNT (SAMPLES_COUNT >> 1)

    typedef struct
    {
        volatile uint8_t amplitudes[ADC_CHANELS_COUNT][SAMPLES_COUNT];
        uint8_t amplitudes_snapshots[ADC_CHANELS_COUNT][SAMPLES_COUNT];
        uint8_t freq_snapshots[ADC_CHANELS_COUNT][FREQ_BANDS_COUNT];
        uint8_t phase_snapshots[ADC_CHANELS_COUNT][FREQ_BANDS_COUNT];
    }Analog_inputs;

    extern Analog_inputs analog;

    void analog_inputs_init(void);
    void analog_inputs_main(void);

#endif