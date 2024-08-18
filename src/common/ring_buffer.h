#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#include <stdbool.h>
#include <stdint.h>

struct ring_buffer {
  uint8_t *buffer; // pointer to the underlying buffer
  uint8_t size;
  uint8_t head;
  uint8_t tail;
};

/*Caller must check if full.
 * This function adds an element to the ring buffer.
 */
void ring_buffer_put(struct ring_buffer *rb, uint8_t data);

/*Caller must check if empty.
 * Retrive an element from the ring buffer.
 */
uint8_t ring_buffer_get(struct ring_buffer *rb);

/*Caller must check if empty.
 * Gets the elemnt of the ring buffer by peeking at the value.
 */
uint8_t ring_buffer_peek(const struct ring_buffer *rb);

// To check whether the ring buffer is empty.
bool ring_buffer_empty(const struct ring_buffer *rb);

// To check whether the ring buffer is full.
bool ring_buffer_full(const struct ring_buffer *rb);

#endif /* RING_BUFFER_H_ */
