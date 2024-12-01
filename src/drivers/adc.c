#include "drivers/adc.h"
#include "common/assert_handler.h"
#include "common/defines.h"
#include "drivers/io.h"
#include <msp430.h>
#include <stdbool.h>

static volatile uint16_t adc_results[4]; // Array for ADC results
static volatile uint16_t adc_cache[4];   // Cache for the last ADC results
static const io_e *adc_pins;             // Array to hold ADC pin configuration
static uint8_t adc_pin_count;            // Count of ADC pins
static uint8_t adc_channel_count;        // Total number of ADC channels

static bool initialized = false; // Initialization flag

void adc_init(void) {
  ASSERT(!initialized); // Ensure ADC is not already initialized
  adc_pins = io_adc_pins(&adc_pin_count); // Get ADC pin configurations
  adc_channel_count =
      adc_pin_count; // Set total channel count based on available pins

  // Enable A/D channel inputs for P6.0 - P6.3
  P6SEL |= 0x0F;

  // Configure DMA for ADC results
  DMACTL0 = DMA0TSEL_30;           // Select ADC12 as DMA trigger
  DMA0SA = (uint16_t)&ADC12MEM0;   // Source address (ADC memory)
  DMA0DA = (uint16_t)&adc_results; // Destination address (adc_results array)
  DMA0SZ = adc_channel_count;      // Number of transfers
  DMA0CTL |= DMADT_4 | DMASRCINCR_0 | DMADSTINCR_3 | DMAEN;

  // Configure ADC
  ADC12CTL0 =
      ADC12ON + ADC12MSC + ADC12SHT0_2; // Turn on ADC12, set sampling time
  ADC12CTL1 = ADC12SHP + ADC12CONSEQ_1; // Use sampling timer, single sequence
  ADC12MCTL0 = ADC12INCH_0;             // Channel = A0
  ADC12MCTL1 = ADC12INCH_1;             // Channel = A1
  ADC12MCTL2 = ADC12INCH_2;             // Channel = A2
  ADC12MCTL3 = ADC12INCH_3 + ADC12EOS;  // Channel = A3, end sequence

  ADC12IE = 0x08; // ADC12IE3         // Enable interrupt for last channel (A3)
  ADC12CTL0 |= ADC12ENC; // Enable conversions

  initialized = true; // Set initialized flag
}

// ADC12 Interrupt Service Routine
INTERRUPT_FUNCTION(ADC12_VECTOR) ADC12ISR(void) {
  switch (__even_in_range(ADC12IV, 34)) {
  case 0:
    break; // No interrupt
  case 14: // ADC12IFG3: End of sequence
    // Cache ADC results
    for (uint8_t i = 0; i < adc_channel_count; i++) {
      adc_cache[i] = adc_results[i];
    }
    __bic_SR_register_on_exit(LPM4_bits); // Exit low power mode
    break;
  default:
    break;
  }
}

// Function to start ADC conversion
static inline void adc_enable_and_start_conversion(void) {
  ADC12CTL0 |= ADC12SC; // Start conversion
}

// Function to get channel values
void adc_get_channel_values(adc_channel_values_t values) {
  _disable_interrupts(); // Disable interrupts globally
  for (uint8_t i = 0; i < adc_pin_count; i++) {
    values[i] = adc_cache[i]; // Copy cached values to output
  }
  _enable_interrupts(); // Enable interrupts
}
