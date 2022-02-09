#ifndef __IR_TX_HANDLER_H
#define __IR_TX_HANDLER_H

typedef enum
{
    IR_TASK_IDLE,
    IR_TASK_WORK,
} ir_task_state_e;


void ir_tx_loop_task(void);
int ir_tx_task_state_set(ir_task_state_e task_state, uint8_t *p_data, uint8_t len);
#endif
