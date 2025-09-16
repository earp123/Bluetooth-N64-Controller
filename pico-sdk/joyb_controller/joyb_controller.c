#include "pico/stdlib.h"
#include "pico/stdio.h"
#include <stdio.h>
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/error.h"
#include "pico/time.h"
#include "pico/multicore.h"

#include "myBLEsrvc.h"

#include "joybus.pio.h"
#include "pulse_to_byte.pio.h"


#define JOYBUS_PIN 21

//N64 Controller Input request poll byte 0x01
//only 8 bits will get shifted in, in opposite order
//0x80 shifted in to the right will give 0x01 on the data line
const uint32_t request_poll = 0x80;

// Fixed 2 us threshold
const uint32_t bit_threshold = 125;

uint32_t input_rsp = 0;

static void ble_thread()
{
    if(app_ble_init() < 0)
    {
        printf("Error: BLE NOT Intialized\n");
    }
}
   
int main()
{
    stdio_init_all();
    multicore_launch_core1(ble_thread);

    PIO pio = pio0;

    const uint sm_p = pio_claim_unused_sm(pio, true);
    uint poll_offset = pio_add_program(pio, &joybus_program);

    gpio_pull_up(JOYBUS_PIN);

    pio_sm_config poll_cfg = joybus_program_get_default_config(poll_offset);
    sm_config_set_set_pins(&poll_cfg, JOYBUS_PIN, 1);
    pio_gpio_init(pio, JOYBUS_PIN);
    pio_sm_set_consecutive_pindirs(pio, sm_p, JOYBUS_PIN, 1, true);
    pio_sm_set_pins(pio, sm_p, 1);

    float poll_clk_div = 12.0f;
    sm_config_set_clkdiv(&poll_cfg, poll_clk_div);

    //shift in the input request poll byte 0x01
    sm_config_set_out_shift(&poll_cfg, true, false, 8);

    pio_sm_init(pio, sm_p, poll_offset, &poll_cfg);

    const uint sm_r = pio_claim_unused_sm(pio, true);
    uint rsp_offset = pio_add_program(pio, &pulse_to_byte_program);

    pio_sm_config rsp_config = pulse_to_byte_program_get_default_config(rsp_offset);
    sm_config_set_in_pins(&rsp_config, JOYBUS_PIN);
    sm_config_set_jmp_pin(&rsp_config, JOYBUS_PIN);
    pio_sm_set_consecutive_pindirs(pio, sm_r, JOYBUS_PIN, 1, false);

    float rsp_clk_div = 1.0f;
    sm_config_set_clkdiv(&rsp_config, rsp_clk_div);
    sm_config_set_in_shift(&rsp_config, true, false, 32);

    pio_sm_init(pio, sm_r, rsp_offset, &rsp_config);
    pio_sm_put(pio, sm_r, bit_threshold);

    // Enable both state machines  
    pio_sm_set_enabled(pio, sm_p, true);
    pio_sm_set_enabled(pio, sm_r, true);

    
    
    while (true)
    {
        pio_sm_put_blocking(pio, sm_p, request_poll);
        
        // Check if RX FIFO gets anything within reasonable time
        absolute_time_t timeout = make_timeout_time_us(130);
        while (!pio_sm_is_rx_fifo_empty(pio, sm_r) && !time_reached(timeout)) {
            tight_loop_contents();
        }
        
        if (!pio_sm_is_rx_fifo_empty(pio, sm_r)) {
            input_rsp = pio_sm_get_blocking(pio, sm_r);
            printf("Got response: 0x%08X\n", input_rsp);
            tx_notify_u32(input_rsp);
        } else {
            printf("No response (stuck waiting?)\n");
        }
        
        sleep_ms(10);

    }

    return 0;
}
