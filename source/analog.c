////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Модуль обработки аналоговых сигналов.
	разработал Николаев Сергей (г. Кулебаки).
	Все права защищены.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "analog.h"
#include "mcu.h"

// Структура данных
Analog_inputs analog;
// Вспомогательные функции (в преоритете скорость)
static inline void result_amplitute_push(uint8_t chanel, uint8_t result_amp) __attribute__((always_inline));
static inline void amplutudes_snapshots_update(void) __attribute__((always_inline));
static inline void freq_and_phase_snapshots_update(void) __attribute__((always_inline));
static inline uint8_t sqrt_calc_and_norm_0_to_255(uint32_t input_value) __attribute__((always_inline));
static inline uint8_t atan_calc_and_norm_0_to_255(int32_t input_value) __attribute__((always_inline));
inline int32_t i32_limmit(int32_t in, int32_t lo_lim, int32_t hi_lim) __attribute__((always_inline));
inline int32_t i32_scale(int32_t x, int32_t x1, int32_t x2, int32_t y1, int32_t y2) __attribute__((always_inline));

// Инициализация
void analog_inputs_init(void)
{
    // На всякий случай заполняем структуру данных нулями
    for(uint16_t cur_byte_num = 0; cur_byte_num < sizeof(analog); cur_byte_num++)
    {
        *(((uint8_t *)&analog) + cur_byte_num) = 0; 
    }

    // Инициализация АЦП
    uint8_t SREG_copy = SREG;
    cli();
        
        ADMUX = (0 << REFS1) | (0 << REFS0) | (1 << ADLAR) | (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0);
        ADCSRA = (1 << ADEN) | (0 << ADSC) | (1 << ADATE) | (0 << ADIF) | (1 <<ADIE) | (1 << ADPS2) | (0 << ADPS1) | (1 << ADPS0);
        ADCSRB = (0 << ACME) | (0 << ADTS2) | (0 << ADTS1) | (0 << ADTS0);
        ADCSRA |= (1 << ADSC);
        
    SREG = SREG_copy;
}

// Главная функция
void analog_inputs_main(void)
{
    amplutudes_snapshots_update();
    freq_and_phase_snapshots_update();
}

// Прерывание по завершению преобразования АЦП
ISR(ADC_vect)
{
    #define ADC_CHANEL (ADMUX & (1 << MUX0))
    #define ADC_CONVERSION_RESULT (ADCH >> 2)
    #define adc_chanel_switch(){ADMUX ^= (1 << MUX0);}
    #define adc_conversion_start(){ADCSRA |= (1 << ADSC);}

    result_amplitute_push(ADC_CHANEL, ADC_CONVERSION_RESULT);
    adc_chanel_switch();
}

/*
    Функция записывает результат текущего преобразования в буффер результатов преобразования.
*/
static inline void result_amplitute_push(uint8_t chanel, uint8_t result_amp)
{
    for(int8_t result_num = (SAMPLES_COUNT - 1); result_num > 0; result_num--)
    {
        analog.amplitudes[chanel][result_num] = analog.amplitudes[chanel][result_num - 1];
    }
    analog.amplitudes[chanel][0] = result_amp;
}

/*
    Функция копирует все результаты преобразования из промежуточного буффера в массив.
*/
static inline void amplutudes_snapshots_update(void)
{
    uint8_t SREG_copy = SREG;
    cli();
        for(uint8_t chanel = 0; chanel < ADC_CHANELS_COUNT; chanel++)
        {
            for(uint8_t result_num = 0; result_num < SAMPLES_COUNT; result_num++)
            {
                analog.amplitudes_snapshots[chanel][result_num] = analog.amplitudes[chanel][result_num];
            }
        }
    SREG = SREG_copy;
}

/*
    Функция берёт массив с амплитудами и рассчитывает АЧХ и ФЧХ, записывает результаты в соответствующие массивы.
*/
static inline void freq_and_phase_snapshots_update(void)
{
    // Таблица синуса и косинуса
    static const int8_t sin_cos_table[] = {0, 12, 23, 30, 32, 30, 23, 12, 0, -12, -23, -30, -32, -30, -23, -12, 0, 12, 23, 30};

    // Для каждого из 2-х каналов
    for (uint8_t chanel = 0; chanel < ADC_CHANELS_COUNT; chanel++)
    {
        // Для каждой полосы частот
        for(uint8_t freq_band = 0; freq_band < FREQ_BANDS_COUNT; freq_band++)
        {
            int16_t Re = 0;
            int16_t Im = 0;

            // Перебор сэмплов
            for(uint8_t sample_num = 0; sample_num < SAMPLES_COUNT; sample_num++)
            {
                #define OFFSET_90_DEGREE (4)
                uint8_t sinus_index = (sample_num * (freq_band + 1)) &  0x0F;
                uint8_t cosinus_index = sinus_index + OFFSET_90_DEGREE;
                Re += (((int16_t)analog.amplitudes_snapshots[chanel][sample_num]) * ((int16_t)sin_cos_table[cosinus_index]));
                Im -= (((int16_t)analog.amplitudes_snapshots[chanel][sample_num]) * ((int16_t)sin_cos_table[sinus_index]));
            }

            // АЧХ
            analog.freq_snapshots[chanel][freq_band] = sqrt_calc_and_norm_0_to_255(((uint32_t)((((int32_t)Re) * ((int32_t )Re)) + (((int32_t)Im) * ((int32_t)Im)))));
            // ФЧХ 
            analog.phase_snapshots[chanel][freq_band] = atan_calc_and_norm_0_to_255(((Re != 0) ? (((int32_t)Im * 8L) / (int32_t)Re) : (Im > 0) ? 0x7FFFFFFF : (Im < 0) ? 0x80000000 : 0L));        
        }
    }
}

/*
    Функция делает следующее:
        1. вычисляет подобие квадратного корня
        2. Нормализует (масштабирует) его значениями от 0 до 255 (делит выходной результат).
        3. ограничивает результат на выходе значениями от 0 до 255.
*/
static inline uint8_t sqrt_calc_and_norm_0_to_255(uint32_t input_value)
{
    static const uint8_t sqrt_table[] = {0, 1, 1, 1, 2, 3, 4, 6, 8, 11, 16, 23, 32, 45, 64, 91, 128, 181, 255};

    #define OUTPUT_POINT_INDEX_MINIMUM (0)
    #define OUTPUT_POINT_INDEX_MAXIMUM (sizeof(sqrt_table) - 1)
    #define INPUT_VALUE_MINIMUM (0UL)
    #define INPUT_VALUE_MAXIMUM (1UL << OUTPUT_POINT_INDEX_MAXIMUM)

    input_value = ((input_value + (1U << 2)) >> 3);

    // Ограничение по минимальному значению
    input_value = (uint32_t)i32_limmit((int32_t)input_value, INPUT_VALUE_MINIMUM, INPUT_VALUE_MAXIMUM);

    // Находим 2 точки между которыми будем производить линейное масштабирование
    uint32_t input_point_2 = INPUT_VALUE_MAXIMUM;
    uint32_t input_point_1 = (input_point_2 >> 1);
    uint8_t output_point_2_index = OUTPUT_POINT_INDEX_MAXIMUM;
    uint8_t output_point_1_index = (output_point_2_index - 1);
    while(output_point_1_index)
    {
        if((input_value >= input_point_1) && (input_value < input_point_2))
        {
            break;
        }
        input_point_2 >>= 1, input_point_1 = input_point_2 >> 1;
        output_point_1_index--;
    }
    output_point_2_index = output_point_1_index + 1;

    // Линейное масштабирование
    int32_t result = i32_scale(input_value, input_point_1, input_point_2, (int32_t)sqrt_table[output_point_1_index], (int32_t)sqrt_table[output_point_2_index]);
    return (uint8_t)(result = i32_limmit(result, 0, 255));
}

/*
    Функция делает следующее:
        1. вычисляет подобие арктангенса
        2. нормализует результат значениями от 0 до 255
        3. ограничивает результат этими значениями от 0 до 255
    
    Если на входе будет 0, то на выходе 128.
*/
static inline uint8_t atan_calc_and_norm_0_to_255(int32_t input_value)
{
    static const uint8_t atan_table[] = {128, 142, 156, 170, 182, 193, 203, 212, 220, 227, 233, 239, 243, 248, 252, 255};
    #define MINIMUM (0) 
    #define MAXIMUM (sizeof(atan_table) - 1)

    uint8_t sign_mask = 0x00;
    if(input_value < 0)
    {
        input_value = (((int16_t)0) - input_value);
        sign_mask = 0xFF;
    }
    input_value = i32_limmit(input_value, MINIMUM, MAXIMUM);
    return (atan_table[input_value] ^ sign_mask);
}

/*
    Ограничение значения переменной на входе
*/
inline int32_t i32_limmit(int32_t in, int32_t lo_lim, int32_t hi_lim)
{
    return ((in >= lo_lim) && (in <= hi_lim)) ? in : (in < lo_lim) ? lo_lim : hi_lim;
}

/*
    Линейное масштабирование
*/
inline int32_t i32_scale(int32_t x, int32_t x1, int32_t x2, int32_t y1, int32_t y2)
{
    return ((((x - x1) * (y2 - y1)) / (x2 - x1)) + y1);
}
