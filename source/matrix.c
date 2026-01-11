////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Модуль управления матрицей адресных светодиодов.
	разработал Николаев Сергей (г. Кулебаки).
	Все права защищены.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "matrix.h"
#include "addr_leds_driver.h"
#include "analog.h"
#include <stdint.h>

#define G_INDEX 0
#define R_INDEX 1
#define B_INDEX 2

#define ROWS_COUNT 16
#define COLS_COUNT 16
#define COLOR_SIZE 3

static uint8_t matrix[ROWS_COUNT][COLS_COUNT][COLOR_SIZE];

static void matrix_clear(void);
static inline void matrix_led_set(uint8_t row, uint8_t col, uint8_t g, uint8_t r, uint8_t b) __attribute__((always_inline));
static inline uint8_t value_with_random_component_calc(uint8_t input, uint8_t random_mask) __attribute__((always_inline));

void matrix_init(void)
{
    matrix_clear();
    addr_leds_init();
    addr_leds_colors_apply((uint16_t)sizeof(matrix), (uint8_t *)matrix);
}

void matrix_main(void)
{
    // Цвета светодиодов в зависимости от звуков
    for (uint8_t chanel = 0; chanel < ADC_CHANELS_COUNT; chanel++)
    {
        uint8_t col_start = chanel + 7;
        uint8_t col_inc = ((uint8_t)1) - (chanel << 1);

        for(uint8_t freq_band = 0, row = 0; (freq_band < FREQ_BANDS_COUNT); freq_band++, row = (freq_band << 1))
        {
            #define RANDOM_COMPONENT_MASK (0x07)
            uint8_t g = value_with_random_component_calc(analog.freq_snapshots[chanel][freq_band],  RANDOM_COMPONENT_MASK);
            uint8_t b = value_with_random_component_calc(analog.phase_snapshots[chanel][freq_band], RANDOM_COMPONENT_MASK);            
            uint8_t enabled_leds_count = (analog.freq_snapshots[chanel][freq_band] >> 5) + 1;
            for(uint8_t led = 0, r = 0, col = col_start; led < (COLS_COUNT >> 1); led++, col += col_inc, r += 16)
            {                
                if(led >= enabled_leds_count)
                {
                    g &= RANDOM_COMPONENT_MASK;
                    b &= RANDOM_COMPONENT_MASK;
                    r &= RANDOM_COMPONENT_MASK;
                }
                matrix_led_set(row, col, g, r, b);
                matrix_led_set((row + 1), col, g, r, b);
            }
        }
    }

    addr_leds_colors_apply((uint16_t)sizeof(matrix), (uint8_t *)matrix);
}

static void matrix_clear(void)
{
    for(uint8_t row = 0; row < ROWS_COUNT; row++)
    {
        for(uint8_t col = 0; col < COLS_COUNT; col++)
        {
            matrix_led_set(row, col, 0, 0, 0);
        }
    }
}

static inline void matrix_led_set(uint8_t row, uint8_t col, uint8_t g, uint8_t r, uint8_t b) 
{
    matrix[row][col][G_INDEX] = g, matrix[row][col][R_INDEX] = r, matrix[row][col][B_INDEX] = b;
}

static inline uint8_t value_with_random_component_calc(uint8_t input, uint8_t random_mask)
{
    static uint8_t memory = 0;
    return memory = input ^ ((((((memory + (input >> 3)) << 1) + (~(memory >> 2))) >> 1) + (input >> 1)) & random_mask);
}