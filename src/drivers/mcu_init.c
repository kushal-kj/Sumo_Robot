#include "drivers/mcu_init.h"
#include "common/assert_handler.h"
#include "drivers/io.h"
#include <msp430.h>

/* static void init_clocks() { */
/*     /1* The MSP430F5529 uses the UCS (Unified Clock System), so */
/*      * we'll configure the DCO to 16 MHz using UCS registers. */
/*      * The DCO (Digitally Controlled Oscillator) is a crucial part */
/*      * of the clock system that provides the main clock signal (MCLK) */
/*      * for the CPU and peripheral subsystems (SMCLK). */
/*      *1/ */

/*     // Set DCO FLL reference to REFO (32.768 kHz crystal) */
/*     // SELREF_2 selects REFOCLK as the FLL reference clock source. */
/*     UCSCTL3 = SELREF_2; */

/*     // Set DCO to 16 MHz */
/*     // DCORSEL_5 sets the DCO range for the desired frequency (16 MHz). */
/*     UCSCTL1 = DCORSEL_5; */

/*     // Set DCO Multiplier for 16MHz */
/*     // FLLD_1 specifies the FLL loop divider, and 488 is the multiplier. */
/*     // The formula used is: (N + 1) * FLLRef = Fdco */
/*     // (488 + 1) * 32768 = 16MHz */
/*     UCSCTL2 = FLLD_1 + 488; */

/*     // Disable the FLL control loop before configuring the DCO */
/*     // SCG0 is a status register bit that controls the FLL. */
/*     // Disabling the FLL (by setting SCG0) prevents it from adjusting the DCO
 */
/*     // frequency during configuration. */
/*     __bis_SR_register(SCG0); */

/*     // Enable the FLL control loop to lock the DCO frequency */
/*     // After configuration, we clear SCG0 to re-enable the FLL. */
/*     // The FLL will now lock the DCO at the desired frequency. */
/*     __bic_SR_register(SCG0); */

/*     // Delay for DCO to settle */
/*     // We delay for 782000 cycles to allow the DCO to stabilize at 16 MHz. */
/*     // This delay is crucial to ensure the clock system is stable before use.
 */
/*     __delay_cycles(782000); */

/*     // Loop until XT1, XT2 & DCO fault flags are cleared */
/*     // UCSCTL7 holds the fault flags for the clock sources. */
/*     // XT2OFFG, XT1LFOFFG, and DCOFFG are flags indicating faults in XT2,
 * XT1, */
/*     // and DCO, respectively. */
/*     // SFRIFG1 holds the global oscillator fault flag (OFIFG). */
/*     // We clear these flags in a loop until all faults are resolved. */
/*     do { */
/*         UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG); */
/*         SFRIFG1 &= ~OFIFG;            // Clear fault flags */
/*     } while (SFRIFG1 & OFIFG);        // Test oscillator fault flag */

/*     // Additional stabilization check to ensure DCO is running at 16 MHz */
/*     // The ASSERT macro checks if the DCO settings are correct: */
/*     // UCSCTL1 should equal DCORSEL_5, and UCSCTL2 should equal 488. */
/*     // If either condition is false, the ASSERT will trigger an error. */
/*     ASSERT((UCSCTL1 == DCORSEL_5) && ((UCSCTL2 & 0x03FF) == 488)); */
/* } */

/* Watchdog is enabled by default and will reset the microcontroller
 * repeatedly if not explicitly stopped. */

static void watchdog_stop(void) { WDTCTL = WDTPW + WDTHOLD; }

void mcu_init(void) {
  // Must stop the watchdog first before anything else
  watchdog_stop();

  // Initializes clock of 16MHZ
  // init_clocks();

  // Initializes Input/Output
  io_init();

  // Enables the Interrupt globally
  _enable_interrupts();
}
