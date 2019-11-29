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

#include <stdio.h>

#include "nimble_riot.h"
#include "net/bluetil/ad.h"

#include "host/ble_gap.h"

#include "net/gnrc/netif/mac.h"

static const char *device_name = "NimBLE on RIOT";

static void start_advertise(void);

// static int gap_event_cb(struct ble_gap_event *event, void *arg)
// {
//     (void)arg;

//     switch (event->type) {
//         case BLE_GAP_EVENT_CONNECT:
//             if (event->connect.status) {
//                 start_advertise();
//             }
//             break;

//         case BLE_GAP_EVENT_DISCONNECT:
//             start_advertise();
//             break;
//     }

//     return 0;
// }

static void start_advertise(void)
{

    struct ble_gap_adv_params advp;
    int rc;

    memset(&advp, 0, sizeof advp);
    /* set mode to non-connectionable, generally discoverable */
    advp.conn_mode = BLE_GAP_CONN_MODE_NON;
    advp.disc_mode = BLE_GAP_DISC_MODE_NON;
    advp.itvl_min  = 1555;
    advp.itvl_max  = 1605;
    // rc = ble_gap_adv_start(nimble_riot_own_addr_type, NULL, BLE_HS_FOREVER,
    //                        &advp, gap_event_cb, NULL);
    rc = ble_gap_adv_start(nimble_riot_own_addr_type, NULL, BLE_HS_FOREVER,
                           &advp, NULL, NULL);
    
    assert(rc == 0);
    (void)rc;
}


int main(void)
{
    puts("NimBLE Beacon");
    printf("Test\n");
    printf("MAC Address: %s\n", NET_GNRC_NETIF_MAC_H);

    /* configure and set the advertising data */
    uint8_t buf[BLE_HS_ADV_MAX_SZ];
    bluetil_ad_t ad;
    bluetil_ad_init_with_flags(&ad, buf, sizeof(buf), BLUETIL_AD_FLAGS_DEFAULT);
    bluetil_ad_add_name(&ad, device_name);
    ble_gap_adv_set_data(ad.buf, ad.pos);

    /* start to advertise this node */
    start_advertise();

    return 0;
}
