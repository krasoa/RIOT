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

#include "msg.h"
#include "udp.c"
#include "xtimer.h"


int main(void)
{
    /* we need a message queue for the thread running the shell in order to
     * receive potentially fast incoming networking packets */
    puts("RIOT network stack example application");

    while(1) {
        for (int i=0; i<3; i++) {
            send("fe80::f337:f9ff:fe8d:264c", "8888", "AAAAAAAAAAAAAAAAAAAA", 1, 10);
        }
        xtimer_sleep(1);
    }

    /* should be never reached */
    return 0;
}
