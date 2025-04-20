/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

//  #include <helpers/nrfx_gppi.h>
//  #include <nrfx_timer.h>
//  #include <nrfx_gpiote.h>
//  #if NRFX_CLOCK_ENABLED && (defined(CLOCK_FEATURE_HFCLK_DIVIDE_PRESENT) || NRF_CLOCK_HAS_HFCLK192M)
//  #include <nrfx_clock.h>
//  #endif
 
//  #include <zephyr/bluetooth/bluetooth.h>
//  #include <zephyr/bluetooth/gap.h>
//  #include <zephyr/bluetooth/uuid.h>
//  #include <zephyr/bluetooth/conn.h>
 
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

//#include "joybus.h"
 
 #include <zephyr/sys/printk.h>

/* STEP 8.1 - Define the macro MAX_NUMBER_FACT that represents the maximum number to calculate its
 * factorial  */
#define MAX_NUMBER_FACT 10

int main(void)
{
	int ret;
	/* STEP 7 - Print a simple banner */
	printk("nRF Connect SDK Fundamentals - Lesson 4 - Exercise 1\n");

    int i;
	long int factorial = 1;

	printk("Calculating the factorials of numbers from 1 to %d:\n", MAX_NUMBER_FACT);
	for (i = 1; i <= MAX_NUMBER_FACT; i++) {
		factorial = factorial * i;
		printk("The factorial of %2d = %ld\n", i, factorial);
	}
	printk("_______________________________________________________\n");

    return 0;
}