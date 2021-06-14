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

/* SPECS: 
    Broadcast Formats: https://github.com/ruuvi/ruuvi-sensor-protocols/blob/4b3a50783bcd5896c619e8bd179bd29dec00ca18/broadcast_formats.md
    Data Format 5: https://github.com/ruuvi/ruuvi-sensor-protocols/blob/4b3a50783bcd5896c619e8bd179bd29dec00ca18/dataformat_05.md 
*/


#include <stdlib.h>

#include "nimble_riot.h"

#include "host/ble_hs.h"

#ifndef PDU_LENGTH
    #define PDU_LENGTH 1
#endif

#ifndef ITVL
    #define ITVL 100
#endif

#define MS_TO_ADV_ITVL(x)       (x * 1000 / BLE_HCI_ADV_ITVL) /* x in ms */
/* increments of 1.25ms possible */

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
    int advp_itvl_min;

    /* check if adv interval in valid range;
    +/- x units to be able to add jitter (advp.itvl_min and max)
    advp.itvl_min is set to advp_itvl
    1 unit == 1.25ms 
     */
    if (MS_TO_ADV_ITVL(ITVL) >  BLE_HCI_ADV_ITVL_MAX) {
        /* +/- 10ms between min and max with upper bound at max */
        advp_itvl_min = BLE_HCI_ADV_ITVL_MAX;
    } else if (MS_TO_ADV_ITVL(ITVL) < BLE_HCI_ADV_ITVL_MIN) {
        /* +/- 10ms between min and max with upper bound at min */
        advp_itvl_min = BLE_HCI_ADV_ITVL_MIN;
    } else {
        /* +/- 10ms between min and max centered around chosen interval */
        advp_itvl_min = MS_TO_ADV_ITVL(ITVL);
    }

    memset(&advp, 0, sizeof advp);
    advp.itvl_min = advp_itvl_min;
    advp.itvl_max = advp_itvl_min;
    advp.conn_mode = BLE_GAP_CONN_MODE_NON;
    advp.disc_mode = BLE_GAP_DISC_MODE_NON;
    rc = ble_gap_adv_start(nimble_riot_own_addr_type, NULL, BLE_HS_FOREVER,
                           &advp, gap_event_cb, NULL);
    assert(rc == 0);
    (void)rc;
}

int main(void)
{
    int pdu_length;
    // uint8_t pdu;

    if (PDU_LENGTH > 31) {
        pdu_length = 31;
    } else if (PDU_LENGTH < 1) {
        pdu_length = 1;
    } else {
        pdu_length = PDU_LENGTH;
    } 

    /* instantiate pdu with some random data */
    /* set data */
    if (pdu_length == 8) {
        uint8_t pdu[8] = { 0x02, 0x01, 0x06, 0x04,
    /*                  length, type, see below, length (26)
                        0x06: 0x00000100 BR/EDR not supported
                              0x00000010 LE General Discovarable */
                          0xff, 0x04, 0x99, 0x05};
        ble_gap_adv_set_data((uint8_t *) &pdu, sizeof(pdu));
    } else {
        uint8_t pdu[pdu_length];
        memset(&pdu, 0x01, pdu_length);
        ble_gap_adv_set_data((uint8_t *) &pdu, sizeof(pdu));
    }

    /* start to advertise this node */
    start_advertise();

    return 0;
}
