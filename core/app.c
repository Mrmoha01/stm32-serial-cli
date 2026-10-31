#include "app.h"

#include "cli.h"
#include "commands.h"
#include "hal.h"
#include "ring_buffer.h"

static uint8_t s_rx_storage[APP_RX_BUFFER_SIZE];
static ring_buffer_t s_rx;
static cli_t s_cli;
static volatile uint32_t s_dropped;   /* written only by app_on_rx_byte() */

void app_init(bool local_echo)
{
    size_t count;
    const cli_command_t *commands = commands_get(&count);

    /* Prepare the buffer BEFORE hal_init() enables the receive interrupt,
     * otherwise a very early byte would be pushed into an unprepared buffer. */
    (void)rb_init(&s_rx, s_rx_storage, sizeof s_rx_storage);
    s_dropped = 0u;
    cli_init(&s_cli, hal_uart_write, commands, count, local_echo);

    hal_init();

    cli_puts(&s_cli, "\r\n=== Serial CLI (");
    cli_puts(&s_cli, hal_platform_name());
    cli_puts(&s_cli, ") ===\r\nType 'help' for a list of commands.\r\n");
    cli_prompt(&s_cli);
}

void app_on_rx_byte(uint8_t byte)
{
    if (!rb_push(&s_rx, byte)) {
        s_dropped = s_dropped + 1u;   /* buffer full: the byte is lost, but counted */
    }
}

void app_poll(void)
{
    uint8_t byte;

    while (rb_pop(&s_rx, &byte)) {
        cli_feed(&s_cli, (char)byte);
    }
}

uint32_t app_rx_dropped(void)
{
    return s_dropped;
}
