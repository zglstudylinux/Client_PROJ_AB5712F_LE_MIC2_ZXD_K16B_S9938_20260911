#include "include.h"
#include "wireless_pwr_ctr.h"

#if WIRELESS_CON_PWR_CTR

static ws_rx_info_str_t ws_rx_info_str[WIRELESS_CON_LINK_NB];

void ws_pwr_ctr_init(void)
{
    memset((u8 *)&ws_rx_info_str, 0 , WIRELESS_CON_LINK_NB*sizeof(ws_rx_info_str_t));
}

void ws_pwr_ctr_reset(uint8_t idx)
{
    memset((u8 *)&ws_rx_info_str[idx], 0 , sizeof(ws_rx_info_str_t));
}

AT(.com_text.mic_dec)
static uint8_t ws_pwr_ctr_pwr_change(uint8_t change_flag, uint8_t pwr_level)
{
    uint8_t change_pwr_level = pwr_level;

    if (change_flag == INC_FLAG) {
        if (change_pwr_level && (change_pwr_level != MAX_PWR_LEVEL)) {
            change_pwr_level -=1;
        }
    } else {
        if (change_pwr_level != MIN_PWR_LEVEL) {
            change_pwr_level +=1;
        }
    }

    return change_pwr_level;
}

AT(.com_text.mic_dec)
void ws_pwr_ctr_rx_process(uint8_t idx, uint8_t bfi)
{
    ws_rx_info_str_t *ws_rx_info_ptr = &ws_rx_info_str[idx];
    uint8_t pwr_level = ws_rx_info_ptr->pwr_level;

    ws_rx_info_ptr->rx_cnt++;
    if (bfi) {
        ws_rx_info_ptr->rx_fail_cnt++;
    }

    if (ws_rx_info_ptr->rx_cnt >= RX_CYCLE_NUM) {
        if (ws_rx_info_ptr->rx_fail_cnt > RX_FAIL_THR) {
            ///增加功率
            pwr_level = ws_pwr_ctr_pwr_change(INC_FLAG, pwr_level);
            if (ws_rx_info_ptr->rx_fail_cnt > RX_FAIL_SP_THR) {
                ///继续增加功率
                pwr_level = ws_pwr_ctr_pwr_change(INC_FLAG, pwr_level);
            }
            ws_rx_info_ptr->rx_ok_cycle = 0;
        } else {
            ws_rx_info_ptr->rx_ok_cycle++;
        }
        ws_rx_info_ptr->rx_cnt = 0;
        ws_rx_info_ptr->rx_fail_cnt = 0;
    }

    if (ws_rx_info_ptr->rx_ok_cycle >= RX_OK_CYCLE_NUM) {
        ws_rx_info_ptr->rx_ok_cycle = 0;
        ///降低功耗
        pwr_level = ws_pwr_ctr_pwr_change(DEC_FLAG, pwr_level);
    }

    if (pwr_level != ws_rx_info_ptr->pwr_level) {
        ws_rx_info_ptr->pwr_level = pwr_level;
        ws_rx_info_ptr->tx_cmd_flag = 1;
    }
}

void ws_pwr_ctr_tx_cmd_process(void)
{
    for (uint8_t i = 0; i < WIRELESS_CON_LINK_NB; i++) {
        ws_rx_info_str_t *ws_rx_info_ptr = &ws_rx_info_str[i];
        if (ws_rx_info_ptr->tx_cmd_flag) {
            ws_rx_info_ptr->tx_cmd_flag = 0;
            wireless_tx_pwr_ctr_cmd(i, ws_rx_info_ptr->pwr_level);
        }
    }
}

#endif
