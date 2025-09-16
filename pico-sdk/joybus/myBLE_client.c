#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "myBLE_client.h"

#include "btstack.h"                 // BTstack core (GAP/L2CAP/GATT, util)
#include "btstack_config.h"

uint32_t input_rsp = 0;

// ==== Edit to match your peripheral ====
static const char *target_name = "PicoW-TX";   // your server's advertised name

// Your 128-bit Service/Characteristic UUIDs (little-endian byte order):
// Example values — replace with the ones you used on the server.
static const uint8_t service_uuid128[16] = {
    0x54,0x6e,0x8c,0x2b,0x1d,0x3f,0x7e,0x9a,0x5a,0x4c,0x2d,0x0b,0x90,0x6c,0x4a,0x8e
};

static const uint8_t char_uuid128[16] = {
    0x54,0x6e,0x8c,0x2b,0x1d,0x3f,0x7e,0x9a,0x5a,0x4c,0x2d,0x0b,0x91,0x6c,0x4a,0x8e
};

// ======================================

// Simple state machine
typedef enum {
    APP_IDLE = 0,
    APP_SCANNING,
    APP_CONNECTING,
    APP_SERVICE_QUERY,
    APP_CHAR_QUERY,
    APP_SUBSCRIBE,
    APP_SUBSCRIBED
} app_state_t;

static app_state_t app_state = APP_IDLE;

static hci_con_handle_t con_handle = HCI_CON_HANDLE_INVALID;
static gatt_client_service_t        svc;
static gatt_client_characteristic_t chr;

// For notifications
static gatt_client_notification_t notification_reg;

// BTstack event registrations
static btstack_packet_callback_registration_t hci_cb;

// Forward decls
static void hci_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static void gatt_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

static bool adv_has_name(const uint8_t *data, uint8_t len, const char *name) {
    // Use BTstack's AD iterator helpers. (ad_iterator_* API) :contentReference[oaicite:0]{index=0}
    ad_context_t ctx;
    for (ad_iterator_init(&ctx, len, (uint8_t *)data); ad_iterator_has_more(&ctx); ad_iterator_next(&ctx)) {
        uint8_t type = ad_iterator_get_data_type(&ctx);
        uint8_t size = ad_iterator_get_data_len(&ctx);
        const uint8_t *d = ad_iterator_get_data(&ctx);
        if (type == BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME || type == BLUETOOTH_DATA_TYPE_SHORTENED_LOCAL_NAME) {
            if (size == strlen(name) && memcmp(d, name, size) == 0) return true;
        }
    }
    return false;
}

int app_ble_client_init(void){


    if (cyw43_arch_init()) {
        printf("CYW43 init failed\n");
        return -1;
    }
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);


    // --- Core inits (LE central) ---
    l2cap_init();
    gatt_client_init();                           // GATT client API :contentReference[oaicite:1]{index=1}
    sm_init();
    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);

    // Register HCI event handler & power on controller. (scan/connect flow) :contentReference[oaicite:2]{index=2}
    hci_cb.callback = &hci_packet_handler;
    hci_add_event_handler(&hci_cb);
    hci_power_control(HCI_POWER_ON);

    printf("Central up: scanning for '%s'\n", target_name);
    btstack_run_loop_execute();                   // run single-threaded BTstack loop :contentReference[oaicite:3]{index=3}
    return 0;
}

static void start_scanning(void){
    app_state = APP_SCANNING;
    // Passive scanning, ~30 ms interval/window. :contentReference[oaicite:4]{index=4}
    gap_set_scan_parameters(0 /* passive */, 0x0030, 0x0030);
    gap_start_scan();
    printf("Scanning...\n");
}

static void hci_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    (void)channel; (void)size;
    if (packet_type != HCI_EVENT_PACKET) return;

    switch (hci_event_packet_get_type(packet)){
        case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING){
                start_scanning();
            }
            break;

        case GAP_EVENT_ADVERTISING_REPORT: {
            if (app_state != APP_SCANNING) break;
            const uint8_t *adv_data = gap_event_advertising_report_get_data(packet);
            uint8_t adv_len = gap_event_advertising_report_get_data_length(packet);
            if (!adv_has_name(adv_data, adv_len, target_name)) break;

            bd_addr_t addr;
            uint8_t addr_type = gap_event_advertising_report_get_address_type(packet);
            gap_event_advertising_report_get_address(packet, addr);

            printf("Found %s @ %s — connecting\n", target_name, bd_addr_to_str(addr));
            gap_stop_scan();
            app_state = APP_CONNECTING;
            gap_connect(addr, addr_type);                               // connect to advertiser :contentReference[oaicite:5]{index=5}
            break;
        }

        case HCI_EVENT_LE_META:
            if (hci_event_le_meta_get_subevent_code(packet) == HCI_SUBEVENT_LE_CONNECTION_COMPLETE){
                con_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
                printf("Connected, handle=0x%04x — discovering service\n", con_handle);
                app_state = APP_SERVICE_QUERY;
                // Discover service by 128-bit UUID. :contentReference[oaicite:6]{index=6}
                uint8_t err = gatt_client_discover_primary_services(gatt_packet_handler, con_handle);
                printf("Discover by UUID returned: %d\n", err);
            }
            break;

        case HCI_EVENT_DISCONNECTION_COMPLETE:
            printf("Disconnected (reason 0x%02x)\n", hci_event_disconnection_complete_get_reason(packet));
            con_handle = HCI_CON_HANDLE_INVALID;
            app_state = APP_IDLE;
            start_scanning(); // auto-reconnect
            break;

        default:
            break;
    }
}

static void gatt_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    (void)packet_type; (void)channel; (void)size;

    switch (hci_event_packet_get_type(packet)){
        case GATT_EVENT_SERVICE_QUERY_RESULT: {
            // Save discovered service. :contentReference[oaicite:7]{index=7}
            gatt_event_service_query_result_get_service(packet, &svc);
            printf("Service %s found: [0x%04x..0x%04x]\n",
                   uuid128_to_str(svc.uuid128), svc.start_group_handle, svc.end_group_handle);
            break;
        }

        case GATT_EVENT_QUERY_COMPLETE: {
            uint8_t status = gatt_event_query_complete_get_att_status(packet);
            if (status) {
                printf("GATT query complete with status 0x%02x\n", status);
                gap_disconnect(con_handle);
                break;
            }

            if (app_state == APP_SERVICE_QUERY){
                // Now find our characteristic by 128-bit UUID within service. :contentReference[oaicite:8]{index=8}
                app_state = APP_CHAR_QUERY;
                gatt_client_discover_characteristics_for_service_by_uuid128(
                    gatt_packet_handler, con_handle, &svc, char_uuid128);
            } else if (app_state == APP_CHAR_QUERY){
                // After characteristic discovery, subscribe.
                app_state = APP_SUBSCRIBE;

                // 1) Register notification listener. :contentReference[oaicite:9]{index=9}
                gatt_client_listen_for_characteristic_value_updates(
                    &notification_reg, gatt_packet_handler, con_handle, &chr);

                // 2) Write CCCD to enable notifications. :contentReference[oaicite:10]{index=10}
                gatt_client_write_client_characteristic_configuration(
                    gatt_packet_handler, con_handle, &chr,
                    GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION);
                printf("Subscribed (writing CCCD)...\n");
            } else if (app_state == APP_SUBSCRIBE){
                app_state = APP_SUBSCRIBED;
                printf("Subscription active — waiting for notifications.\n");
            }
            break;
        }

        case GATT_EVENT_CHARACTERISTIC_QUERY_RESULT: {
            // Save discovered characteristic (holds value_handle). :contentReference[oaicite:11]{index=11}
            gatt_event_characteristic_query_result_get_characteristic(packet, &chr);
            printf("Characteristic %s value_handle=0x%04x props=0x%02x\n",
                   uuid128_to_str(chr.uuid128), chr.value_handle, chr.properties);
            break;
        }

        case GATT_EVENT_NOTIFICATION: {
            // Extract value pointer + length. :contentReference[oaicite:12]{index=12}
            const uint8_t *value = gatt_event_notification_get_value(packet);
            int len = gatt_event_notification_get_value_length(packet);
            if (len == 4){
                input_rsp = little_endian_read_32(value, 0);
                printf("TX notify: %lu\n", (unsigned long) input_rsp);
            } else {
                printf("Notify len=%d (expected 4)\n", len);
            }
            break;
        }

        default:
            break;
    }
}
