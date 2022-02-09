#include "stdio.h"
#include "board_config.h"
#include "bk953x_handler.h"
#include "lcd_display_handler.h"

static bk953x_object_t m_l_bk9532_obj;
static bk953x_object_t m_r_bk9532_obj;

static gpio_object_t   m_l_bk9532_rst;
static gpio_object_t   m_r_bk9532_rst;


typedef struct
{
    bk953x_lr_e         lr_flag;
    bk953x_task_stage_e stage;
    uint64_t            rd_spec_data_old_ticks;
    bk953x_object_t     *p_bk953x_object;
    union user_spec_data user_date;
} bk953x_task_t;

static bk953x_task_t m_l_bk953x_task = {
    .p_bk953x_object = &m_l_bk9532_obj,
    .stage = BK_STAGE_INIT,
    .lr_flag = BK953X_L,
    .rd_spec_data_old_ticks = 0,
    .user_date.byte = 0,
};

static bk953x_task_t m_r_bk953x_task = {
    .p_bk953x_object = &m_r_bk9532_obj,
    .stage = BK_STAGE_INIT,
    .lr_flag = BK953X_R,
    .rd_spec_data_old_ticks = 0,
    .user_date.byte = 0,
};


int bk9532_lr_init(void)
{
    int err_code = 0;

#ifdef FT32
    m_l_bk9532_rst.gpio_port_periph_clk = L_BK9532_CE_PERIPH_CLK;
    m_l_bk9532_rst.p_gpio_port = L_BK9532_CE_PORT;

    m_r_bk9532_rst.gpio_port_periph_clk = R_BK9532_CE_PERIPH_CLK;
    m_r_bk9532_rst.p_gpio_port = R_BK9532_CE_PORT;
#endif

    m_l_bk9532_rst.gpio_dir = GPIO_DIR_OUTPUR;
    m_l_bk9532_rst.gpio_pin = L_BK9532_CE_PIN;

    m_r_bk9532_rst.gpio_dir = GPIO_DIR_OUTPUR;
    m_r_bk9532_rst.gpio_pin = R_BK9532_CE_PIN;

#ifdef FT32
    m_r_bk9532_obj.mid_bk953x_object.virt_i2c_object.sda_port_periph_clk = R_VIRT_SDA_GPIO_CLK;
    m_r_bk9532_obj.mid_bk953x_object.virt_i2c_object.scl_port_periph_clk = R_VIRT_SCL_GPIO_CLK;
    m_r_bk9532_obj.mid_bk953x_object.virt_i2c_object.p_sda_gpio_port = R_VIRT_SDA_GPIO_PORT;
    m_r_bk9532_obj.mid_bk953x_object.virt_i2c_object.p_scl_gpio_port = R_VIRT_SCL_GPIO_PORT;
    m_r_bk9532_obj.mid_bk953x_object.virt_i2c_object.sda_gpio_pin = R_VIRT_SDA_PIN;
    m_r_bk9532_obj.mid_bk953x_object.virt_i2c_object.scl_gpio_pin = R_VIRT_SCL_PIN;

    m_l_bk9532_obj.mid_bk953x_object.virt_i2c_object.sda_port_periph_clk = L_VIRT_SDA_GPIO_CLK;
    m_l_bk9532_obj.mid_bk953x_object.virt_i2c_object.scl_port_periph_clk = L_VIRT_SCL_GPIO_CLK;
    m_l_bk9532_obj.mid_bk953x_object.virt_i2c_object.p_sda_gpio_port = L_VIRT_SDA_GPIO_PORT;
    m_l_bk9532_obj.mid_bk953x_object.virt_i2c_object.p_scl_gpio_port = L_VIRT_SCL_GPIO_PORT;
    m_l_bk9532_obj.mid_bk953x_object.virt_i2c_object.sda_gpio_pin = L_VIRT_SDA_PIN;
    m_l_bk9532_obj.mid_bk953x_object.virt_i2c_object.scl_gpio_pin = L_VIRT_SCL_PIN;
#endif
    m_r_bk9532_obj.p_rst_gpio = (void *)&m_r_bk9532_rst;
    m_l_bk9532_obj.p_rst_gpio = (void *)&m_l_bk9532_rst;

    bk9532_res_init(&m_r_bk9532_obj);
    bk9532_res_init(&m_l_bk9532_obj);

    return err_code;
}

/**
 * @brief 搜频
 */
static int bk953x_rx_channel_search(bk953x_object_t *p_bk953x_object)
{
    int err_code = 0;


    return err_code;
}

static int bk953x_singal_check(bk953x_object_t *p_bk953x_object)
{
    IS_NULL(p_bk953x_object);

    if(bk9532_is_receive_single(p_bk953x_object))
    {

    }
    else
    {

    }
}

static void bk953x_stage_task_run(bk953x_task_t *p_task)
{
    int err_code = 0;

    switch(p_task->stage)
    {
        case BK_STAGE_INIT:

            /**
             * 硬件复位，复位要适当的延时，别太快
             */
            gpio_config(p_task->p_bk953x_object->p_rst_gpio);

            gpio_output_set(p_task->p_bk953x_object->p_rst_gpio, 1);
            delay_ms(50);
            gpio_output_set(p_task->p_bk953x_object->p_rst_gpio, 0);
            delay_ms(50);
            gpio_output_set(p_task->p_bk953x_object->p_rst_gpio, 1);
            delay_ms(100);

            bk9532_chip_id_get(p_task->p_bk953x_object);

            if(p_task->lr_flag == BK953X_L)
            {
                trace_debug("l_chip_id = 0x%08x\n\r",p_task->p_bk953x_object->chip_id);
            }
            else
            {
                trace_debug("r_chip_id = 0x%08x\n\r",p_task->p_bk953x_object->chip_id);
            }

            /**
             * 寄存器配置
             */
            err_code = bk9532_config_init(p_task->p_bk953x_object);
            if(err_code == 0)
            {
                trace_debug("bk953x_config_init success\n\r");
            }

            /**
             * 通道设置
             */
            err_code = bk9532_ch_index_set(p_task->lr_flag, p_task->p_bk953x_object->freq_chan_index);
            if(err_code)
            {
                trace_error("ch index set error\n\r");
            }
            else
            {
                trace_debug("ch index %d set success\n\r",p_task->p_bk953x_object->freq_chan_index);
            }

            p_task->stage++;
            break;

        case BK_STAGE_NORMAL:
            if( mid_timer_ticks_get() - p_task->rd_spec_data_old_ticks > 500)
            {
                p_task->rd_spec_data_old_ticks = mid_timer_ticks_get();
                err_code = bk9532_rx_spec_data_get(p_task->p_bk953x_object, &p_task->user_date.byte);
                if(!err_code)
                {
                    trace_debug("lr_flag %d user_date = 0x%02x\n\r",p_task->lr_flag,p_task->user_date.byte);
                }
            }
            bk953x_singal_check(p_task->p_bk953x_object);
            break;

        case BK_STAGE_SEARCHING:

            break;

        case BK_STATE_IDLE:
            /**
             * IDLE 状态下，关闭射频
             */
            gpio_output_set(p_task->p_bk953x_object->p_rst_gpio, 0);
            break;

        default:
            break;
    }
}

void bk953x_loop_task(void)
{
    bk953x_stage_task_run(&m_l_bk953x_task);
    bk953x_stage_task_run(&m_r_bk953x_task);
}


void bk953x_task_stage_set(bk953x_lr_e lr, bk953x_task_stage_e stage)
{
    if(lr == BK953X_L)
    {
        m_l_bk953x_task.stage = stage;
    }
    else
    {
        m_r_bk953x_task.stage = stage;
    }
}

/**
 * @brief 设置通道序号来获取对应寄存器的值
 */
int bk9532_ch_index_set(bk953x_lr_e lr, uint16_t chan_index)
{
    freq_chan_object_t freq_chan_obj;

    freq_chan_obj.chan_index = chan_index;

    if(lr == BK953X_L)
    {
        if((chan_index < 1) && (chan_index > 100))
        {
            return -EINVAL;
        }

        freq_chan_obj.reg_value = BK9532_FREQ_632_MHZ + BK9532_FREQ_0_3_MHZ * (chan_index - SCREEN_L_CHANNEL_INDEX_MIN);

        trace_debug("l bk9532_ch_index_set reg_val 0x%08x\n\r",freq_chan_obj.reg_value);

        return bk9532_freq_chan_set(&m_l_bk9532_obj, &freq_chan_obj);
    }
    else
    {
        if((chan_index < 101) && (chan_index > 200))
        {
            return -EINVAL;
        }

        freq_chan_obj.reg_value = BK9532_FREQ_660_MHZ + BK9532_FREQ_0_3_MHZ * (chan_index - SCREEN_R_CHANNEL_INDEX_MIN);

        trace_debug("r bk9532_ch_index_set reg_val 0x%08x\n\r",freq_chan_obj.reg_value);

        return bk9532_freq_chan_set(&m_r_bk9532_obj, &freq_chan_obj);
    }
}

int bk953x_rf_rssi_get(bk953x_lr_e lr, uint8_t *p_level)
{
    int err_code = 0;
    uint8_t rssi = 0;

    if(lr == BK953X_L)
    {
        err_code = bk9532_rx_rssi_get(&m_l_bk9532_obj, &rssi);
    }
    else
    {
        err_code = bk9532_rx_rssi_get(&m_r_bk9532_obj, &rssi);
    }

    /**
     * rssi: 8 bit
     */
    if(rssi > 70)
    {
        *p_level = 5;
    }
    else if(rssi > 65)
    {
        *p_level = 4;
    }
    else if(rssi > 60)
    {
        *p_level = 3;
    }
    else if(rssi > 55)
    {
        *p_level = 2;
    }
    else if(rssi > 50)
    {
        *p_level = 1;
    }
    else
    {
         *p_level = 0;
    }

    return err_code;
} 

int bk953x_af_get(bk953x_lr_e lr, uint8_t *p_level)
{
    int err_code = 0;
    uint16_t af_vol = 0;

    if(lr == BK953X_L)
    {
        err_code = bk9532_rx_vol_get(&m_l_bk9532_obj, &af_vol);
    }
    else
    {
        err_code = bk9532_rx_vol_get(&m_r_bk9532_obj, &af_vol);
    }

    /**
     * rssi: 16 bit
     */
    if(af_vol > 500)
    {
        *p_level = 5;
    }
    else if(af_vol > 400)
    {
        *p_level = 4;
    }
    else if(af_vol > 300)
    {
        *p_level = 3;
    }
    else if(af_vol > 200)
    {
        *p_level = 2;
    }
    else if(af_vol > 100)
    {
        *p_level = 1;
    }
    else
    {
         *p_level = 0;
    }

    return err_code;
} 

uint8_t bk953x_user_data_get(bk953x_lr_e lr)
{
    if(lr == BK953X_L)
    {
        return m_l_bk953x_task.user_date.byte;
    }

    return m_r_bk953x_task.user_date.byte;
}