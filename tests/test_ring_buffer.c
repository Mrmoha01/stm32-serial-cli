#include <stdint.h>

#include "ring_buffer.h"
#include "test_framework.h"

#define SIZE 8u

static uint8_t storage[SIZE];
static ring_buffer_t rb;

static void setup(void)
{
    CHECK(rb_init(&rb, storage, SIZE));
}

static void test_init_rejects_bad_arguments(void)
{
    ring_buffer_t other;

    CHECK(!rb_init(NULL, storage, SIZE));
    CHECK(!rb_init(&other, NULL, SIZE));
    CHECK(!rb_init(&other, storage, 0u));
    CHECK(!rb_init(&other, storage, 6u));    /* not a power of two */
    CHECK(!rb_init(&other, storage, 100u));  /* not a power of two */
    CHECK(rb_init(&other, storage, 1u));     /* smallest valid size */
}

static void test_starts_empty(void)
{
    uint8_t byte = 0xAAu;

    setup();
    CHECK(rb_is_empty(&rb));
    CHECK(!rb_is_full(&rb));
    CHECK(rb_count(&rb) == 0u);
    CHECK(rb_capacity(&rb) == SIZE);
    CHECK(!rb_pop(&rb, &byte));
    CHECK(byte == 0xAAu);                    /* untouched when empty */
}

static void test_first_in_first_out(void)
{
    uint8_t byte;

    setup();
    CHECK(rb_push(&rb, 10u));
    CHECK(rb_push(&rb, 20u));
    CHECK(rb_push(&rb, 30u));
    CHECK(rb_count(&rb) == 3u);

    CHECK(rb_pop(&rb, &byte) && byte == 10u);
    CHECK(rb_pop(&rb, &byte) && byte == 20u);
    CHECK(rb_pop(&rb, &byte) && byte == 30u);
    CHECK(rb_is_empty(&rb));
}

static void test_uses_every_slot_then_rejects(void)
{
    uint8_t byte;

    setup();
    for (uint8_t i = 0u; i < SIZE; i++) {
        CHECK(rb_push(&rb, i));
    }
    CHECK(rb_is_full(&rb));
    CHECK(rb_count(&rb) == SIZE);
    CHECK(!rb_push(&rb, 99u));               /* full: new byte is refused */
    CHECK(rb_count(&rb) == SIZE);

    for (uint8_t i = 0u; i < SIZE; i++) {    /* nothing was overwritten */
        CHECK(rb_pop(&rb, &byte) && byte == i);
    }
}

static void test_wraps_around_many_times(void)
{
    uint8_t byte;
    uint8_t next_in = 0u;
    uint8_t next_out = 0u;

    setup();
    /* Push 3, pop 2, over and over: the indices wrap the storage many times. */
    for (int round = 0; round < 1000; round++) {
        for (int i = 0; i < 3; i++) {
            if (rb_push(&rb, next_in)) {
                next_in++;
            }
        }
        for (int i = 0; i < 2; i++) {
            if (rb_pop(&rb, &byte)) {
                CHECK(byte == next_out);
                next_out++;
            }
        }
    }
    while (rb_pop(&rb, &byte)) {
        CHECK(byte == next_out);
        next_out++;
    }
    CHECK(next_in == next_out);
}

static void test_counters_wrap_past_size_max(void)
{
    uint8_t byte;

    setup();
    /* Start the free-running counters just below their maximum value so that
     * they overflow while the buffer is in use. */
    rb.head = SIZE_MAX - 2u;
    rb.tail = SIZE_MAX - 2u;

    for (uint8_t i = 0u; i < SIZE; i++) {
        CHECK(rb_push(&rb, (uint8_t)(100u + i)));
    }
    CHECK(rb_is_full(&rb));
    CHECK(rb_count(&rb) == SIZE);
    CHECK(rb.head < rb.tail);                /* head has wrapped, tail has not */

    for (uint8_t i = 0u; i < SIZE; i++) {
        CHECK(rb_pop(&rb, &byte) && byte == (uint8_t)(100u + i));
    }
    CHECK(rb_is_empty(&rb));
}

int main(void)
{
    printf("test_ring_buffer\n");
    RUN_TEST(test_init_rejects_bad_arguments);
    RUN_TEST(test_starts_empty);
    RUN_TEST(test_first_in_first_out);
    RUN_TEST(test_uses_every_slot_then_rejects);
    RUN_TEST(test_wraps_around_many_times);
    RUN_TEST(test_counters_wrap_past_size_max);
    return test_summary();
}
