#ifndef IO_H
#define IO_H

#include <stdbool.h>

/* IO pins handling including pinmapping,initialization, and configuration.
 * This wraps the more crude register defines provided in the headers from
 * Texas instrument */

// TODO: Improve multiple HW targets handling

//#define LAUNCHPAD

// TODO: enums

typedef enum {
  IO_10,
  IO_11,
  IO_12,
  IO_13,
  IO_14,
  IO_15,
  IO_16,
  IO_17,
  IO_20,
  IO_21,
  IO_22,
  IO_23,
  IO_24,
  IO_25,
  IO_26,
  IO_27,
  IO_30,
  IO_31,
  IO_32,
  IO_33,
  IO_34,
  IO_35,
  IO_36,
  IO_37,
  IO_40,
  IO_41,
  IO_42,
  IO_43,
  IO_44,
  IO_45,
  IO_46,
  IO_47,

} io_generic_e;

//clang-format on

typedef enum {
#if defined(LAUNCHPAD)

  IO_TEST_LED = IO_10,
  IO_UNUSED_1 = IO_11,
  IO_UNUSED_2 = IO_12,
  IO_UNUSED_3 = IO_13,
  IO_PWM_MOTORS_LEFT = IO_14,
  IO_PWM_MOTORS_RIGHT = IO_15,
  IO_MOTORS_RIGHT_CC_1 = IO_16,
  IO_UNUSED_7 = IO_17,
  IO_IR_REMOTE = IO_20,
  IO_UNUSED_8 = IO_21,
  IO_UNUSED_9 = IO_22,
  IO_MOTORS_LEFT_CC_2 = IO_23,
  IO_UNUSED_11 = IO_24,
  IO_UNUSED_12 = IO_25,
  IO_UNUSED_13 = IO_26,
  IO_MOTORS_RIGHT_CC_2 = IO_27,
  IO_UART_RXD = IO_34,
  IO_UART_TXD = IO_33,
  IO_MOTORS_LEFT_CC_1 = IO_37,

#endif
} io_e;

typedef enum {
  IO_SELECT_GPIO,
  IO_SELECT_ALT1,
  // IO_SELECT_ALT2,
  // IO_SELECT_ALT3,
} io_select_e; // function selection registers

typedef enum {
  IO_DIR_INPUT,
  IO_DIR_OUTPUT,
} io_dir_e; // direction registers

typedef enum {
  IO_PUPD_DISABLED,
  IO_PUPD_ENABLED,
} io_pupd_e; // pull-up pull-down resistors enable registers

typedef enum {
  IO_OUT_LOW,  // pull-down
  IO_OUT_HIGH, // pull-up
} io_out_e;

typedef enum {
  IO_IN_LOW,
  IO_IN_HIGH,
} io_in_e; // input register

typedef enum {
  IO_TRIGGER_RISING,
  IO_TRIGGER_FALLING,
} io_trigger_e;

// TODO: structs

struct io_config {
  io_select_e select;
  io_pupd_e pupd_resistor;
  io_dir_e dir;
  io_out_e out;
};

// TODO: functions

void io_init(void);

void io_configure(io_e io, const struct io_config *config);

void io_get_current_config(io_e io, struct io_config *current_config);
bool io_config_compare(const struct io_config *cfg1,
                       const struct io_config *cfg2);

void io_set_select(io_e io, io_select_e select);
void io_set_direction(io_e io, io_dir_e direction);
void io_set_pupd(io_e io, io_pupd_e pupd_resistor);
void io_set_out(io_e io, io_out_e out);
io_in_e io_get_input(io_e io); // the input register function returns a value

// INTERRUPTS
typedef void (*isr_function)(void); // function pointer of type void
void io_configure_interrupt(io_e io, io_trigger_e trigger, isr_function isr);
void io_deconfigure_interrupt(io_e io);
void io_enable_interrupt(io_e io);
void io_disable_interrupt(io_e io);

#endif // IO_H
