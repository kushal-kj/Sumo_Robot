/*
 * ir_remote.h
 *
 *  Created on: 06-Sep-2024
 *      Author: kj
 */

#ifndef IR_REMOTE_H_
#define IR_REMOTE_H_

// A driver that decodes the commands sent to the IR receiver (NEC protocol)

typedef enum {
  IR_CMD_0,
  IR_CMD_1,
  IR_CMD_2,
  IR_CMD_3,
  IR_CMD_4,
  IR_CMD_5,
  IR_CMD_6,
  IR_CMD_7,
  IR_CMD_8,
  IR_CMD_9,
  IR_CMD_STAR,
  IR_CMD_HASH,
  IR_CMD_UP,
  IR_CMD_DOWN,
  IR_CMD_LEFT,
  IR_CMD_RIGHT,
  IR_CMD_OK,
  IR_CMD_NONE,

} ir_cmd_e;

void ir_remote_init(void);
ir_cmd_e ir_remote_get_cmd(void);

const char *ir_remote_cmd_to_string(ir_cmd_e cmd);

#endif /* IR_REMOTE_H_ */
