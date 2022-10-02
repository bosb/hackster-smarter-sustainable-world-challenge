/*
 * Copyright (c) 2022 bosb
 *
 * SPDX-License-Identifier: GPL-3.0
 */

// Runs on Thingy:53 + nrf53 DK // some functions not available on DK

#include <drivers/sensor.h>
#include <stdio.h>
#include <sys/printk.h>
#include <usb/usb_device.h>
#include <dk_buttons_and_leds.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000
static int led_value = 6;

static void button_changed(uint32_t button_state, uint32_t has_changed) {
    /* Calculate bitmask of buttons that are pressed and have changed their state. */
    uint32_t buttons = button_state & has_changed;
 
    if (buttons & DK_BTN1_MSK) {
        printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
        led_value +=1;
        // jump from all rgb on to r on
        if (led_value > 7) led_value = 1;
    }
}

void main(void) {
    bool led_is_on = false;
    struct sensor_value temp, humidity;

    // INIT
    // temperature sensor
    const struct device *dev_sensor = device_get_binding(DT_LABEL(DT_INST(0, bosch_bme680)));
    if (dev_sensor == NULL) {
        printf("bme688 failed\n");
    } else {
        printf("Device %p name is %s\n", dev_sensor, dev_sensor->name);
    }
    // USB UART: Thingy usb-c, DK via JTAG
    if (usb_enable(NULL) != 0) {
        printk("Failed to enable USB");
    }
    // button
    if (dk_buttons_init(button_changed) != 0) {
        printk("Cannot init buttons");
    }   
    // leds
    if (dk_leds_init() != 0) {
        printk("Cannot init LEDs");
    }   
    printk("INIT finished %s\n", CONFIG_ARCH);

    // repeat every second
    while (1) {
        // flash led to indicate running device
        led_is_on = !led_is_on;
        if (led_is_on) {
            dk_set_leds(led_value);
        } else {
            dk_set_leds(0);
        }

        // does not run on DK
        if (dev_sensor != NULL) {
            sensor_sample_fetch(dev_sensor);
            sensor_channel_get(dev_sensor, SENSOR_CHAN_AMBIENT_TEMP, &temp);
            sensor_channel_get(dev_sensor, SENSOR_CHAN_HUMIDITY, &humidity);
  
            printf("T: %d; H: %d; \n", temp.val1, humidity.val1);
        }
        
        k_msleep(SLEEP_TIME_MS);
    }
}
