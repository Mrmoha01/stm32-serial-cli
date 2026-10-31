#include "commands.h"

#include <string.h>

#include "app.h"
#include "hal.h"

#ifndef FW_VERSION
#define FW_VERSION "1.0.0"
#endif

#define HELP_USAGE_COLUMN 26u   /* width of the usage column printed by "help" */

/* Print s, then pad with spaces up to width (always at least one space). */
static void put_padded(cli_t *cli, const char *s, size_t width)
{
    size_t n = strlen(s);

    cli_puts(cli, s);
    do {
        cli_puts(cli, " ");
        n++;
    } while (n < width);
}

/* ------------------------------------------------------------------------- */
/* Command handlers                                                          */
/* ------------------------------------------------------------------------- */

static int cmd_help(cli_t *cli, int argc, char *argv[])
{
    (void)argv;
    if (argc != 1) {
        return CLI_ERR_USAGE;
    }
    /* The list is generated from the command table, so a new command shows up
     * here automatically. */
    cli_puts(cli, "Available commands:\r\n");
    for (size_t i = 0u; i < cli->command_count; i++) {
        cli_puts(cli, "  ");
        put_padded(cli, cli->commands[i].usage, HELP_USAGE_COLUMN);
        cli_puts(cli, cli->commands[i].help);
        cli_puts(cli, "\r\n");
    }
    return CLI_OK;
}

static int cmd_led(cli_t *cli, int argc, char *argv[])
{
    if (argc != 2) {
        return CLI_ERR_USAGE;
    }
    if (strcmp(argv[1], "on") == 0) {
        hal_led_set(true);
    } else if (strcmp(argv[1], "off") == 0) {
        hal_led_set(false);
    } else if (strcmp(argv[1], "toggle") == 0) {
        hal_led_set(!hal_led_get());
    } else if (strcmp(argv[1], "status") != 0) {
        return CLI_ERR_USAGE;
    }
    cli_puts(cli, hal_led_get() ? "LED is ON\r\n" : "LED is OFF\r\n");
    return CLI_OK;
}

static int cmd_temp(cli_t *cli, int argc, char *argv[])
{
    int32_t deci;
    uint32_t magnitude;

    (void)argv;
    if (argc != 1) {
        return CLI_ERR_USAGE;
    }
    if (!hal_temp_read(&deci)) {
        cli_puts(cli, "error: temperature sensor not available\r\n");
        return CLI_ERR_FAILED;
    }
    magnitude = (deci < 0) ? 0u - (uint32_t)deci : (uint32_t)deci;

    cli_puts(cli, "Temperature: ");
    if (deci < 0) {
        cli_puts(cli, "-");
    }
    cli_put_uint(cli, magnitude / 10u);
    cli_puts(cli, ".");
    cli_put_uint(cli, magnitude % 10u);
    cli_puts(cli, " C\r\n");
    return CLI_OK;
}

static int cmd_uptime(cli_t *cli, int argc, char *argv[])
{
    uint32_t seconds;

    (void)argv;
    if (argc != 1) {
        return CLI_ERR_USAGE;
    }
    seconds = hal_millis() / 1000u;

    cli_puts(cli, "Uptime: ");
    cli_put_uint(cli, seconds / 3600u);
    cli_puts(cli, "h ");
    cli_put_uint(cli, (seconds % 3600u) / 60u);
    cli_puts(cli, "m ");
    cli_put_uint(cli, seconds % 60u);
    cli_puts(cli, "s\r\n");
    return CLI_OK;
}

static int cmd_echo(cli_t *cli, int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        if (i > 1) {
            cli_puts(cli, " ");
        }
        cli_puts(cli, argv[i]);
    }
    cli_puts(cli, "\r\n");
    return CLI_OK;
}

static int cmd_stats(cli_t *cli, int argc, char *argv[])
{
    (void)argv;
    if (argc != 1) {
        return CLI_ERR_USAGE;
    }
    cli_puts(cli, "RX buffer size:   ");
    cli_put_uint(cli, APP_RX_BUFFER_SIZE);
    cli_puts(cli, " bytes\r\nRX bytes dropped: ");
    cli_put_uint(cli, app_rx_dropped());
    cli_puts(cli, "\r\n");
    return CLI_OK;
}

static int cmd_version(cli_t *cli, int argc, char *argv[])
{
    (void)argv;
    if (argc != 1) {
        return CLI_ERR_USAGE;
    }
    cli_puts(cli, "Serial CLI firmware v" FW_VERSION " (");
    cli_puts(cli, hal_platform_name());
    cli_puts(cli, ")\r\n");
    return CLI_OK;
}

/* ------------------------------------------------------------------------- */
/* The command table. To add a command: write a handler above, add a row here. */
/* ------------------------------------------------------------------------- */

static const cli_command_t s_commands[] = {
    /* name       usage                       help                              handler */
    { "help",    "help",                     "Show this list",                 cmd_help    },
    { "led",     "led on|off|toggle|status", "Control the user LED",           cmd_led     },
    { "temp",    "temp",                     "Read the temperature",           cmd_temp    },
    { "uptime",  "uptime",                   "Time since power-up",            cmd_uptime  },
    { "echo",    "echo [text...]",           "Print the given text",           cmd_echo    },
    { "stats",   "stats",                    "UART receive statistics",        cmd_stats   },
    { "version", "version",                  "Show firmware version",          cmd_version },
};

const cli_command_t *commands_get(size_t *count)
{
    *count = sizeof s_commands / sizeof s_commands[0];
    return s_commands;
}
