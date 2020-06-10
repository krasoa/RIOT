/*
 * Copyright (C) 2018 Freie Universität Berlin
 *               2018 Codecoup
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
 * @brief       BLE peripheral example using NimBLE
 *
 * Have a more detailed look at the api here:
 * https://mynewt.apache.org/latest/tutorials/ble/bleprph/bleprph.html
 *
 * More examples (not ready to run on RIOT) can be found here:
 * https://github.com/apache/mynewt-nimble/tree/master/apps
 *
 * Test this application e.g. with Nordics "nRF Connect"-App
 * iOS: https://itunes.apple.com/us/app/nrf-connect/id1054362403
 * Android: https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Andrzej Kaczmarek <andrzej.kaczmarek@codecoup.pl>
 * @author      Hendrik van Essen <hendrik.ve@fu-berlin.de>
 * @author      Aleks Krasowski <a.krasowski@fu-berlin.de>
 *
 * @}
 */

 // base current consumption: 194uA
 // node addr: D6:55:05:B5:52:C5

#include "nimble_riot.h"
#include "net/bluetil/ad.h"
#include "host/ble_gap.h"
#include "vendor/nrf52.h"
#include "board.h"
#ifdef HFCLK_TIMERS
    #include "periph/timer.h"
#endif

#ifdef RTT
    #include "periph/rtt.h"
#endif

#ifndef ITVL
    #define ITVL             (1000)
#endif

#ifndef PAYLOAD_SIZE
    #define PAYLOAD_SIZE    (20)
#endif

#define MS_TO_ADV_ITVL(x)    (x * 1000 / BLE_HCI_ADV_ITVL)
#define F_TIMER              (16000000U)

//static const char *_device_name = "nimble beacon";

static uint8_t _buf[PAYLOAD_SIZE];

static void start_advertise(void)
{
    memset(_buf, 'a', sizeof(_buf));

    struct ble_gap_adv_params advp;
    int rc;

    memset(&advp, 0, sizeof(advp));
    /* set mode to non-connectionable, non-discoverable*/
    advp.conn_mode = BLE_GAP_CONN_MODE_NON;
    advp.disc_mode = BLE_GAP_DISC_MODE_NON;
    /* connection interval 1s+-5ms */
    advp.itvl_min = MS_TO_ADV_ITVL((1000 - 15));
    advp.itvl_max = MS_TO_ADV_ITVL((1000 + 15));
    // advp.itvl_min = 1000 * 1000 / BLE_HCI_ADV_ITVL;
    // advp.itvl_max = 1000 * 1000 / BLE_HCI_ADV_ITVL;
    rc = ble_gap_adv_set_data(_buf, sizeof(_buf));
    rc = ble_gap_adv_start(nimble_riot_own_addr_type, NULL, BLE_HS_FOREVER,
                           &advp, NULL, NULL);
    
    assert(rc == 0);
    (void)rc;
}


int main(void)
{
    /* dcdc test 
       more efficient with high current - less efficient with low current */
    #ifdef DCDC
        NRF_POWER->DCDCEN = 1;
    #endif

    /* periph_timer test */
    #ifdef HFCLK_TIMERS
        timer_cb_t cb = 0;
        printf("Number of timers: %d\n", HFCLK_TIMERS);
        int j = (int) HFCLK_TIMERS;
        for (int i = 0; i < j; i++) {
            printf("Initializing timer %d\n", i);
            if (timer_init((tim_t) i, 16000000U >> CLK_SHIFT, cb, NULL) == 0) {
                printf("Initialized timer %d\n", i);
            } else {
                printf("Failed to initialize timer %d\n", i);
                LED1_ON;
            }
        }
    #endif

#ifdef RTT
    rtt_init();
    rtt_poweroff();
#endif

    // enable internal oscillator (HFINT instead of crystal oscillator - saves 187uA!)
    // consumption: 310uA (mit stdio_null 2uA)
    // NRF_CLOCK->TASKS_HFCLKSTOP=1;

    // enable LFCLK internal RC (slightly higher consumption 0.6 vs 0.25 according to datasheet)
    // NRF_CLOCK->LFCLKSRC = 0;
    // NRF_CLOCK->TASKS_LFCLKSTOP = 1;
    // NRF_CLOCK->TASKS_LFCLKSTART = 1;

    //  enable LFCLK synthesized from HFCLK (lose 3uA)
    // consumption: 502uA
    // NRF_CLOCK->LFCLKSRC = 2;
    // NRF_CLOCK->TASKS_LFCLKSTOP = 1;
    // NRF_CLOCK->TASKS_LFCLKSTART = 1;

    // disabling UART skyrockets the current consumption to 2,518mA
    // NRF_UART0->ENABLE = 0;
    // NRF_UART0->TASKS_STOPRX = 1;
    // NRF_UART0->TASKS_STOPTX = 1;
    // NRF_UART0->BAUDRATE = 0x0004F000;

    /* configure and set the advertising data */
    uint8_t buf[BLE_HS_ADV_MAX_SZ];
    bluetil_ad_t ad;
    bluetil_ad_init_with_flags(&ad, buf, sizeof(buf), BLUETIL_AD_FLAGS_DEFAULT);
	//bluetil_ad_add_name(&ad, _device_name);
    ble_gap_adv_set_data(ad.buf, ad.pos);

    /* start to advertise this node */
    start_advertise();

    return 0;
}
