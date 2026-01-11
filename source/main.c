////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	разработал Николаев Сергей (г. Кулебаки).
	Все права защищены.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "analog.h"
#include "matrix.h"
#include "watchdog.h"
#include "mcu.h"

int main(void)
{
    cli();
        watchdog_init(WATCHDOG_SETUP_TIME_500_MS);
        matrix_init();
        analog_inputs_init();
    sei();

    while (1)
    {
        watchdog_reset();
        analog_inputs_main();
        matrix_main();
    }

    return 0;
}