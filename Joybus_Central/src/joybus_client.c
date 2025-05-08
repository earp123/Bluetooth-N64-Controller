/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/kernel.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#include "joybus_client.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(joybus_client, 3);

/**
 * @brief Process input response notification
 *
 * Internal function to process report notification and pass it further.
 *
 * @param conn   Connection handler.
 * @param params Notification parameters structure - the pointer
 *               to the structure provided to subscribe function.
 * @param data   Pointer to the data buffer.
 * @param length The size of the received data.
 *
 * @retval BT_GATT_ITER_STOP     Stop notification
 * @retval BT_GATT_ITER_CONTINUE Continue notification
 */
static uint8_t notify_process(struct bt_conn *conn,
			   struct bt_gatt_subscribe_params *params,
			   const void *data, uint16_t length)
{
	struct bt_joybus_client *joy;
	uint32_t input_response;
	const uint32_t* rsp_data = data;
	

	joy = CONTAINER_OF(params, struct bt_joybus_client, input_rsp_notify_params);
	if (!data || !length) {
		LOG_INF("Notifications disabled.");
		if (joy->input_rsp_notify_cb) {
			joy->input_rsp_notify_cb(joy, BT_INPUT_RESPONSE_VAL_INVALID);
		}
		return BT_GATT_ITER_STOP;
	}
	if (length != 4) {
		LOG_ERR("Unexpected notification value size.");
		if (joy->input_rsp_notify_cb) {
			joy->input_rsp_notify_cb(joy, BT_INPUT_RESPONSE_VAL_INVALID);
		}
		return BT_GATT_ITER_STOP;
	}

	input_response = *rsp_data;
	
	joy->input_response = input_response;

	if (joy->input_rsp_notify_cb) {
		joy->input_rsp_notify_cb(joy, input_response);
	}

	return BT_GATT_ITER_CONTINUE;
}

/**
 * @brief Process input response value read
 *
 * Internal function to process report read and pass it further.
 *
 * @param conn   Connection handler.
 * @param err    Read ATT error code.
 * @param params Notification parameters structure - the pointer
 *               to the structure provided to read function.
 * @param data   Pointer to the data buffer.
 * @param length The size of the received data.
 *
 * @retval BT_GATT_ITER_STOP     Stop notification
 * @retval BT_GATT_ITER_CONTINUE Continue notification
 */
static uint8_t read_process(struct bt_conn *conn, uint8_t err,
			     struct bt_gatt_read_params *params,
			     const void *data, uint16_t length)
{
	struct bt_joybus_client *joy;
	uint32_t input_response = 0;
	const uint32_t* rsp_data = data;

	joy = CONTAINER_OF(params, struct bt_joybus_client, input_rsp_read_params);

	if (!joy->input_rsp_read_cb) {
		LOG_ERR("No read callback present");
	} else  if (err) {
		LOG_ERR("Read value error: %d", err);
		joy->input_rsp_read_cb(joy,  input_response, err);
	} else if (!data || length != 1) {
		joy->input_rsp_read_cb(joy,  input_response, -EMSGSIZE);
	} else {
		input_response = *rsp_data;
		joy->input_response = input_response;
		joy->input_rsp_read_cb(joy, input_response, err);
		return BT_GATT_ITER_CONTINUE;
	}

	joy->input_rsp_read_cb = NULL;

	return BT_GATT_ITER_STOP;
}

/**
 * @brief Process periodic battery level value read
 *
 * Internal function to process report read and pass it further.
 * And the end the new read request is triggered.
 *
 * @param conn   Connection handler.
 * @param err    Read ATT error code.
 * @param params Notification parameters structure - the pointer
 *               to the structure provided to read function.
 * @param data   Pointer to the data buffer.
 * @param length The size of the received data.
 *
 * @retval BT_GATT_ITER_STOP     Stop notification
 * @retval BT_GATT_ITER_CONTINUE Continue notification
 */
static uint8_t periodic_read_process(struct bt_conn *conn, uint8_t err,
				  struct bt_gatt_read_params *params,
				  const void *data, uint16_t length)
{
	int32_t interval;
	struct bt_joybus_client *joy;
	uint32_t input_response;
	const uint32_t* rsp_data = data;

	joy = CONTAINER_OF(params, struct bt_joybus_client,
			input_rsp_periodic_read.params);

	if (!joy->input_rsp_notify_cb) {
		LOG_ERR("No notification callback present");
	} else  if (err) {
		LOG_ERR("Read value error: %d", err);
	} else if (!data || length != 1) {
		LOG_ERR("Unexpected read value size.");
	} else {
		input_response = *rsp_data;
		if (joy->input_response != input_response) {
			joy->input_response = input_response;
			joy->input_rsp_notify_cb(joy, input_response);
		} else {
			/* Do nothing. */
		}
	}

	interval = atomic_get(&joy->input_rsp_periodic_read.interval);
	if (interval) {
		k_work_schedule(&joy->input_rsp_periodic_read.read_work,
				K_MSEC(interval));
	}
	return BT_GATT_ITER_STOP;
}

/**
 * @brief Process joybus button press indications
 *
 * Internal function to process indication and pass it further.
 *
 * @param conn   Connection handler.
 * @param err    Read ATT error code.
 * @param params Notification parameters structure - the pointer
 *               to the structure provided to read function.
 * @param data   Pointer to the data buffer.
 * @param length The size of the received data.
 *
 * @retval BT_GATT_ITER_STOP     Stop notification
 * @retval BT_GATT_ITER_CONTINUE Continue notification
 */
static uint8_t button_indicate_process(struct bt_conn *conn,
				      struct bt_gatt_subscribe_params *params,
				      const void *data, uint16_t length)
{
	struct bt_joybus_client *joy;
	uint8_t button_state;
	const uint8_t* button_state_p = data;
	joy = CONTAINER_OF(params, struct bt_joybus_client, button_indicate_params);

	if (!data || !length) {
		LOG_INF("Notifications disabled.");
		if (joy->button_indicate_cb) {
			joy->button_indicate_cb(joy, BT_INPUT_RESPONSE_VAL_INVALID);
		}
		return BT_GATT_ITER_STOP;
	}
	if (length != 1) {
		LOG_ERR("Unexpected notification value size.");
		if (joy->button_indicate_cb) {
			joy->button_indicate_cb(joy, BT_INPUT_RESPONSE_VAL_INVALID);
		}
		return BT_GATT_ITER_STOP;
	}

	button_state = *button_state_p;
	joy->button_state = button_state;

	if (joy->button_indicate_cb) {
		joy->button_indicate_cb(joy, button_state);
	}
	else {
		LOG_DBG("No indicate callback present");
		return BT_GATT_ITER_STOP;
	}

	return BT_GATT_ITER_CONTINUE;

}


/**
 * @brief Periodic read workqueue handler.
 *
 * @param work Work instance.
 */
static void joybus_input_rsp_value_handler(struct k_work *work)
{
	int err;
	struct bt_joybus_client *joy;
    struct k_work_delayable *work_del = work;

	joy = CONTAINER_OF(work_del, struct bt_joybus_client,
			           input_rsp_periodic_read.read_work);

	if (!atomic_get(&joy->input_rsp_periodic_read.interval)) {
		/* disabled */
		return;
	}

	if (!joy->conn) {
		LOG_ERR("No connection object.");
		return;
	}

	joy->input_rsp_periodic_read.params.func = periodic_read_process;
	joy->input_rsp_periodic_read.params.handle_count  = 1;
	joy->input_rsp_periodic_read.params.single.handle = joy->input_rsp_val_handle;
	joy->input_rsp_periodic_read.params.single.offset = 0;

	err = bt_gatt_read(joy->conn, &joy->input_rsp_periodic_read.params);

	/* Do not treats reading after disconnection as error.
	 * Periodic read process is stopped after disconnection.
	 */
	if (err) {
		LOG_ERR("Periodic Imput Response characteristic read error: %d",
			err);
	}
}


/**
 * @brief Reinitialize the JOY Client.
 *
 * @param joy JOY Client object.
 */
static void joybus_reinit(struct bt_joybus_client *joy)
{
	joy->input_rsp_ccc_handle = 0;
	joy->input_rsp_val_handle = 0;
	joy->input_response = 0;
	joy->conn = NULL;
	joy->input_rsp_notify_cb = NULL;
	joy->input_rsp_read_cb = NULL;
	joy->input_rsp_notify = false;

	joy->button_ccc_handle = 0;
	joy->button_state_handle = 0;
	joy->button_state = 0;
	joy->button_indicate = false;
	joy->button_indicate_cb = NULL;

	joy->led_state_handle = 0;
}


void bt_joybus_client_init(struct bt_joybus_client *joy)
{
	memset(joy, 0, sizeof(*joy));
	joy->input_response = 0;

	k_work_init_delayable(&joy->input_rsp_periodic_read.read_work,
			      joybus_input_rsp_value_handler);
}


int bt_joybus_handles_assign(struct bt_gatt_dm *dm,
				 struct bt_joybus_client *joy)
{
	const struct bt_gatt_dm_attr *gatt_service_attr =
			bt_gatt_dm_service_get(dm);
	const struct bt_gatt_service_val *gatt_service =
			bt_gatt_dm_attr_service_val(gatt_service_attr);
	const struct bt_gatt_dm_attr *gatt_chrc;
	const struct bt_gatt_dm_attr *gatt_desc;
	const struct bt_gatt_chrc *chrc_val;

	if (bt_uuid_cmp(gatt_service->uuid, BT_UUID_JOYBUS)) {
		return -ENOTSUP;
	}
	LOG_DBG("Getting handles from joybus service.");

	/* If connection is established again, cancel previous read request. */
	k_work_cancel_delayable(&joy->input_rsp_periodic_read.read_work);
	/* When workqueue is used its instance cannont be cleared. */
	joybus_reinit(joy);

	/* Input Response characteristic */
	gatt_chrc = bt_gatt_dm_char_by_uuid(dm, BT_UUID_JOYBUS_INPUT_RESP);
	if (!gatt_chrc) {
		LOG_ERR("No Input Response characteristic found.");
		return -EINVAL;
	}
	chrc_val = bt_gatt_dm_attr_chrc_val(gatt_chrc);
	__ASSERT_NO_MSG(chrc_val); /* This is internal function and it has to
				    * be called with characteristic attribute
				    */
	joy->input_rsp_properties = chrc_val->properties;
	
	gatt_desc = bt_gatt_dm_desc_by_uuid(dm, gatt_chrc,
					    BT_UUID_JOYBUS_INPUT_RESP);
	if (!gatt_desc) {
		LOG_ERR("No Input Response characteristic value found.");
		return -EINVAL;
	}
	joy->input_rsp_val_handle = gatt_desc->handle;

	gatt_desc = bt_gatt_dm_desc_by_uuid(dm, gatt_chrc, BT_UUID_GATT_CCC);
	if (!gatt_desc) {
		LOG_INF("No battery CCC descriptor found. Battery service do not supported notification.");
	} else {
		joy->input_rsp_notify = true;
		joy->input_rsp_ccc_handle = gatt_desc->handle;
	}

	//Button Characteristic w/ CCCD
	gatt_chrc = bt_gatt_dm_char_by_uuid(dm, BT_UUID_JOYBUS_BUTTON);
	if (!gatt_chrc) {
		LOG_ERR("No Button characteristic found.");
		return -EINVAL;
	}
	chrc_val = bt_gatt_dm_attr_chrc_val(gatt_chrc);
	__ASSERT_NO_MSG(chrc_val); /* This is internal function and it has to
				    * be called with characteristic attribute
				    */
	joy->button_properties = chrc_val->properties;
	
	gatt_desc = bt_gatt_dm_desc_by_uuid(dm, gatt_chrc,
					    BT_UUID_JOYBUS_BUTTON);
	if (!gatt_desc) {
		LOG_ERR("No Button characteristic value found.");
		return -EINVAL;
	}
	joy->button_state_handle = gatt_desc->handle;
	joy->button_indicate_params.value_handle = gatt_desc->handle;

	gatt_desc = bt_gatt_dm_desc_by_uuid(dm, gatt_chrc, BT_UUID_GATT_CCC);
	if (!gatt_desc) {
		LOG_INF("No Button CCC descriptor found.");
	} else {
		joy->button_indicate = true;
		joy->button_ccc_handle = gatt_desc->handle;
		joy->button_indicate_params.ccc_handle = gatt_desc->handle;
		
	}

	//LED Characteristic
	gatt_chrc = bt_gatt_dm_char_by_uuid(dm, BT_UUID_JOYBUS_LED);
	if (!gatt_chrc) {
		LOG_ERR("No LED characteristic found.");
		return -EINVAL;
	}
	chrc_val = bt_gatt_dm_attr_chrc_val(gatt_chrc);
	__ASSERT_NO_MSG(chrc_val); /* This is internal function and it has to
				    * be called with characteristic attribute
				    */
	joy->led_properties = chrc_val->properties;
	
	gatt_desc = bt_gatt_dm_desc_by_uuid(dm, gatt_chrc,
					    BT_UUID_JOYBUS_LED);
	if (!gatt_desc) {
		LOG_ERR("No LED characteristic value found.");
		return -EINVAL;
	}
	joy->led_state_handle = gatt_desc->handle;

	/* Finally - save connection object */
	joy->conn = bt_gatt_dm_conn_get(dm);
	return 0;
}

int bt_joybus_subscribe_input_rsp(struct bt_joybus_client *joy,
				   bt_input_rsp_notify_cb func)
{
	int err;

	if (!joy || !func) {
		return -EINVAL;
	}
	if (!joy->conn) {
		return -EINVAL;
	}
	if (!(joy->input_rsp_properties & BT_GATT_CHRC_NOTIFY)) {
		return -ENOTSUP;
	}
	if (joy->input_rsp_notify_cb) {
		return -EALREADY;
	}

	joy->input_rsp_notify_cb = func;

	joy->input_rsp_notify_params.notify = notify_process;
	joy->input_rsp_notify_params.value = BT_GATT_CCC_NOTIFY;
	joy->input_rsp_notify_params.value_handle = joy->input_rsp_val_handle;
	joy->input_rsp_notify_params.ccc_handle = joy->input_rsp_ccc_handle;
	atomic_set_bit(joy->input_rsp_notify_params.flags,
		       BT_GATT_SUBSCRIBE_FLAG_VOLATILE);

	LOG_DBG("Subscribe: val: %u, ccc: %u",
		joy->input_rsp_notify_params.value_handle,
		joy->input_rsp_notify_params.ccc_handle);
	err = bt_gatt_subscribe(joy->conn, &joy->input_rsp_notify_params);
	if (err) {
		LOG_ERR("Report notification subscribe error: %d.", err);
		joy->input_rsp_notify_cb = NULL;
		return err;
	}
	LOG_DBG("Report subscribed.");
	return err;
}


int bt_joybus_unsubscribe_input_rsp(struct bt_joybus_client *joy)
{
	int err;

	if (!joy) {
		return -EINVAL;
	}

	if (!joy->input_rsp_notify_cb) {
		return -EFAULT;
	}

	err = bt_gatt_unsubscribe(joy->conn, &joy->input_rsp_notify_params);
	joy->input_rsp_notify_cb = NULL;
	return err;
}

int bt_joy_button_enable_indicate(struct bt_joybus_client *joy, bt_button_indicate_cb func)
{
	
	int err;

	if (!joy || !func) {
		return -EINVAL;
	}
	if (!joy->conn) {
		return -EINVAL;
	}
	if (!(joy->button_properties & BT_GATT_CHRC_INDICATE)) {
		return -ENOTSUP;
	}
	if (joy->button_indicate_cb) {
		return -EALREADY;
	}

	joy->button_indicate_cb = func;
	joy->button_indicate_params.value = BT_GATT_CCC_INDICATE;
	joy->button_indicate_params.value_handle = joy->button_state_handle;
	joy->button_indicate_params.ccc_handle = joy->button_ccc_handle;
	joy->button_indicate_params.notify = button_indicate_process;

	atomic_set_bit(joy->button_indicate_params.flags,
		       BT_GATT_SUBSCRIBE_FLAG_VOLATILE);

	LOG_DBG("Subscribe: val: %u, ccc: %u",
		joy->button_indicate_params.value_handle,
		joy->button_indicate_params.ccc_handle);
	err = bt_gatt_subscribe(joy->conn, &joy->button_indicate_params);
	if (err) {
		LOG_ERR("Report indication subscribe error: %d.", err);
		joy->button_indicate_cb = NULL;
		return err;
	}
	LOG_DBG("Report subscribed.");
	return err;


}

struct bt_conn *bt_joybus_conn(const struct bt_joybus_client *joy)
{
	return joy->conn;
}


int bt_joybus_read_input_rsp(struct bt_joybus_client *joy, bt_input_rsp_read_cb func)
{
	int err;

	if (!joy || !func) {
		return -EINVAL;
	}
	if (!joy->conn) {
		return -EINVAL;
	}
	if (joy->input_rsp_read_cb) {
		return -EBUSY;
	}
	joy->input_rsp_read_cb = func;
	joy->input_rsp_read_params.func = read_process;
	joy->input_rsp_read_params.handle_count  = 1;
	joy->input_rsp_read_params.single.handle = joy->input_rsp_val_handle;
	joy->input_rsp_read_params.single.offset = 0;

	err = bt_gatt_read(joy->conn, &joy->input_rsp_read_params);
	if (err) {
		joy->input_rsp_read_cb = NULL;
		return err;
	}
	return 0;
}


int bt_joybus_get_input_rsp(struct bt_joybus_client *joy)
{
	if (!joy) {
		return -EINVAL;
	}

	return joy->input_response;
}


int input_rsp_start_periodic_read(struct bt_joybus_client *joy,
					int32_t interval,
					bt_input_rsp_read_cb func)
{
	if (!joy || !func || !interval) {
		return -EINVAL;
	}

	if (bt_joy_input_rsp_notify_supported(joy)) {
		return -ENOTSUP;
	}

	joy->input_rsp_read_cb = func;
	atomic_set(&joy->input_rsp_periodic_read.interval, interval);
	k_work_schedule(&joy->input_rsp_periodic_read.read_work, K_MSEC(interval));

	return 0;
}


void input_rsp_stop_periodic_read(struct bt_joybus_client *joy)
{
	/* If read is proccesed now, prevent triggering new
	 * characteristic read.
	 */
	atomic_set(&joy->input_rsp_periodic_read.interval, 0);

	/* If delayed workqueue pending, cancel it. If this fails, we'll exit
	 * early in the read handler due to the interval.
	 */
	k_work_cancel_delayable(&joy->input_rsp_periodic_read.read_work);
}

void led_write_cb(struct bt_conn *conn, uint8_t err,
				     struct bt_gatt_write_params *params){
	//Do nothing
}

void joybus_led_write(struct bt_joybus_client *joy, uint32_t led_state){
	
	int err;

	joy->led_write_params.func = led_write_cb;
	joy->led_write_params.handle = joy->led_state_handle;
	joy->led_write_params.data = &led_state;
	joy->led_write_params.length = 1;
	joy->led_write_params.offset = 0;

	err = bt_gatt_write(joy->conn, &joy->led_write_params);
	if(err){
		LOG_ERR("Failed to write to LED Characteristic (err %d)", err);
	}
	else {
		LOG_INF("Sent LED GATT write: %d", led_state);
	}
}
