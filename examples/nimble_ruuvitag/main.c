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

#define MS_TO_ADV_ITVL(x)       (x * 1000 / BLE_HCI_ADV_ITVL) /* x in ms */
/* increments of 1.25ms possible */
typedef struct __attribute__((packed)) {
    uint8_t prefix[8];
    uint16_t temperature;
    uint16_t humidity;
    uint16_t pressure;
    uint16_t acceleration_x;
    uint16_t acceleration_y;
    uint16_t acceleration_z;
    uint16_t power_info;
    uint8_t movement_counter;
    uint16_t sequence_number;
    uint8_t mac_addr[6]; /* ??? isn't this already in the PDU ???*/
} ruuvitag_t;

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
    /* https://github.com/ruuvi/ruuvitag_fw/blob/f637cb8693beba37e3a5f6349b2b613813b2f45c/ruuvi_examples/ruuvi_firmware/bluetooth_application_config.h */
    // advp.itvl_min = MS_TO_ADV_ITVL(995);
    // advp.itvl_max = MS_TO_ADV_ITVL(1005);
    // advp.itvl_min = MS_TO_ADV_ITVL(95);
    // advp.itvl_max = MS_TO_ADV_ITVL(105);
    /* ruuvi fast, lowest interval by Apples guidelines */
    // advp.itvl_min = MS_TO_ADV_ITVL(1280);
    // advp.itvl_max = MS_TO_ADV_ITVL(1290);
    /* ruuvi slow, lowest * 5*/
    advp.itvl_min = MS_TO_ADV_ITVL((1285 * 5) - 5);
    advp.itvl_max = MS_TO_ADV_ITVL((1285 * 5) + 5);
    rc = ble_gap_adv_start(nimble_riot_own_addr_type, NULL, BLE_HS_FOREVER,
                           &advp, gap_event_cb, NULL);
    assert(rc == 0);
    (void)rc;
}

int main(void)
{
    /* instantiate pdu with some random data */
    ruuvitag_t pdu;
    uint8_t prefix[8] = { 0x02, 0x01, 0x06, 0x1b,
    /*                  length, type, see below, length (26)
                        0x06: 0x00000100 BR/EDR not supported
                              0x00000010 LE General Discovarable */
                          0xff, 0x04, 0x99, 0x05};
    /*                  manuf. specific format, ruuvi (0x0499), 
                        Datatype: Data Format 5 (RAWv2) (0x05) */
    uint16_t temperature = 0x0001;
    uint16_t humidity = 0x0203;
    uint16_t pressure = 0x0405;
    uint16_t acceleration_x = 0x0607;
    uint16_t acceleration_y = 0x0809;
    uint16_t acceleration_z = 0x0a0b;
    uint16_t power_info = 0x0c0d;
    uint8_t movement_counter = 0x0e;
    uint16_t sequence_number = 0x0f00;
    uint8_t mac_addr[6] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };

    memcpy(pdu.prefix, prefix, sizeof(prefix));
    memcpy(&pdu.temperature, &temperature, sizeof(temperature));
    memcpy(&pdu.humidity, &humidity, sizeof(humidity));
    memcpy(&pdu.pressure, &pressure, sizeof(pressure));
    memcpy(&pdu.acceleration_x, &acceleration_x, sizeof(acceleration_x));
    memcpy(&pdu.acceleration_y, &acceleration_y, sizeof(acceleration_y));
    memcpy(&pdu.acceleration_z, &acceleration_z, sizeof(acceleration_z));
    memcpy(&pdu.power_info, &power_info, sizeof(power_info));
    memcpy(&pdu.movement_counter, &movement_counter, sizeof(movement_counter));
    memcpy(&pdu.sequence_number, &sequence_number, sizeof(sequence_number));
    memcpy(&pdu.mac_addr, &mac_addr, sizeof(mac_addr));

    /* set data */
    ble_gap_adv_set_data((uint8_t *) &pdu, sizeof(pdu));

    /* start to advertise this node */
    start_advertise();

    return 0;
}
