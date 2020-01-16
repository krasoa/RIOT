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

#include "shell.h"
#include "msg.h"
#include "udp.c"
#include "nimble_riot.h"
#include "nimble_netif.h"
#include "nimble_netif_conn.h"
#include "net/bluetil/ad.h"
#include "net/bluetil/addr.h"

#define ENABLE_DEBUG            (1)
#define MAIN_QUEUE_SIZE         (8)
#define DEFAULT_CONN_TIMEOUT    (500U)      /* 500ms */
#define MS_TO_CONN_ITVL(x)      (x * 1000 / BLE_HCI_CONN_ITVL) /* x in ms */
#define MS_TO_SCAN_ITVL(x)      (x * 1000 / BLE_HCI_SCAN_ITVL) /* x in ms */
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

extern int udp_cmd(int argc, char **argv);

int conn_cmd(int argc, char **argv) 
{
    if (argc < 2) {
        printf("usage: %s <addr> [itvl in ms]\n", argv[0]);
        return 1;
    }

    uint8_t addr_in[BLE_ADDR_LEN];
    /* Bluetooth address strings have size 17 if colons are counted */
    /* 12 without */
    if (bluetil_addr_from_str(addr_in, argv[1]) != NULL) { 
        ble_addr_t addr = { .type = BLE_ADDR_RANDOM };
        /* NimBLE expects address in little endian, so swap */
        bluetil_addr_swapped_cp(addr_in, addr.val);
        int res;
        printf("%s: %d, %d\n", argv[2],(int) argv[2], atoi(argv[2]));
        if (argc == 3) {
            const struct ble_gap_conn_params _conn_params = {
                .scan_itvl = MS_TO_SCAN_ITVL(100),
                .scan_window = MS_TO_SCAN_ITVL(100),
                .itvl_min = MS_TO_CONN_ITVL(atoi(argv[2])),
                .itvl_max = MS_TO_CONN_ITVL(atoi(argv[2])),
                .latency = 0,
                .supervision_timeout = (500),
                .min_ce_len = 0,
                .max_ce_len = 0,
            };
            res = nimble_netif_connect(&addr, &_conn_params, DEFAULT_CONN_TIMEOUT);
        }
        else {
            res = nimble_netif_connect(&addr, NULL, DEFAULT_CONN_TIMEOUT);
        }
        if (res < 0) {
            printf("err: unable to trigger connection sequence (%i)\n", res);
            return 0;
        }
        printf("initiated connection procedure with ");
        uint8_t addrn[BLE_ADDR_LEN];
        bluetil_addr_swapped_cp(addr.val, addrn);
        bluetil_addr_print(addrn);
        puts("");
    }
    else {
        puts("error: invalid command");
    }
    return 0;  
}

// extern void start_server(char *port_str);

static const shell_command_t shell_commands[] = {
    { "udp", "send data over UDP and listen on UDP ports", udp_cmd },
    { "conn", "connect to BLE node", conn_cmd},
    { NULL, NULL, NULL }
};


int main(void)
{
    /* we need a message queue for the thread running the shell in order to
     * receive potentially fast incoming networking packets */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    puts("RIOT network stack example application");

    start_server("8888");
    /* start shell */
    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should be never reached */
    return 0;
}
