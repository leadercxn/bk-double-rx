#include "board_config.h"

#include "bk953x_handler.h"
#include "adc_button_handler.h"
#include "ad22650_handler.h"
#include "flash_handler.h"
#include "lcd_display_handler.h"
#include "ir_tx_handler.h"

#define SCHED_MAX_EVENT_DATA_SIZE   8
#define SCHED_QUEUE_SIZE            20

static app_scheduler_t  m_app_scheduler;

uint8_t data[4] = {0x64, 0x23, 0x18, 0x74};

/**
 * 系统关机状态
 */
uint8_t g_sys_power_off = 0;
uint8_t g_old_sys_power_off = 0;

/**
 * 拨动开关
 */
static gpio_object_t   m_sys_power_sw_gpio = 
                {
                    .gpio_port_periph_clk = SYS_POWER_SWITCH_PORT_PERIPH_CLK,
                    .p_gpio_port = SYS_POWER_SWITCH_PORT,
                    .gpio_dir = GPIO_DIR_INPUT,
                    .gpio_pin = SYS_POWER_SWITCH_PIN,
                };

typedef enum
{
    CMD_FREQ = 0x01,
    CMD_VOL,
    CMD_TX_POWER,
} ir_tx_cmd_e;

/**
 * ir红外传输数据结构体
 */
typedef struct
{
    uint8_t len;
    uint8_t band_type;
    uint8_t region_band;
    uint8_t freq_ch;
} __attribute__((__packed__ )) ir_command_freq_t;

typedef struct main
{
    ir_tx_cmd_e cmd;

    union data 
    {
      ir_command_freq_t freq_data;
      uint8_t           cmd_data[4];
    } __attribute__((__packed__ )) tx_data;

    uint8_t crc;
} __attribute__((__packed__ )) ir_tx_data_t;



/**
 * @brief 右侧adc按键回调
 */
static void r_adc_button_handler(adc_button_event_e event)
{
    int err_code = 0;
    uint16_t r_index = 0;

    static uint64_t pre_ticks = 0;
    uint64_t now_ticks = mid_timer_ticks_get();

    if(g_sys_power_off)
    {
      return;
    }

    /**
     * 两个时间得间隔200ms
     */
    if((now_ticks - pre_ticks > 200) || (event == BUTTON_R_EVENT_SET_RELEASE))
    {
      pre_ticks = now_ticks;
    }
    else
    {
      return;
    }

    /**
     * 上下按键
     */
    if((event >= BUTTON_R_EVENT_UP_PUSH) && (event <= BUTTON_R_EVENT_DOWN_RELEASE))
    {
        r_index = channel_index_lr_get(SCREEN_R);
        /* 右边 */
        if((BUTTON_R_EVENT_UP_RELEASE == event) || (BUTTON_R_EVENT_UP_PUSH == event))
        {
          r_index += 1;

          if(r_index > SCREEN_R_CHANNEL_INDEX_MAX)
          {
            r_index = SCREEN_R_CHANNEL_INDEX_MIN;
          }
        }
        else
        {
          r_index -= 1;

          if(r_index < SCREEN_R_CHANNEL_INDEX_MIN)
          {
            r_index = SCREEN_R_CHANNEL_INDEX_MAX;
          }
        }

        channel_index_lr_set(SCREEN_R, r_index);
        g_app_param.r_ch_index = (uint8_t)r_index;

        bk9532_ch_index_set(BK953X_R, r_index);
        app_param_flash_update();
    }

    /**
     * SET
     */
    if(event == BUTTON_R_EVENT_SET_RELEASE)
    {
        if(R_SETTING_MODE != channel_settings_mode_get())
        {
            ir_tx_data_t ir_tx_data;

            ir_tx_data.cmd = CMD_FREQ;
            ir_tx_data.tx_data.freq_data.len = 3;
            ir_tx_data.tx_data.freq_data.band_type = g_app_param.band_type;
            ir_tx_data.tx_data.freq_data.region_band = g_app_param.region_band;
            ir_tx_data.tx_data.freq_data.freq_ch = g_app_param.r_ch_index;

            ir_tx_data.crc = ir_tx_data.tx_data.freq_data.band_type + ir_tx_data.tx_data.freq_data.region_band  \
                              + ir_tx_data.tx_data.freq_data.freq_ch;

            channel_settings_mode_set(R_SETTING_MODE);
            ir_tx_task_state_set(IR_TASK_WORK, (uint8_t *)&ir_tx_data, sizeof(ir_tx_data_t));
        }
        else
        {
            channel_settings_mode_set(EXIT_SET_MODE);
            ir_tx_task_state_set(IR_TASK_IDLE, NULL, 0);
        }
    }

    /**
     * LONG_SET
     */

}

/**
 * @brief 左侧adc按键回调
 */
static void l_adc_button_handler(adc_button_event_e event)
{
    int err_code = 0;
    uint16_t l_index = 0;

    static uint64_t pre_ticks = 0;

    uint64_t now_ticks = mid_timer_ticks_get();

    if(g_sys_power_off)
    {
      return;
    }

    /**
     * 两个时间得间隔200ms
     */
    if((now_ticks - pre_ticks > 200) || (event == BUTTON_L_EVENT_SET_RELEASE))
    {
      pre_ticks = now_ticks;
    }
    else
    {
      return;
    }

    /* 左边 */
    if((event >= BUTTON_L_EVENT_UP_PUSH) && (event <= BUTTON_L_EVENT_DOWN_RELEASE))
    {
        l_index = channel_index_lr_get(SCREEN_L);

        if((BUTTON_L_EVENT_UP_RELEASE == event)||(BUTTON_L_EVENT_UP_PUSH == event))
        {
          l_index += 1;

          if(l_index > SCREEN_L_CHANNEL_INDEX_MAX)
          {
            l_index = SCREEN_L_CHANNEL_INDEX_MIN;
          }
        }
        else
        {
          l_index -= 1;

          if(l_index < SCREEN_L_CHANNEL_INDEX_MIN)
          {
            l_index = SCREEN_L_CHANNEL_INDEX_MAX;
          }
        }
      
        channel_index_lr_set(SCREEN_L, l_index);
        g_app_param.l_ch_index = (uint8_t)l_index;

        bk9532_ch_index_set(BK953X_L, l_index);
        app_param_flash_update();
    }

    /**
     * SET
     */
    if(event == BUTTON_L_EVENT_SET_RELEASE)
    {
        if(L_SETTING_MODE != channel_settings_mode_get())
        {
            ir_tx_data_t ir_tx_data;

            ir_tx_data.cmd = CMD_FREQ;
            ir_tx_data.tx_data.freq_data.len = 3;
            ir_tx_data.tx_data.freq_data.band_type = g_app_param.band_type;
            ir_tx_data.tx_data.freq_data.region_band = g_app_param.region_band;
            ir_tx_data.tx_data.freq_data.freq_ch = g_app_param.l_ch_index;

            ir_tx_data.crc = ir_tx_data.tx_data.freq_data.band_type + ir_tx_data.tx_data.freq_data.region_band  \
                              + ir_tx_data.tx_data.freq_data.freq_ch;

            channel_settings_mode_set(L_SETTING_MODE);
            ir_tx_task_state_set(IR_TASK_WORK, (uint8_t *)&ir_tx_data, sizeof(ir_tx_data_t));
        }
        else
        {
            channel_settings_mode_set(EXIT_SET_MODE);
            ir_tx_task_state_set(IR_TASK_IDLE, NULL, 0);
        }
    }

    /**
     * LONG_SET
     */
}

static void app_evt_schedule(void * p_event_data)
{
    trace_debug("app_evt_schedule\n\r");
}

/**
 * @brief 参数初始化
 */
static void app_param_init(void)
{
    channel_index_lr_set(SCREEN_L, g_app_param.l_ch_index);
    channel_index_lr_set(SCREEN_R, g_app_param.r_ch_index);
}

/**
 * @brief 拨动开关开关机handler
 */
static void sys_power_sw_handler(void)
{
    gpio_input_get(&m_sys_power_sw_gpio, &g_sys_power_off);
    
    if(g_old_sys_power_off != g_sys_power_off)
    {
        if(g_sys_power_off)
        {
            lcd_off_status_set(true);
            bk953x_task_stage_set(BK953X_L, BK_STATE_IDLE);
            bk953x_task_stage_set(BK953X_R, BK_STATE_IDLE);
            trace_debug("TURN OFF POWER-SWITCH\n\r");
        }
        else
        {
            lcd_off_status_set(false);
            bk953x_task_stage_set(BK953X_L, BK_STAGE_INIT);
            bk953x_task_stage_set(BK953X_R, BK_STAGE_INIT);
            trace_debug("TURN ON POWER-SWITCH\n\r");
        }

        g_old_sys_power_off = g_sys_power_off;
    }
}

/**
 * @brief host-task
 */
static void host_loop_task(void)
{
  int err_code = 0;

  uint8_t l_rf_level = 0;
  uint8_t r_rf_level = 0;

  uint8_t l_af_level = 0;
  uint8_t r_af_level = 0;

  union user_spec_data l_user_data = {
    .byte = 0,
  };

  union user_spec_data r_user_data = {
    .byte = 0,
  };

  static uint64_t old_bk9532_ticks = 0;
  
  /* 15ms 刷新一次 af 和 rf */
  if( mid_timer_ticks_get() - old_bk9532_ticks > 100)
  {
    old_bk9532_ticks = mid_timer_ticks_get();

    bk953x_rf_rssi_get(BK953X_L, &l_rf_level);
    channel_rf_level_lr_set(SCREEN_L, l_rf_level);

    bk953x_rf_rssi_get(BK953X_R, &r_rf_level);
    channel_rf_level_lr_set(SCREEN_R, r_rf_level);

    bk953x_af_get(BK953X_L, &l_af_level);
    channel_af_level_lr_set(SCREEN_L, l_af_level);

    bk953x_af_get(BK953X_R, &r_af_level);
    channel_af_level_lr_set(SCREEN_R, r_af_level);
  }

  /**
   * 收到同步信息，退出红外发射模式
   */
  l_user_data.byte = bk953x_user_data_get(BK953X_L);
  if(l_user_data.user_data.chan_sync_flag)
  {
      channel_settings_mode_set(EXIT_SET_MODE);
      ir_tx_task_state_set(IR_TASK_IDLE, NULL, 0);
  }

  r_user_data.byte = bk953x_user_data_get(BK953X_R);
  if(r_user_data.user_data.chan_sync_flag)
  {
      channel_settings_mode_set(EXIT_SET_MODE);
      ir_tx_task_state_set(IR_TASK_IDLE, NULL, 0);
  }
}

int main(void)
{
  trace_init();

  /*greeting*/
  trace_info("\n\r\n\r\n\r\n\r");
  trace_info("       *** Welcome to the meeting-machine Project ***\n\r");
  trace_info("\n\r");

  TIMER_INIT();

  /**
   * delay 函数的初始化
   */
  mid_system_tick_init();

  app_param_flash_init();
  app_param_init();

  /* 提前初始化lcd硬件，预防上次掉电后马上插上，会显示旧数据 */
  lcd_hw_init();

  gpio_config(&m_sys_power_sw_gpio);
  do
  {
      lcd_black_light_enable(false);    //  关掉LCD显示
      lcd_ctrl_enable(false);
      delay_ms(50);
      gpio_input_get(&m_sys_power_sw_gpio, &g_sys_power_off);
      g_old_sys_power_off = g_sys_power_off;
  } while(g_sys_power_off);
  trace_info("POWER On\n\r");
  
  lcd_display_init();

  ir_tx_init();

  /* adc 相关 */
  adc_init();
  l_adc_button_event_handler_register(l_adc_button_handler);
  r_adc_button_event_handler_register(r_adc_button_handler);

  bk9532_lr_init();
  bk9532_ch_index_set(BK953X_L, g_app_param.l_ch_index);
  bk9532_ch_index_set(BK953X_R, g_app_param.r_ch_index);

  ad22650_lr_init();

  APP_SCHED_INIT(&m_app_scheduler, SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);

  trace_info("Start loop\n\r");
  while(1)
  {
    app_sched_execute(&m_app_scheduler);
    adc_button_loop_task();
    mid_timer_loop_task();
    bk953x_loop_task();
    lcd_display_loop_task();
    ir_tx_loop_task();
    host_loop_task();
    sys_power_sw_handler();
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif




