/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef BT_LBS_H_
#define BT_LBS_H_

/**@file
 * @defgroup bt_lbs LED Button Service API
 * @{
 * @brief API for the LED Button Service (LBS).
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/types.h>

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

/** @brief Callback type for when an LED state change is received. */
typedef void (*led_cb_t)(const bool led_state);

/** @brief Callback type for when the button state is pulled. */
typedef bool (*button_cb_t)(void);

/** @brief Callback struct used by the LBS Service. */
struct joybus_cb {
	/** LED state change callback. */
	led_cb_t led_cb;
	/** Button read callback. */
	button_cb_t button_cb;
};

/** @brief Initialize the LBS Service.
 *
 * This function registers application callback functions with the My LBS
 * Service
 *
 * @param[in] callbacks Struct containing pointers to callback functions
 *			used by the service. This pointer can be NULL
 *			if no callback functions are defined.
 *
 *
 * @retval 0 If the operation was successful.
 *           Otherwise, a (negative) error code is returned.
 */
int joybus_init(struct joybus_cb *callbacks);

/** @brief Send the button state as indication.
 *
 * This function sends a binary state, typically the state of a
 * button, to all connected peers.
 *
 * @param[in] button_state The state of the button.
 *
 * @retval 0 If the operation was successful.
 *           Otherwise, a (negative) error code is returned.
 */
int joybus_send_button_state_indicate(bool button_state);

/** @brief Send the button state as notification.
 *
 * This function sends a binary state, typically the state of a
 * button, to all connected peers.
 *
 * @param[in] button_state The state of the button.
 *
 * @retval 0 If the operation was successful.
 *           Otherwise, a (negative) error code is returned.
 */
int joybus_send_button_state_notify(bool button_state);

/** @brief Send the sensor value as notification.
 *
 * This function sends an uint32_t  value, typically the value
 * of a simulated sensor to all connected peers.
 *
 * @param[in] sensor_value The value of the simulated sensor.
 *
 * @retval 0 If the operation was successful.
 *           Otherwise, a (negative) error code is returned.
 */
int joybus_send_input_response_notify(uint32_t sensor_value);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* BT_LBS_H_ */
