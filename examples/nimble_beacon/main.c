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

#define MS_TO_ADV_ITVL(x)    (x * 1000 / BLE_HCI_ADV_ITVL)

static const char *_device_name = "nimble beacon";

static uint8_t _buf[20];

static void start_advertise(void)
{
    memset(_buf, 0, sizeof(_buf));

    struct ble_gap_adv_params advp;
    int rc;

    memset(&advp, 0, sizeof(advp));
    /* set mode to non-connectionable, non-discoverable*/
    advp.conn_mode = BLE_GAP_CONN_MODE_NON;
    advp.disc_mode = BLE_GAP_DISC_MODE_NON;
    /* connection interval 1s+-5ms */
    advp.itvl_min = MS_TO_ADV_ITVL(995);
    advp.itvl_max = MS_TO_ADV_ITVL(1005);
    rc = ble_gap_adv_set_data(_buf, sizeof(_buf));
    rc = ble_gap_adv_start(nimble_riot_own_addr_type, NULL, BLE_HS_FOREVER,
                           &advp, NULL, NULL);
    
    assert(rc == 0);
    (void)rc;
}


int main(void)
{
    puts("NimBLE Beacon");

    /* configure and set the advertising data */
    uint8_t buf[BLE_HS_ADV_MAX_SZ];
    bluetil_ad_t ad;
    bluetil_ad_init_with_flags(&ad, buf, sizeof(buf), BLUETIL_AD_FLAGS_DEFAULT);
    bluetil_ad_add_name(&ad, _device_name);
    ble_gap_adv_set_data(ad.buf, ad.pos);

    /* start to advertise this node */
    start_advertise();

    return 0;
}
