/**
 * @file app.h
 * @brief The application: connects the UART receive path to the CLI.
 *
 * Data flow:
 *   UART receive interrupt --app_on_rx_byte()--> ring buffer --app_poll()--> CLI
 *
 * The interrupt (producer) only drops the byte into the ring buffer and returns
 * quickly. The main loop (consumer) takes bytes out and runs the CLI, so slow
 * work such as printing never happens inside an interrupt.
 */
#ifndef APP_H
#define APP_H

#include <stdbool.h>
#include <stdint.h>

/** Size of the receive ring buffer in bytes. Must be a power of two. */
#define APP_RX_BUFFER_SIZE 64u

/**
 * Initialise everything and print the welcome banner and prompt.
 * @param local_echo true if this program must echo typed characters back
 *                   (a real serial terminal does not echo on its own).
 */
void app_init(bool local_echo);

/** Call for every byte received. Safe to call from an interrupt handler. */
void app_on_rx_byte(uint8_t byte);

/** Call repeatedly from the main loop: processes all bytes received so far. */
void app_poll(void);

/** Number of received bytes lost because the ring buffer was full. */
uint32_t app_rx_dropped(void);

#endif /* APP_H */
