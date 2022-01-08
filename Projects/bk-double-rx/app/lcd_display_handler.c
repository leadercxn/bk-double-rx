#include "board_config.h"
#include "lcd_display_handler.h"

union data 
{
    uint8_t byte;

    struct
    {   
        uint8_t com1 : 1;       //低位
        uint8_t com2 : 1;
        uint8_t com3 : 1;
        uint8_t com4 : 1;
        uint8_t reserve : 4;
    } seg;
};

typedef struct
{
    uint8_t         seg_index;
    union data      seg_data;
} lcd_seg_cell_t;


/* seg 表 
 *
 * seg_index + 1 = LCD图中的 SEG
 */
static lcd_seg_cell_t m_seg_table[] = {
    {.seg_index = 0, .seg_data.byte = 0},  //seg1
    {.seg_index = 1, .seg_data.byte = 0},  //seg2
    {.seg_index = 2, .seg_data.byte = 0},
    {.seg_index = 3, .seg_data.byte = 0},
    {.seg_index = 4, .seg_data.byte = 0},
    {.seg_index = 5, .seg_data.byte = 0},
    {.seg_index = 6, .seg_data.byte = 0},
    {.seg_index = 7, .seg_data.byte = 0},
    {.seg_index = 8, .seg_data.byte = 0},
    {.seg_index = 9, .seg_data.byte = 0},
    {.seg_index = 10, .seg_data.byte = 0},
    {.seg_index = 11, .seg_data.byte = 0},
    {.seg_index = 12, .seg_data.byte = 0},
    {.seg_index = 13, .seg_data.byte = 0},
    {.seg_index = 14, .seg_data.byte = 0},
    {.seg_index = 15, .seg_data.byte = 0},
    {.seg_index = 16, .seg_data.byte = 0},
    {.seg_index = 17, .seg_data.byte = 0},
    {.seg_index = 18, .seg_data.byte = 0},
    {.seg_index = 19, .seg_data.byte = 0},
    {.seg_index = 20, .seg_data.byte = 0},
    {.seg_index = 21, .seg_data.byte = 0},
    {.seg_index = 22, .seg_data.byte = 0},
    {.seg_index = 23, .seg_data.byte = 0},
    {.seg_index = 24, .seg_data.byte = 0},
    {.seg_index = 25, .seg_data.byte = 0},
    {.seg_index = 26, .seg_data.byte = 0},
    {.seg_index = 27, .seg_data.byte = 0},
    {.seg_index = 28, .seg_data.byte = 0},
    {.seg_index = 29, .seg_data.byte = 0},
    {.seg_index = 30, .seg_data.byte = 0},
    {.seg_index = 31, .seg_data.byte = 0}, //seg32
};

static uint16_t m_l_ch_index = 2;
static uint16_t m_r_ch_index = 102;

static uint16_t m_l_ch_freq = 6320;
static uint16_t m_r_ch_freq = 6600;

static uint8_t m_l_ch_af_level = 0;
static uint8_t m_r_ch_af_level = 0;

static uint8_t m_l_ch_rf_level = 0;
static uint8_t m_r_ch_rf_level = 0;

static lcd_settings_mode_e current_settings_mode = EXIT_SET_MODE;
static lcd_settings_mode_e old_settings_mode = EXIT_SET_MODE;

static bool l_af_rf_refresh = false;
static bool r_af_rf_refresh = false;
static bool l_channel_index_refresh = false;
static bool r_channel_index_refresh = false;
static bool l_channel_freq_refresh = false;
static bool r_channel_freq_refresh = false;
static bool m_lcd_off = false;
static bool m_old_lcd_off = false;

/**
 * 相关IO配置
 */
static lcd_display_obj_t    m_lcd_display_obj = {
    .ht162x.wr_clk_pin.gpio_port_periph_clk = HT1621_WR_PORT_PERIPH_CLK,
    .ht162x.wr_clk_pin.p_gpio_port = HT1621_WR_PORT,
    .ht162x.wr_clk_pin.gpio_dir = GPIO_DIR_OUTPUR,
    .ht162x.wr_clk_pin.gpio_pin = HT1621_WR_PIN,

    .ht162x.cs_pin.gpio_port_periph_clk = HT1621_CS_PORT_PERIPH_CLK,
    .ht162x.cs_pin.p_gpio_port = HT1621_CS_PORT,
    .ht162x.cs_pin.gpio_dir = GPIO_DIR_OUTPUR,
    .ht162x.cs_pin.gpio_pin = HT1621_CS_PIN,

    .ht162x.data_pin.gpio_port_periph_clk = HT1621_DATA_PORT_PERIPH_CLK,
    .ht162x.data_pin.p_gpio_port = HT1621_DATA_PORT,
    .ht162x.data_pin.gpio_dir = GPIO_DIR_OUTPUR,
    .ht162x.data_pin.gpio_pin = HT1621_DATA_PIN,

    .lcd_back_light_pin.gpio_port_periph_clk = LCD_BACK_LIGHT_PORT_PERIPH_CLK,
    .lcd_back_light_pin.p_gpio_port = LCD_BACK_LIGHT_PORT,
    .lcd_back_light_pin.gpio_dir = GPIO_DIR_OUTPUR,
    .lcd_back_light_pin.gpio_pin = LCD_BACK_LIGHT_PIN,

    .lcd_ctrl_pin.gpio_port_periph_clk = LCD_CTRL_PORT_PERIPH_CLK,
    .lcd_ctrl_pin.p_gpio_port = LCD_CTRL_PORT,
    .lcd_ctrl_pin.gpio_dir = GPIO_DIR_OUTPUR,
    .lcd_ctrl_pin.gpio_pin = LCD_CTRL_PIN,
};

/**
 *@brief 适用于 显示数字channel index 和 channel freq 的数码管
 */
static void digital_number_show(lcd_part_e part, uint8_t data)
{
    if(part > DIGITAL_S4)
    {
        return;
    }

    encode_seg_code_t seg_code;
	digital_to_segdata(&seg_code, data);

    lcd_seg_cell_t seg_cell_l;
    lcd_seg_cell_t seg_cell_h;

    switch (part)
    {
        case DIGITAL_1:
        case DIGITAL_2:
        case DIGITAL_3:
                if(part == DIGITAL_1)
                {
                    seg_cell_l.seg_index = 4;
                    seg_cell_h.seg_index = 5;
                }
                else if(part == DIGITAL_2)
                {
                    seg_cell_l.seg_index = 2;
                    seg_cell_h.seg_index = 3;
                }
                else
                {
                    seg_cell_l.seg_index = 0;
                    seg_cell_h.seg_index = 1;
                }

                //    m_seg_table[seg_cell_l.seg_index].seg_data.seg.com1   //com1保留旧值
                m_seg_table[seg_cell_l.seg_index].seg_data.seg.com2 = seg_code.seg_c;
                m_seg_table[seg_cell_l.seg_index].seg_data.seg.com3 = seg_code.seg_g;
                m_seg_table[seg_cell_l.seg_index].seg_data.seg.com4 = seg_code.seg_b;

                m_seg_table[seg_cell_h.seg_index].seg_data.seg.com1 = seg_code.seg_d;
                m_seg_table[seg_cell_h.seg_index].seg_data.seg.com2 = seg_code.seg_e;
                m_seg_table[seg_cell_h.seg_index].seg_data.seg.com3 = seg_code.seg_f;
                m_seg_table[seg_cell_h.seg_index].seg_data.seg.com4 = seg_code.seg_a;
            break;

        case DIGITAL_4:
        case DIGITAL_5:
        case DIGITAL_6:
                if(part == DIGITAL_4)
                {
                    seg_cell_l.seg_index = 30;
                    seg_cell_h.seg_index = 31;
                }
                else if(part == DIGITAL_5)
                {
                    seg_cell_l.seg_index = 28;
                    seg_cell_h.seg_index = 29;
                }
                else
                {
                    seg_cell_l.seg_index = 26;
                    seg_cell_h.seg_index = 27;
                }

                //    m_seg_table[seg_cell_l.seg_index].seg_data.seg.com1   //com1保留旧值
                m_seg_table[seg_cell_l.seg_index].seg_data.seg.com2 = seg_code.seg_b;
                m_seg_table[seg_cell_l.seg_index].seg_data.seg.com3 = seg_code.seg_g;
                m_seg_table[seg_cell_l.seg_index].seg_data.seg.com4 = seg_code.seg_c;

                m_seg_table[seg_cell_h.seg_index].seg_data.seg.com1 = seg_code.seg_a;
                m_seg_table[seg_cell_h.seg_index].seg_data.seg.com2 = seg_code.seg_f;
                m_seg_table[seg_cell_h.seg_index].seg_data.seg.com3 = seg_code.seg_e;
                m_seg_table[seg_cell_h.seg_index].seg_data.seg.com4 = seg_code.seg_d;
            break;
        
        case DIGITAL_7:
        case DIGITAL_8:
        case DIGITAL_9:
        case DIGITAL_10:
        case DIGITAL_12:
        case DIGITAL_13:
        case DIGITAL_14:
        case DIGITAL_15:
                if(part == DIGITAL_7)
                {
                    seg_cell_l.seg_index = 8;
                    seg_cell_h.seg_index = 9;
                }
                else if(part == DIGITAL_8)
                {
                    seg_cell_l.seg_index = 10;
                    seg_cell_h.seg_index = 11;
                }
                else if(part == DIGITAL_9)
                {
                    seg_cell_l.seg_index = 12;
                    seg_cell_h.seg_index = 13;
                }
                else if(part == DIGITAL_10)
                {
                    seg_cell_l.seg_index = 14;
                    seg_cell_h.seg_index = 15;
                }
                else if(part == DIGITAL_12)
                {
                    seg_cell_l.seg_index = 16;
                    seg_cell_h.seg_index = 17;
                }
                else if(part == DIGITAL_13)
                {
                    seg_cell_l.seg_index = 18;
                    seg_cell_h.seg_index = 19;
                }
                else if(part == DIGITAL_14)
                {
                    seg_cell_l.seg_index = 20;
                    seg_cell_h.seg_index = 21;
                }
                else
                {
                    seg_cell_l.seg_index = 22;
                    seg_cell_h.seg_index = 23;
                }

                m_seg_table[seg_cell_l.seg_index].seg_data.seg.com1 = seg_code.seg_a;
                m_seg_table[seg_cell_l.seg_index].seg_data.seg.com2 = seg_code.seg_f;
                m_seg_table[seg_cell_l.seg_index].seg_data.seg.com3 = seg_code.seg_e;
                m_seg_table[seg_cell_l.seg_index].seg_data.seg.com4 = seg_code.seg_d;

                //m_seg_table[seg_cell_h.seg_index].seg_data.seg.com1 = seg_code.seg_a; //com1保留旧值
                m_seg_table[seg_cell_h.seg_index].seg_data.seg.com2 = seg_code.seg_b;
                m_seg_table[seg_cell_h.seg_index].seg_data.seg.com3 = seg_code.seg_g;
                m_seg_table[seg_cell_h.seg_index].seg_data.seg.com4 = seg_code.seg_c;
            break;

        //显示0
        case DIGITAL_11:
        case DIGITAL_16:
                if(part == DIGITAL_11)
                {
                    seg_cell_l.seg_index = 13;
                    seg_cell_h.seg_index = 15;
                }
                else
                {
                    seg_cell_l.seg_index = 21;
                    seg_cell_h.seg_index = 23;
                }

                m_seg_table[seg_cell_l.seg_index].seg_data.seg.com1 = 1;
                m_seg_table[seg_cell_h.seg_index].seg_data.seg.com1 = 1;
            break;

        //显示0
        case DIGITAL_S3:
        case DIGITAL_S4:
                if(part == DIGITAL_S3)
                {
                    seg_cell_l.seg_index = 9;
                }
                else
                {
                    seg_cell_l.seg_index = 17;
                }

                m_seg_table[seg_cell_l.seg_index].seg_data.seg.com1 = 1;
            break;

        default:
            break;
    }

    trace_verbose("l.byte = 0x%02x\n\r",m_seg_table[seg_cell_l.seg_index].seg_data.byte);
    ht162x_write(&m_lcd_display_obj.ht162x, seg_cell_l.seg_index, m_seg_table[seg_cell_l.seg_index].seg_data.byte);

    if((part != DIGITAL_S3) && (part != DIGITAL_S4))
    {
        trace_verbose("h.byte = 0x%02x\n\r",m_seg_table[seg_cell_h.seg_index].seg_data.byte);
        ht162x_write(&m_lcd_display_obj.ht162x, seg_cell_h.seg_index, m_seg_table[seg_cell_h.seg_index].seg_data.byte);
    }
    
}

/**
 * @brief 适用于特别的字符显示
 */
static void digital_special_show(lcd_part_e part , bool enable)
{
    if(part < DIGITAL_S4)
    {
        return;
    }

    lcd_seg_cell_t seg_cell;

    switch(part)
    {
        case DIGITAL_S1:
                seg_cell.seg_index = 30;
                m_seg_table[seg_cell.seg_index].seg_data.seg.com1 = enable;
            break;

        case DIGITAL_S2:
                seg_cell.seg_index = 4;
                m_seg_table[seg_cell.seg_index].seg_data.seg.com1 = enable;
            break;

        case DIGITAL_T1:
                seg_cell.seg_index = 2;
                m_seg_table[seg_cell.seg_index].seg_data.seg.com1 = enable;
            break;

        case DIGITAL_T2:
                seg_cell.seg_index = 6;
                m_seg_table[seg_cell.seg_index].seg_data.seg.com1 = enable;
            break;

        case DIGITAL_T3:
                seg_cell.seg_index = 6;
                m_seg_table[seg_cell.seg_index].seg_data.seg.com2 = enable;
            break;

        case DIGITAL_T4:
                seg_cell.seg_index = 6;
                m_seg_table[seg_cell.seg_index].seg_data.seg.com3 = enable;
            break;

        case DIGITAL_T5:
                seg_cell.seg_index = 6;
                m_seg_table[seg_cell.seg_index].seg_data.seg.com4 = enable;
            break;


        case DIGITAL_X1:
                seg_cell.seg_index = 0;
                m_seg_table[seg_cell.seg_index].seg_data.seg.com1 = enable;
            break;

        case DIGITAL_X2:
                seg_cell.seg_index = 7;
                m_seg_table[seg_cell.seg_index].seg_data.seg.com1 = enable;
            break;

        case DIGITAL_X3:
                seg_cell.seg_index = 7;
                m_seg_table[seg_cell.seg_index].seg_data.seg.com2 = enable;
            break;

        case DIGITAL_X4:
                seg_cell.seg_index = 7;
                m_seg_table[seg_cell.seg_index].seg_data.seg.com3 = enable;
            break;

        case DIGITAL_X5:
                seg_cell.seg_index = 7;
                m_seg_table[seg_cell.seg_index].seg_data.seg.com4 = enable;
            break;


        case DIGITAL_M1:
                seg_cell.seg_index = 26;
                m_seg_table[seg_cell.seg_index].seg_data.seg.com1 = enable;
            break;

        case DIGITAL_M2:
                seg_cell.seg_index = 24;
                m_seg_table[seg_cell.seg_index].seg_data.seg.com1 = enable;
            break;

        case DIGITAL_M3:
                seg_cell.seg_index = 24;
                m_seg_table[seg_cell.seg_index].seg_data.seg.com2 = enable;
            break;

        case DIGITAL_M4:
                seg_cell.seg_index = 24;
                m_seg_table[seg_cell.seg_index].seg_data.seg.com3 = enable;
            break;

        case DIGITAL_M5:
                seg_cell.seg_index = 24;
                m_seg_table[seg_cell.seg_index].seg_data.seg.com4 = enable;
            break;


        case DIGITAL_W1:
                seg_cell.seg_index = 28;
                m_seg_table[seg_cell.seg_index].seg_data.seg.com1 = enable;
            break;

        case DIGITAL_W2:
                seg_cell.seg_index = 25;
                m_seg_table[seg_cell.seg_index].seg_data.seg.com1 = enable;
            break;

        case DIGITAL_W3:
                seg_cell.seg_index = 25;
                m_seg_table[seg_cell.seg_index].seg_data.seg.com2 = enable;
            break;

        case DIGITAL_W4:
                seg_cell.seg_index = 25;
                m_seg_table[seg_cell.seg_index].seg_data.seg.com3 = enable;
            break;

        case DIGITAL_W5:
                seg_cell.seg_index = 25;
                m_seg_table[seg_cell.seg_index].seg_data.seg.com4 = enable;
            break;
    }

    ht162x_write(&m_lcd_display_obj.ht162x, seg_cell.seg_index, m_seg_table[seg_cell.seg_index].seg_data.byte);
}

/**
 * @brief 清除freq的显示
 */
static void channel_freq_lr_clear(screen_lr_e lr)
{
    uint8_t start_index;
    uint8_t i;

    if(SCREEN_L == lr)
    {
        start_index = 8;
        m_seg_table[8].seg_data.byte = 0;
        m_seg_table[9].seg_data.byte = 0;
        m_seg_table[10].seg_data.byte = 0;
        m_seg_table[11].seg_data.byte = 0;
        m_seg_table[12].seg_data.byte = 0;
        m_seg_table[13].seg_data.byte = 0;
        m_seg_table[14].seg_data.byte = 0;
        m_seg_table[15].seg_data.byte = 0;
    }
    else
    {
        start_index = 16;
        m_seg_table[16].seg_data.byte = 0;
        m_seg_table[17].seg_data.byte = 0;
        m_seg_table[18].seg_data.byte = 0;
        m_seg_table[19].seg_data.byte = 0;
        m_seg_table[20].seg_data.byte = 0;
        m_seg_table[21].seg_data.byte = 0;
        m_seg_table[22].seg_data.byte = 0;
        m_seg_table[23].seg_data.byte = 0;
    }

    for(i = 0; i < 8; i++)
    {
        ht162x_write(&m_lcd_display_obj.ht162x, start_index + i, m_seg_table[start_index + i].seg_data.byte);
    }
    
}

/**
 * @brief 显示关机logo
 */
static void channel_off_show(void)
{
    uint8_t i = 0;

    /**
     * 清屏
     */
    for(i = 0; i < 32; i++)
    {
        m_seg_table[i].seg_data.byte = 0;
        ht162x_write(&m_lcd_display_obj.ht162x, i, m_seg_table[i].seg_data.byte);
    }

    /**
     * 两边显示OFF
     */
    digital_number_show(DIGITAL_7,0x0);
    digital_number_show(DIGITAL_8,0xF);
    digital_number_show(DIGITAL_9,0xF);

    digital_number_show(DIGITAL_12,0x0);
    digital_number_show(DIGITAL_13,0xF);
    digital_number_show(DIGITAL_14,0xF);

    delay_ms(500);
    gpio_output_set(&m_lcd_display_obj.lcd_ctrl_pin, 1);
    gpio_output_set(&m_lcd_display_obj.lcd_back_light_pin, 0);
}

/**
 * @brief 显示通道数
 */
static void channel_index_lr_show(screen_lr_e lr, uint16_t index)
{
    seg_data_t seg_data;
	segdata_convertor(index, &seg_data);

    if(SCREEN_L == lr)
    {
        digital_number_show(DIGITAL_1, seg_data.high);
        digital_number_show(DIGITAL_2, seg_data.mid);
        digital_number_show(DIGITAL_3, seg_data.low);
    }
    else
    {
        digital_number_show(DIGITAL_4, seg_data.high);
        digital_number_show(DIGITAL_5, seg_data.mid);
        digital_number_show(DIGITAL_6, seg_data.low);
    }
}

/**
 * @brief 显示通道频率
 * 
 * @param [in] data 单位 百M
 */
static void channel_freq_lr_show(screen_lr_e lr, uint16_t hund_freq)
{
    uint8_t hundred = hund_freq / 1000;
    uint8_t decade = hund_freq % 1000 / 100;
    uint8_t unit =   hund_freq % 100 / 10;
    uint8_t point = hund_freq % 10;

    if(SCREEN_L == lr)
    {
        digital_number_show(DIGITAL_7, hundred);
        digital_number_show(DIGITAL_8, decade);
        digital_number_show(DIGITAL_9, unit);
        digital_number_show(DIGITAL_10, point);

        digital_number_show(DIGITAL_11, 0);
        digital_number_show(DIGITAL_S3, 0);
    }
    else
    {
        digital_number_show(DIGITAL_12, hundred);
        digital_number_show(DIGITAL_13, decade);
        digital_number_show(DIGITAL_14, unit);
        digital_number_show(DIGITAL_15, point);

        digital_number_show(DIGITAL_16, 0);
        digital_number_show(DIGITAL_S4, 0);
    }
}

/**
 * @brief 显示af幅度
 */
static void channel_af_lr_show(screen_lr_e lr, uint8_t level)
{
    if(SCREEN_L == lr)
    {
        if(level > 4)
        {
            digital_special_show(DIGITAL_X1 , 1);
        }
        else
        {
            digital_special_show(DIGITAL_X1 , 0);
        }
        if(level > 3)
        {
            digital_special_show(DIGITAL_X2 , 1);
        }
        else
        {
            digital_special_show(DIGITAL_X2 , 0);
        }       
        if(level > 2)
        {
            digital_special_show(DIGITAL_X3 , 1);
        }
        else
        {
            digital_special_show(DIGITAL_X3 , 0);
        }
        if(level > 1)
        {
            digital_special_show(DIGITAL_X4 , 1);
        }
        else
        {
            digital_special_show(DIGITAL_X4 , 0);
        }

        if(level > 0)
        {
            digital_special_show(DIGITAL_X5 , 1);
        }
        else
        {
            digital_special_show(DIGITAL_X5 , 0);
        }
    }
    else
    {
        if(level > 4)
        {
            digital_special_show(DIGITAL_M1 , 1);
        }
        else
        {
            digital_special_show(DIGITAL_M1 , 0);
        }

        if(level > 3)
        {
            digital_special_show(DIGITAL_M2 , 1);
        }
        else
        {
            digital_special_show(DIGITAL_M2 , 0);
        }

        if(level > 2)
        {
            digital_special_show(DIGITAL_M3 , 1);
        }
        else
        {
            digital_special_show(DIGITAL_M3 , 0);
        }

        if(level > 1)
        {
            digital_special_show(DIGITAL_M4 , 1);
        }
        else
        {
            digital_special_show(DIGITAL_M4 , 0);
        }

        if(level > 0)
        {
            digital_special_show(DIGITAL_M5 , 1);
        }
        else
        {
            digital_special_show(DIGITAL_M5 , 0);
        }
    }
}

/**
 * @brief 显示rf幅度
 */
static void channel_rf_lr_show(screen_lr_e lr, uint8_t level)
{
    if(SCREEN_L == lr)
    {
        if(level > 4)
        {
            digital_special_show(DIGITAL_T1 , 1);
        }
        else
        {
            digital_special_show(DIGITAL_T1 , 0);
        }

        if(level > 3)
        {
            digital_special_show(DIGITAL_T2 , 1);
        }
        else
        {
            digital_special_show(DIGITAL_T2 , 0);
        }

        if(level > 2)
        {
            digital_special_show(DIGITAL_T3 , 1);
        }
        else
        {
            digital_special_show(DIGITAL_T3 , 0);
        }

        if(level > 1)
        {
            digital_special_show(DIGITAL_T4 , 1);
        }
        else
        {
            digital_special_show(DIGITAL_T4 , 0);
        }

        if(level > 0)
        {
            digital_special_show(DIGITAL_T5 , 1);
        }
        else
        {
            digital_special_show(DIGITAL_T5 , 0);
        }
    }
    else
    {
        if(level > 4)
        {
            digital_special_show(DIGITAL_W1 , 1);
        }
        else
        {
            digital_special_show(DIGITAL_W1 , 0);
        }

        if(level > 3)
        {
            digital_special_show(DIGITAL_W2 , 1);
        }
        else
        {
            digital_special_show(DIGITAL_W2 , 0);
        }

        if(level > 2)
        {
            digital_special_show(DIGITAL_W3 , 1);
        }
        else
        {
            digital_special_show(DIGITAL_W3 , 0);
        }

        if(level > 1)
        {
            digital_special_show(DIGITAL_W4 , 1);
        }
        else
        {
            digital_special_show(DIGITAL_W4 , 0);
        }

        if(level > 0)
        {
            digital_special_show(DIGITAL_W5 , 1);
        }
        else
        {
            digital_special_show(DIGITAL_W5 , 0);
        }
    }
}

int lcd_hw_init(void)
{
    gpio_config(&m_lcd_display_obj.lcd_ctrl_pin);
    gpio_config(&m_lcd_display_obj.lcd_back_light_pin);
}

int lcd_display_init(void)
{
    ht162x_init(&m_lcd_display_obj.ht162x);
    ht162x_all_clean(&m_lcd_display_obj.ht162x);

    gpio_config(&m_lcd_display_obj.lcd_ctrl_pin);
    gpio_config(&m_lcd_display_obj.lcd_back_light_pin);
    gpio_output_set(&m_lcd_display_obj.lcd_ctrl_pin, 0);
    gpio_output_set(&m_lcd_display_obj.lcd_back_light_pin, 1);

    /**
     * 基本显示
     */
    digital_special_show(DIGITAL_S2 , true);

    m_l_ch_freq = SCREEN_L_CHANNEL_FREQ_MIN + (m_l_ch_index - SCREEN_L_CHANNEL_INDEX_MIN) * 3;
    m_r_ch_freq = SCREEN_R_CHANNEL_FREQ_MIN + (m_r_ch_index - SCREEN_R_CHANNEL_INDEX_MIN) * 3;

    channel_index_lr_show(SCREEN_L, m_l_ch_index);
    channel_index_lr_show(SCREEN_R, m_r_ch_index);

    channel_freq_lr_show(SCREEN_L, m_l_ch_freq);
    channel_freq_lr_show(SCREEN_R, m_r_ch_freq);

    return 0;
}

/**
 * @brief 设频显示
 */
static void lcd_set_channel_display(void)
{
    static uint8_t l_setting_mode_stage = 0;
    static uint8_t r_setting_mode_stage = 0;
    static uint64_t ticks = 0;

    if(current_settings_mode != old_settings_mode)
    {
        switch(old_settings_mode)
        {
            case EXIT_SET_MODE:
                break;

            case L_SETTING_MODE:
                channel_freq_lr_clear(SCREEN_L);
                l_channel_freq_refresh = true;
                l_setting_mode_stage = 0;
                break;

            case R_SETTING_MODE:
                channel_freq_lr_clear(SCREEN_R);
                r_channel_freq_refresh = true;
                r_setting_mode_stage = 0;
                break;

            default:
                break;
        }
    }

    /* 200ms 刷新一次 */
    if(mid_timer_ticks_get() - ticks > 200)
    {
        ticks = mid_timer_ticks_get();
    }
    else
    {
        return;
    }

    if(current_settings_mode == L_SETTING_MODE)
    {
        switch(l_setting_mode_stage)
        {
            case 0:
                l_setting_mode_stage++;
                channel_freq_lr_clear(SCREEN_L);
                break;

            case 1:
                l_setting_mode_stage++;
                m_seg_table[8].seg_data.seg.com3 = 1;
                m_seg_table[9].seg_data.seg.com3 = 1;
                ht162x_write(&m_lcd_display_obj.ht162x, 8, m_seg_table[8].seg_data.byte);
                ht162x_write(&m_lcd_display_obj.ht162x, 9, m_seg_table[9].seg_data.byte);
                break;

            case 2:
                l_setting_mode_stage++;
                m_seg_table[11].seg_data.seg.com3 = 1;
                ht162x_write(&m_lcd_display_obj.ht162x, 11, m_seg_table[11].seg_data.byte);
                break;

            case 3:
                l_setting_mode_stage++;
                m_seg_table[13].seg_data.seg.com3 = 1;
                ht162x_write(&m_lcd_display_obj.ht162x, 13, m_seg_table[13].seg_data.byte);
                break;

            case 4:
                l_setting_mode_stage++;
                m_seg_table[15].seg_data.seg.com3 = 1;
                ht162x_write(&m_lcd_display_obj.ht162x, 15, m_seg_table[15].seg_data.byte);
                break;

            case 5:
                l_setting_mode_stage = 0;
                m_seg_table[11].seg_data.seg.com1 = 1;
                ht162x_write(&m_lcd_display_obj.ht162x, 11, m_seg_table[11].seg_data.byte);
                break;

            default:
                break;
        }
    }
    else if(current_settings_mode == R_SETTING_MODE)
    {
        switch(r_setting_mode_stage)
        {
            case 0:
                r_setting_mode_stage++;
                channel_freq_lr_clear(SCREEN_R);
                break;

            case 1:
                r_setting_mode_stage++;
                m_seg_table[16].seg_data.seg.com3 = 1;
                m_seg_table[17].seg_data.seg.com3 = 1;
                ht162x_write(&m_lcd_display_obj.ht162x, 16, m_seg_table[16].seg_data.byte);
                ht162x_write(&m_lcd_display_obj.ht162x, 17, m_seg_table[17].seg_data.byte);
                break;

            case 2:
                r_setting_mode_stage++;
                m_seg_table[19].seg_data.seg.com3 = 1;
                ht162x_write(&m_lcd_display_obj.ht162x, 19, m_seg_table[19].seg_data.byte);
                break;

            case 3:
                r_setting_mode_stage++;
                m_seg_table[21].seg_data.seg.com3 = 1;
                ht162x_write(&m_lcd_display_obj.ht162x, 21, m_seg_table[21].seg_data.byte);
                break;

            case 4:
                r_setting_mode_stage++;
                m_seg_table[23].seg_data.seg.com3 = 1;
                ht162x_write(&m_lcd_display_obj.ht162x, 23, m_seg_table[23].seg_data.byte);
                break;

            case 5:
                r_setting_mode_stage = 0;
                m_seg_table[19].seg_data.seg.com1 = 1;
                ht162x_write(&m_lcd_display_obj.ht162x, 19, m_seg_table[19].seg_data.byte);
                break;

            default:
                break;
        }
    }

    old_settings_mode = current_settings_mode;
}


void lcd_display_loop_task(void)
{
    /* 要等settings mode 完全退出才退出刷新 */
    if((old_settings_mode != EXIT_SET_MODE) || (current_settings_mode != old_settings_mode))
    {
        lcd_set_channel_display();
    }

    if(l_channel_index_refresh)
    {
        l_channel_index_refresh = false;
        channel_index_lr_show(SCREEN_L, m_l_ch_index);
    }

    if(r_channel_index_refresh)
    {
        r_channel_index_refresh = false;
        channel_index_lr_show(SCREEN_R, m_r_ch_index);
    }

    if(l_channel_freq_refresh)
    {
        l_channel_freq_refresh = false;
        m_l_ch_freq = SCREEN_L_CHANNEL_FREQ_MIN + (m_l_ch_index - SCREEN_L_CHANNEL_INDEX_MIN) * 3;
        channel_freq_lr_show(SCREEN_L,m_l_ch_freq);
    }

    if(r_channel_freq_refresh)
    {
        r_channel_freq_refresh = false;
        m_r_ch_freq = SCREEN_R_CHANNEL_FREQ_MIN + (m_r_ch_index - SCREEN_R_CHANNEL_INDEX_MIN) * 3;
        channel_freq_lr_show(SCREEN_R,m_r_ch_freq);
    }

    if(l_af_rf_refresh)
    {
        l_af_rf_refresh = false;
        channel_af_lr_show(SCREEN_L, m_l_ch_af_level);
        channel_rf_lr_show(SCREEN_L, m_l_ch_rf_level);
    }

    if(r_af_rf_refresh)
    {
        r_af_rf_refresh = false;
        channel_af_lr_show(SCREEN_R, m_r_ch_af_level);
        channel_rf_lr_show(SCREEN_R, m_r_ch_rf_level);
    }

    if(m_lcd_off != m_old_lcd_off)
    {
        if(m_lcd_off)
        {
            channel_off_show();
        }
        else
        {
            gpio_output_set(&m_lcd_display_obj.lcd_ctrl_pin, 0);
            gpio_output_set(&m_lcd_display_obj.lcd_back_light_pin, 1);

            delay_ms(10);

            digital_special_show(DIGITAL_S2 , true);
            l_channel_index_refresh = true;
            r_channel_index_refresh = true;
            l_channel_freq_refresh = true;
            r_channel_freq_refresh = true;
        }

        m_old_lcd_off = m_lcd_off;
    }
}


uint16_t channel_index_lr_get(screen_lr_e lr)
{
    if(SCREEN_L == lr)
    {
        return m_l_ch_index;
    }

    return m_r_ch_index;
}

void channel_index_lr_set(screen_lr_e lr, uint16_t index)
{
    if(SCREEN_L == lr)
    {
        m_l_ch_index = index;
        l_channel_index_refresh = true;
        if(((current_settings_mode == EXIT_SET_MODE) && (old_settings_mode == EXIT_SET_MODE)) || (current_settings_mode == R_SETTING_MODE))
        {
            l_channel_freq_refresh = true;
        }
    }
    else
    {
        m_r_ch_index = index;
        r_channel_index_refresh = true;
        if(((current_settings_mode == EXIT_SET_MODE) && (old_settings_mode == EXIT_SET_MODE)) ||(current_settings_mode == L_SETTING_MODE))
        {
            r_channel_freq_refresh = true;
        }
    }
}

void channel_af_level_lr_set(screen_lr_e lr, uint8_t level)
{
    if(SCREEN_L == lr)
    {
        m_l_ch_af_level = level;
        l_af_rf_refresh = true;
    }
    else
    {
        m_r_ch_af_level = level;
        r_af_rf_refresh = true;
    }
}

void channel_rf_level_lr_set(screen_lr_e lr, uint8_t level)
{
    if(SCREEN_L == lr)
    {
        m_l_ch_rf_level = level;
        l_af_rf_refresh = true;
    }
    else
    {
        m_r_ch_rf_level = level;
        r_af_rf_refresh = true;
    }

}

lcd_settings_mode_e channel_settings_mode_get(void)
{
    return current_settings_mode;
}

void channel_settings_mode_set(lcd_settings_mode_e mode)
{
    current_settings_mode = mode;
}

void lcd_black_light_enable(bool enable)
{
    if(enable)
    {
        gpio_output_set(&m_lcd_display_obj.lcd_back_light_pin, 1);
    }
    else
    {
        gpio_output_set(&m_lcd_display_obj.lcd_back_light_pin, 0);
    }
}

void lcd_ctrl_enable(bool enable)
{
    if(enable)
    {
        gpio_output_set(&m_lcd_display_obj.lcd_ctrl_pin, 0);
    }
    else
    {
        gpio_output_set(&m_lcd_display_obj.lcd_ctrl_pin, 1);
    }
}

bool lcd_off_status_get(void)
{
    return m_lcd_off;
}

void lcd_off_status_set(bool enable)
{
    m_lcd_off = enable;
}
