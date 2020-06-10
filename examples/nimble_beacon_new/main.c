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
 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nimble_riot.h"
#include "net/bluetil/ad.h"

#include "host/ble_hs.h"
#include "host/util/util.h"
#include "host/ble_gatt.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "board.h"

#define GATT_DEVICE_INFO_UUID                   0x180A
#define GATT_MANUFACTURER_NAME_UUID             0x2A29
#define GATT_MODEL_NUMBER_UUID                  0x2A24

#define STR_ANSWER_BUFFER_SIZE 100

#define MS_TO_ADV_ITVL(x)    (x * 1000 / BLE_HCI_ADV_ITVL)

static const char *device_name = "NimBLE on RIOT";

static void start_advertise(void);


static int gap_event_cb(struct ble_gap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            if (event->connect.status) {
                start_advertise();
            }
            break;

        case BLE_GAP_EVENT_DISCONNECT:
            start_advertise();
            break;
    }

    return 0;
}

static void start_advertise(void)
{
    static uint8_t buf[40];
    memset(buf, 41, sizeof(buf));

    struct ble_gap_adv_params advp;
    int rc;

    memset(&advp, 0, sizeof advp);
    advp.conn_mode = BLE_GAP_CONN_MODE_UND;
    advp.disc_mode = BLE_GAP_DISC_MODE_GEN;
    advp.itvl_min = MS_TO_ADV_ITVL(1000- 15);
    advp.itvl_max = MS_TO_ADV_ITVL(1000+ 5);
    // rc = ble_gap_adv_set_data(buf, sizeof(buf));
    // NRF_CLOCK->TASKS_HFCLKSTOP=1;
    rc = ble_gap_adv_start(nimble_riot_own_addr_type, NULL, BLE_HS_FOREVER,
                           &advp, gap_event_cb, NULL);
    assert(rc == 0);
    (void)rc;
}

int main(void)
{
    int rc = 0;
    (void)rc;

    /* verify and add our custom services */
    // rc = ble_gatts_count_cfg(gatt_svr_svcs);
    // assert(rc == 0);
    // rc = ble_gatts_add_svcs(gatt_svr_svcs);
    // assert(rc == 0);

    /* set the device name */
    // ble_svc_gap_device_name_set(device_name);
    /* reload the GATT server to link our added services */
    // ble_gatts_start();

    /* configure and set the advertising data */
    uint8_t buf[BLE_HS_ADV_MAX_SZ];
    memset(&buf, 1, sizeof(buf));
    bluetil_ad_t ad;
    bluetil_ad_init_with_flags(&ad, buf, sizeof(buf), BLUETIL_AD_FLAGS_DEFAULT);
    bluetil_ad_add_name(&ad, device_name);
    ble_gap_adv_set_data(ad.buf, ad.pos);

    /* start to advertise this node */
    start_advertise();

    return 0;
}
