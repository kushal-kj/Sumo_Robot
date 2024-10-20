#include "drivers/io.h"
#include "common/assert_handler.h"
#include "common/defines.h"

#include <assert.h>
#include <msp430.h>
#include <stddef.h>
#include <stdint.h>

#if defined(LAUNCHPAD)

#define IO_PORT_CNT (4u)
#endif

#define IO_PIN_CNT_PER_PORT (8u)

#define IO_INTERRUPT_PORT_CNT (3u)

/* To extract port and pin bit from enum io_generic_e (io_e).
 * With complier flag "-fshort-enums", the enums are represented
 * as a single byte(8-bit), so given that the pins are ordered
 * in increasing order(see io_generic_e), and that there are
 * 2 ports and 8 pins, the enum value can be viewed as:
 * [Zeros (3-bits) | Port (2-bits) | Pin(3-bits)] */

static_assert(sizeof(io_generic_e) == 1,
              "Unexpected size, -fshort-enums missing?");

#define IO_PORT_OFFSET (3u)
#define IO_PORT_MASK (0x3u << IO_PORT_OFFSET)
#define IO_PIN_MASK (0x7u)

static inline uint8_t
io_port(io_e io) // They optimize better as inline functions instead of macros
{
  return (io & IO_PORT_MASK) >> IO_PORT_OFFSET;
}

static inline uint8_t io_pin_idx(io_e io) { return io & IO_PIN_MASK; }

static inline uint8_t io_pin_bit(io_e io) { return 1 << io_pin_idx(io); }

typedef enum { IO_PORT1, IO_PORT2, IO_PORT3 } io_port_e;

/* TI's helper header (msp430.h) provides defines/variables for accessing the
 * registers, and the address of these are resolved during linking. For cleaner
 * code, smaller executable, and to avoid mapping between IO_PORT-enum and these
 * variables using if/switch-statements, store the addresses in arrays and
 * access them through array indexing. */

#if defined(LAUNCHPAD)
static volatile uint8_t *const port_dir_regs[IO_PORT_CNT] = {&P1DIR, &P2DIR,
                                                             &P3DIR};
static volatile uint8_t *const port_ren_regs[IO_PORT_CNT] = {&P1REN, &P2REN,
                                                             &P3REN};
static volatile uint8_t *const port_out_regs[IO_PORT_CNT] = {&P1OUT, &P2OUT,
                                                             &P3OUT};
static volatile uint8_t *const port_in_regs[IO_PORT_CNT] = {&P1IN, &P2IN,
                                                            &P3IN};
static volatile uint8_t *const port_sel1_regs[IO_PORT_CNT] = {&P1SEL, &P2SEL,
                                                              &P3SEL};
// static volatile uint8_t *const port_sel2_regs[IO_PORT_CNT] = {&P1SEL2,
// &P2SEL2};
#endif

// INTERRUPTS
static volatile uint8_t *const port_interrupt_flag_regs[IO_INTERRUPT_PORT_CNT] =
    {&P1IFG, &P2IFG};
static volatile uint8_t
    *const port_interrupt_enable_regs[IO_INTERRUPT_PORT_CNT] = {&P1IE, &P2IE};
static volatile uint8_t *const
    port_interrupt_edge_select_register[IO_INTERRUPT_PORT_CNT] = {&P1IES,
                                                                  &P2IES};

// CREATING A INDIVIDUAL TABLE TO HOLD INTERRUPTS
static isr_function isr_functions[IO_INTERRUPT_PORT_CNT][IO_PIN_CNT_PER_PORT] =
    {
        [IO_PORT1] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
        [IO_PORT2] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
};

#define UNUSED_CONFIG                                                          \
  { IO_SELECT_GPIO, IO_PUPD_ENABLED, IO_DIR_OUTPUT, IO_OUT_LOW }

/* This array holds the initial configuration of all IO pins/
 */
static const struct io_config
    io_initial_configs[IO_PORT_CNT * IO_PIN_CNT_PER_PORT] = {

        // Output
        [IO_TEST_LED] = {IO_SELECT_GPIO, IO_PUPD_DISABLED, IO_DIR_OUTPUT,
                         IO_OUT_LOW},

        /* UART TX/RX
         * Resistor: not needed (pulled by transmitter / receiver)
         * Direction: not applicable
         * Output: not applicable */
        [IO_UART_RXD] = {IO_SELECT_ALT1, IO_PUPD_DISABLED, IO_DIR_OUTPUT,
                         IO_OUT_LOW},
        [IO_UART_TXD] = {IO_SELECT_ALT1, IO_PUPD_DISABLED, IO_DIR_OUTPUT,
                         IO_OUT_LOW},

        /* Input (no resitor required according to data sheet of IR receiver)
         */
        [IO_IR_REMOTE] = {IO_SELECT_GPIO, IO_PUPD_DISABLED, IO_DIR_INPUT,
                          IO_OUT_LOW},

        // Output drivern by Timer A0, direction must be set to output
        [IO_PWM_MOTORS_LEFT] = {IO_SELECT_ALT1, IO_PUPD_DISABLED, IO_DIR_OUTPUT,
                                IO_OUT_LOW},

        // Output driven by Timer A0, direction must be set to output
        //[IO_PWM_MOTORS_RIGHT] = {IO_SELECT_ALT1, IO_PUPD_DISABLED,
        // IO_DIR_OUTPUT, IO_OUT_LOW},

        // Output
        [IO_MOTORS_LEFT_CC_1] = {IO_SELECT_GPIO, IO_PUPD_DISABLED,
                                 IO_DIR_OUTPUT, IO_OUT_LOW},
        [IO_MOTORS_LEFT_CC_2] = {IO_SELECT_GPIO, IO_PUPD_DISABLED,
                                 IO_DIR_OUTPUT, IO_OUT_LOW},
/*
[IO_MOTORS_RIGHT_CC_1] = {IO_SELECT_GPIO, IO_PUPD_DISABLED, IO_DIR_OUTPUT,
IO_OUT_LOW}, [IO_MOTORS_RIGHT_CC_2] = {IO_SELECT_GPIO, IO_PUPD_DISABLED,
IO_DIR_OUTPUT, IO_OUT_LOW},
*/

#if defined(LAUNCHPAD)
        // Unused pins
        [IO_UNUSED_1] = UNUSED_CONFIG,
        [IO_UNUSED_2] = UNUSED_CONFIG,
        [IO_UNUSED_3] = UNUSED_CONFIG,
        [IO_UNUSED_7] = UNUSED_CONFIG,
        [IO_UNUSED_8] = UNUSED_CONFIG,
        [IO_UNUSED_9] = UNUSED_CONFIG,
        [IO_UNUSED_11] = UNUSED_CONFIG,
        [IO_UNUSED_12] = UNUSED_CONFIG,
        [IO_UNUSED_13] = UNUSED_CONFIG,

#endif
};

void io_init(void) {
  // Initialize all pins
  for (io_e io = (io_e)IO_10; io < ARRAY_SIZE(io_initial_configs); io++) {
    io_configure(io, &io_initial_configs[io]);
  }
}

void io_configure(io_e io, const struct io_config *config) {
  io_set_select(io, config->select);
  io_set_direction(io, config->dir);
  io_set_out(io, config->out);
  io_set_pupd(io, config->pupd_resistor);
}

void io_get_current_config(io_e io, struct io_config *current_config) {
  const uint8_t port = io_port(io);
  const uint8_t pin = io_pin_bit(io);
  const uint8_t sel1 = (*port_sel1_regs[port] & pin) ? 1 : 0;
  current_config->select = (io_select_e)(sel1);
  current_config->pupd_resistor =
      (*port_ren_regs[port] & pin) ? IO_PUPD_ENABLED : IO_PUPD_DISABLED;
  current_config->dir =
      (*port_dir_regs[port] & pin) ? IO_DIR_OUTPUT : IO_DIR_INPUT;
  current_config->out = (*port_out_regs[port] & pin) ? IO_OUT_HIGH : IO_OUT_LOW;
}

bool io_config_compare(const struct io_config *cfg1,
                       const struct io_config *cfg2) {
  return (cfg1->dir == cfg2->dir) && (cfg1->out == cfg2->out) &&
         (cfg1->pupd_resistor == cfg2->pupd_resistor) &&
         (cfg1->select == cfg2->select);
}

void io_set_select(io_e io, io_select_e select) {
  const uint8_t port = io_port(io);
  const uint8_t pin = io_pin_bit(io);
  switch (select) {
  case IO_SELECT_GPIO:
    *port_sel1_regs[port] &= ~pin; // 0
    //*port_sel2_regs[port] &= ~pin;
    break;

  case IO_SELECT_ALT1:
    *port_sel1_regs[port] |= pin; // 1
    //*port_sel2_regs[port] &= ~pin;
    break;
    /*
              case IO_SELECT_ALT2:
                      *port_sel1_regs[port] &= ~pin;		//2
                      *port_sel2_regs[port] |= ~pin;
                      break;

              case IO_SELECT_ALT3:
                      *port_sel1_regs[port] |= pin;		//3
                      *port_sel2_regs[port] |= pin;
                      break;

    */
  }
}

void io_set_direction(io_e io, io_dir_e direction) {
  uint8_t port = io_port(io);
  uint8_t pin = io_pin_bit(io);
  switch (direction) {
  case IO_DIR_INPUT:
    *port_dir_regs[port] &= ~pin;
    break;

  case IO_DIR_OUTPUT:
    *port_dir_regs[port] |= pin;
    break;
  }
}

void io_set_pupd(io_e io, io_pupd_e pupd_resistor) {
  uint8_t port = io_port(io);
  uint8_t pin = io_pin_bit(io);
  switch (pupd_resistor) {
  case IO_PUPD_DISABLED:
    *port_ren_regs[port] &= ~pin;
    break;

  case IO_PUPD_ENABLED:
    *port_ren_regs[port] |= pin;
    break;
  }
}

void io_set_out(io_e io, io_out_e out) {
  uint8_t port = io_port(io);
  uint8_t pin = io_pin_bit(io);
  switch (out) {
  case IO_OUT_LOW:
    *port_out_regs[port] &= ~pin;
    break;

  case IO_OUT_HIGH:
    *port_out_regs[port] |= pin;
    break;
  }
}

io_in_e io_get_input(io_e io) {
  return (*port_in_regs[io_port(io)] & io_pin_bit(io)) ? IO_IN_HIGH : IO_IN_LOW;
}

// INTERRUPTS

// Corresponding port interrupt will be disabled
static void io_clear_interrupt(io_e io) {
  *port_interrupt_flag_regs[io_port(io)] &= ~io_pin_bit(io);
}

/*This function also disables the interrupt because selecting the
 * edge might trigger one according to the datasheet */
static void io_set_interrupt_trigger(io_e io, io_trigger_e trigger) {
  const uint8_t port = io_port(io);
  const uint8_t pin = io_pin_bit(io);

  io_disable_interrupt(io);

  switch (trigger) {
  case IO_TRIGGER_RISING:
    *port_interrupt_edge_select_register[port] |= pin;
    break;

  case IO_TRIGGER_FALLING:
    *port_interrupt_edge_select_register[port] &= ~pin;
    break;
  }
  /* Also clear the interrupt here, because even if interrupt
   * is disabled, the flag is still set */
  io_clear_interrupt(io);
}

static void io_register_isr(io_e io, isr_function isr) {
  const uint8_t port = io_port(io);
  const uint8_t pin_idx = io_pin_idx(io);
  ASSERT(isr_functions[port][pin_idx] == NULL);
  isr_functions[port][pin_idx] = isr;
}

void io_configure_interrupt(io_e io, io_trigger_e trigger, isr_function isr) {
  io_set_interrupt_trigger(io, trigger);
  io_register_isr(io, isr);
}

static inline void io_unregister_isr(io_e io) {
  const uint8_t port = io_port(io);
  const uint8_t pin_idx = io_pin_idx(io);
  isr_functions[port][pin_idx] = NULL;
}

void io_deconfigure_interrupt(io_e io) {
  io_unregister_isr(io);
  io_disable_interrupt(io);
}

void io_enable_interrupt(io_e io) {
  *port_interrupt_enable_regs[io_port(io)] |= io_pin_bit(io);
}

void io_disable_interrupt(io_e io) {
  *port_interrupt_enable_regs[io_port(io)] &= ~io_pin_bit(io);
}

static void io_isr(io_e io) {
  const uint8_t port = io_port(io);
  const uint8_t pin = io_pin_bit(io);
  const uint8_t pin_idx = io_pin_idx(io);

  if (*port_interrupt_flag_regs[port] &
      pin) // checks if the interrupt flag is enabled or not on that particular
           // port and pin
  {
    if (isr_functions[port][pin_idx] !=
        NULL) // If it is not NULL, then it means there is a isr_function
              // register to this interrupt
    {
      isr_functions[port][pin_idx]();
    }

    // Must explicitly clear interrupt in software
    io_clear_interrupt(io);
  }
}

INTERRUPT_FUNCTION(PORT1_VECTOR) isr_port_1(void) {
  for (io_generic_e io = IO_10; io <= IO_17; io++) {
    io_isr(io);
  }
}

INTERRUPT_FUNCTION(PORT2_VECTOR) isr_port_2(void) {
  for (io_generic_e io = IO_20; io <= IO_27; io++) {
    io_isr(io);
  }
}
