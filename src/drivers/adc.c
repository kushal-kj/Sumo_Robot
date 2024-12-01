#include "drivers/adc.h"
#include "drivers/io.h"
#include "common/defines.h"
#include "common/assert_handler.h"
#include <msp430.h>
#include <stdbool.h>

#define Num_of_Results   8

// Array for ADC results, corresponding to 4 ADC channels
volatile unsigned int A0results[Num_of_Results];
volatile unsigned int A1results[Num_of_Results];
volatile unsigned int A2results[Num_of_Results];
volatile unsigned int A3results[Num_of_Results];

static volatile uint16_t adc_results[4]; // Array to store results of ADC channels A0, A1, A2, A3
static volatile uint16_t adc_cache[4];   // Cache for storing last ADC conversion results
static const io_e *adc_pins;             // Pointer to hold ADC pin configurations
static uint8_t adc_pin_count;            // Number of ADC pins available
static uint8_t adc_channel_count;        // Total number of ADC channels (based on available pins)

static bool initialized = false; // Flag for initialization

// Function to start ADC conversion
static inline void adc_enable_and_start_conversion(void)
{
    ADC12CTL0 |= ADC12ENC + ADC12SC;  // Enable conversion and start it
}

void adc_init(void)
{
    ASSERT(!initialized); // Ensure ADC is not already initialized

    adc_pins = io_adc_pins(&adc_pin_count); // Get ADC pin configurations
    adc_channel_count = adc_pin_count;      // Set total number of channels to the number of available pins

    // Enable A/D channel inputs for P6.0 - P6.3
    P6SEL |= 0x0F;

    // Configure ADC
    ADC12CTL0 = ADC12ON + ADC12MSC + ADC12SHT0_8;  // Turn on ADC12, set extended sampling time
    ADC12CTL1 = ADC12SHP+ADC12CONSEQ_3+ADC12SSEL1;          // Use sampling timer, repeated sequence mode
    ADC12MCTL0 = ADC12INCH_0;                      // Channel = A0
    ADC12MCTL1 = ADC12INCH_1;                      // Channel = A1
    ADC12MCTL2 = ADC12INCH_2;                      // Channel = A2
    ADC12MCTL3 = ADC12INCH_3 + ADC12EOS;           // Channel = A3, end of sequence

    ADC12IE = 0x08;                                // Enable interrupt for last channel (A3)
    ADC12CTL0 |= ADC12ENC;                         // Enable ADC conversion
    adc_enable_and_start_conversion();             // Start ADC conversion

    initialized = true; // Set initialization flag
}

// ADC12 Interrupt Service Routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC12_VECTOR
__interrupt void ADC12ISR(void)
#elif defined(__GNUC__)
void __attribute__((interrupt(ADC12_VECTOR))) ADC12ISR(void)
#else
#error Compiler not supported!
#endif
{
    //static uint8_t index = 0;

    switch (__even_in_range(ADC12IV, 34))
    {
        case 0: break;  // No interrupt
        case 2: break;  // ADC overflow
        case 4: break;  // ADC timing overflow
        case 6: break;  // ADC12IFG0
        case 8: break;  // ADC12IFG1
        case 10: break; // ADC12IFG2
        case 12: // ADC12IFG3 (End of sequence)
           /* A0results[index] = ADC12MEM0;  // Store A0 result
            A1results[index] = ADC12MEM1;  // Store A1 result
            A2results[index] = ADC12MEM2;  // Store A2 result
            A3results[index] = ADC12MEM3;  // Store A3 result
            */
			adc_results[0] = ADC12MEM0;    // Update adc_results for A0
            adc_results[1] = ADC12MEM1;    // Update adc_results for A1
            adc_results[2] = ADC12MEM2;    // Update adc_results for A2
            adc_results[3] = ADC12MEM3;    // Update adc_results for A3

            // Cache ADC results into adc_cache
            for (uint8_t i = 0; i < adc_channel_count; i++)
            {
                adc_cache[i] = adc_results[i];
            }

            // Increment the index and reset if necessary
            /*
			index++;
            if (index == Num_of_Results)
            {
                index = 0;
            }
*/
            // Start the next conversion
            adc_enable_and_start_conversion();

            __bic_SR_register_on_exit(LPM4_bits); // Exit low power mode if used
            break;
        case 14: break; // ADC12IFG4
        case 16: break; // ADC12IFG5
        case 18: break; // ADC12IFG6
        case 20: break; // ADC12IFG7
        case 22: break; // ADC12IFG8
        case 24: break; // ADC12IFG9
        case 26: break; // ADC12IFG10
        case 28: break; // ADC12IFG11
        case 30: break; // ADC12IFG12
        case 32: break; // ADC12IFG13
        case 34: break; // ADC12IFG14
        default: break;
    }
}

// Function to get the ADC channel values
void adc_get_channel_values(adc_channel_values_t values)
{
    _disable_interrupts();  // Disable interrupts globally

    // Copy the cached values to the provided array
    for (uint8_t i = 0; i < adc_pin_count; i++)
    {
        values[i] = adc_cache[i];
    }

    _enable_interrupts();  // Re-enable interrupts
}

