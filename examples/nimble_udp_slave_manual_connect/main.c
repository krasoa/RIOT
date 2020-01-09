/*
 * Copyright (C) 2015 Freie Universität Berlin
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
 * @brief       Application sending UDP packets through BLE
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *              Aleks Krasowski <a.krasowski@fu-berlin.de>
 *
 * @}
 */

// #include <stdio.h>
// #include <stdlib.h>

// #include "fmt.h"
#include "net/sock/udp.h"
#include "fmt.h"
#include "xtimer.h"
#include "nimble_riot.h"
#include "nimble_netif.h"
#include "nimble_netif_conn.h"
#include "net/bluetil/ad.h"
#include "net/bluetil/addr.h"


#define UDP_DEFAULT_PORT 8888 /* Default UDP port */
#define CONN_TIMEOUT_MUL (5U)
#define DEFAULT_SCAN_DURATION       (500U)      /* 500ms */
#define DEFAULT_CONN_TIMEOUT        (500U)      /* 500ms */
#define DEFAULT_NODE_NAME           "udpSlave"

uint8_t buf[20]; /* We send 20 Byte of payload */
ble_addr_t addr;

int spam_master(sock_udp_t sock, sock_udp_ep_t remote) {
    ssize_t res;

    /* Initialize 20 Bytes of Payload */
    for (int i = 0; i < 20; i++) {
        buf[i] = (uint8_t) 41;
    }

    /* Send forever 3 UDP packets in an interval of 1s */
    while (1) {
        for (int i = 0; i < 3; i++) {
            if ((res = sock_udp_send(&sock, &buf, sizeof(buf), &remote) < 0)) {
                puts("Error sending message");
                sock_udp_close(&sock);
                return 1;
            }
        }
        xtimer_sleep(1);
    }
}

int main(void)
{
    xtimer_sleep(2);
    puts("Nimble 'Send 3 UDP Packets every Second' Application");
    ssize_t res;
    
    int res_ble;
    (void)res_ble;
    uint8_t buf[BLE_HS_ADV_MAX_SZ];
    bluetil_ad_t ad;
    const struct ble_gap_adv_params _adv_params = {
        .conn_mode = BLE_GAP_CONN_MODE_UND,
        .disc_mode = BLE_GAP_DISC_MODE_LTD,
        .itvl_min = BLE_GAP_ADV_FAST_INTERVAL2_MIN,
        .itvl_max = BLE_GAP_ADV_FAST_INTERVAL2_MAX,
    };

    /* build advertising data */
    res_ble = bluetil_ad_init_with_flags(&ad, buf, BLE_HS_ADV_MAX_SZ,
                                     BLUETIL_AD_FLAGS_DEFAULT);

    uint16_t ipss = BLE_GATT_SVC_IPSS;
    res_ble = bluetil_ad_add(&ad, BLE_GAP_AD_UUID16_INCOMP, &ipss, sizeof(ipss));
    assert(res_ble == BLUETIL_AD_OK);
    const char *name = DEFAULT_NODE_NAME;
    res_ble = bluetil_ad_add(&ad, BLE_GAP_AD_NAME, name, strlen(name));
    if (res_ble != BLUETIL_AD_OK) {
        puts("err: the given name is too long");
        return 0;
    }
    
    /* start listening for incoming connections */
    res_ble = nimble_netif_accept(ad.buf, ad.pos, &_adv_params);
    if (res_ble != NIMBLE_NETIF_OK) {
        //printf("err: unable to start advertising (%i)\n", res);
        puts("err: unable to start advertising!");
    }
    else {
        puts("Success: advertising this node!");
    }
    /* Create socket */
    sock_udp_t sock;
    sock_udp_ep_t local = SOCK_IPV6_EP_ANY;
    sock_udp_ep_t remote = SOCK_IPV6_EP_ANY;
    local.port = UDP_DEFAULT_PORT;
    if (sock_udp_create(&sock, &local, NULL, 0) < 0) {
        puts("Error creating UDP socket");
        return 1;
    }
    puts("UDP Socket created successfully");

    // conn_init();
    // if ((ble_res = nimble_netif_connect(&addr, &_conn_params, (uint32_t) 0)) < 0) {
    //     puts("NIMBLE: Failed to connect to device");
    // }
    // puts("NIMBLE: Connected to device");

    /* Bind remote socket by receiving a packet from it */
    if ((res = sock_udp_recv(&sock, buf, sizeof(buf), SOCK_NO_TIMEOUT, &remote)) >= 0) {
        puts("Received a message, starting SPAM_MASTER!");
            
        spam_master(sock, remote);

    } else {
        puts("Error receiving message");
    }


    /* should be never reached */
    return 0;
}
