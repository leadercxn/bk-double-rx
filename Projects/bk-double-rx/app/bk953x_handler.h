#ifndef __BK953X_HANDLER_H
#define __BK953X_HANDLER_H

/**
 * 自定义用户数据结构 1B
 * 
 * 
 */

typedef enum
{
    BK953X_L,
    BK953X_R,
} bk953x_lr_e;

typedef enum
{
    BK_STAGE_INIT,
    BK_STAGE_NORMAL,
    BK_STAGE_SEARCHING,
    BK_STATE_IDLE,
    BK_STAGE_MAX,
} bk953x_task_stage_e;

typedef enum
{
    BK953X_CFG_FREQ,
    BK953X_CFG_RF_POWER,
    BK953X_CFG_MUTE,
    BK953X_CFG_I2S,
    BK953X_CFG_USER_DATA,
    BK953X_CFG_RF_RSSI,
    BK953X_CFG_AF_VOL,
    BK953X_CFG_RX_ID,
} bk953x_cfg_option_e;

union user_spec_data
{
  uint8_t byte;

  struct
  {
    uint8_t chan_sync_flag : 1; //信道同步完成标志
    uint8_t batt_level     : 2; //电池电平
    uint8_t reserve        : 5;
  } user_data;
};

int bk9532_lr_init(void);

void bk953x_loop_task(void);

void bk953x_task_stage_set(bk953x_lr_e lr, bk953x_task_stage_e stage);

int bk9532_ch_index_set(bk953x_lr_e lr, uint16_t chan_index);

int bk953x_rf_rssi_get(bk953x_lr_e lr, uint8_t *p_level);

int bk953x_af_get(bk953x_lr_e lr, uint8_t *p_level);

uint8_t bk953x_user_data_get(bk953x_lr_e lr);

#endif
