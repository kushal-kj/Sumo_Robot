#include "common/assert_handler.h"
#include "common/defines.h"
#include "drivers/io.h"
#include "drivers/led.h"
#include "drivers/mcu_init.h"
#include <msp430.h>

static void test_setup(void) { mcu_init(); }

/*
static void test_assert(void) {
  test_setup();
  ASSERT(0);
}
*/

static void test_blink_led(void) {
  test_setup();
  led_init();
  // led_init(); // To check whether the assert_handler triggers or not

  led_state_e led_state = LED_STATE_OFF;

  while (1) {
    led_state = (led_state == LED_STATE_OFF) ? LED_STATE_ON : LED_STATE_OFF;
    led_set(LED_TEST, led_state);
    BUSY_WAIT_ms(1000); // 1 sec delay
  }
}

int main(void) {
  //  WDTCTL = WDTPW + WDTHOLD;
  test_blink_led();
  // test_assert();
  return 0;
}
