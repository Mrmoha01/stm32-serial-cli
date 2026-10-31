/*
 * A fake hardware layer for unit tests: it records everything the code sends to
 * the "UART" and lets a test choose what the "sensor" and "clock" report.
 */
#ifndef FAKE_HAL_H
#define FAKE_HAL_H

#include <stdbool.h>
#include <stdint.h>

void fake_hal_reset(void);                     /* clear output, LED off, temp 20.0 C, time 0 */
void fake_hal_clear_output(void);
const char *fake_hal_output(void);             /* everything written to the UART so far */
bool fake_hal_led(void);
void fake_hal_set_temp(int32_t deci_celsius, bool available);
void fake_hal_set_millis(uint32_t ms);

#endif /* FAKE_HAL_H */
