/*
 * Copyright (c) 2018 Nordic Semiconductor
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */


/**
 * @file
 * @defgroup bt_bas_client_api Battery Service Client API
 * @{
 * @brief API for the Bluetooth LE GATT Battery Service (JOY) Client.
 */

#include <zephyr/kernel.h>
#include <sys/types.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <bluetooth/gatt_dm.h>

/** @brief JOYBUS Service UUID. */
#define BT_UUID_JOYBUS_VAL BT_UUID_128_ENCODE(0x9c0cf328, 0x5dde, 0xefde, 0x47e6, 0x917590558b99)

/** @brief Button Characteristic UUID. */
#define BT_UUID_JOYBUS_BUTTON_VAL                                                                     \
	BT_UUID_128_ENCODE(0x9c0cf329, 0x5dde, 0xefde, 0x47e6, 0x917590558b99)

/** @brief LED Characteristic UUID. */
#define BT_UUID_JOYBUS_LED_VAL BT_UUID_128_ENCODE(0x9c0cf32a, 0x5dde, 0xefde, 0x47e6, 0x917590558b99)

/** @brief INPUT RESPONSE Characteristic UUID. */
#define BT_UUID_JOYBUS_INPUT_RESP_VAL                                                                   \
	BT_UUID_128_ENCODE(0x9c0cf32b, 0x5dde, 0xefde, 0x47e6, 0x917590558b99)

#define BT_UUID_JOYBUS BT_UUID_DECLARE_128(BT_UUID_JOYBUS_VAL)
#define BT_UUID_JOYBUS_BUTTON BT_UUID_DECLARE_128(BT_UUID_JOYBUS_BUTTON_VAL)
#define BT_UUID_JOYBUS_LED BT_UUID_DECLARE_128(BT_UUID_JOYBUS_LED_VAL)

#define BT_UUID_JOYBUS_INPUT_RESP BT_UUID_DECLARE_128(BT_UUID_JOYBUS_INPUT_RESP_VAL)

/**
 * @brief Value that shows that the battery level is invalid.
 *
 * This value is stored in the JOY Client object when the battery level
 * information is unknown.
 *
 * The same value is used to mark the fact that a notification has been
 * aborted (see @ref bt_bas_notify_cb).
 */
#define BT_INPUT_RESPONSE_VAL_INVALID (-1)



struct bt_joybus_client;

/**
 * @brief Value notification callback.
 *
 * This function is called every time the server sends a notification
 * for a changed value.
 *
 * @param joy           JOY Client object.
 * @param battery_level The notified battery level value, or
 *                      @ref BT_BAS_VAL_INVALID if the notification
 *                      was interrupted by the server
 *                      (NULL received from the stack).
 */
typedef void (*bt_input_rsp_notify_cb)(struct bt_joybus_client *joy,
				 uint32_t input_response);

/**
 * @brief Read complete callback.
 *
 * This function is called when the read operation finishes.
 *
 * @param joy           JOY Client object.
 * @param battery_level The battery level value that was read.
 * @param err           ATT error code or 0.
 */
typedef void (*bt_input_rsp_read_cb)(struct bt_joybus_client *joy,
			       uint32_t input_response,
			       int err);

/**
 * @brief Read complete callback.
 *
 * This function is called when the read operation finishes.
 *
 * @param joy           JOY Client object.
 * @param button_state  The button state of the server device that was indicated.
 */
typedef void (*bt_button_indicate_cb)(struct bt_joybus_client *joy,
			       uint8_t button_state);

/**
 * @brief Read complete callback.
 *
 * This function is called when the read operation finishes.
 *
 * @param joy           JOY Client object.
 * @param button_state  The button state of the server device that was indicated.
 * @param err           ATT error code or 0.
 */
typedef void (*bt_button_read_cb)(struct bt_joybus_client *joy,
			       uint8_t button_state,
			       int err);

/**
 * @brief Writes to the Joybus LED Characteristic.
 *
 *
 * @param joy           JOY Client object.
 * @param led_state		1 or 0 to turn the LED ON or OFF
 */
void joybus_led_write(struct bt_joybus_client *joy, uint32_t led_state);

/* @brief Battery Service Client characteristic periodic read. */
struct bt_input_rsp_periodic_read {
	/** Work queue used to measure the read interval. */
	struct k_work_delayable read_work;
	/** Read parameters. */
	struct bt_gatt_read_params params;
	/** Read value interval. */
	atomic_t interval;
};

/** @brief Joybus Client instance. */
struct bt_joybus_client {
	/** Connection handle. */
	struct bt_conn *conn;
	/** Notification parameters. */
	struct bt_gatt_subscribe_params input_rsp_notify_params;
	/** Read parameters. */
	struct bt_gatt_read_params input_rsp_read_params;

	/** Read characteristic value timing. Used when characteristic do not
	 *  have a CCCD descriptor.
	 */
	struct bt_input_rsp_periodic_read input_rsp_periodic_read;
	/** Notification callback. */
	bt_input_rsp_notify_cb input_rsp_notify_cb;
	/** Read value callback. */
	bt_input_rsp_read_cb input_rsp_read_cb;
	/** Handle of the Input Response Characteristic. */
	uint16_t input_rsp_val_handle;
	/** Handle of the CCCD of the Input Response Characteristic. */
	uint16_t input_rsp_ccc_handle;
	/** Current battery value. */
	uint32_t input_response;
	/** Notification supported. */
	bool input_rsp_notify;
	/** Properties of the input response characteristic. */
	uint8_t input_rsp_properties;

	/** Notification parameters. */
	struct bt_gatt_subscribe_params button_indicate_params;
	/** Notification callback. */
	bt_button_indicate_cb button_indicate_cb;
	/** Handle of the Button Characteristic. */
	uint16_t button_state_handle;
	/** Handle of the CCCD of the Button Characteristic. */
	uint16_t button_ccc_handle;
	/** Current button_state. */
	uint8_t button_state;
	/** Indications supported. */
	bool button_indicate;
	/** Properties of the button characteristic. */
	uint8_t button_properties;

	/** Handle of the LED Characteristic. */
	uint16_t led_state_handle;
	/** Properties of the LED characteristic. */
	uint8_t led_properties;

	struct bt_gatt_write_params led_write_params;

	
};

/**
 * @brief Initialize the JOY Client instance.
 *
 * You must call this function on the JOY Client object before
 * any other function.
 *
 * @param joy  JOY Client object.
 */
void bt_joybus_client_init(struct bt_joybus_client *joy);

/**
 * @brief Assign handles to the JOY Client instance.
 *
 * This function should be called when a connection with a peer has been
 * established, to associate the connection to this instance of the module.
 * This makes it possible to handle multiple connections and associate each
 * connection to a particular instance of this module.
 * The GATT attribute handles are provided by the GATT Discovery Manager.
 *
 * @param dm    Discovery object.
 * @param joy JOY Client object.
 *
 * @retval 0 If the operation was successful.
 *           Otherwise, a (negative) error code is returned.
 * @retval (-ENOTSUP) Special error code used when the UUID
 *         of the service does not match the expected UUID.
 */
int bt_joybus_handles_assign(struct bt_gatt_dm *dm,
			  struct bt_joybus_client *joy);

/**
 * @brief Subscribe to the battery level value change notification.
 *
 * @param joy JOY Client object.
 * @param func  Callback function handler.
 *
 * @retval 0 If the operation was successful.
 *           Otherwise, a (negative) error code is returned.
 * @retval -ENOTSUP Special error code used if the connected server
 *         does not support notifications.
 */
int bt_joybus_subscribe_input_rsp(struct bt_joybus_client *joy,
				   bt_input_rsp_notify_cb func);

/**
 * @brief Enable Joybus Client button press indications.
 *
 * @param joy JOY Client object.
 * @param func  Callback function handler.
 *
 * @retval 0 If the operation was successful.
 *           Otherwise, a (negative) error code is returned.
 * @retval -ENOTSUP Special error code used if the connected server
 *         does not support notifications.
 */
int bt_joy_button_enable_indicate(struct bt_joybus_client *joy, bt_button_indicate_cb func);

/**
 * @brief Remove the subscription.
 *
 * @param joy JOY Client object.
 *
 * @retval 0 If the operation was successful.
 *           Otherwise, a (negative) error code is returned.
 */
int bt_joybus_unsubscribe_input_rsp(struct bt_joybus_client *joy);

/**
 * @brief Get the connection object that is used with a given JOY Client.
 *
 * @param joy JOY Client object.
 *
 * @return Connection object.
 */
struct bt_conn *bt_joybus_conn(const struct bt_joybus_client *joy);


/**
 * @brief Read the battery level value from the device.
 *
 * This function sends a read request to the connected device.
 *
 * @param joy JOY Client object.
 * @param func  The callback function.
 *
 * @retval 0 If the operation was successful.
 *           Otherwise, a (negative) error code is returned.
 */
int bt_joybus_read_input_rsp(struct bt_joybus_client *joy, bt_input_rsp_read_cb func);

/**
 * @brief Get the last known battery level.
 *
 * This function returns the last known battery level value.
 * The battery level is stored when a notification or read response is
 * received.
 *
 * @param joy JOY Client object.
 *
 * @return Battery level or negative error code.
 */
int bt_joybus_get_input_rsp(struct bt_joybus_client *joy);

/**
 * @brief Check whether notification is supported by the service.
 *
 * @param joy JOY Client object.
 *
 * @retval true If notifications are supported.
 *              Otherwise, @c false is returned.
 */
static inline bool bt_joy_input_rsp_notify_supported(struct bt_joybus_client *joy)
{
	return joy->input_rsp_notify;
}

/**
 * @brief Periodically read the battery level value from the device with
 *        specific time interval.
 *
 * @param joy JOY Client object.
 * @param interval Characteristic Read interval in milliseconds.
 * @param func The callback function.
 *
 * @retval 0 If the operation was successful.
 *           Otherwise, a (negative) error code is returned.
 */
int input_rsp_start_periodic_read(struct bt_joybus_client *joy,
					int32_t interval,
					bt_input_rsp_read_cb func);

/**
 * @brief Stop periodic reading of the battery value from the device.
 *
 * @param joy JOY Client object.
 */
void input_rsp_stop_periodic_read(struct bt_joybus_client *joy);

/**
 * @}
 */



