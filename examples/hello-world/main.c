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

#include <stdio.h>
#include <xtimer.h>
#include <vendor/nrf52.h>
// #include <irq.h>
// #include <irq_handler.h>
// #include <boards/nrf52dk/include/board.h>
#include <periph/gpio.h>
// #include <ps.h>

static void cb(void *arg)
{
    printf("INT: external interrupt from pin %i\n", (int)arg);
    // printf("Doing soft-reset\n");
}

int main(void)
{
    // uint32_t addr = 0x00000608;
    //int test =123;
    //__uint32_t val = 1;
    //*(volatile __uint32_t*) 0x10001208 = val;
    // __asm (
    //     "ldr r1, =0x1\n" 
    //     // "mov %[test], r1"
    //     "mov 0x40000608, r1"
    //     : [test] "=r" (test)
    //     :
    //     : "r1"
    //     );
    *((uint32_t *) 0x40000544) = (uint32_t) 1;
    NRF_UICR->APPROTECT = (uint32_t) 0;
    xtimer_sleep(3);
    //printf("%d zahl\n", test);
    puts("Hello World!");
    switch(NRF_POWER->RESETREAS) {
        case 1:
            printf("%d: Reset from pin-reset\n", 1);
            break;
        case 2:
            printf("%d: Reset from watchdog\n", 2);
            break;
        case 4:
            printf("%d: Reset from soft reset\n", 4);
            break;
        case 8:
            printf("%d: Reset from CPU lock-up\n", 8);
            break;
        case 16:
            printf("%d: Reset due to wake up from System OFF mode when      triggered from DETECT signal from GPIO\n", 16);
            break;
        case 32: 
            printf("%d: Reset due to wake up from System OFF mode when                   triggered from ANADETECT signal from LPCOMP\n", 32);
            break;
        case 64:
            printf("%d: Reset due to wakeup from System OFF mode when                     triggered from entering into debug interface mode\n", 64);
            break;
        default:
            printf("%ld: Could not determine\n", NRF_POWER->RESETREAS);
    }
    // printf("%ld\n", *(volatile __uint32_t*) 0x40000400);
    // printf("Access Port Protection: %lu\n", NRF_UICR->APPROTECT);
    // printf("Overwriting APP!\n");
    // printf("Access Port Protection: %lu\n", NRF_UICR->APPROTECT);
    // printf("%lu\n", *((uint32_t *) 0x40000544));
    printf("DWT->CYCCNT: %lu\n", *(uint32_t *) 0xe0001004);  /* DWT->CYCCNT */
    printf("DHCSR: %lx\n", *(uint32_t *) 0xE000EDF0);
    if (CoreDebug->DHCSR % 2 == 1) {
        printf("DHCSR: %lx\n", *(uint32_t *) 0xE000EDF0);
        printf("DHCSR: %lx\n", *(uint32_t *) 0xE000EDF0);
        LED0_ON;
    } else {
        LED0_OFF;
    }
    // NRF_POWER->SYSTEMOFF = 1;

    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_MCU);

    // uint32_t resetreas = NRF_POWER->RESETREAS;
    // printf('%ld\n', *(volatile __uint32_t*) (NRF_POWER));
    // printf(resetreas);
    //ps();

    gpio_init_int(GPIO_PIN(0,13), GPIO_IN_PU, GPIO_FALLING, &cb, NULL);

    return 0;
}
