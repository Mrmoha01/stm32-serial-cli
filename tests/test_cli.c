#include <stdint.h>
#include <string.h>

#include "cli.h"
#include "commands.h"
#include "fake_hal.h"
#include "hal.h"
#include "test_framework.h"

static cli_t cli;

static void setup(bool echo)
{
    size_t count;
    const cli_command_t *commands = commands_get(&count);

    fake_hal_reset();
    cli_init(&cli, hal_uart_write, commands, count, echo);
}

static void feed(const char *text)
{
    while (*text != '\0') {
        cli_feed(&cli, *text++);
    }
}

/* Type some text and return only what the CLI printed in response. */
static const char *run(const char *text)
{
    fake_hal_clear_output();
    feed(text);
    return fake_hal_output();
}

/* --- Tests ---------------------------------------------------------------- */

static void test_help_is_generated_from_the_command_table(void)
{
    size_t count;
    const cli_command_t *commands = commands_get(&count);
    const char *out;

    setup(false);
    out = run("help\r");
    CHECK(count > 0u);
    for (size_t i = 0u; i < count; i++) {
        CHECK_CONTAINS(out, commands[i].usage);
        CHECK_CONTAINS(out, commands[i].help);
    }
}

static void test_unknown_command(void)
{
    setup(false);
    CHECK_CONTAINS(run("frobnicate\r"), "error: unknown command 'frobnicate'");
}

static void test_led_commands(void)
{
    setup(false);

    CHECK_CONTAINS(run("led on\r"), "LED is ON");
    CHECK(fake_hal_led());
    CHECK_CONTAINS(run("led off\r"), "LED is OFF");
    CHECK(!fake_hal_led());

    run("led toggle\r");
    CHECK(fake_hal_led());
    run("led toggle\r");
    CHECK(!fake_hal_led());

    run("led on\r");
    CHECK_CONTAINS(run("led status\r"), "LED is ON");
    CHECK(fake_hal_led());                   /* status must not change it */
}

static void test_led_usage_errors(void)
{
    setup(false);

    CHECK_CONTAINS(run("led\r"), "usage: led on|off|toggle|status");
    CHECK_CONTAINS(run("led blink\r"), "usage: led on|off|toggle|status");
    CHECK_CONTAINS(run("led on now\r"), "usage: led on|off|toggle|status");
    CHECK(!fake_hal_led());
}

static void test_temp(void)
{
    setup(false);

    fake_hal_set_temp(235, true);
    CHECK_CONTAINS(run("temp\r"), "Temperature: 23.5 C");
    fake_hal_set_temp(-5, true);
    CHECK_CONTAINS(run("temp\r"), "Temperature: -0.5 C");
    fake_hal_set_temp(-123, true);
    CHECK_CONTAINS(run("temp\r"), "Temperature: -12.3 C");
    fake_hal_set_temp(0, true);
    CHECK_CONTAINS(run("temp\r"), "Temperature: 0.0 C");

    fake_hal_set_temp(0, false);
    CHECK_CONTAINS(run("temp\r"), "sensor not available");

    CHECK_CONTAINS(run("temp now\r"), "usage: temp");
}

static void test_uptime(void)
{
    setup(false);

    fake_hal_set_millis(0u);
    CHECK_CONTAINS(run("uptime\r"), "Uptime: 0h 0m 0s");
    fake_hal_set_millis(3725000u);           /* 1 h 2 min 5 s */
    CHECK_CONTAINS(run("uptime\r"), "Uptime: 1h 2m 5s");
}

static void test_echo_joins_words_with_single_spaces(void)
{
    setup(false);
    CHECK_STR_EQ(run("echo hello   world\r"), "\r\nhello world\r\n> ");
    CHECK_STR_EQ(run("echo\r"), "\r\n\r\n> ");
}

static void test_version(void)
{
    setup(false);
    CHECK_CONTAINS(run("version\r"), "(fake)");
}

static void test_empty_line_only_shows_a_new_prompt(void)
{
    setup(false);
    CHECK_STR_EQ(run("\r"), "\r\n> ");
    CHECK_STR_EQ(run("   \r"), "\r\n> ");
}

static void test_cr_lf_and_lf_each_count_once(void)
{
    setup(false);
    CHECK_STR_EQ(run("\r\n"), "\r\n> ");     /* Windows style: one Enter */
    CHECK_STR_EQ(run("\n"), "\r\n> ");       /* Unix style */
    CHECK_STR_EQ(run("\r"), "\r\n> ");       /* old serial style */
}

static void test_line_exactly_at_the_limit_is_accepted(void)
{
    char line[CLI_LINE_MAX + 8];
    size_t n;

    setup(false);
    memcpy(line, "echo ", 5u);
    n = 5u;
    while (n < CLI_LINE_MAX - 1u) {          /* total 63 characters */
        line[n++] = 'x';
    }
    line[n++] = '\r';
    line[n] = '\0';

    const char *out = run(line);
    CHECK(strstr(out, "too long") == NULL);
    CHECK_CONTAINS(out, "xxxxxxxx");
}

static void test_line_too_long_is_rejected_and_cli_recovers(void)
{
    char line[CLI_LINE_MAX + 8];
    size_t n;

    setup(false);
    memcpy(line, "echo ", 5u);
    n = 5u;
    while (n < CLI_LINE_MAX) {               /* total 64 characters: one too many */
        line[n++] = 'x';
    }
    line[n++] = '\r';
    line[n] = '\0';

    CHECK_CONTAINS(run(line), "error: line too long (max 63 characters)");

    /* A very long line is also fine, and the next command still works. */
    char huge[300];
    memset(huge, 'a', sizeof huge - 2u);
    huge[sizeof huge - 2u] = '\r';
    huge[sizeof huge - 1u] = '\0';
    CHECK_CONTAINS(run(huge), "line too long");
    CHECK_CONTAINS(run("led on\r"), "LED is ON");
}

static void test_too_many_arguments(void)
{
    setup(false);

    CHECK_CONTAINS(run("echo 1 2 3 4 5 6 7 8\r"), "too many arguments");   /* 9 words */
    CHECK_CONTAINS(run("echo 1 2 3 4 5 6 7\r"), "1 2 3 4 5 6 7");          /* 8 words */
}

static void test_backspace_edits_the_line(void)
{
    setup(false);
    /* Type "lex", delete the "x", then "d on" -> "led on". */
    CHECK_CONTAINS(run("lex\x7f" "d on\r"), "LED is ON");

    setup(false);
    CHECK_CONTAINS(run("led of\x08" "n\r"), "LED is ON");   /* Ctrl+H also works */

    setup(false);
    CHECK_STR_EQ(run("\x7f\x7f"), "");       /* backspace on an empty line does nothing */
}

static void test_echo_mode_shows_typed_characters(void)
{
    setup(true);
    CHECK_STR_EQ(run("hi"), "hi");
    CHECK_STR_EQ(run("\x7f"), "\b \b");      /* erases the 'i' */
    CHECK_STR_EQ(run("\x7f"), "\b \b");      /* erases the 'h' */
    CHECK_STR_EQ(run("\x7f"), "");           /* line is empty: nothing to erase */
    CHECK_STR_EQ(run("\r"), "\r\n> ");

    setup(false);
    CHECK_STR_EQ(run("hi"), "");             /* echo off: nothing printed */
}

static void test_ctrl_c_cancels_the_line(void)
{
    setup(false);
    feed("led on");
    CHECK_CONTAINS(run("\x03"), "^C");
    CHECK_STR_EQ(run("\r"), "\r\n> ");       /* the typed text is gone */
    CHECK(!fake_hal_led());
}

static void test_arrow_keys_and_escape_codes_are_ignored(void)
{
    setup(false);
    CHECK_CONTAINS(run("led \x1b[A\x1b[Bon\r"), "LED is ON");   /* up, down arrows */

    setup(false);
    CHECK_CONTAINS(run("led \x1bOAon\r"), "LED is ON");         /* application mode */

    setup(false);
    CHECK_CONTAINS(run("led \x1b[1;5Con\r"), "LED is ON");      /* Ctrl+Right */
}

static void test_other_control_characters_are_ignored(void)
{
    setup(false);
    CHECK_CONTAINS(run("led\x01 on\r"), "LED is ON");
}

static void test_number_printing(void)
{
    setup(false);
    /* Exercise cli_put_int / cli_put_uint through the public helpers. */
    fake_hal_clear_output();
    cli_put_uint(&cli, 0u);
    cli_puts(&cli, " ");
    cli_put_uint(&cli, 4294967295u);
    cli_puts(&cli, " ");
    cli_put_int(&cli, -42);
    cli_puts(&cli, " ");
    cli_put_int(&cli, INT32_MIN);
    CHECK_STR_EQ(fake_hal_output(), "0 4294967295 -42 -2147483648");
}

int main(void)
{
    printf("test_cli\n");
    RUN_TEST(test_help_is_generated_from_the_command_table);
    RUN_TEST(test_unknown_command);
    RUN_TEST(test_led_commands);
    RUN_TEST(test_led_usage_errors);
    RUN_TEST(test_temp);
    RUN_TEST(test_uptime);
    RUN_TEST(test_echo_joins_words_with_single_spaces);
    RUN_TEST(test_version);
    RUN_TEST(test_empty_line_only_shows_a_new_prompt);
    RUN_TEST(test_cr_lf_and_lf_each_count_once);
    RUN_TEST(test_line_exactly_at_the_limit_is_accepted);
    RUN_TEST(test_line_too_long_is_rejected_and_cli_recovers);
    RUN_TEST(test_too_many_arguments);
    RUN_TEST(test_backspace_edits_the_line);
    RUN_TEST(test_echo_mode_shows_typed_characters);
    RUN_TEST(test_ctrl_c_cancels_the_line);
    RUN_TEST(test_arrow_keys_and_escape_codes_are_ignored);
    RUN_TEST(test_other_control_characters_are_ignored);
    RUN_TEST(test_number_printing);
    return test_summary();
}
