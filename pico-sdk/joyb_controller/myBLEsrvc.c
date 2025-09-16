#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "btstack.h"
#include "btstack_config.h"

#include "ble_tx_u32.h"   // generated from the .gatt file

// ----- Advertising payload: Flags + Complete Local Name -----
static uint8_t adv_data[] = {
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, 0x06,                         // LE General + BR/EDR Not Supported
    0x09, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'P','i','c','o','W','-','T','X'
};

static hci_con_handle_t conn_handle = HCI_CON_HANDLE_INVALID;

enum { TX_PAYLOAD_LEN = 4 };                 // fixed 4-byte word
static uint8_t  tx_buf[TX_PAYLOAD_LEN];
static bool     tx_pending = false;

// Forward
static void att_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

// Write + notify a single uint32_t (little-endian on the wire)
static inline void u32_to_le(uint32_t v, uint8_t out[4]) {
    out[0] = (uint8_t)(v      );
    out[1] = (uint8_t)(v >>  8);
    out[2] = (uint8_t)(v >> 16);
    out[3] = (uint8_t)(v >> 24);
}
void tx_notify_u32(uint32_t word) {
    if (conn_handle == HCI_CON_HANDLE_INVALID) return;   // no client connected

    u32_to_le(word, tx_buf);

    // Keep GATT DB value in sync for READs
    att_server_request_can_send_now_event(conn_handle);

    // Queue a notification
    tx_pending = true;
    att_server_request_can_send_now_event(conn_handle);
}

int app_ble_init(void) {


    if (cyw43_arch_init()) {
        printf("CYW43 init failed\n");
        return -1;
    }

    // ---- Minimal BTstack init for LE GATT server ----
    l2cap_init();
    sm_init();                                  // no security for this template
    att_server_init(profile_data, NULL, NULL);  // static DB
    att_server_register_packet_handler(att_packet_handler);

    // turn on bluetooth!
    hci_power_control(HCI_POWER_ON);

    // ---- GAP advertising ----
    gap_advertisements_set_params(
        0x0030, 0x0060,   // 18.75–37.5 ms
        0,                // connectable undirected
        0, NULL,          // no direct address
        0x07,             // channels 37,38,39
        0                 // allow all
    );
    gap_advertisements_set_data(sizeof(adv_data), adv_data);
    gap_advertisements_enable(1);

    printf("BLE TX-u32 server up; advertising as PicoW-TX\n");

    btstack_run_loop_execute();
    
    return 0;
}

static void att_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    (void)channel; (void)size;
    if (packet_type != HCI_EVENT_PACKET) return;

    switch (hci_event_packet_get_type(packet)) {
        case ATT_EVENT_CONNECTED:
            conn_handle = att_event_connected_get_handle(packet);
            printf("ATT connected, handle=0x%04x\n", conn_handle);
            break;

        case ATT_EVENT_DISCONNECTED:
            printf("ATT disconnected\n");
            conn_handle = HCI_CON_HANDLE_INVALID;
            tx_pending = false;
            break;

        case ATT_EVENT_CAN_SEND_NOW:
            if (tx_pending && conn_handle != HCI_CON_HANDLE_INVALID) {
                att_server_notify(conn_handle,
                                  ATT_CHARACTERISTIC_128_TX_VALUE_HANDLE,
                                  tx_buf,
                                  TX_PAYLOAD_LEN);
                tx_pending = false;
            }
            break;

        default:
            break;
    }
}

