#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"

#define BLINK_DURATION 300
#define BLINK_PERIOD   500

#define BLINK_PAUSE_BETWEEN_LEDS 1000

void blink_led(int led_id)
{
    bsp_board_led_invert(led_id);
    nrf_delay_ms(BLINK_DURATION);
    bsp_board_led_invert(led_id);
}

void blink_led_many(int led_id, int n)
{
    if (n == 0) return;

    for (int i = 0; i < n - 1; i++)
    {
        blink_led(led_id);
        nrf_delay_ms(BLINK_PERIOD - BLINK_DURATION);
    }

    blink_led(led_id);
}

// Device ID is 7202
const int BLINKS[LEDS_NUMBER] = { 7, 2, 0, 2 };

int main(void)
{
    bsp_board_init(BSP_INIT_LEDS);

    while (true)
    {
        for (int i = 0; i < LEDS_NUMBER; i++)
        {
	    blink_led_many(i, BLINKS[i]);
	    nrf_delay_ms(BLINK_PAUSE_BETWEEN_LEDS);
        }
    }
}
