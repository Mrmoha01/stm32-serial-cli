/*
 * Host (PC) implementation of the HAL.
 *
 * Lets you run the exact same CLI code in a normal terminal on Linux/macOS.
 * "UART" is stdout, the "LED" is a variable, and the clock is the system clock.
 */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <time.h>

#include "hal.h"
#include "temp_sim.h"

static bool s_led;
static struct timespec s_start;

void hal_init(void)
{
    s_led = false;
    clock_gettime(CLOCK_MONOTONIC, &s_start);
}

void hal_uart_write(const char *data, size_t len)
{
    fwrite(data, 1u, len, stdout);
    fflush(stdout);
}

void hal_led_set(bool on)
{
    s_led = on;
}

bool hal_led_get(void)
{
    return s_led;
}

uint32_t hal_millis(void)
{
    struct timespec now;
    int64_t ms;

    clock_gettime(CLOCK_MONOTONIC, &now);
    ms = (int64_t)(now.tv_sec - s_start.tv_sec) * 1000
       + (int64_t)(now.tv_nsec - s_start.tv_nsec) / 1000000;
    return (uint32_t)ms;
}

bool hal_temp_read(int32_t *deci_celsius)
{
    *deci_celsius = temp_sim_deci_celsius(hal_millis());
    return true;
}

const char *hal_platform_name(void)
{
    return "host";
}
