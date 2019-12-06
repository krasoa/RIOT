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

#include <stdio.h>
#include <inttypes.h>

#include "msg.h"
#include "net/sock/udp.h"
#include "xtimer.h"


#define UDP_DEFAULT_PORT 8888 /* Default UDP port */

uint8_t buf[20]; /* We send 20 Byte of payload */

int spam_master(sock_udp_t sock, sock_udp_ep_t remote) {
    ssize_t res;

    for (int i = 0; i < 20; i++) {
        buf[i] = (uint8_t) 41;
    }

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
    ssize_t res;

    /* Create socket */
    sock_udp_t sock;
    sock_udp_ep_t local = SOCK_IPV6_EP_ANY;
    sock_udp_ep_t remote = SOCK_IPV6_EP_ANY;
    local.port = UDP_DEFAULT_PORT;
    if (sock_udp_create(&sock, &local, NULL, 0) < 0) {
        puts("Error creating UDP socket");
        return 1;
    }

    /* Bind remote socket by receiving a packet from it */
    if ((res = sock_udp_recv(&sock, buf, sizeof(buf), SOCK_NO_TIMEOUT, &remote)) >= 0) {
        puts("Received a message, starting SPAM_MASTER!");
            
        spam_master(sock, remote);

    /* Spam the master by sending 3 Packets in an interval of 1s */
        // while (1) {
        //     for (int i = 0; i < 3; i++) {
        //         if ((res = sock_udp_send(&sock, buf, sizeof(buf), &remote) < 0)) {
        //             puts("Error sending message");
        //             sock_udp_close(&sock);
        //             return 1;
        //         }
        //     }
        //     xtimer_sleep(1);
        // }
    } else {
        puts("Error receiving message");
    }


    /* should be never reached */
    return 0;
}
