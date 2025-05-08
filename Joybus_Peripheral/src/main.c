#include <helpers/nrfx_gppi.h>
#include <nrfx_timer.h>
#include <nrfx_gpiote.h>
#if NRFX_CLOCK_ENABLED && (defined(CLOCK_FEATURE_HFCLK_DIVIDE_PRESENT) || NRF_CLOCK_HAS_HFCLK192M)
#include <nrfx_clock.h>
#endif

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/conn.h>

#include "joybus.h"

#include <zephyr/sys/printk.h>


/** @brief Symbol specifying timer instance to be used. */
#define TIMER_INST_IDX   1
#define OUTPUT_TIMER_IDX 2

/** @brief Symbol specifying GPIOTE instance to be used. */
#define GPIOTE_INST_IDX   0

/** @brief Symbol specifying ouput pin associated with the task. */
#define INPUT_PIN1 9
#define INPUT_PIN2 8
#define POLL_PIN_HI   7
#define POLL_PIN_LO   25


#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

bool bt_connected = false;
static bool app_button_state;


nrfx_gpiote_t const gpiote_inst = NRFX_GPIOTE_INSTANCE(GPIOTE_INST_IDX);
nrfx_timer_t const timer_inst = NRFX_TIMER_INSTANCE(TIMER_INST_IDX);

nrfx_timer_t const output_timer = NRFX_TIMER_INSTANCE(OUTPUT_TIMER_IDX);


static uint8_t gppi_channel_period;
static uint8_t gppi_channel_pw;
static uint8_t output_gppi_channel;


static uint32_t cont_resp = 0;

static uint8_t bit_count = 0;

bool print_f = false;

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),

};

static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_JOYBUS_VAL),
};


static const struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
	(BT_LE_ADV_OPT_CONNECTABLE |
	 BT_LE_ADV_OPT_USE_IDENTITY), /* Connectable advertising and use identity address */
	800, /* Min Advertising Interval 500ms (800*0.625ms) */
	801, /* Max Advertising Interval 500.625ms (801*0.625ms) */
	NULL); /* Set to NULL for undirected advertising */

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
		printk("bt_conn_le_phy_update() returned %d", err);
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
		printk("bt_conn_get_info() returned %d", err);
		return;
	}

	double connection_interval = info.le.interval * 1.25; // in ms
	uint16_t supervision_timeout = info.le.timeout * 10; // in ms
	printk("Connection parameters: interval %e ms, latency %d intervals, timeout %d ms",
		connection_interval, info.le.latency, supervision_timeout);

	update_phy(conn);

	bt_connected = true;

}

void on_le_phy_updated(struct bt_conn *conn, struct bt_conn_le_phy_info *param)
{
	// PHY Updated
	if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_1M) {
		printk("PHY updated. New PHY: 1M");
	} else if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_2M) {
		printk("PHY updated. New PHY: 2M");
	} else if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_CODED_S8) {
		printk("PHY updated. New PHY: Long Range");
	}
}

static void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason %u)\n", reason);

	bt_connected = false;

}

struct bt_conn_cb connection_callbacks = {
	.connected = on_connected,
	.disconnected = on_disconnected,
	.le_phy_updated = on_le_phy_updated,
};

static void bit_capture_handler(nrfx_gpiote_pin_t pin, nrfx_gpiote_trigger_t trigger, void *p_context)
{
    if(timer_inst.p_reg->CC[0] > 32) cont_resp |= (1<< (31-bit_count));
    else                             cont_resp &= ~(1<<(31-bit_count));                            
    
    bit_count++;

    //TODO Synchronize here
    if(bit_count >= 32)
    {
        bit_count = 0;
        print_f = true;
    } 

}

static void output_timer_handler(nrf_timer_event_t event_type, void * p_context)
{
    
}

static void app_led_cb(bool led_state)
{
	
}

static bool app_button_cb(void)
{
	return app_button_state;
}

static struct joybus_cb app_callbacks = {
	.led_cb = app_led_cb,
	.button_cb = app_button_cb,
};

static void app_gpiote_init()
{
    nrfx_err_t status;
    (void)status;

    static uint8_t period_channel;
    static uint8_t pw_channel;
    static uint8_t output_channel;

    
    status = nrfx_gpiote_init(&gpiote_inst, NRFX_GPIOTE_DEFAULT_CONFIG_IRQ_PRIORITY);
    NRFX_ASSERT(status == NRFX_SUCCESS);
    printk("INPUT GPIOTE status: %s\n",
                   nrfx_gpiote_init_check(&gpiote_inst) ? "initialized" : "not initialized");

    // status = nrfx_gpiote_channel_alloc(&gpiote_inst, &period_channel);
    // NRFX_ASSERT(status == NRFX_SUCCESS);

    // static const nrfx_gpiote_handler_config_t handler_config = {
    //     .handler = bit_capture_handler
    // };

    // static const nrfx_gpiote_trigger_config_t period_trigger_config = {
    //     .p_in_channel = &period_channel,
    //     .trigger      = NRFX_GPIOTE_TRIGGER_LOTOHI
    // };

    // static const nrfx_gpiote_input_pin_config_t period_config = 
    // {
    //     .p_handler_config = &handler_config,
    //     .p_pull_config    = (nrf_gpio_pin_pull_t *) NRF_GPIO_PIN_NOPULL,
    //     .p_trigger_config = &period_trigger_config
    // };


    // nrfx_gpiote_input_configure(&gpiote_inst, INPUT_PIN1, &period_config);
    // nrfx_gpiote_trigger_enable(&gpiote_inst, INPUT_PIN1, true);
    

    // status = nrfx_gpiote_channel_alloc(&gpiote_inst, &pw_channel);
    // NRFX_ASSERT(status == NRFX_SUCCESS);

    // static const nrfx_gpiote_trigger_config_t pw_trigger_config = {
    //     .p_in_channel = &pw_channel,
    //     .trigger      = NRFX_GPIOTE_TRIGGER_HITOLO
    // };

    // static const nrfx_gpiote_input_pin_config_t pw_config = 
    // {
    //     .p_handler_config = NULL,//&handler_config,
    //     .p_pull_config    = (nrf_gpio_pin_pull_t *) NRF_GPIO_PIN_NOPULL,
    //     .p_trigger_config = &pw_trigger_config
    // };

    // nrfx_gpiote_input_configure(&gpiote_inst, INPUT_PIN2, &pw_config);
    // nrfx_gpiote_trigger_enable(&gpiote_inst, INPUT_PIN2, false);

    //Configure output GPIOTE
    status = nrfx_gpiote_channel_alloc(&gpiote_inst, &output_channel);
    NRFX_ASSERT(status == NRFX_SUCCESS);

    const nrfx_gpiote_task_config_t hi_drv_task = {
        .task_ch = output_channel,
        .init_val = NRF_GPIOTE_INITIAL_VALUE_LOW,
        .polarity = NRF_GPIOTE_POLARITY_LOTOHI
    };

    const nrfx_gpiote_task_config_t lo_drv_task = {
        .task_ch = output_channel,
        .init_val = NRF_GPIOTE_INITIAL_VALUE_HIGH,
        .polarity = NRF_GPIOTE_POLARITY_HITOLO
    };

    static const nrfx_gpiote_output_config_t output_config = {
        .drive = NRF_GPIO_PIN_S0S1,
        .pull = NRF_GPIO_PIN_NOPULL
    };

    nrfx_gpiote_output_configure(&gpiote_inst, POLL_PIN_HI, &output_config, &hi_drv_task);
    nrfx_gpiote_output_configure(&gpiote_inst, POLL_PIN_LO, &output_config, &lo_drv_task);
    

}

static void app_timer_init()
{
    nrfx_err_t status;
    (void)status;

    
    uint32_t base_frequency = NRF_TIMER_BASE_FREQUENCY_16MHZ;
    nrfx_timer_config_t timer_config = NRFX_TIMER_DEFAULT_CONFIG(base_frequency);

    // status = nrfx_timer_init(&timer_inst, &timer_config, NULL);
    // if(status != NRFX_SUCCESS)
    // {
    //     printk("Error Timer Init No: %d\n", status);
    //     return;
    // }
    // else
    //     printk("Controller Response Timer Initialized\n");

    // nrfx_timer_clear(&timer_inst);

    status = nrfx_timer_init(&output_timer, &timer_config, output_timer_handler);
    if(status != NRFX_SUCCESS)
    {
        printk("Error Timer Init No: %d\n", status);
        return;
    }
    else
        printk("Poll Timer Initialized\n");

    uint32_t period_len = nrfx_timer_us_to_ticks(&output_timer, (uint32_t) 100);
    uint32_t zero_interval = nrfx_timer_us_to_ticks(&output_timer, (uint32_t) 50);

    
    nrfx_timer_extended_compare(&output_timer, NRF_TIMER_CC_CHANNEL0, period_len, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
    nrfx_timer_compare(&output_timer, NRF_TIMER_CC_CHANNEL1, zero_interval, true);
                                                        
    nrfx_timer_clear(&output_timer);

}

static void app_gppi_setup()
{
    nrfx_err_t status;
    (void)status;

    status = nrfx_gppi_channel_alloc(&gppi_channel_period);
    NRFX_ASSERT(status == NRFX_SUCCESS);

    /*
     * Use this to start the timer on the rising edge
     */
    // nrfx_gppi_channel_endpoints_setup(gppi_channel_period,
    //     nrfx_gpiote_in_event_address_get(&gpiote_inst, INPUT_PIN1),
    //     nrfx_timer_task_address_get(&timer_inst, NRF_TIMER_TASK_CLEAR));

    // nrfx_gppi_channels_enable(BIT(gppi_channel_period));

    // status = nrfx_gppi_channel_alloc(&gppi_channel_pw);
    // NRFX_ASSERT(status == NRFX_SUCCESS);

    // /*
    //  * Use this for capturing the timer count on the falling edge to measure the pulse width
    //  */
    // nrfx_gppi_channel_endpoints_setup(gppi_channel_pw,
    //     nrfx_gpiote_in_event_address_get(&gpiote_inst, INPUT_PIN2),
    //     nrfx_timer_capture_task_address_get(&timer_inst, NRF_TIMER_CC_CHANNEL0));

    // nrfx_gppi_channels_enable(BIT(gppi_channel_pw));

    status = nrfx_gppi_channel_alloc(&output_gppi_channel);
    NRFX_ASSERT(status == NRFX_SUCCESS);

    //output gpppi
    nrfx_gppi_channel_endpoints_setup(output_gppi_channel,
        nrfx_timer_compare_event_address_get(&output_timer, NRF_TIMER_CC_CHANNEL0),
        nrfx_gpiote_out_task_address_get(&gpiote_inst, POLL_PIN_HI));

    nrfx_gppi_channel_endpoints_setup(output_gppi_channel,
        nrfx_timer_compare_event_address_get(&output_timer, NRF_TIMER_CC_CHANNEL1),
        nrfx_gpiote_out_task_address_get(&gpiote_inst, POLL_PIN_LO));

    nrfx_gppi_channels_enable(BIT(gppi_channel_pw));


    printk("GPPI channels setup and enabled\n");
}


int main(void)
{
    
    printk("Starting nrfx_gppi pulse width sketch.\n");

    #if defined(__ZEPHYR__)
        IRQ_CONNECT(NRFX_IRQ_NUMBER_GET(NRF_TIMER_INST_GET(TIMER_INST_IDX)), IRQ_PRIO_LOWEST,
                    NRFX_TIMER_INST_HANDLER_GET(TIMER_INST_IDX), 0, 0);
        IRQ_CONNECT(NRFX_IRQ_NUMBER_GET(NRF_TIMER_INST_GET(OUTPUT_TIMER_IDX)), IRQ_PRIO_LOWEST,
                    NRFX_TIMER_INST_HANDLER_GET(OUTPUT_TIMER_IDX), 0, 0);
        IRQ_CONNECT(NRFX_IRQ_NUMBER_GET(NRF_GPIOTE_INST_GET(GPIOTE_INST_IDX)), IRQ_PRIO_LOWEST,
                    NRFX_GPIOTE_INST_HANDLER_GET(GPIOTE_INST_IDX), 0, 0);
    #endif

    printk("Connected Interrupts.\n");

    //FAST AF BOOOII
    #if NRFX_CLOCK_ENABLED && (defined(CLOCK_FEATURE_HFCLK_DIVIDE_PRESENT) || NRF_CLOCK_HAS_HFCLK192M)
	    nrfx_clock_divider_set(NRF_CLOCK_DOMAIN_HFCLK, NRF_CLOCK_HFCLK_DIV_1);
    #endif

    // int err;
    
    app_timer_init();

    app_gpiote_init();

    app_gppi_setup();

 
    //nrfx_timer_enable(&timer_inst);

    nrfx_timer_enable(&output_timer);
    
    // printk("Controller Response Timer status: %s\n", nrfx_timer_is_enabled(&timer_inst) ? "enabled" : "disabled");
    // printk("Poll Timer status: %s\n", nrfx_timer_is_enabled(&output_timer) ? "enabled" : "disabled");
    

    // err = bt_enable(NULL);
	// if (err) {
	// 	printk("Bluetooth init failed (err %d)\n", err);
	// 	return -1;
	// }

	// err = bt_conn_cb_register(&connection_callbacks);
    // if (err) {
	// 	printk("Bluetooth callbacks failed to register. (err %d)\n", err);
	// 	return -1;
	// }

	// err = joybus_init(&app_callbacks);
	// if (err) {
	// 	printk("Failed to init Joybus (err:%d)\n", err);
	// 	return -1;
	// }
	// printk("Bluetooth initialized\n");
    

	// err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	// if (err) {
	// 	printk("Advertising failed to start (err %d)\n", err);
	// 	return -1;
	// }

    //printk("Started Advertising....\n");
    k_msleep(1500);

    printk("Enter Main Loop....\n");


    while (1)
    {

        //TODO build the main loop out

        //Generate Poll

        //Start the pulse width timer

        //wait for the gppi to build the controller response ~200uS

        //send the controller response over BT

        // if(print_f)
        // {
        //     printk("PERIPHERAL: %x\n", cont_resp);

        //     if(bt_connected)
        //     {
        //         /* Send notification, the function sends notifications only if a client is subscribed */
        //         err = joybus_send_input_response_notify(cont_resp);
        //         if(err)
        //         {
        //             printk("Error sending controller response. Err: %d\n", err);
        //         }
        //     }
            
        //     print_f = false;
        // }
        k_msleep(2);

    }

    return 0;

}

/** @} */
