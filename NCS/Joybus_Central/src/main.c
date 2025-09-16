/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *  @brief Nordic Battery Service Client sample
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <inttypes.h>
#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <bluetooth/gatt_dm.h>
#include <bluetooth/scan.h>

#include <zephyr/drivers/pwm.h>

#include "joybus_client.h"


#define BAS_READ_VALUE_INTERVAL (10 * MSEC_PER_SEC)

#define BT_LE_JOYBUS_CONN_PARAMS BT_LE_CONN_PARAM(0x08, 0x09, 0, 400)

static bool bt_connected = false;

//#define WORKQ_THREAD_STACK_SIZE 1024
//#define WORKQ_PRIORITY 4

// Define stack area used by workqueue thread
//static K_THREAD_STACK_DEFINE(my_stack_area, WORKQ_THREAD_STACK_SIZE);

static struct bt_conn *default_conn;
static struct bt_joybus_client joy;

static uint32_t app_input_rsp = 0;

static void notify_input_rsp_cb(struct bt_joybus_client *joy,
				    uint32_t input_response);

static void read_input_response_cb(struct bt_joybus_client *joy,
				  uint32_t input_response,
				  int err);

static void app_button_indicated_cb(struct bt_joybus_client *joy, uint8_t button_state);

uint32_t led_write_state = -1;

struct work_info {
    struct k_work work;
    char name[25];
} my_work;

void offload_function(struct k_work *work_term)
{

	
}

static void scan_filter_match(struct bt_scan_device_info *device_info,
			      struct bt_scan_filter_match *filter_match,
			      bool connectable)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(device_info->recv_info->addr, addr, sizeof(addr));

	printk("Filters matched. Address: %s connectable: %s\n",
		addr, connectable ? "yes" : "no");
}

static void scan_connecting_error(struct bt_scan_device_info *device_info)
{
	printk("Connecting failed\n");
}

static void scan_connecting(struct bt_scan_device_info *device_info,
			    struct bt_conn *conn)
{
	default_conn = bt_conn_ref(conn);
}

static void scan_filter_no_match(struct bt_scan_device_info *device_info,
				 bool connectable)
{
	int err;
	struct bt_conn *conn;
	char addr[BT_ADDR_LE_STR_LEN];

	if (device_info->recv_info->adv_type == BT_GAP_ADV_TYPE_ADV_DIRECT_IND) {
		bt_addr_le_to_str(device_info->recv_info->addr, addr,
				  sizeof(addr));
		printk("Direct advertising received from %s\n", addr);
		bt_scan_stop();

		err = bt_conn_le_create(device_info->recv_info->addr,
					BT_CONN_LE_CREATE_CONN,
					device_info->conn_param, &conn);

		if (!err) {
			default_conn = bt_conn_ref(conn);
			bt_conn_unref(conn);
		}
	}
}

BT_SCAN_CB_INIT(scan_cb, scan_filter_match, scan_filter_no_match,
		scan_connecting_error, scan_connecting);

static void discovery_completed_cb(struct bt_gatt_dm *dm,
				   void *context)
{
	int err;

	printk("The discovery procedure succeeded\n");

	bt_gatt_dm_data_print(dm);

	err = bt_joybus_handles_assign(dm, &joy);
	if (err) {
		printk("Could not init JOY client object, error: %d\n", err);
	}

	if (bt_joy_input_rsp_notify_supported(&joy)) {
		err = bt_joybus_subscribe_input_rsp(&joy,
						     notify_input_rsp_cb);
		if (err) {
			printk("Cannot subscribe to input_response notification "
				"(err: %d)\n", err);
			/* Continue anyway */
		}
	} else {
		err = input_rsp_start_periodic_read(
			&joy, BAS_READ_VALUE_INTERVAL, read_input_response_cb);
		if (err) {
			printk("Could not start periodic read of JOY value\n");
		}
	}

	err = bt_joy_button_enable_indicate(&joy, app_button_indicated_cb);
	if (err) {
		printk("Cannot subscribe to button indications (err: %d)\n", err);
		/* Continue anyway */
	}

	bt_connected = true;


	err = bt_gatt_dm_data_release(dm);
	if (err) {
		printk("Could not release the discovery data, error "
		       "code: %d\n", err);
	}
}

static void discovery_service_not_found_cb(struct bt_conn *conn,
					   void *context)
{
	printk("The service could not be found during the discovery\n");
}

static void discovery_error_found_cb(struct bt_conn *conn,
				     int err,
				     void *context)
{
	printk("The discovery procedure failed with %d\n", err);
}

static struct bt_gatt_dm_cb discovery_cb = {
	.completed = discovery_completed_cb,
	.service_not_found = discovery_service_not_found_cb,
	.error_found = discovery_error_found_cb,
};

static void gatt_discover(struct bt_conn *conn)
{
	int err;

	if (conn != default_conn) {
		return;
	}

	err = bt_gatt_dm_start(conn, BT_UUID_JOYBUS, &discovery_cb, NULL);
	if (err) {
		printk("Could not start the discovery procedure, error "
		       "code: %d\n", err);
	}
}

static void connected(struct bt_conn *conn, uint8_t conn_err)
{
	int err;
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (conn_err) {
		printk("Failed to connect to %s (%u)\n", addr, conn_err);
		if (conn == default_conn) {
			bt_conn_unref(default_conn);
			default_conn = NULL;

			/* This demo doesn't require active scan */
			err = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
			if (err) {
				printk("Scanning failed to start (err %d)\n",
				       err);
			}
		}

		return;
	}
	else {
		printk("Connected: %s - starting discovery...\n", addr);
		gatt_discover(conn);
	}
	

}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];
	int err;

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Disconnected: %s (reason %u)\n", addr, reason);

	if (default_conn != conn) {
		return;
	}

	bt_conn_unref(default_conn);
	default_conn = NULL;

	bt_connected = false;

	/* This demo doesn't require active scan */
	err = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
	if (err) {
		printk("Scanning failed to start (err %d)\n", err);
	}
}



BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
	
};

static void scan_init(void)
{
	int err;

	struct bt_scan_init_param scan_init = {
		.connect_if_match = 1,
		.scan_param = NULL,
		.conn_param = BT_LE_JOYBUS_CONN_PARAMS,
	};

	bt_scan_init(&scan_init);
	bt_scan_cb_register(&scan_cb);

	err = bt_scan_filter_add(BT_SCAN_FILTER_TYPE_UUID, BT_UUID_JOYBUS);
	if (err) {
		printk("Scanning filters cannot be set (err %d)\n", err);

		return;
	}

	err = bt_scan_filter_enable(BT_SCAN_UUID_FILTER, false);
	if (err) {
		printk("Filters cannot be turned on (err %d)\n", err);
	}
}

static void notify_input_rsp_cb(struct bt_joybus_client *joy,
				    uint32_t input_response)
{
	
	// uint32_t *rsp_p = &input_response;

	// if (input_response == BT_INPUT_RESPONSE_VAL_INVALID) {
	// 	printk("Input Response notification aborted\n");
		
	// } else {
	// 	printk("Input Response notification: %x \n", input_response);
	// 	k_msgq_put(&uart_queue, rsp_p, K_NO_WAIT);
	// }

    app_input_rsp = input_response;
}

static void read_input_response_cb(struct bt_joybus_client *joy,
				  uint32_t input_response,
				  int err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(bt_joybus_conn(joy)),
			  addr, sizeof(addr));
	if (err) {
		printk("[%s] Input Response read ERROR: %d\n", addr, err);
		return;
	}

	printk("[%s] Input Response read: %x \n", addr, input_response);
}

static void app_button_indicated_cb(struct bt_joybus_client *joy, uint8_t button_state)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(bt_joybus_conn(joy)), addr, sizeof(addr));

	if(button_state < 0)//invalid value is -1
	{
		printk("Invalid button_state indication received from %s.", addr);
	}
	else {
		printk("[%s] Button %s \n", addr, (button_state ? "Pressed" : "Released"));
	}
}


// static void button_handler(uint32_t button_state, uint32_t has_changed)
// {
// 	if(bt_joybus_conn(&joy) != default_conn){
// 		printk("Not connected, cannot write to LED.\n");
// 		return;
// 	}
	
// 	led_write_state = button_state & has_changed;


// }


int  main(void)
{
	int err;


	printk("Starting Bluetooth Central JOY example\n");

	bt_joybus_client_init(&joy);

	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return -1;
	}

	printk("Bluetooth initialized\n");

	scan_init();

	err = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
	if (err) {
		printk("Scanning failed to start (err %d)\n", err);
		return -1;
	}

	printk("Scanning successfully started\n");

	while(1)
	{

        printk("CENTRAL: %x\n", app_input_rsp);
        k_msleep(2);
	}
}
