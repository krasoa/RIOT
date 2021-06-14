// ### Konfiguration ###

// # Hab nochmal drüber nachgedacht: Wenn du die fehlenden Parameter in Haukes netif hinzufügst
// # (RIOT/pkg/nimble/Makefile.include unter nimble_netif), sollten extended ohne probleme auch mit Haukes
// # netif funktionieren. Klar kein IPv6, aber du solltest easy in eigenen funktionen nimble dafür nutzen können

// # Falls hier im Code defines sind mit JELLING_BLABLA: Alle meine wichtigen defines und extended advertising defaults sind hier:
// # https://github.com/janosbrodbeck/RIOT/blob/bachelor/jelling/pkg/nimble/jelling/include/jelling.h
// # Für den Zugriff auf Haukes address definitionen (e.g. nimble_riot_own_addr_type)
// # -> #include "nimble_riot.h"
#include <stdlib.h>
#include "nimble_riot.h"
#include "net/gnrc/pktbuf.h"
#include "net/ble.h"
#include "host/ble_hs.h"
#include <stdio.h>

#ifndef PDU_SIZE
#define PDU_SIZE 1000
#endif

#ifndef ITVL
#define ITVL 1000
#endif

#define MS_TO_ADV_ITVL(x)       (x * 1000 / BLE_HCI_ADV_ITVL) /* x in ms */


static int _gap_event(struct ble_gap_event *event, void *arg)
{
    (void) arg;

    switch(event->type) {
        case BLE_GAP_EVENT_ADV_COMPLETE:
            return 0;
        case BLE_GAP_EVENT_EXT_DISC:
            // _on_data(event, arg);
            return 0;
        case BLE_GAP_EVENT_DISC_COMPLETE:
            return 0;

    }
    return 0;
}


// ### Advertising  ######
// # Nur einmalig notwendig, oder wenn man die Parameter ändern möchte
// # Instance 0 ist die erste, mehr willst du nicht
static int _configure_adv_instance(uint8_t instance) {
    struct ble_gap_ext_adv_params params = { 0 };
    int8_t selected_tx_power;

    memset(&params, 0, sizeof(params));

    /* set advertise parameters */
    params.scannable = 0;
    params.directed = 0;
    params.own_addr_type = nimble_riot_own_addr_type;
    params.channel_map = BLE_GAP_ADV_DFLT_CHANNEL_MAP;
    params.filter_policy = 0;
    // params.itvl_min = _config.advertiser_itvl_min;
    // params.itvl_max = _config.advertiser_itvl_max;
    params.itvl_min = MS_TO_ADV_ITVL(ITVL);
    params.itvl_max = MS_TO_ADV_ITVL(ITVL);
    params.tx_power = 127;
    params.primary_phy = BLE_HCI_LE_PHY_1M;
    params.secondary_phy = BLE_HCI_LE_PHY_1M;
    //params.sid = 0;

    int rc = ble_gap_ext_adv_configure(instance, &params, &selected_tx_power, _gap_event, NULL);
    if (rc) {
        printf("_configure_adv_instance: failed to configure advertising instance. "
            "Return code: 0x%02X\n", rc);
        return -1;
    }

    // # adresse, mit der advertised wird, setzen für diese instanz
    ble_addr_t addr;
    rc = ble_hs_id_copy_addr(nimble_riot_own_addr_type, addr.val, NULL);
    addr.type = nimble_riot_own_addr_type;
    if (rc) {
        printf("_configure_adv_instance: failed to retrieve BLE address\n");
        return -1;
    }
    rc = ble_gap_ext_adv_set_addr(instance, &addr);
    if (rc) {
        printf("_configure_adv_instance: could not set address\n");
        return -1;
    }

    return rc;
}



// // # tatsächliches senden
static int _send_pkt(uint8_t instance, struct os_mbuf *mbuf)
{
    int res = ble_gap_ext_adv_set_data(instance, mbuf);
    if (res) {
        return res;
    }

    res = ble_gap_ext_adv_start(instance, 0,
                            1);
    if (res) {
        printf("Couldn't start advertising. Return code: 0x%02X\n", res);
        return -1;
    }

    if(IS_ACTIVE(JELLING_DEBUG_ADVERTISING_PROCESS)) {
        printf("DEBUG: Advertising for packet started successfully\n");
    }
    return res;
}

int main(void) {
    uint8_t data[PDU_SIZE];
    struct os_mbuf *mbuf;
    memset(data, 0x41, sizeof(data));
    int res;
    // /* allocate mbuf */
    mbuf = os_msys_get_pkthdr(PDU_SIZE, 0);
    if (mbuf == NULL) {
        return -ENOBUFS;
    }

    res = os_mbuf_append(mbuf, data, sizeof(data));
    if (res != 0 ) {
        return res;
    }

    _configure_adv_instance(0);
    while (1) {
        mbuf = os_msys_get_pkthdr(PDU_SIZE, 0);
        if (mbuf == NULL) {
            return -ENOBUFS;
        }

        res = os_mbuf_append(mbuf, data, sizeof(data));
        if (res != 0 ) {
            return res;
        }
        _send_pkt( 0, mbuf);
        ztimer_sleep(ZTIMER_MSEC, ITVL);
    }
    return 0;
}