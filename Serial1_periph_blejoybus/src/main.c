/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/conn.h>
#include <dk_buttons_and_leds.h>

#include <zephyr/drivers/uart.h>
#include "joybus.h"

static struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
	(BT_LE_ADV_OPT_CONNECTABLE |
	 BT_LE_ADV_OPT_USE_IDENTITY), /* Connectable advertising and use identity address */
	800, /* Min Advertising Interval 500ms (800*0.625ms) */
	801, /* Max Advertising Interval 500.625ms (801*0.625ms) */
	NULL); /* Set to NULL for undirected advertising */

LOG_MODULE_REGISTER(Joybus_Transport_Service, LOG_LEVEL_INF);

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define RUN_STATUS_LED DK_LED1
#define CON_STATUS_LED DK_LED2
#define USER_LED DK_LED3
#define USER_BUTTON DK_BTN1_MSK

#define STACKSIZE 4096
#define PRIORITY 7

bool bt_connected = false;

K_MSGQ_DEFINE(uart_queue, sizeof(uint32_t), 1, 4);

#define RUN_LED_BLINK_INTERVAL 1000

#define UART_RX_BUFF_SIZE 4

/* Define the receiving timeout period for UART */
#define RECEIVE_TIMEOUT 100

static bool app_button_state;

//UART1 named as uart throughout the code
const struct device *uart= DEVICE_DT_GET(DT_NODELABEL(uart1));

/*Define the receive buffer */
static uint8_t rx_buf[UART_RX_BUFF_SIZE] = {0};

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),

};

static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_JOYBUS_VAL),
};


static void app_led_cb(bool led_state)
{
	dk_set_led(USER_LED, led_state);
}

static bool app_button_cb(void)
{
	return app_button_state;
}

/* Define the thread function  */
void send_data_thread(void)
{
	int err;
	uint32_t controller_input_rsp;
	while (1) {
		
		err = k_msgq_get(&uart_queue, &controller_input_rsp, K_FOREVER);
		if(err){
		
		}

		/* Send notification, the function sends notifications only if a client is subscribed */
		err = joybus_send_input_response_notify(controller_input_rsp);
		if(err){
		
		}

	}
}

static struct joybus_cb app_callbacks = {
	.led_cb = app_led_cb,
	.button_cb = app_button_cb,
};

static void button_changed(uint32_t button_state, uint32_t has_changed)
{
	int err = 1;
	if (has_changed & USER_BUTTON) {
		uint32_t user_button_state = button_state & USER_BUTTON;
		/* Send indication on a button press */
		//TODO this is crashing on button press
		err = joybus_send_button_state_indicate(user_button_state);
		if(err){
			LOG_ERR("Failed to send button indication (err: %d)", err);
		}
		else LOG_INF("Sent Button %s Indication", (button_state ? "Pressed" : "Released"));
		app_button_state = user_button_state ? true : false;
	}
}

static void update_phy(struct bt_conn *conn)
{
	int err;
	const struct bt_conn_le_phy_param preferred_phy = {
		.options = BT_CONN_LE_PHY_OPT_NONE,
		.pref_rx_phy = BT_GAP_LE_PHY_2M,
		.pref_tx_phy = BT_GAP_LE_PHY_2M,
	};
	err = bt_conn_le_phy_update(conn, &preferred_phy);
	if (err) {
		LOG_ERR("bt_conn_le_phy_update() returned %d", err);
	}
}

static void on_connected(struct bt_conn *conn, uint8_t conn_err)
{
	int err;

	if (conn_err) {
		printk("Connection failed (err %u)\n", conn_err);
		return;
	}

	struct bt_conn_info info;
	err = bt_conn_get_info(conn, &info);
	if (err) {
		LOG_ERR("bt_conn_get_info() returned %d", err);
		return;
	}

	float connection_interval = info.le.interval * 1.25; // in ms
	uint16_t supervision_timeout = info.le.timeout * 10; // in ms
	printk("Connection parameters: interval %.2f ms, latency %d intervals, timeout %d ms",
		connection_interval, info.le.latency, supervision_timeout);

	update_phy(conn);

	bt_connected = true;

	dk_set_led_on(CON_STATUS_LED);

}

void on_le_phy_updated(struct bt_conn *conn, struct bt_conn_le_phy_info *param)
{
	// PHY Updated
	if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_1M) {
		LOG_INF("PHY updated. New PHY: 1M");
	} else if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_2M) {
		LOG_INF("PHY updated. New PHY: 2M");
	} else if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_CODED_S8) {
		LOG_INF("PHY updated. New PHY: Long Range");
	}
}

static void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason %u)\n", reason);

	bt_connected = false;


	dk_set_led_off(CON_STATUS_LED);
}

struct bt_conn_cb connection_callbacks = {
	.connected = on_connected,
	.disconnected = on_disconnected,
	.le_phy_updated = on_le_phy_updated
};

static int init_button(void)
{
	int err;

	err = dk_buttons_init(button_changed);
	if (err) {
		printk("Cannot init buttons (err: %d)\n", err);
	}

	return err;
}


/* Define the callback function for UART */
static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
	uint32_t* rsp_p = (uint32_t *) evt->data.rx.buf;
	int err;

	switch (evt->type) {

		case UART_RX_RDY:

			if(bt_connected){

				//Get controller input response from UART RX buffer
				err = k_msgq_put(&uart_queue, rsp_p, K_NO_WAIT);
				if(err == -ENOMSG)
				{
					//Do nothing for now
				}
			}
			break;

		case UART_RX_DISABLED:
			uart_rx_enable(dev ,rx_buf,sizeof rx_buf,RECEIVE_TIMEOUT);
			break;
			
		default:
			break;
		}

}

int main(void)
{
	
	int err;

	err = dk_leds_init();
	if (err) {
		LOG_ERR("LEDs init failed (err %d)\n", err);
		return -1;
	}

	err = init_button();
	if (err) {
		printk("Button init failed (err %d)\n", err);
		return -1;
	}

	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
		return -1;
	}
	bt_conn_cb_register(&connection_callbacks);

	err = joybus_init(&app_callbacks);
	if (err) {
		printk("Failed to init LBS (err:%d)\n", err);
		return -1;
	}
	LOG_INF("Bluetooth initialized\n");
	err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)\n", err);
		return -1;
	}
	
	LOG_INF("Advertising successfully started\n");

	if (!device_is_ready(uart)){
		LOG_ERR("UART device not ready\r\n");
		return -1 ;
	}

	err = uart_callback_set(uart, uart_cb, NULL);
	if (err) {
		LOG_ERR("Failed to set UART callbacks (err %d)\n", err);
		return -1;
	}

	err = uart_rx_enable(uart ,rx_buf,sizeof rx_buf,RECEIVE_TIMEOUT);
	if (err) {
		LOG_ERR("Failed to enable UART (err %d)\n", err);
		return -1;
	}

	LOG_INF("UART Sucessfully enabled.\n");

	return 0;

}
/* Define and initialize a thread to send data periodically */
K_THREAD_DEFINE(send_data_thread_id, STACKSIZE, send_data_thread, NULL, NULL, NULL, PRIORITY, 0, 0);

