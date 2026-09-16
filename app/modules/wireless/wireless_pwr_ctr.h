#ifndef _WIRELESS_PWR_CTR_H
#define _WIRELESS_PWR_CTR_H

#define RX_CYCLE_NUM                           400
#define RX_OK_CYCLE_NUM                        3
#define RX_FAIL_THR                            1
#define RX_FAIL_SP_THR                         4
#define MAX_PWR_LEVEL                          0
#define MIN_PWR_LEVEL                          3
#define INC_FLAG                               1
#define DEC_FLAG                               0

typedef struct rx_info_t {
    uint16_t rx_cnt;
    uint8_t  rx_ok_cycle;
    uint16_t rx_fail_cnt;
    uint8_t  pwr_level;
    volatile uint8_t tx_cmd_flag;
}ws_rx_info_str_t;

void ws_pwr_ctr_init(void);
void ws_pwr_ctr_reset(uint8_t idx);

void ws_pwr_ctr_rx_process(uint8_t idx, uint8_t bfi);
void ws_pwr_ctr_tx_cmd_process(void);

#endif
