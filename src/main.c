#include "drivers/io.h"
#include "drivers/mcu_init.h"
#include <msp430.h>

static void test_setup(void) { mcu_init(); }

static void test_blink_led(void) {
  test_setup();

  const struct io_config led_config = {.dir = IO_DIR_OUTPUT,
                                       .select = IO_SELECT_GPIO,
                                       .pupd_resistor = IO_PUPD_DISABLED,
                                       .out = IO_OUT_LOW};

  io_configure(IO_TEST_LED, &led_config);
  io_out_e out = IO_OUT_LOW;
  while (1) {
    out = (out == IO_OUT_LOW) ? IO_OUT_HIGH : IO_OUT_LOW;
    io_set_out(IO_TEST_LED, out);
    __delay_cycles(250000); // 250ms
  }
}

int main(void) {
  //  WDTCTL = WDTPW + WDTHOLD;
  test_blink_led();
  return 0;
}
