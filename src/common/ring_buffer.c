#include "common/ring_buffer.h"

// This function adds an element to the ring buffer.
void ring_buffer_put(struct ring_buffer *rb, uint8_t data) {
  rb->buffer[rb->head] = data;
  rb->head++;

  /* To check whether the head is larger than the size of underlying buffer,
   * if yes then it has to over-wrap by pointing to the first index of the
   * underlying buffer.*/
  if (rb->head == rb->size) {
    rb->head = 0;
  }

  // If ring_buffer is full, remove the oldest element
  if (rb->head == rb->tail) {
    rb->tail++;
  }
}

// Retrive an element from the ring buffer
uint8_t ring_buffer_get(struct ring_buffer *rb) {
  const uint8_t data = rb->buffer[rb->tail];
  rb->tail++;

  /* To check whether the tail is larger than the size of underlying buffer,
   * if yes then it has to over-wrap by pointing to the first index of the
   * underlying buffer.*/
  if (rb->tail == rb->size) {
    rb->tail = 0;
  }
  return data;
}

// Gets the elemnt of the ring buffer by peeking at the value.
uint8_t ring_buffer_peek(const struct ring_buffer *rb) {
  return rb->buffer[rb->tail];
}

// To check whether the ring buffer is empty.
bool ring_buffer_empty(const struct ring_buffer *rb) {
  /* Adding characters will increment the head, while
   * fetching the characters will increment the tail,
   * when tail becomes equal to head, then the buffer is empty.
   */
  return rb->head == rb->tail;
}

// To check whether the ring buffer is full.
bool ring_buffer_full(const struct ring_buffer *rb) {
  uint8_t idx_after_head = rb->head + 1;

  if (idx_after_head == rb->size) {
    idx_after_head = 0;
  }
  return idx_after_head == rb->tail;
}
