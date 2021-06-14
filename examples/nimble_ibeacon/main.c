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

/* SPECS: https://developer.apple.com/ibeacon/ */

#include <stdlib.h>

#include "nimble_riot.h"

#include "host/ble_hs.h"

#define MS_TO_ADV_ITVL(x)       (x * 1000 / BLE_HCI_ADV_ITVL) /* x in ms */

typedef struct __attribute__((packed)) {
    uint8_t prefix[9];
    uint8_t uuid[16];
    uint16_t major;
    uint16_t minor;
    uint8_t txpower;
} ibeacon_t;

// static int gap_event_cb(struct ble_gap_event, void);
static void start_advertise(void);

static int gap_event_cb(struct ble_gap_event *event, void *arg)
{
    (void)arg;
    (void)event;

    return 0;
}

static void start_advertise(void)
{
    struct ble_gap_adv_params advp;
    int rc;

    memset(&advp, 0, sizeof advp);
    // advp.itvl_min = MS_TO_ADV_ITVL(995);
    // advp.itvl_max = MS_TO_ADV_ITVL(1005);
    advp.itvl_min = MS_TO_ADV_ITVL(95);
    advp.itvl_max = MS_TO_ADV_ITVL(105);
    rc = ble_gap_adv_start(nimble_riot_own_addr_type, NULL, BLE_HS_FOREVER,
                           &advp, gap_event_cb, NULL);
    assert(rc == 0);
    (void)rc;
}

int main(void)
{
    /* instantiate pdu with some random data */
    ibeacon_t pdu;
    /* prefix is defined by Apple, 
    see https://developer.apple.com/ibeacon/ */
    uint8_t prefix[9] = { 0x02, 0x01, 0x06, 0x1a,
    /*                  length, type, see below, length (26)
                        0x06: 0x00000100 BR/EDR not supported
                              0x00000010 LE General Discovarable */
                          0xff, 0x4c, 0x00, 0x02,
    /*                  manuf. specific format, apple (0x4c00), iBeacon Datatype (0x0215) */
                          0x15};
    uint8_t uuid[16] = { 0xa0, 0xa1, 0xa2, 0xa3,
                         0xa4, 0xa5, 0xa6, 0xa7,
                         0xa8, 0xa9, 0xaa, 0xab,
                         0xac, 0xad, 0xad, 0xaf};
    uint16_t major = 0xffff;
    uint16_t minor = 0x0000;
    uint8_t txpower = 0x00;

    memcpy(pdu.prefix, prefix, sizeof(prefix));
    memcpy(pdu.uuid, uuid, sizeof(uuid));
    memcpy(&pdu.major, &major, sizeof(major));
    memcpy(&pdu.minor, &minor, sizeof(minor));
    memcpy(&pdu.txpower, &txpower, sizeof(txpower));

    /* set data */
    ble_gap_adv_set_data((uint8_t *) &pdu, sizeof(pdu));

    /* start to advertise this node */
    start_advertise();

    return 0;
}
