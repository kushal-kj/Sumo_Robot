#include "common/assert_handler.h"
#include "common/defines.h"
#include "drivers/uart.h"
#include "external/printf/printf.h"
#include <msp430.h>

/* The TI compiler provides intrinsic support for calling a specific opcode,
 * which means you can write __op_code(0x4343) to triger a software breakpoint
 * (when launchpad FET degubber is attached). MSP430-GCC does not have this
 * intrinsic, but 0x4343 corresponds to assembly instruction "CLR.B R3". */

#define BREAKPOINT __asm volatile("CLR.B R3");

/* Since the max length os program counter is 6 i.e., 0xFFFF,
 * (Text + Program_counter + Null termination)
 */
#define ASSERT_STRING_MAX_SIZE (15u + 6u + 1U)

static void assert_trace(uint16_t program_counter) {
  // UART TX
  P3SEL |= BIT3;

  uart_init_assert();

  char assert_string[ASSERT_STRING_MAX_SIZE];
  snprintf(assert_string, sizeof(assert_string), "ASSERT 0x%x\n",
           program_counter);

  uart_trace_assert(assert_string);
}

static void assert_blink_led(void) {

  // Blink LED indefinetly when assertion is triggered.

  // Configure TEST_LED pin on launchpad
  P1SEL &= ~(BIT0);
  P1DIR |= (BIT0);
  P1REN &= ~(BIT0);

  while (1) {
    // Blink LED on target in case the wrong target was flashed
    P1OUT ^= BIT0;
    BUSY_WAIT_ms(250); // 250ms delay
  }
}

/*Minimize code dependency in this function to reduce the risk of accidently
 * calling a function with an assert in it, in which would cause the
 * assert_handler to be called recursively until statck overflow. */

void assert_handler(uint16_t program_counter) {
  // Turn off motors ("safe state")

  // Breakpoint
  BREAKPOINT

  // Trace assert to console
  assert_trace(program_counter);

  // Blink LED indefinetly when assertion is triggered.
  assert_blink_led();
}
