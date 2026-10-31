/**
 * @file ring_buffer.h
 * @brief Fixed-size byte FIFO for exactly one producer and one consumer.
 *
 * Typical use on a microcontroller: the UART receive interrupt is the only
 * producer (rb_push) and the main loop is the only consumer (rb_pop). Each side
 * writes only its own index, so no interrupt masking or locking is needed.
 *
 * Design notes:
 *  - The capacity must be a power of two, so "index % capacity" becomes the
 *    cheap "index & mask".
 *  - head and tail are free-running counters. They are never reset to zero;
 *    they simply wrap around at the size of size_t. The number of stored bytes
 *    is always (head - tail), even after the counters wrap. This lets the buffer
 *    use every slot, with no "keep one slot empty" trick to tell full from empty.
 */
#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    volatile uint8_t *data;  /* storage; volatile so the compiler cannot reorder
                                the data write after the head update */
    size_t mask;             /* capacity - 1 */
    volatile size_t head;    /* next position to write (changed by producer only) */
    volatile size_t tail;    /* next position to read  (changed by consumer only) */
} ring_buffer_t;

/**
 * Prepare a ring buffer that uses the caller's storage.
 * @return false if an argument is NULL or capacity is not a power of two.
 */
bool rb_init(ring_buffer_t *rb, uint8_t *storage, size_t capacity);

/** Producer side: add one byte. @return false (byte dropped) if the buffer is full. */
bool rb_push(ring_buffer_t *rb, uint8_t byte);

/** Consumer side: remove the oldest byte. @return false if the buffer is empty. */
bool rb_pop(ring_buffer_t *rb, uint8_t *byte);

size_t rb_count(const ring_buffer_t *rb);     /* bytes currently stored */
size_t rb_capacity(const ring_buffer_t *rb);  /* maximum bytes it can hold */
bool rb_is_empty(const ring_buffer_t *rb);
bool rb_is_full(const ring_buffer_t *rb);

#endif /* RING_BUFFER_H */
