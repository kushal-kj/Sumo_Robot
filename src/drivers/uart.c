#include "drivers/uart.h"
#include "common/assert_handler.h"
#include "common/defines.h"
#include "common/ring_buffer.h"
#include <assert.h>
#include <msp430.h>
#include <stdint.h>

#define UART_BUFFER_SIZE (16)

static uint8_t buffer[UART_BUFFER_SIZE];
static struct ring_buffer tx_buffer = {.buffer = buffer,
                                       .size = sizeof(buffer)};

/* Calcualte the integer and fractional part of the divisor
 * N = (Clock source / Desired baudrate)
 * for Low-frequency baud rate mode.
 *
 * These are used to configure the desired baudrate.
 *
 * For more info on corresponding transmission error
 * for common values, refer the table provided in the
 * msp430x5xx family user guide.
 */

#define SMCLK (16000000u)
#define BRCLK (SMCLK)
#define UART_BAUD_RATE (115200u)

static_assert(
    UART_BAUD_RATE < (BRCLK / 3.0f),
    "Baudrate must be smaller than 1/3 of input clock in Low-Frequency Mode");

#define UART_DIVISOR ((float)BRCLK / UART_BAUD_RATE)
static_assert(UART_DIVISOR < 0xFFFFu, "Sanity check divisor fits in 16-bit");

#define UART_DIVISOR_INT_16BIT ((uint16_t)UART_DIVISOR)
#define UART_DIVISOR_INT_LOW_BYTE (UART_DIVISOR_INT_16BIT & 0xFF)
#define UART_DIVISOR_INT_HIGH_BYTE (UART_DIVISOR_INT_16BIT >> 8)

#define UART_DIVISOR_FRACTIONAL (UART_DIVISOR - UART_DIVISOR_INT_16BIT)

#define UART_UCBRS ((uint8_t)(8 * UART_DIVISOR_FRACTIONAL))
static_assert(UART_UCBRS < 8,
              "Sanity check second modulation stage value fits in 3-bit");

#define UART_UCBRF (0)
#define UART_UCOS16 (0)

/* Clear the Transmit Interrupt flag and
 * inline function is used to avoid function call-overhead. */
static inline void uart_tx_clear_interrupt(void) { UCA0IFG &= ~UCTXIFG; }

static inline void uart_tx_enable_interrupt(void) {
  // Enable the TX interrupt
  UCA0IE |= UCTXIE;
}

static inline void uart_tx_disable_interrupt(void) {
  // Disable the TX interrupt
  UCA0IE &= ~UCTXIE;
}

static void uart_tx_start(void) {
  // Check whether the buffer is empty, if not, then transmit the data remaining
  // in the buffer until it is empty.

  if (!ring_buffer_empty(&tx_buffer)) {
    UCA0TXBUF = ring_buffer_peek(&tx_buffer);
  }
}

INTERRUPT_FUNCTION(USCI_A0_VECTOR) USCI_A0_ISR(void) {
  switch (__even_in_range(UCA0IV, 4)) {
  case 0:
    break; // No interrupt
  case 2:
    break; // RX interrupt (ignored)
  case 4:  // TX interrupt

    ASSERT_INTERRUPT(!ring_buffer_empty(&tx_buffer));

    // Remove the transmitted data byte from the buffer
    ring_buffer_get(&tx_buffer);

    // Clear interrupt here to avoid accidently clearing interrupt for next
    // transmission
    uart_tx_clear_interrupt();

    // Check whether the buffer is empty, if not, then transmit the data
    // remaining in the buffer until it is empty.
    if (!ring_buffer_empty(&tx_buffer)) {
      uart_tx_start();
    }
    break;
  default:
    break;
  }
  // If there's a stray closing brace or an incomplete do-while loop,
  // the compiler might throw the error here.
}

static void uart_configure(void) {
  /* Reset module. It stays in reset until cleared. The module should be in
   * reset condition while configured, according to msp430x5xx family user
   * guide.
   */
  UCA0CTL1 |= UCSWRST;

  /*Use default (data word length 8 bits, 1 stop bit, no parity bit)
   * [ start (1 bit) | data (8 bits) | stop (1 bit) ]
   */
  UCA0CTL0 = 0;

  // Set SMCLK as clock source
  UCA0CTL1 |= UCSSEL_2;

  // Set clock prescaler  to the integer part of divisor N.
  UCA0BR0 = UART_DIVISOR_INT_LOW_BYTE;
  UCA0BR1 = UART_DIVISOR_INT_HIGH_BYTE;

  /* Set modulation to account for fractional part of divisor N.
   * UCA0MCTL = [UCBRF (4-bits) | UCBRS(3-bits) | UC0S16(1-bit)]
   */
  UCA0MCTL = (UART_UCBRF << 4) + (UART_UCBRS << 1) + UART_UCOS16;

  // Clear reset to release module for operation.
  UCA0CTL1 &= ~UCSWRST;
}

static bool initialized = false;
// This function will initialize the UART peripheral
void uart_init(void) {

  ASSERT(!initialized);

  uart_configure();

  // Interrupt triggers when TX buffer is empty, which it is after boot, so need
  // to clear it here.
  uart_tx_clear_interrupt();

  // Enable the TX interrupt
  uart_tx_enable_interrupt();

  initialized = true;
}

// This function will send a single character through polling method
void uart_putchar_polling(char c) {

  // Carriage return(\r) after line feed(\n) for proper new line.
  if (c == '\n') {
    uart_putchar_polling('\r');
  }

  // Wait for any ongoing transmission to finish.
  // Check for USCI A0 Interrupt flag register i.e. UCA0IFG which contains TX
  // and RX interrupt buffer flags
  while (!(UCA0IFG & UCTXIFG)) {
  }
  UCA0TXBUF = c;
}

/*
// This function will send characters through Interrupt method
void uart_putchar_interrupt(char c) {
  // Poll is full
  while (ring_buffer_full(&tx_buffer))
    ;

  // Disable the tx interrupt before starting the transmission
  uart_tx_disable_interrupt();

  // Check if there is any ongoing transmission.
  const bool tx_ongoing = !ring_buffer_empty(&tx_buffer);

  ring_buffer_put(&tx_buffer, c);

  if (!tx_ongoing) {
    uart_tx_start();
  }

  uart_tx_enable_interrupt();

  // Carriage return(\r) after line feed(\n) for proper new line.
  if (c == '\n') {
    uart_putchar_interrupt('\r');
  }
}

*/
// mpland/printf needs this to be named _putchar.
// Custom printf implementation.
void _putchar(char c) {

  // Carriage return(\r) before line feed(\n) for proper new line.
  if (c == '\n') {
    _putchar('\r');
  }

  // Poll is full
  while (ring_buffer_full(&tx_buffer)) {
  }

  // Disable the tx interrupt before starting the transmission
  uart_tx_disable_interrupt();

  // Check if there is any ongoing transmission.
  const bool tx_ongoing = !ring_buffer_empty(&tx_buffer);

  ring_buffer_put(&tx_buffer, c);

  if (!tx_ongoing) {
    uart_tx_start();
  }

  uart_tx_enable_interrupt();
}

/*
// To print an entire string of characters
void uart_print_interrupt(const char *string) {
  int i = 0;
  while (string[i] != '\0') {
    uart_putchar_interrupt(string[i]);
    i++;
  }
}
*/

// FOR ASSERTION
void uart_init_assert(void) {
  uart_tx_disable_interrupt();
  uart_configure();
}

void uart_trace_assert(const char *string) {
  int i = 0;
  while (string[i] != '\0') {
    uart_putchar_polling(string[i]);
    i++;
  }
}
