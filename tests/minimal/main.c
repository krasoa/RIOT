/*
 * Copyright (C) 2015 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       minimal RIOT application, intended as size test
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 *
 * @}
 */
#include <vendor/nrf52.h>
int main(void)
{
    int i;
    for (i = 0; i < 9; i++) {
        NRF_POWER->RAM[i].POWER = NRF_POWER->RAM[i].POWER & 0xFFFFFFFC; 
    }
    return 0;
}
