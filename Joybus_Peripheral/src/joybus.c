 /*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *  @brief LED Button Service (LBS) sample
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#include "joybus.h"

LOG_MODULE_DECLARE(Lesson4_Exercise2);

static bool notify_input_response_enabled;
static bool indicate_enabled;
static bool button_state;
static struct joybus_cb jbs_cb;

/* Define an indication parameter */
static struct bt_gatt_indicate_params ind_params;

/* Implement the configuration change callback function */
static void joybus_ccc_button_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	indicate_enabled = (value == BT_GATT_CCC_INDICATE);
}

/* Define the configuration change callback function for the MYSENSOR characteristic */
static void joybus_ccc_input_response_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	notify_input_response_enabled = (value == BT_GATT_CCC_NOTIFY);
}

// This function is called when a remote device has acknowledged the indication at its host layer
static void indicate_cb(struct bt_conn *conn, struct bt_gatt_indicate_params *params, uint8_t err)
{
	LOG_DBG("Indication %s\n", err != 0U ? "fail" : "success");
}
static ssize_t write_led(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf,
			 uint16_t len, uint16_t offset, uint8_t flags)
{
	LOG_DBG("Attribute write, handle: %u, conn: %p", attr->handle, (void *)conn);

	if (len != 4) {
		LOG_DBG("Write led: Incorrect data length");
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}

	if (offset != 0) {
		LOG_DBG("Write led: Incorrect data offset");
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	if (jbs_cb.led_cb) {
		// Read the received value
		uint32_t val = *((uint32_t *)buf);

		if (val == 0x0000 || val == 0x0001) {
			// Call the application callback function to update the LED state
			jbs_cb.led_cb(val ? true : false);
		} else {
			LOG_DBG("Write led: Incorrect value");
			return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
		}
	}

	return len;
}

static ssize_t read_button(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
			   uint16_t len, uint16_t offset)
{
	// get a pointer to button_state which is passed in the BT_GATT_CHARACTERISTIC() and stored in attr->user_data
	const char *value = attr->user_data;

	LOG_DBG("Attribute read, handle: %u, conn: %p", attr->handle, (void *)conn);

	if (jbs_cb.button_cb) {
		// Call the application callback function to update the get the current value of the button
		button_state = jbs_cb.button_cb();
		return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(*value));
	}

	return 0;
}

/* Joybus Service Declaration */
BT_GATT_SERVICE_DEFINE(
	joybus_svc, BT_GATT_PRIMARY_SERVICE(BT_UUID_JOYBUS),
	/* Button Characteristic */
	BT_GATT_CHARACTERISTIC(BT_UUID_JOYBUS_BUTTON, BT_GATT_CHRC_INDICATE,
			       BT_GATT_PERM_READ, read_button, NULL, &button_state),
	/*Create and add the Client Characteristic Configuration Descriptor */
	BT_GATT_CCC(joybus_ccc_button_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

	BT_GATT_CHARACTERISTIC(BT_UUID_JOYBUS_LED, BT_GATT_CHRC_WRITE, BT_GATT_PERM_WRITE, NULL,
			       write_led, NULL),

	/*Create and add the INPUT RESPONSE characteristic and its CCCD  */
	BT_GATT_CHARACTERISTIC(BT_UUID_JOYBUS_INPUT_RESP, BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_READ, NULL,
			       NULL, NULL),

	BT_GATT_CCC(joybus_ccc_input_response_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

);
/* A function to register application callbacks for the LED and Button characteristics  */
int joybus_init(struct joybus_cb *callbacks)
{
	if (callbacks) {
		jbs_cb.led_cb = callbacks->led_cb;
		jbs_cb.button_cb = callbacks->button_cb;
	}

	return 0;
}

/* Define the function to send indications */
int joybus_send_button_state_indicate(bool button_state)
{
	if (!indicate_enabled) {
		return -EACCES;
	}
	ind_params.attr = &joybus_svc.attrs[2];
	ind_params.func = indicate_cb; // A remote device has ACKed at its host layer (ATT ACK)
	ind_params.destroy = NULL;
	ind_params.data = &button_state;
	ind_params.len = sizeof(button_state);
	return bt_gatt_indicate(NULL, &ind_params);
}

/* Define the function to send notifications for the INPUT RESPONSE characteristic */
int joybus_send_input_response_notify(uint32_t input_response)
{
	if (!notify_input_response_enabled) {
		return -EACCES;
	}

	return bt_gatt_notify(NULL, &joybus_svc.attrs[7], &input_response, sizeof(input_response));
}
