#include "app/drive.h"
#include "common/assert_handler.h"
#include "common/defines.h"
#include "drivers/l298n_motordriver.h"
#include <assert.h>
#include <stdbool.h>

struct drive_speeds {
  uint8_t left;
  uint8_t right;
};

/*Drive directions come in pair (e.g FORRWARD and REVERSE, ROTATE_LEFT and
 * ROTATE_RIGHT). To save flash space and minimize typos, only save the speeds
 * for one direction(primary), and create a macro to get the corresponding
 * primary direction for every direction
 * and inverse the speeds when its not the primary direction. */

#define DRIVE_PRIMARY_DIRECTION(dir) (dir - MODULO_2(dir))
static_assert(DRIVE_PRIMARY_DIRECTION(DRIVE_DIR_REVERSE) == DRIVE_DIR_FORWARD);
static const struct drive_speeds drive_primary_speeds[][4] = {
    [DRIVE_DIR_FORWARD] =
        {
            [DRIVE_SPEED_SLOW] = {25, 25},
            [DRIVE_SPEED_MEDIUM] = {45, 45},
            [DRIVE_SPEED_FAST] = {55, 55},
            [DRIVE_SPEED_MAX] = {100, 100},
        },
    [DRIVE_DIR_REVERSE] =
        {
            [DRIVE_SPEED_SLOW] = {-25, 25},
            [DRIVE_SPEED_MEDIUM] = {-45, 45},
            [DRIVE_SPEED_FAST] = {-55, 55},
            [DRIVE_SPEED_MAX] = {-100, 100},
        },
    [DRIVE_DIR_ROTATE_LEFT] =
        {
            [DRIVE_SPEED_SLOW] = {-25, 25},
            [DRIVE_SPEED_MEDIUM] = {-50, 50},
            [DRIVE_SPEED_FAST] = {-60, 60},
            [DRIVE_SPEED_MAX] = {-100, 100},
        },

    [DRIVE_DIR_ARCTURN_SHARP_LEFT] =
        {
            [DRIVE_SPEED_SLOW] = {-10, 25},
            [DRIVE_SPEED_MEDIUM] = {-10, 50},
            [DRIVE_SPEED_FAST] = {-25, 75},
            [DRIVE_SPEED_MAX] = {-20, 100},
        },
    [DRIVE_DIR_ARCTURN_MID_LEFT] =
        {
            [DRIVE_SPEED_SLOW] = {15, 30},
            [DRIVE_SPEED_MEDIUM] = {25, 50},
            [DRIVE_SPEED_FAST] = {35, 70},
            [DRIVE_SPEED_MAX] = {50, 100},
        },
    [DRIVE_DIR_ARCTURN_WIDE_LEFT] =
        {
            [DRIVE_SPEED_SLOW] = {20, 25},
            [DRIVE_SPEED_MEDIUM] = {40, 50},
            [DRIVE_SPEED_FAST] = {60, 70},
            [DRIVE_SPEED_MAX] = {85, 100},
        },

};

static void drive_inverse_speeds(int8_t *speed_left, int8_t *speed_right) {
  if (*speed_left == *speed_right) {
    *speed_left = -*speed_left;
    *speed_right = -*speed_right;
  } else {
    // swap
    const uint8_t temp = *speed_right;
    *speed_right = *speed_left;
    *speed_left = temp;
  }
}

void drive_set(drive_dir_e direction, drive_speed_e speed) {
  drive_dir_e primary_direction = DRIVE_PRIMARY_DIRECTION(direction);
  int8_t speed_left = drive_primary_speeds[primary_direction][speed].left;
  int8_t speed_right = drive_primary_speeds[primary_direction][speed].right;

  if (direction != primary_direction) {
    drive_inverse_speeds(&speed_left, &speed_right);
  }
  ASSERT(speed_left != 0 && speed_right != 0);
  const l298n_mode_e mode_left =
      speed_left > 0 ? L298N_MODE_FORWARD : L298N_MODE_REVERSE;
  l298n_set_mode(L298N_LEFT, mode_left);
  const l298n_mode_e mode_right =
      speed_right > 0 ? L298N_MODE_FORWARD : L298N_MODE_REVERSE;
  l298n_set_mode(L298N_RIGHT, mode_right);
  l298n_set_pwm(L298N_LEFT, ABS(speed_left));
  l298n_set_pwm(L298N_RIGHT, ABS(speed_right));
}

void drive_stop(void) {
  l298n_set_mode(L298N_LEFT, L298N_MODE_STOP);
  l298n_set_mode(L298N_RIGHT, L298N_MODE_STOP);
  l298n_set_pwm(L298N_LEFT, 0);
  l298n_set_pwm(L298N_RIGHT, 0);
}

static bool initialized = false;
void drive_init(void) {
  ASSERT(!initialized);
  l298n_init();
}
