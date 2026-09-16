#include "include.h"
#include "vusb_test.h"
//#include "driver_com.h"
//#include "driver_gpio.h"

#if BSP_TBOX_TEST_EN
static vusb_test_cmd_t vusb_test_tx_cmd;
static vusb_test_str_t vusb_test_str;
static huart_t huart0;

AT(.com_text.mic_emit.proc.wireless_uart)
void vusb_test_rx_callback(u8 *buf, u16 rx_len)
{
    vusb_test_cmd_t *huart_rx_ptr = &vusb_test_str.huart_test_cmd ;//(vusb_test_cmd_t *)buf;
    if ((huart_rx_ptr->vusb_head == VUSB_TEST_HEAD) && (vusb_test_str.huart_test_cmd.len == rx_len) && (rx_len >= 6)) {
        vusb_test_str.rx_kick_flag = 1;
    }
}

AT(.com_text.mic_emit.proc.wireless_huart)
bool vusb_test_huart_done(void)
{
    vusb_test_rx_callback((u8 *)&vusb_test_str.huart_test_cmd, huart_get_rxcnt());
    if(vusb_test_str.rx_kick_flag) {
        return true;
    }
    return false;
}

AT(.com_text.mic_emit.proc.wireless_huart)
void vusb_test_start_rsp_send(void)
{
    vusb_test_tx_cmd.vusb_head = VUSB_TEST_HEAD;
    vusb_test_tx_cmd.cmd = VUSB_TEST_START_RSP;
    vusb_test_tx_cmd.len = 6;
    vusb_test_tx_cmd.crc8 = bb_crc8((void *)&vusb_test_tx_cmd.cmd, vusb_test_tx_cmd.len - 4);
    huart_tx((u8 *)&vusb_test_tx_cmd, vusb_test_tx_cmd.len);
}

AT(.com_text.mic_emit.proc.wireless_huart)
void vusb_test_get_info_rsp_send(void)
{
    vusb_set_config_t *vusb_get_info_rsp = (vusb_set_config_t *)&vusb_test_tx_cmd.buf[0];

    vusb_test_tx_cmd.vusb_head = VUSB_TEST_HEAD;
    vusb_test_tx_cmd.cmd = VUSB_GET_INFO_RSP;
    vusb_get_info_rsp->freq_band = WIRELESS_CON_FREQ_BAND;
    vusb_get_info_rsp->tx_intv = WIRELESS_MIC_TX_INTERVAL;
    vusb_get_info_rsp->con_intv = WIRELESS_CON_INTERVAL*2;
    vusb_get_info_rsp->retry = WIRELESS_MIC_RETRY_NB;
    vusb_get_info_rsp->ws_feat = cfg_wireless_feat;
    vusb_get_info_rsp->codec[0] = cfg_wireless_codec[0];
    vusb_get_info_rsp->codec[1] = cfg_wireless_codec[1];
    vusb_get_info_rsp->config_rssi = WIRELESS_CON_RSSI_THR;
    u8 len = strlen(xcfg_cb.ws_name);
    vusb_get_info_rsp->ws_name_len = (len > VUSB_TEST_WS_NAME_LEN) ? VUSB_TEST_WS_NAME_LEN : len;
    memcpy(vusb_get_info_rsp->ws_name, xcfg_cb.ws_name, vusb_get_info_rsp->ws_name_len);
    vusb_test_tx_cmd.len = vusb_get_info_rsp->ws_name_len + (sizeof(vusb_test_cmd_t) - VUSB_TEST_WS_NAME_LEN);
    vusb_test_tx_cmd.crc8 = bb_crc8((void *)&vusb_test_tx_cmd.cmd, vusb_test_tx_cmd.len - 4);

    huart_tx((u8 *)&vusb_test_tx_cmd, vusb_test_tx_cmd.len);
}

AT(.com_text.mic_emit.proc.wireless_huart)
void vusb_set_config_req_do(uint8_t *buf, uint8_t buf_len)
{
    vusb_set_config_t *vusb_set_config_cmd = (vusb_set_config_t *)buf;

    cfg_bb_rf_freq_bands = vusb_set_config_cmd->freq_band;
    cfg_wireless_tx_interval = vusb_set_config_cmd->tx_intv;
    cfg_wireless_con_interval = vusb_set_config_cmd->con_intv;
    cfg_wireless_tx_retry = vusb_set_config_cmd->retry;
    cfg_wireless_feat = vusb_set_config_cmd->ws_feat;
    cfg_discon_auto_pwroff = vusb_set_config_cmd->discon_auto_pwroff;
    cfg_wireless_codec[0] = vusb_set_config_cmd->codec[0];
    cfg_wireless_codec[1] = vusb_set_config_cmd->codec[1];
    vusb_test_str.config_rssi = vusb_set_config_cmd->config_rssi;
    vusb_test_str.ws_name_len = vusb_set_config_cmd->ws_name_len;
    memcpy((u8 *)&vusb_test_str.ws_name[0], (u8 *)&vusb_set_config_cmd->ws_name[0], vusb_set_config_cmd->ws_name_len);
    vusb_test_str.ws_config_sucess_flag = 1;
}

AT(.com_text.mic_emit.proc.wireless_huart)
void vusb_test_set_config_rsp_send(void)
{
    vusb_test_tx_cmd.vusb_head = VUSB_TEST_HEAD;
    vusb_test_tx_cmd.cmd = VUSB_SET_CONFIG_RSP;
    vusb_test_tx_cmd.len = 6;
    vusb_test_tx_cmd.crc8 = bb_crc8((void *)&vusb_test_tx_cmd.cmd, vusb_test_tx_cmd.len - 4);

    huart_tx((u8 *)&vusb_test_tx_cmd, vusb_test_tx_cmd.len);
}

AT(.com_text.at_vusb)
void vusb_test_rx_process(void)
{
    vusb_test_cmd_t *huart_rx_ptr = &vusb_test_str.huart_test_cmd;
    if (vusb_test_str.rx_kick_flag) {
        print_r(huart_rx_ptr, huart_rx_ptr->len);
        if (huart_rx_ptr->crc8 == bb_crc8((void *)&huart_rx_ptr->cmd, huart_rx_ptr->len - 4)) {
            memset((u8 *)&vusb_test_tx_cmd, 0, sizeof(vusb_test_tx_cmd));
            switch(huart_rx_ptr->cmd) {
                case VUSB_TEST_START_REQ:
                    vusb_test_str.vusb_start_test_flag = 1;
                    vusb_test_start_rsp_send();
                    break;
                case VUSB_GET_INFO_REQ:
                    vusb_test_get_info_rsp_send();
                    break;
                case VUSB_SET_CONFIG_REQ:
                    if (vusb_test_str.vusb_start_test_flag) {
                        vusb_set_config_req_do(huart_rx_ptr->buf, huart_rx_ptr->len);
                        vusb_test_set_config_rsp_send();
                        ///延时一会让他能发的出去
                        delay_5ms(10);
                    }
                    break;
                default:
                    break;
            }
        }
        vusb_test_str.rx_kick_flag = 0;

    }
}

uint8_t vusb_test_is_sucess(void)
{
    return vusb_test_str.ws_config_sucess_flag;
}

uint8_t vusb_test_rssi_get(void)
{
    return vusb_test_str.config_rssi;
}

uint8_t vusb_test_name_get(u8 *le_name)
{
    memcpy(le_name, (u8 *)&vusb_test_str.ws_name[0], vusb_test_str.ws_name_len);

    return vusb_test_str.ws_name_len;
}

void vusb_test_init(void)
{
    memset((u8 *)&vusb_test_tx_cmd, 0, sizeof(vusb_test_tx_cmd));
    memset((u8 *)&vusb_test_str, 0, sizeof(vusb_test_str_t));
    if (UART0_PRINTF_SEL == PRINTF_PB3) {
        FUNCMCON0 = (0xf << 12) | (0xf << 8);           //clear uart0 mapping;
    }
    RTCCON |= BIT(6);                               //VUSB insert reset system enable
    vusb_reset_set_time(2);                         //VUSB reset delay time
    vusb_test_huart_init();
}

void vusb_test_exit(void)
{
    huart_exit();
    if (UART0_PRINTF_SEL == PRINTF_PB3) {
        uart0_mapping_sel();
    } else if (xcfg_cb.bled_io_sel == 19) {
#if BSP_LED_EN
        bled_init();
        bled_set_off();
#endif // BSP_LED_EN
    }
}

AT(.com_text.at_vusb)
void bsp_vusb_test_process(void)
{
    ///在这里做检测，检测50ms内是否收到vusb_test_start命令，是的话预留1s做通信
    ///如果1s内收到正确的产测命令则唤醒且无视按键唤醒及充电流程，否则正常跑
    u16 test_start_to = VUSB_TEST_START_TO;

    vusb_test_init();
    while(test_start_to) {
        delay_5ms(1);
        vusb_test_rx_process();
        test_start_to--;
    }
    if (vusb_test_str.vusb_start_test_flag) {
        test_start_to = VUSB_SET_CONFIG_TO;
        while(test_start_to) {
           delay_5ms(1);
            vusb_test_rx_process();
            test_start_to--;
            if (vusb_test_is_sucess()) {
                break;
            }
        }
    }

    vusb_test_exit();

}

AT(text.huart)
void vusb_test_huart_init(void)
{
    my_printf("%s %d\n",__func__,__LINE__);
//    memset(&hci_rx_buffer, 0x00, sizeof(hci_rx_buffer));
    memset(&huart0, 0x00, sizeof(huart0));

    huart0.rx_port = HUART_TR_PB3;
    huart0.tx_port = HUART_TR_PB3;
//    huart0.txisr_en = 0;
//    huart0.rxbuf_loop = 1;
    huart0.rxisr_en = 1;
    huart0.txisr_en = 0;
    huart0.rxbuf      = (u8 *)&vusb_test_str.huart_test_cmd;
    huart0.rxbuf_size =  sizeof(vusb_test_cmd_t);

    huart_init(&huart0, VUSB_TEST_BAUD);
}

#endif
