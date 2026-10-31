#include <stdint.h>
#include <string.h>

#include "app.h"
#include "fake_hal.h"
#include "test_framework.h"

/* Pretend these bytes just arrived on the UART (what the interrupt does). */
static void receive(const char *text)
{
    while (*text != '\0') {
        app_on_rx_byte((uint8_t)*text++);
    }
}

static void test_banner_and_prompt_are_printed_at_start(void)
{
    const char *out;
    size_t n;

    app_init(false);
    out = fake_hal_output();
    n = strlen(out);

    CHECK_CONTAINS(out, "Serial CLI (fake)");
    CHECK_CONTAINS(out, "Type 'help'");
    CHECK(n >= 2u && strcmp(out + n - 2u, "> ") == 0);
}

static void test_bytes_are_processed_only_when_the_main_loop_polls(void)
{
    app_init(false);

    receive("led on\r");
    CHECK(!fake_hal_led());                  /* only buffered so far */
    app_poll();
    CHECK(fake_hal_led());                   /* now it has run */
}

static void test_commands_can_arrive_in_pieces(void)
{
    app_init(false);

    receive("le");
    app_poll();
    receive("d o");
    app_poll();
    receive("n\r");
    app_poll();
    CHECK(fake_hal_led());
}

static void test_full_buffer_drops_bytes_and_counts_them(void)
{
    char flood[101];

    app_init(false);
    CHECK(app_rx_dropped() == 0u);

    memset(flood, 'a', 100u);
    flood[100] = '\0';
    receive(flood);                          /* 100 bytes, but the buffer holds 64 */
    CHECK(app_rx_dropped() == 100u - APP_RX_BUFFER_SIZE);

    app_poll();                              /* drain it; the line is now too long */
    receive("\r");
    app_poll();

    fake_hal_clear_output();
    receive("stats\r");
    app_poll();
    CHECK_CONTAINS(fake_hal_output(), "RX bytes dropped: 36");
}

static void test_init_resets_the_drop_counter(void)
{
    char flood[101];

    app_init(false);
    memset(flood, 'a', 100u);
    flood[100] = '\0';
    receive(flood);
    CHECK(app_rx_dropped() > 0u);

    app_init(false);
    CHECK(app_rx_dropped() == 0u);
}

int main(void)
{
    printf("test_app\n");
    RUN_TEST(test_banner_and_prompt_are_printed_at_start);
    RUN_TEST(test_bytes_are_processed_only_when_the_main_loop_polls);
    RUN_TEST(test_commands_can_arrive_in_pieces);
    RUN_TEST(test_full_buffer_drops_bytes_and_counts_them);
    RUN_TEST(test_init_resets_the_drop_counter);
    return test_summary();
}
