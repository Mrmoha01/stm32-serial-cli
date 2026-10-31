/**
 * @file hal.h
 * @brief Hardware abstraction layer: everything the portable code needs from
 *        the outside world.
 *
 * The code in core/ never touches registers. It only calls these functions.
 * Each platform provides its own implementation:
 *   - platform/stm32c0/hal_stm32c0.c  real STM32 hardware (or the Wokwi simulator)
 *   - platform/host/hal_host.c        your PC terminal
 *   - tests/fake_hal.c                a fake used by the unit tests
 */
#ifndef HAL_H
#define HAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** Set up clocks, GPIO, UART and the millisecond timer. */
void hal_init(void);

/** Send bytes over the serial port. May block until they are sent. */
void hal_uart_write(const char *data, size_t len);

/** Turn the user LED on or off. */
void hal_led_set(bool on);

/** @return true if the user LED is currently on. */
bool hal_led_get(void);

/**
 * Read the temperature in tenths of a degree Celsius (235 means 23.5 C).
 * @return false if no temperature is available.
 */
bool hal_temp_read(int32_t *deci_celsius);

/** Milliseconds since power-up. */
uint32_t hal_millis(void);

/** Short name of the platform, shown by the "version" command. */
const char *hal_platform_name(void);

#endif /* HAL_H */
