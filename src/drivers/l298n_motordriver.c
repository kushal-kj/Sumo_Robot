#include "drivers/l298n_motordriver.h"
#include "common/assert_handler.h"
#include "common/defines.h"
#include "drivers/io.h"
#include "drivers/pwm.h"
#include <assert.h>

struct cc_pins {
  io_e cc1;
  io_e cc2;
};

static struct cc_pins l298n_cc_pins[] = {
    [L298N_LEFT] = {IO_MOTORS_LEFT_CC_1, IO_MOTORS_LEFT_CC_2},
#if defined(LAUNCHPAD)
    [L298N_RIGHT] = {IO_MOTORS_LEFT_CC_1, IO_MOTORS_LEFT_CC_2},

//[L298N_RIGHT] {IO_MOTORS_RIGHT_CC_1, IO_MOTORS_RIGHT_CC_2},
#endif
};

void l298n_set_mode(l298n_e l298n, l298n_mode_e mode) {
  switch (mode) {
  case L298N_MODE_STOP:
    io_set_out(l298n_cc_pins[l298n].cc1, IO_OUT_LOW);
    io_set_out(l298n_cc_pins[l298n].cc2, IO_OUT_LOW);
    break;

  case L298N_MODE_FORWARD:
    io_set_out(l298n_cc_pins[l298n].cc1, IO_OUT_HIGH);
    io_set_out(l298n_cc_pins[l298n].cc2, IO_OUT_LOW);
    break;

  case L298N_MODE_REVERSE:
    io_set_out(l298n_cc_pins[l298n].cc1, IO_OUT_LOW);
    io_set_out(l298n_cc_pins[l298n].cc2, IO_OUT_HIGH);
    break;
  }
}

static_assert(L298N_LEFT == (int)PWM_L298N_LEFT, "Enum mismatch");
static_assert(L298N_RIGHT == (int)PWM_L298N_RIGHT, "Enum mismatch");

void l298n_set_pwm(l298n_e l298n, uint8_t duty_cycle) {
  pwm_set_duty_cycle((pwm_e)l298n, duty_cycle);
}

static void l298n_assert_io_config(void) {
  static const struct io_config cc_io_config = {
      .select = IO_SELECT_GPIO,
      .pupd_resistor = IO_PUPD_DISABLED,
      .dir = IO_DIR_OUTPUT,
      .out = IO_OUT_LOW,
  };
  struct io_config current_config;
  io_get_current_config(IO_MOTORS_LEFT_CC_1, &current_config);
  ASSERT(io_config_compare(&current_config, &cc_io_config));
  io_get_current_config(IO_MOTORS_LEFT_CC_2, &current_config);
  ASSERT(io_config_compare(&current_config, &cc_io_config));
  /*
  io_get_current_config(IO_MOTORS_RIGHT_CC_1, &current_config);
  ASSERT(io_config_compare(&current_config, &cc_io_config));
  io_get_current_config(IO_MOTORS_RIGHT_CC_2, &current_config);
  ASSERT(io_config_compare(&current_config, &cc_io_config));
  */
}

static bool initialized = false;
void l298n_init(void) {
  ASSERT(!initialized);
  l298n_assert_io_config();
  pwm_init();
  initialized = true;
}
