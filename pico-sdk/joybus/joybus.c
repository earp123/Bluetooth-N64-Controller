#include "pico/stdlib.h"
#include "pico/stdio.h"
#include <stdio.h>
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/error.h"
#include "pico/time.h"
#include "pico/multicore.h"

#include "myBLE_client.h"

#include "joybus.pio.h"
#include "pulse_to_byte.pio.h"

#define JOYBUS_PIN 6
#define VCC_PIN    1

// Fixed 2 us threshold
const uint32_t bit_threshold = 175;

const uint16_t info_rsp = 0x0050;

uint8_t poll_byte;

static void ble_client_thread()
{
    app_ble_client_init();
}
   
int main()
{
    stdio_init_all();
    //gpio_pull_up(VCC_PIN);
    multicore_launch_core1(ble_client_thread);

    PIO pio = pio0;

    //For reading the poll byte as input!
    const uint sm_p = pio_claim_unused_sm(pio, true);
    uint poll_offset = pio_add_program(pio, &pulse_to_byte_program);

    pio_sm_config poll_cfg = pulse_to_byte_program_get_default_config(poll_offset);
    sm_config_set_in_pins(&poll_cfg, JOYBUS_PIN);
    sm_config_set_jmp_pin(&poll_cfg, JOYBUS_PIN);
    pio_gpio_init(pio, JOYBUS_PIN);
    //gpio_pull_up(JOYBUS_PIN);
    

    float poll_clk_div = 1.0f;
    sm_config_set_clkdiv(&poll_cfg, poll_clk_div);

    //shift in the input request poll byte 0x01
    sm_config_set_in_shift(&poll_cfg, false, true, 8);

    pio_sm_init(pio, sm_p, poll_offset, &poll_cfg);
    pio_sm_set_consecutive_pindirs(pio, sm_p, JOYBUS_PIN, 1, false);
    pio_sm_put_blocking(pio, sm_p, bit_threshold);

    const uint sm_r = pio_claim_unused_sm(pio, true);
    uint rsp_offset = pio_add_program(pio, &joybus_program);

    pio_sm_config rsp_config = joybus_program_get_default_config(rsp_offset);
    sm_config_set_set_pins(&rsp_config, JOYBUS_PIN, 1);
    
    float rsp_clk_div = 12.0f;
    sm_config_set_clkdiv(&rsp_config, rsp_clk_div);
    pio_sm_set_enabled(pio, sm_p, true);
    

    while (true)
    {
        poll_byte = (uint8_t) pio_sm_get_blocking(pio, sm_p);
        sleep_us(6);
        
        if(poll_byte == 0x01)
        {
            sm_config_set_out_shift(&rsp_config, true, false, 32);
            pio_sm_init(pio, sm_r, rsp_offset, &rsp_config);
            pio_sm_put_blocking(pio, sm_r, input_rsp);
            pio_sm_set_enabled(pio, sm_r, true);
            sleep_ms(2);
        }
        else if(poll_byte == 0x00)
        {
            sm_config_set_out_shift(&rsp_config, true, false, 16);
            pio_sm_init(pio, sm_r, rsp_offset, &rsp_config);
            pio_sm_put_blocking(pio, sm_r, info_rsp);
            pio_sm_set_enabled(pio, sm_r, true);
            sleep_ms(2);
        }
            
        //printf("%x\n", poll_byte);
        pio_sm_put_blocking(pio, sm_p, 0xaa);//not using the word here
        
    }

    return 0;
}
