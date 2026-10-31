#include "cli.h"

#include <string.h>

#define CHAR_CTRL_C   0x03
#define CHAR_BACKSPACE 0x08
#define CHAR_ESC      0x1B
#define CHAR_DELETE   0x7F

/* States for skipping ANSI escape sequences such as the arrow keys (ESC [ A). */
enum { ESC_NONE = 0, ESC_GOT_ESC, ESC_IN_SEQUENCE };

/* ------------------------------------------------------------------------- */
/* Output helpers                                                            */
/* ------------------------------------------------------------------------- */

void cli_puts(cli_t *cli, const char *s)
{
    if (s != NULL) {
        cli->write(s, strlen(s));
    }
}

static void put_char(cli_t *cli, char c)
{
    cli->write(&c, 1u);
}

void cli_put_uint(cli_t *cli, uint32_t value)
{
    char buf[11];                  /* 10 digits for 4294967295 + terminator */
    size_t i = sizeof buf - 1u;

    buf[i] = '\0';
    do {
        buf[--i] = (char)('0' + (value % 10u));
        value /= 10u;
    } while (value != 0u);
    cli_puts(cli, &buf[i]);
}

void cli_put_int(cli_t *cli, int32_t value)
{
    if (value < 0) {
        put_char(cli, '-');
        cli_put_uint(cli, 0u - (uint32_t)value);   /* safe even for INT32_MIN */
    } else {
        cli_put_uint(cli, (uint32_t)value);
    }
}

void cli_prompt(cli_t *cli)
{
    cli_puts(cli, CLI_PROMPT);
}

/* ------------------------------------------------------------------------- */
/* Line handling                                                             */
/* ------------------------------------------------------------------------- */

static void reset_line(cli_t *cli)
{
    cli->len = 0u;
    cli->line[0] = '\0';
    cli->overflow = false;
}

/**
 * Split a line into words in place: spaces are replaced by '\0' and argv[]
 * points into the line. @return the word count, or -1 if there are too many.
 */
static int tokenize(char *line, char *argv[], int max_args)
{
    int argc = 0;
    char *p = line;

    while (*p != '\0') {
        while (*p == ' ' || *p == '\t') {
            *p++ = '\0';
        }
        if (*p == '\0') {
            break;
        }
        if (argc == max_args) {
            return -1;
        }
        argv[argc++] = p;
        while (*p != '\0' && *p != ' ' && *p != '\t') {
            p++;
        }
    }
    return argc;
}

static void dispatch(cli_t *cli, int argc, char *argv[])
{
    for (size_t i = 0u; i < cli->command_count; i++) {
        const cli_command_t *cmd = &cli->commands[i];

        if (strcmp(argv[0], cmd->name) == 0) {
            if (cmd->handler(cli, argc, argv) == CLI_ERR_USAGE) {
                cli_puts(cli, "usage: ");
                cli_puts(cli, cmd->usage);
                cli_puts(cli, "\r\n");
            }
            return;
        }
    }
    cli_puts(cli, "error: unknown command '");
    cli_puts(cli, argv[0]);
    cli_puts(cli, "' (type 'help')\r\n");
}

static void execute_line(cli_t *cli)
{
    char *argv[CLI_MAX_ARGS];

    cli->line[cli->len] = '\0';
    cli_puts(cli, "\r\n");

    if (cli->overflow) {
        cli_puts(cli, "error: line too long (max ");
        cli_put_uint(cli, CLI_LINE_MAX - 1u);
        cli_puts(cli, " characters)\r\n");
    } else {
        int argc = tokenize(cli->line, argv, CLI_MAX_ARGS);

        if (argc < 0) {
            cli_puts(cli, "error: too many arguments (max ");
            cli_put_uint(cli, CLI_MAX_ARGS);
            cli_puts(cli, " words)\r\n");
        } else if (argc > 0) {
            dispatch(cli, argc, argv);
        }
    }
    reset_line(cli);
    cli_prompt(cli);
}

/* ------------------------------------------------------------------------- */
/* Public API                                                                */
/* ------------------------------------------------------------------------- */

void cli_init(cli_t *cli, cli_write_fn write, const cli_command_t *commands,
              size_t command_count, bool echo)
{
    cli->write = write;
    cli->commands = commands;
    cli->command_count = command_count;
    cli->echo = echo;
    cli->prev_cr = false;
    cli->esc_state = ESC_NONE;
    reset_line(cli);
}

void cli_feed(cli_t *cli, char c)
{
    unsigned char ch = (unsigned char)c;

    /* Skip ANSI escape sequences (arrow keys, Home, End ...). */
    if (cli->esc_state == ESC_GOT_ESC) {
        cli->esc_state = (ch == '[' || ch == 'O') ? ESC_IN_SEQUENCE : ESC_NONE;
        return;
    }
    if (cli->esc_state == ESC_IN_SEQUENCE) {
        if (ch >= 0x40u && ch <= 0x7Eu) {      /* final byte ends the sequence */
            cli->esc_state = ESC_NONE;
        }
        return;
    }
    if (ch == CHAR_ESC) {
        cli->esc_state = ESC_GOT_ESC;
        return;
    }

    /* Enter. Terminals send '\r', '\n' or both ("\r\n"); count it only once. */
    if (ch == '\r' || ch == '\n') {
        if (ch == '\n' && cli->prev_cr) {
            cli->prev_cr = false;
            return;
        }
        cli->prev_cr = (ch == '\r');
        execute_line(cli);
        return;
    }
    cli->prev_cr = false;

    if (ch == CHAR_CTRL_C) {                   /* cancel the current line */
        cli_puts(cli, "^C\r\n");
        reset_line(cli);
        cli_prompt(cli);
        return;
    }

    if (ch == CHAR_BACKSPACE || ch == CHAR_DELETE) {
        if (!cli->overflow && cli->len > 0u) {
            cli->len--;
            if (cli->echo) {
                cli_puts(cli, "\b \b");        /* move back, blank it, move back */
            }
        }
        return;
    }

    if (ch >= 0x20u && ch <= 0x7Eu) {          /* printable ASCII */
        if (cli->overflow) {
            return;                            /* drop the rest until Enter */
        }
        if (cli->len >= CLI_LINE_MAX - 1u) {
            cli->overflow = true;
            return;
        }
        cli->line[cli->len++] = (char)ch;
        if (cli->echo) {
            put_char(cli, (char)ch);
        }
    }
    /* Any other control character is ignored. */
}
