/**
 * @file cli.h
 * @brief Tiny command-line interpreter for a serial port.
 *
 * Feed it one received character at a time with cli_feed(). It edits the line
 * (backspace, Ctrl+C, ignores arrow-key escape codes), and when Enter is pressed
 * it splits the line into words and runs the matching entry of a command table.
 *
 * It has no dynamic memory and does not use printf. All output goes through a
 * write function supplied by the caller.
 */
#ifndef CLI_H
#define CLI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CLI_LINE_MAX 64      /* size of the line buffer (63 characters + terminator) */
#define CLI_MAX_ARGS 8       /* maximum words per line, including the command name */
#define CLI_PROMPT   "> "

/** Values a command handler returns. */
typedef enum {
    CLI_OK        = 0,  /* success */
    CLI_ERR_USAGE = 1,  /* wrong arguments: the CLI prints "usage: ..." for you */
    CLI_ERR_FAILED = 2  /* failed: the handler already printed its own message */
} cli_status_t;

struct cli;

/** A command handler. argv[0] is the command name. */
typedef int (*cli_handler_t)(struct cli *cli, int argc, char *argv[]);

/** One row of the command table. */
typedef struct {
    const char   *name;     /* what the user types, e.g. "led" */
    const char   *usage;    /* e.g. "led on|off|toggle|status" (shown by help) */
    const char   *help;     /* one-line description (shown by help) */
    cli_handler_t handler;
} cli_command_t;

/** Function the CLI uses to send bytes to the terminal. */
typedef void (*cli_write_fn)(const char *data, size_t len);

typedef struct cli {
    cli_write_fn         write;
    const cli_command_t *commands;
    size_t               command_count;
    bool                 echo;                 /* echo typed characters back? */
    char                 line[CLI_LINE_MAX];
    size_t               len;                  /* characters currently in line[] */
    bool                 overflow;             /* line got too long, ignore the rest */
    bool                 prev_cr;              /* last char was '\r' (to skip the '\n' of CR+LF) */
    uint8_t              esc_state;            /* inside an ANSI escape sequence? */
} cli_t;

/**
 * Initialise the CLI.
 * @param echo true when the terminal does not echo by itself (a real serial
 *             terminal), false when something else already shows what you type.
 */
void cli_init(cli_t *cli, cli_write_fn write, const cli_command_t *commands,
              size_t command_count, bool echo);

/** Give the CLI one received character. */
void cli_feed(cli_t *cli, char c);

/** Print the prompt. */
void cli_prompt(cli_t *cli);

/* Output helpers for command handlers. */
void cli_puts(cli_t *cli, const char *s);
void cli_put_uint(cli_t *cli, uint32_t value);
void cli_put_int(cli_t *cli, int32_t value);

#endif /* CLI_H */
