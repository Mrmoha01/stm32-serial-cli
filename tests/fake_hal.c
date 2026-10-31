#include "fake_hal.h"

#include <string.h>

#include "hal.h"

#define OUTPUT_SIZE 4096u

static char s_output[OUTPUT_SIZE];
static size_t s_output_len;
static bool s_led;
static int32_t s_temp;
static bool s_temp_available;
static uint32_t s_millis;

void fake_hal_reset(void)
{
    fake_hal_clear_output();
    s_led = false;
    s_temp = 200;
    s_temp_available = true;
    s_millis = 0u;
}

void fake_hal_clear_output(void)
{
    s_output_len = 0u;
    s_output[0] = '\0';
}

const char *fake_hal_output(void)
{
    return s_output;
}

bool fake_hal_led(void)
{
    return s_led;
}

void fake_hal_set_temp(int32_t deci_celsius, bool available)
{
    s_temp = deci_celsius;
    s_temp_available = available;
}

void fake_hal_set_millis(uint32_t ms)
{
    s_millis = ms;
}

/* --- hal.h ---------------------------------------------------------------- */

void hal_init(void)
{
    fake_hal_reset();
}

void hal_uart_write(const char *data, size_t len)
{
    for (size_t i = 0u; i < len && s_output_len < OUTPUT_SIZE - 1u; i++) {
        s_output[s_output_len++] = data[i];
    }
    s_output[s_output_len] = '\0';
}

void hal_led_set(bool on)
{
    s_led = on;
}

bool hal_led_get(void)
{
    return s_led;
}

bool hal_temp_read(int32_t *deci_celsius)
{
    *deci_celsius = s_temp;
    return s_temp_available;
}

uint32_t hal_millis(void)
{
    return s_millis;
}

const char *hal_platform_name(void)
{
    return "fake";
}
