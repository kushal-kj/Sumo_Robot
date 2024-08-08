#include "common/assert_handler.h"
#include "common/defines.h"
#include <msp430.h>

/* The TI compiler provides intrinsic support for calling a specific opcode,
 * which means you can write __op_code(0x4343) to triger a software breakpoint
 * (when launchpad FET degubber is attached). MSP430-GCC does not have this
 * intrinsic, but 0x4343 corresponds to assembly instruction "CLR.B R3". */

#define BREAKPOINT __asm volatile("CLR.B R3");

/*Minimize code dependency in this function to reduce the risk of accidently
 * calling a function with an assert in it, in which would cause the
 * assert_handler to be called recursively until statck overflow. */

void assert_handler(void) {
  // Turn off motors ("safe state")

  // Trace to console

  // Breakpoint

  BREAKPOINT

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
