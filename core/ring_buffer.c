#include "ring_buffer.h"

static bool is_power_of_two(size_t n)
{
    return n != 0u && (n & (n - 1u)) == 0u;
}

bool rb_init(ring_buffer_t *rb, uint8_t *storage, size_t capacity)
{
    if (rb == NULL || storage == NULL || !is_power_of_two(capacity)) {
        return false;
    }
    rb->data = storage;
    rb->mask = capacity - 1u;
    rb->head = 0u;
    rb->tail = 0u;
    return true;
}

bool rb_push(ring_buffer_t *rb, uint8_t byte)
{
    size_t head = rb->head;

    if ((head - rb->tail) > rb->mask) {   /* count == capacity: full */
        return false;
    }
    rb->data[head & rb->mask] = byte;     /* 1. store the data ...        */
    rb->head = head + 1u;                 /* 2. ... then publish it       */
    return true;
}

bool rb_pop(ring_buffer_t *rb, uint8_t *byte)
{
    size_t tail = rb->tail;

    if (rb->head == tail) {               /* empty */
        return false;
    }
    *byte = rb->data[tail & rb->mask];    /* 1. read the data ...         */
    rb->tail = tail + 1u;                 /* 2. ... then free the slot    */
    return true;
}

size_t rb_count(const ring_buffer_t *rb)
{
    return rb->head - rb->tail;
}

size_t rb_capacity(const ring_buffer_t *rb)
{
    return rb->mask + 1u;
}

bool rb_is_empty(const ring_buffer_t *rb)
{
    return rb->head == rb->tail;
}

bool rb_is_full(const ring_buffer_t *rb)
{
    return rb_count(rb) > rb->mask;
}
