#ifndef PWM_H
#define PWM_H

#include <stdint.h>

// Driver that emulates hardware PWM with Timer peripheral

typedef enum {
  PWM_L298N_LEFT,
  PWM_L298N_RIGHT,
} pwm_e;

void pwm_init(void);

void pwm_set_duty_cycle(pwm_e pwm, uint8_t duty_cycle_percent);

#endif // PWM_H
