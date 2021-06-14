// ### Konfiguration ###

// # Hab nochmal drüber nachgedacht: Wenn du die fehlenden Parameter in Haukes netif hinzufügst
// # (RIOT/pkg/nimble/Makefile.include unter nimble_netif), sollten extended ohne probleme auch mit Haukes
// # netif funktionieren. Klar kein IPv6, aber du solltest easy in eigenen funktionen nimble dafür nutzen können

// # Falls hier im Code defines sind mit JELLING_BLABLA: Alle meine wichtigen defines und extended advertising defaults sind hier:
// # https://github.com/janosbrodbeck/RIOT/blob/bachelor/jelling/pkg/nimble/jelling/include/jelling.h
// # Für den Zugriff auf Haukes address definitionen (e.g. nimble_riot_own_addr_type)
// # -> #include "nimble_riot.h"
#include <stdlib.h>
#include <stdio.h>
// #include "nimble_riot.h"
// #include "net/gnrc/pktbuf.h"
// #include "net/ble.h"
// #include "host/ble_hs.h"

#include "net/ble.h"
#include "net/bluetil/addr.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/netif/hdr.h"

#include "net/gnrc/netreg.h"
#include "net/gnrc/pktbuf.h"
#include "net/gnrc/nettype.h"

#include "net/ble.h"

// #include "host/ble_hs.h"

#include "nimble_riot.h"
#include "host/ble_gap.h"
#include "host/ble_hs.h"
#include "host/ble_hs_adv.h"

#include "jelling.h"

#define PACKET_NEXT_HOP_OFFSET  (4)
#define PACKET_PKG_NUM_OFFSET   (10)
#define PACKET_DATA_OFFSET      (11)

typedef struct {
    bool ongoing;
    uint8_t pkt_num;
    uint8_t next_hop_match;
    uint8_t data[MYNEWT_VAL_BLE_EXT_ADV_MAX_SIZE];
    size_t len;
} _chained_packet;

// static int _send_pkt(uint8_t instance, struct os_mbuf *mbuf);
static int _start_scanner(void);
static int _gap_event(struct ble_gap_event *event, void *arg);
// static void _on_data(struct ble_gap_event *event, void *arg);
// static int _configure_adv_instance(uint8_t instance);
// static bool _filter_manufacturer_id(uint8_t *data, uint8_t len);
// static uint8_t _filter_next_hop_addr(uint8_t *data, uint8_t len);
// static size_t _prepare_ipv6_packet(uint8_t *data, size_t len);
// void bluetil_addr_print(const uint8_t *addr);

static _chained_packet _chain;
static jelling_config_t _config;

static const char _hex_chars[16] = "0123456789ABCDEF";

size_t fmt_byte_hex(char *out, uint8_t byte)
{
    if (out) {
        *out++ = _hex_chars[byte >> 4];
        *out = _hex_chars[byte & 0x0F];
    }
    return 2;
}

void bluetil_addr_sprint(char *out, const uint8_t *addr)
{
    assert(out);
    assert(addr);

    fmt_byte_hex(out, addr[0]);
    for (unsigned i = 1; i < BLE_ADDR_LEN; i++) {
        out += 2;
        *out++ = ':';
        fmt_byte_hex(out, addr[i]);
    }
    out += 2;
    *out = '\0';
}

void bluetil_addr_print(const uint8_t *addr)
{
    assert(addr);

    char str[BLUETIL_ADDR_STRLEN];
    bluetil_addr_sprint(str, addr);
    printf("%s", str);
}

// static int _gap_event(struct ble_gap_event *event, void *arg)
// {
//     (void) arg;

//     switch(event->type) {
//         case BLE_GAP_EVENT_ADV_COMPLETE:
//             return 0;
//         case BLE_GAP_EVENT_EXT_DISC:
//             // _on_data(event, arg);
//             return 0;
//         case BLE_GAP_EVENT_DISC_COMPLETE:
//             return 0;

//     }
//     return 0;
// }


// ### Advertising  ######
// # Nur einmalig notwendig, oder wenn man die Parameter ändern möchte
// # Instance 0 ist die erste, mehr willst du nicht
// static int _configure_adv_instance(uint8_t instance) {
//     struct ble_gap_ext_adv_params params = { 0 };
//     int8_t selected_tx_power;

//     memset(&params, 0, sizeof(params));

//     /* set advertise parameters */
//     params.scannable = 0;
//     params.directed = 0;
//     params.own_addr_type = nimble_riot_own_addr_type;
//     params.channel_map = BLE_GAP_ADV_DFLT_CHANNEL_MAP;
//     params.filter_policy = 0;
//     // params.itvl_min = _config.advertiser_itvl_min;
//     // params.itvl_max = _config.advertiser_itvl_max;
//     params.itvl_min = BLE_GAP_ADV_FAST_INTERVAL1_MIN;
//     params.itvl_max = BLE_GAP_ADV_FAST_INTERVAL1_MIN;
//     params.tx_power = 127;
//     params.primary_phy = BLE_HCI_LE_PHY_1M;
//     params.secondary_phy = BLE_HCI_LE_PHY_1M;
//     //params.sid = 0;

//     int rc = ble_gap_ext_adv_configure(instance, &params, &selected_tx_power, _gap_event, NULL);
//     if (rc) {
//         printf("_configure_adv_instance: failed to configure advertising instance. "
//             "Return code: 0x%02X\n", rc);
//         return -1;
//     }

//     // # adresse, mit der advertised wird, setzen für diese instanz
//     ble_addr_t addr;
//     rc = ble_hs_id_copy_addr(nimble_riot_own_addr_type, addr.val, NULL);
//     addr.type = nimble_riot_own_addr_type;
//     if (rc) {
//         printf("_configure_adv_instance: failed to retrieve BLE address\n");
//         return -1;
//     }
//     rc = ble_gap_ext_adv_set_addr(instance, &addr);
//     if (rc) {
//         printf("_configure_adv_instance: could not set address\n");
//         return -1;
//     }

//     return rc;
// }



// // // # tatsächliches senden
// static int _send_pkt(uint8_t instance, struct os_mbuf *mbuf)
// {
//     int res = ble_gap_ext_adv_set_data(instance, mbuf);
//     if (res) {
//         return res;
//     }

//     res = ble_gap_ext_adv_start(instance, 0,
//                             3);
//     if (res) {
//         printf("Couldn't start advertising. Return code: 0x%02X\n", res);
//         return -1;
//     }

//     if(IS_ACTIVE(JELLING_DEBUG_ADVERTISING_PROCESS)) {
//         printf("DEBUG: Advertising for packet started successfully\n");
//     }
//     return res;
// }

// static bool _filter_manufacturer_id(uint8_t *data, uint8_t len) {
//     if (len > 4) {
//         if (data[len - 3] == 0x41 && data[len -4] == 0x41) {
//             return true;
//         }
//     }
//     return false;
// }


static void _on_data(struct ble_gap_event *event, void *arg)
{
    (void) arg;

    /* TRUNCATED status -> recv of chained packet failed */
    if (event->ext_disc.data_status == BLE_GAP_EXT_ADV_DATA_STATUS_TRUNCATED) {
        _chain.ongoing = false;
        return;
    }
    

    printf("Address: ");
    bluetil_addr_print(event->ext_disc.addr.val);
    printf("    Data Length: %d bytes    \n", event->ext_disc.length_data);
    // printf("    Data: ");
    // for (int i = 0; i < min(32, event->ext_disc.length_data) ; i++) {
    //     printf("0x%x ", event->ext_disc.data[i]);
    // }
    // printf("    ");

    switch(event->ext_disc.data_status) {
        case BLE_GAP_EXT_ADV_DATA_STATUS_COMPLETE:
            printf("COMPLETE    ");
            break;
        case BLE_GAP_EXT_ADV_DATA_STATUS_INCOMPLETE:
            printf("INCOMPLETE    ");
            break;
        case BLE_GAP_EXT_ADV_DATA_STATUS_TRUNCATED:
            printf("TRUNCATED    ");
            break;
    }
    printf("\n");



//     /* No chained packets ongoing -> needs to be the first packet containing
//        the jelling header */
//     if (!_chain.ongoing) {
//        _chain.next_hop_match = _filter_next_hop_addr((uint8_t *)event->ext_disc.data+PACKET_NEXT_HOP_OFFSET,
//             event->ext_disc.length_data-PACKET_NEXT_HOP_OFFSET);
//         if(!_chain.next_hop_match) {
//             return;
//         }

//         /* duplicate detection */
//         _chain.pkt_num = event->ext_disc.data[PACKET_PKG_NUM_OFFSET];

//         /* incomplete event, prepare for more data  */
//         if (event->ext_disc.data_status == BLE_GAP_EXT_ADV_DATA_STATUS_INCOMPLETE) {
//             _chain.ongoing = true;
//         }
//         /* copy data into buffer */
//         memcpy(_chain.data, event->ext_disc.data, event->ext_disc.length_data);
//         _chain.len = event->ext_disc.length_data;

//     } else { /* subsequent packet without jelling header */
//         /* sanity check */
//         if (_chain.len+event->ext_disc.length_data > sizeof(_chain.data)) {
//             // DEBUG("Broken packets from nimBLE\n");
// #ifdef MODULE_LLSTATS_JELLING
//             llstats_jelling_broken_data();
// #endif
//             _chain.ongoing = false;
//             return;
//         }
//         /* copy data into buffer */
//         memcpy(_chain.data+_chain.len, event->ext_disc.data,
//                event->ext_disc.length_data);
//         _chain.len += event->ext_disc.length_data;

//         if (event->ext_disc.data_status == BLE_GAP_EXT_ADV_DATA_STATUS_COMPLETE) {
//             _chain.ongoing = false;
//         }
//     }

//     if (_chain.ongoing) {
//         return;
//     }

//     uint32_t received_bytes = _chain.len;
    /* Process BLE data */
    // size_t ipv6_packet_size = _prepare_ipv6_packet(_chain.data, _chain.len);
//     if (ipv6_packet_size == -1) {
// #ifdef MODULE_LLSTATS_JELLING
//         llstats_jelling_broken_data();
// #endif
//         if (IS_ACTIVE(JELLING_DEBUG_BROKEN_BLE_DATA_MSG)) {
//             printf("Broken BLE data\n");
//         }
//         return;
//     }

    // /* add to duplicate detection */
    // if (IS_ACTIVE(JELLING_DUPLICATE_DETECTION_FEATURE_ENABLE)) {
    //     if (_config.duplicate_detection_enable) {
    //         jelling_dd_add(event->ext_disc.addr.val, _chain.pkt_num);
    //     }
    // }

    // if(IS_ACTIVE(JELLING_DEBUG_IPV6_PACKET_SIZES)) {
    //     printf("Received IPv6 packet of %d bytes\n", ipv6_packet_size);
    // }

    /* prepare gnrc pkt */
    // gnrc_pktsnip_t *if_snip;
    // /* destination can be multicast or unicast addr */
    // if (_chain.next_hop_match == ADDR_UNICAST) {
    //     if_snip = gnrc_netif_hdr_build(event->ext_disc.addr.val, BLE_ADDR_LEN,
    //                     _ble_addr, BLE_ADDR_LEN);
    // } else {
    //     if_snip = gnrc_netif_hdr_build(event->ext_disc.addr.val, BLE_ADDR_LEN,
    //                     _ble_mc_addr, BLE_ADDR_LEN);
    // }

    /* we need to add the device PID to the netif header */
    // gnrc_netif_hdr_set_netif(if_snip->data, _netif);

    // /* allocate space in the pktbuf to store the packet */
    // gnrc_pktsnip_t *payload = gnrc_pktbuf_add(if_snip, NULL, ipv6_packet_size,
    //                             _nettype);
    // if (payload == NULL) {
    //     gnrc_pktbuf_release(if_snip);
    //     printf("Payload allocation failed\n");
    //     return;
    // }

    /* copy payload from event into pktbuffer */
    // memcpy(payload->data, _chain.data, ipv6_packet_size);

// #ifdef MODULE_DATARATE
//     datarate_recv_add(received_bytes);
// #endif
    /* finally dispatch the receive packet to gnrc */
    // if (!gnrc_netapi_dispatch_receive(payload->type, GNRC_NETREG_DEMUX_CTX_ALL,
    //                                 payload)) {
    //     printf("Could not dispatch\n");
    //     gnrc_pktbuf_release(payload);
    // }
}


// // ### Scanner + Advertising (callback handler)

static int _gap_event(struct ble_gap_event *event, void *arg)
{
    (void) arg;
    uint8_t target_addr[] = {0xC5, 0x52, 0xB5, 0x05, 0x55, 0xD6};
    if (memcmp(event->ext_disc.addr.val, (const void*) target_addr, 6) == 0 ){

    switch(event->type) {
        case BLE_GAP_EVENT_ADV_COMPLETE:
            printf("BLE_GAP_EVENT_ADV_COMPLETE\n");
            return 0;
        case BLE_GAP_EVENT_DISC: //normales event gescannt
            printf("BLE_GAP_EVENT_DISC\n");
            return 1337;
        case BLE_GAP_EVENT_EXT_DISC: // extended gescannt
            _on_data(event, arg);
            // printf("BLE_GAP_EVENT_EXT_DISC\n");
            return 0;
        case BLE_GAP_EVENT_DISC_COMPLETE:
            printf("BLE_GAP_EVENT_DISC_COMPLETE\n");
            return 0;

    }
    return 0;
    }
    return 1;
}


// // ### Scanner ##########

static int _start_scanner(void) {
    struct ble_gap_ext_disc_params uncoded = {0};
    uint8_t limited = 0;
    uint8_t filter_duplicates = 1;
    uint16_t duration = 0;
    uint16_t period = 0;

    /* Set uncoded parameters */
    uncoded.passive = 1;
    uncoded.itvl = BLE_GAP_SCAN_FAST_INTERVAL_MIN;
    uncoded.window = BLE_GAP_SCAN_FAST_WINDOW;

    /* start scan */
    int rc = ble_gap_ext_disc(nimble_riot_own_addr_type, duration, period, filter_duplicates,
                            BLE_HCI_SCAN_FILT_NO_WL, limited, &uncoded, NULL, _gap_event, NULL);
    if(rc != 0) {
        printf("_start_scanner failed. Return code %02X\n", rc);
    }
    return rc;
}



// // ### Appendix ###
// // # Das laden meiner defaults. Damit dus easy zuordnen kannst, falls iwas nicht klar ist

void jelling_load_default_config(void)
{
    _config.advertiser_enable = JELLING_ADVERTISING_ENABLE_DFLT;
    _config.advertiser_verbose = JELLING_ADVERTISING_VERBOSE_DFLT;
    _config.advertiser_duration = JELLING_ADVERTISING_DURATION_DFLT;
    _config.advertiser_max_events = JELLING_ADVERTISING_MAX_EVENTS_DFLT;
    _config.advertiser_itvl_min = JELLING_ADVERTISING_ITVL_MIN_DFLT;
    _config.advertiser_itvl_max = JELLING_ADVERTISING_ITVL_MAX_DFLT;

    _config.scanner_enable = JELLING_SCANNER_ENABLE_DFLT;
    _config.scanner_verbose = 1;
    _config.scanner_itvl = JELLING_SCANNER_ITVL_DFLT;
    _config.scanner_window = JELLING_SCANNER_WINDOW_DFLT;
    _config.scanner_period = JELLING_SCANNER_PERIOD_DFLT;
    _config.scanner_duration = JELLING_SCANNER_DURATION_DFLT;
    _config.scanner_filter_duplicates = 0;

    if (IS_ACTIVE(JELLING_DUPLICATE_DETECTION_FEATURE_ENABLE)) {
        _config.duplicate_detection_enable = JELLING_DUPLICATE_DETECTION_ACTIVATION_DFTL;
    }
}

int main(void) {
    jelling_load_default_config();
    _start_scanner();
    return 0;
}