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
 * @brief       Example application for demonstrating the RIOT network stack
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <inttypes.h>


#include "net/sock/udp.h"
#include "nimble_netif.h"
#include "xtimer.h"
#include "nimble_autoconn.h"
#define NIMBLE_AUTOCONN_CONN_ITVL (1000U)

// #define MS_TO_CONN_ITVL(x)      (x * 1000 / BLE_HCI_CONN_ITVL) /* x in ms */
#define UDP_DEFAULT_PORT 8888 /* Default UDP port */

static uint8_t _buf[20]; /* We send 20 Byte of payload */

int spam_master(sock_udp_t sock, sock_udp_ep_t remote) {
    ssize_t res;

    memset(_buf,0,sizeof(_buf));

    while (1) {
        for (int i = 0; i < 3; i++) {
            if ((res = sock_udp_send(&sock, &_buf, sizeof(_buf), &remote) < 0)) {
                sock_udp_close(&sock);
                return 1;
            }
        }
        xtimer_sleep(1);
    }
}

int main(void)
{
    // const struct ble_gap_upd_params _conn_params = {
    // .itvl_min = MS_TO_CONN_ITVL(5000),
    // .itvl_max = MS_TO_CONN_ITVL(5000),
    // .latency = 0,
    // .supervision_timeout = (500),
    // .min_ce_len = 1,
    // .max_ce_len = 1,
    // };
     if (CoreDebug->DHCSR % 2 == 1) {
        LED0_ON;
    } else {
        LED0_OFF;
    }

    ssize_t res;

    /* Create socket */
    sock_udp_t sock;
    sock_udp_ep_t local = SOCK_IPV6_EP_ANY;
    sock_udp_ep_t remote = SOCK_IPV6_EP_ANY;
    local.port = UDP_DEFAULT_PORT;
    if (sock_udp_create(&sock, &local, NULL, 0) < 0) {
        return 1;
    }

    /* Bind remote socket by receiving a packet from it */
    if ((res = sock_udp_recv(&sock, _buf, sizeof(_buf), SOCK_NO_TIMEOUT, &remote)) >= 0) {
        // nimble_netif_update(0, &_conn_params);
        spam_master(sock, remote);
    }

    /* should be never reached */
    return 0;
}

