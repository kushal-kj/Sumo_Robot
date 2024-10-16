#ifndef L298N_H
#define L298N_H

// Driver for motor driver L298N
#include <stdint.h>

typedef enum { L298N_LEFT, L298N_RIGHT } l298n_e;

typedef enum {
  L298N_MODE_STOP,
  L298N_MODE_FORWARD, // Clockwise(CC)
  L298N_MODE_REVERSE, // CounterClockwise(CCW)
} l298n_mode_e;

void l298n_init(void);
void l298n_set_mode(l298n_e l298n, l298n_mode_e mode);
void l298n_set_pwm(l298n_e l298n, uint8_t duty_cycle);

#endif // L298N_H
