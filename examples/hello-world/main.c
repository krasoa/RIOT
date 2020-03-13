/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Hello World application
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 *
 * @}
 */

#ifndef _CLK_SHIFT
    #define _CLK_SHIFT   (0)
#endif
#ifdef _HFCLK_TIMERS
    #include "periph/timer.h"
#endif

#ifdef _GPIO_MODE
    #include "vendor/nrf52.h"
#endif
#include "board.h"

#ifdef _BUTTON_TIMER
    #include "periph/gpio.h"
    #include "periph_conf.h"
    #include "periph/timer.h"
#endif

#ifdef _BUTTON_TIMER
    static int _timer_state[4] = {0,0,0,0};

    static void cb_timer_init(void *arg)
    {
        // (void) arg;
        if (_timer_state[(int)arg - 1] == 0) {
            timer_cb_t cb = 0;
            timer_init(TIMER_DEV((int)arg), 16000000U >> _CLK_SHIFT, cb, NULL);
        } else {
            timer_stop(TIMER_DEV((int)arg));
        }
        _timer_state[(int)arg - 1] ^= 1;

        if (_timer_state[(int)arg -1] == 1) {
            if ((int)arg == 1) {
                // LED0_ON;
            } else if ((int)arg == 2) {
                LED1_ON;
            } else if ((int)arg == 3) {
                LED2_ON;
            } else if ((int)arg == 4) {
                LED3_ON;
            }
        } else {
            if ((int)arg == 1) {
                LED0_OFF;
            } else if ((int)arg == 2) {
                LED1_OFF;
            } else if ((int)arg == 3) {
                LED2_OFF;
            } else if ((int)arg == 4) {
                LED3_OFF;
            }
        }
    }
#endif

int main(void)
{
    #ifdef _DCDC
        NRF_POWER->DCDCEN = 1;
    #endif

    /* periph_timer test */
    #ifdef _HFCLK_TIMERS
        timer_cb_t cb = 0;
        printf("Number of timers: %d\n", _HFCLK_TIMERS);
        int j = (int) _HFCLK_TIMERS;
        for (int i = 0; i < j; i++) {
            printf("Initializing timer %d\n", i);
            if (timer_init(TIMER_DEV(i), 16000000U >> _CLK_SHIFT, cb, NULL) == 0) {
                printf("Initialized timer %d\n", i);
            } else {
                printf("Failed to initialize timer %d\n", i);
            }
        }
    #endif

    #ifdef _GPIO_MODE
        int mode = -1;
        #ifdef _GPIO_OFF
            mode = 0;
            LED0_ON;
        #endif 
        #ifdef _GPIO_IN
            mode = 1;
            LED1_ON;
        #endif 
        #ifdef _GPIO_OUT
            mode = 3;
            LED2_ON;
        #endif
        if (mode == -1) {
            LED3_ON;
            return 1;
        }
        for (int i = 0; i < 8; i++) {
            NRF_GPIOTE->CONFIG[i] = mode;
        }
    #endif

    #ifdef _BUTTON_TIMER
        gpio_init_int(BTN0_PIN, BTN0_MODE, GPIO_FALLING, cb_timer_init, (void *)1);
        gpio_init_int(BTN1_PIN, BTN1_MODE, GPIO_FALLING, cb_timer_init, (void *)2);
        gpio_init_int(BTN2_PIN, BTN2_MODE, GPIO_FALLING, cb_timer_init, (void *)3);
        gpio_init_int(BTN3_PIN, BTN3_MODE, GPIO_FALLING, cb_timer_init, (void *)4);
    #endif

    return 0;
}
