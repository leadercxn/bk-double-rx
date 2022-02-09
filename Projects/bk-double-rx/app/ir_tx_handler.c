#include "stdio.h"
#include "string.h"
#include "board_config.h"
#include "ir_tx_handler.h"

#define IR_TX_INTERVAL_TIME 1000        //uint ms
#define IR_TX_LEN_MAX       16

static uint8_t          m_tx_data[IR_TX_LEN_MAX] = {0};
static uint8_t          m_tx_data_len = 0;
static ir_task_state_e  m_task_state = IR_TASK_IDLE;

int ir_tx_task_state_set(ir_task_state_e task_state, uint8_t *p_data, uint8_t len)
{
    m_task_state = task_state;

    if(len > IR_TX_LEN_MAX)
    {
        trace_error("ERROR: ir tx data so long\n\r");

        return -EINVAL;
    }

    if((p_data) && (len > 0))
    {
        memcpy(m_tx_data, p_data, len);
        m_tx_data_len = len;
    }

    return 0;
}

void ir_tx_loop_task(void)
{
    static uint64_t pre_tx_ticks = 0;
    int err_code = 0;

    switch(m_task_state)
    {
        case IR_TASK_IDLE:

            break;

        case IR_TASK_WORK:
            /**
             * 每隔1s发送一次红外数据
             */
            if( mid_timer_ticks_get() - pre_tx_ticks > 1000)
            {
                pre_tx_ticks = mid_timer_ticks_get();

                err_code = ir_tx_start(m_tx_data, m_tx_data_len);
                if(err_code)
                {
                    trace_error("ir_tx_start error %d\n\r",err_code);
                }
                else
                {
                    trace_debug("ir_tx_start success:\n\r");
                    trace_dump(m_tx_data, m_tx_data_len);
                }
            }
            break;

        default:
            break;
    }
}
