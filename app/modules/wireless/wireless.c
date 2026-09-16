#include "include.h"
#include "api.h"

#if LE_FCC_TEST_EN
    #define WORK_MODE           MODE_FCC_TEST
#elif LE_BQB_RF_EN
    #define WORK_MODE           MODE_BQB_RF_BLE
#else
    #define WORK_MODE           MODE_NORMAL
#endif

#define D2A_CODEC               (WIRELESS_CON_CODEC_SEL | WS_CODEC_MONO)
#define A2D_CODEC               0

//link
uint8_t cfg_wireless_cntl_bit       = (WIRELESS_CON_CRC_EN*0x1);
uint8_t cfg_wireless_full_duplex    = 0;
uint8_t cfg_le_rssi_thr             = WIRELESS_CON_RSSI_THR;
uint8_t cfg_wireless_link_nb        = WIRELESS_CON_LINK_NB;
uint8_t cfg_wireless_d2a_tx_size    = WIRELESS_MIC_TX_BUFFER_SIZE;                      //传输封包大小
uint8_t cfg_wireless_a2d_tx_size    = WIRELESS_SPK_TX_BUFFER_SIZE;
uint8_t cfg_wireless_con_interval   = WIRELESS_CON_INTERVAL*WIRELESS_CON_LINK_NB;       //连接interval（V1的传输interval），单位1.25ms
uint8_t cfg_wireless_tx_interval    = WIRELESS_MIC_TX_INTERVAL;                         //V2、V3的传输interval，单位1.25ms
uint8_t cfg_wireless_tx_retry       = WIRELESS_MIC_RETRY_NB;                            //V2、V3的传输次数
uint8_t cfg_wireless_tx_comb_nb     = WIRELESS_MIC_COMB_NB;                             //传输封包组合数量
uint8_t cfg_wireless_a2d_tx_retry   = WIRELESS_SPK_RETRY_NB;                            //V2、V3、V4的SPK的传输次数
uint8_t cfg_wireless_a2d_tx_comb_nb = WIRELESS_SPK_COMB_NB;
uint16_t cfg_wireless_feat          = ((WIRELESS_CON_VERS<<8)&FEAT_VERS) | (FEAT_D2A*WIRELESS_MIC_TX_EN) | (FEAT_HOP_V1) | (FEAT_A2D*WIRELESS_SPK_TX_EN) | (FEAT_BONDING*WIRELESS_CON_BONDING_EN) | (WIRELESS_CON_LINK_NB&0xf);
uint16_t cfg_wireless_ext_feat      = ((EXT_FEAT_ADV_EXT*0) | (((WIRELESS_CON_VERS>>3)<<1)&EXT_FEAT_VERS));
uint16_t cfg_wireless_d2a_enc_us    = (WIRELESS_MIC_SOFT_DRC_DELAY + WIRELESS_MIC_ENC_MAX_US + WIRELESS_MIC_ECHO_DELAY + WIRELESS_MIC_MAGIC_DELAY +
                                       WIRELESS_MIC_SRC_DELAY + WIRELESS_MIC_AGC_DELAY + WIRELESS_MIC_EQ_DRC_DELAY + WIRELESS_MIC_ROOM_REVERB_DELAY + WIRELESS_MIC_YLCRN_16K_DELAY +
                                       WIRELESS_ALLPASS_FILTER_CHANGE_DELAY + WIRELESS_FREQ_SHIFT2_DELAY + WIRELESS_MIC_PLAT_REVERB_DELAY + WIRELESS_MIC_DNR_FRE_DELAY);
uint16_t cfg_wireless_d2a_dec_us    = WIRELESS_MIC_DEC_MAX_US + ADAPTER_MIX_DRC_DELAY + ADAPTER_SOFT_EQ_DELAY;
uint16_t cfg_wireless_a2d_enc_us    = (WIRELESS_MIC_SOFT_DRC_DELAY + WIRELESS_MIC_ENC_MAX_US + WIRELESS_MIC_ECHO_DELAY + WIRELESS_MIC_MAGIC_DELAY +
                                       WIRELESS_MIC_SRC_DELAY + WIRELESS_MIC_AGC_DELAY + WIRELESS_MIC_EQ_DRC_DELAY + WIRELESS_MIC_ROOM_REVERB_DELAY +
                                       WIRELESS_ALLPASS_FILTER_CHANGE_DELAY + WIRELESS_FREQ_SHIFT2_DELAY + WIRELESS_MIC_PLAT_REVERB_DELAY);;
uint16_t cfg_wireless_a2d_dec_us    = WIRELESS_MIC_DEC_MAX_US+ADAPTER_MIX_DRC_DELAY+ADAPTER_SOFT_EQ_DELAY;

uint8_t cfg_wireless_con_id         = WIRELESS_CON_CON_ID_SEL;   //配对码id值，接收端通过动态切换广播id值，达到连接不同的id值设备

uint8_t cfg_wireless_codec[2]       = {
    D2A_CODEC,      //D2A Codec
    A2D_CODEC,      //A2D Codec
};


uint8_t cfg_wireless_normal_ble_en = ADAPTER_NORMAL_LE_EN;

uint8_t cfg_discon_auto_pwroff      = 0;

//rf
uint8_t cfg_bb_rf_freq_bands        = WIRELESS_CON_FREQ_BAND; //0:2402M~2480M  1:2200M~2278M  2:2320M~2398M  3:2500M~2578M  4: 2482M~2560M
uint8_t cfg_bb_rf_def_txpwr         = 0;        //降低预置参数RF发射功率，单位0.5dbm
uint8_t cfg_bb_rf_gfsk_pwr          = 127;      //调节GFSK TXPWR，范围：90~127
uint8_t cfg_ble_page_txpwr          = 0;        //降低组队RF发射功率，单位3dbm
uint8_t cfg_ble_page_rssi_thr       = 0;        //设置组队范围rssi

//stack
uint8_t cfg_bt_work_mode            = WORK_MODE;
uint8_t cfg_bt_rf_def_txpwr         = 0;

bool cfg_wireless_broadcast_ch_scan_en = 1;
//设备组队时，首先用设备名称le_name匹配，在setting中可以设置
//同时也检查设备的单/双工、编解码特性，特性不同不允许组队，参见cfg_wireless_feat和cfg_wireless_codec

#if WIRELESS_MIC_2TNR_EN
uint8_t cfg_wireless_2tnr_nb        = WIRELESS_CON_2TNR_NB;
#endif

AT(.com_text.mic_dec)
void wireless_channel_status(u8 idx, u8 chidx, s8 rssi, u8 bfi, u8 chstatus)
{
    if(bfi || chstatus != 0) {
        if(rssi > -80 || (rssi > -85 && chstatus == 250)) {
            ble_con_channel_assess(chidx, false);
        }
    } else {
        ble_con_channel_assess(chidx, true);
    }

    wireless_dump_set_rx_status(idx, chidx, rssi, (bfi || chstatus != 0));
}

//------------------------------------------------------------------------------------------
AT(.text.wireless.cb)
int wireless_get_local_name(uint8_t *ws_name)
{
#if BSP_TBOX_TEST_EN
    if (vusb_test_is_sucess()) {
        return vusb_test_name_get(ws_name);
    }
#endif

    int len;
    len = strlen(xcfg_cb.ws_name);

    memcpy(ws_name, xcfg_cb.ws_name, len);

    return len;
}

AT(.text.wireless.cb)
void wireless_get_local_bd_addr(u8 *addr)
{
    memcpy(addr, xcfg_cb.bt_addr, 6);
}

AT(.text.wireless.cb)
void wireless_con_set_chmap_cb(u8 idx, const u8 *chmap)
{
    wireless_dump_set_chmap_cb(idx, chmap);
}

//------------------------------------------------------------------------------------------
void bt_get_link_info_flash(void *buf, u16 addr, u16 size)
{
//    printf("bt_read: %04x,%04x, %08lx\n", addr, size, BT_CM_PAGE(addr));
    if ((addr + size) <= PAGE_DATA_SIZE) {
        cm_read(buf, BT_CM_PAGE(addr), size);
    }
//    print_r(buf, size);
}

void bt_put_link_info_flash(void *buf, u16 addr, u16 size)
{
//    printf("bt_write: %04x,%04x, %08lx\n", addr, size, BT_CM_PAGE(addr));
//    print_r(buf, size);
    if ((addr + size) <= PAGE_DATA_SIZE) {
        cm_write(buf, BT_CM_PAGE(addr), size);

    }
}

void bt_get_link_info(void *buf, u16 addr, u16 size)
{
//#if TEST_MODE_BT_INFO
//    if (bt_is_test_mode()) {
//        bt_get_link_info_ram(buf,addr,size);
//    } else
//#endif
    {
        cm_read(buf, BT_CM_PAGE(addr), size);
    }
}

void bt_put_link_info(void *buf, u16 addr, u16 size)
{
//#if TEST_MODE_BT_INFO
//    if (bt_is_test_mode()) {
//        bt_put_link_info_ram(buf,addr,size);
//    } else
//#endif
    {
        cm_write(buf, BT_CM_PAGE(addr), size);
    }
}

void bt_sync_link_info(void)
{
    cm_sync();
}


u8 con_req_addr[6];

#if WIRELESS_MIC_2TNR_EN
struct con_info_t{
    uint8_t save_addr1[6];
    uint8_t save_addr2[6];
    uint8_t save_addr_sta;
    uint8_t req_con_sta;            //当前状态下需要请求连接的链路
} con_info_2tnr;

bool wireless_scan_2tnr_can_deal(uint8_t *addr, uint8_t addr_type, uint8_t *adv_ext_data)
{
    bool deal_flag = 0;

    if (wireless_role_is_adapter()) {
        if ((wireless_cb.connected_sta == 0) && (con_info_2tnr.req_con_sta&BIT(0))) {
            memcpy(con_info_2tnr.save_addr1, addr, 6);
        }

        if (((wireless_cb.connected_sta&BIT(0)) == 0) && (con_info_2tnr.req_con_sta&BIT(0)) && !memcmp(con_info_2tnr.save_addr1, addr, 6)) {
            func_adapter_set_create_con();
        }

        if ((wireless_cb.connected_sta == BIT(0)) && (con_info_2tnr.req_con_sta&BIT(1)) && !memcmp(con_info_2tnr.save_addr2, addr, 6)) {
            func_adapter_set_create_con();
        }

        deal_flag = 1;
    }

    return deal_flag;
}

void wireless_second_emit_rep_cb(uint8_t *addr, uint8_t con_sta, uint8_t status, uint8_t index)
{
    printf("wireless_second_emit_rep_cb, con_sta = %d, status = %d, index = %d, wireless_cb.connected_sta = %d\n", con_sta, status, index, wireless_cb.connected_sta);
    printf("addr: ");
    print_r(addr, 6);

    if (con_sta) {
        if (wireless_cb.connected_sta == BIT(0)) {
            memcpy(con_info_2tnr.save_addr2, addr, 6);
            con_info_2tnr.req_con_sta |= BIT(1);
        } else if (wireless_cb.connected_sta == BIT(1)) {
            memcpy(con_info_2tnr.save_addr1, addr, 6);
            con_info_2tnr.req_con_sta |= BIT(0);
        } else if (wireless_cb.connected_sta == (BIT(0) | BIT(1))) {
            ///收到断开指令后还没来得及断开或蓝牙线程没那么快回调处理完，马上又发来了连接
            if (con_info_2tnr.req_con_sta == BIT(0)) {
                memcpy(con_info_2tnr.save_addr2, addr, 6);
                con_info_2tnr.req_con_sta |= BIT(1);
            } else if (con_info_2tnr.req_con_sta == BIT(1)) {
                memcpy(con_info_2tnr.save_addr1, addr, 6);
                con_info_2tnr.req_con_sta |= BIT(0);
            }
        }
    } else {
//        if (wireless_cb.connected_sta == (BIT(0) | BIT(1))) {
            if (!memcmp(con_info_2tnr.save_addr1, addr, 6)) {
                con_info_2tnr.req_con_sta &= ~BIT(0);
            } else if (!memcmp(con_info_2tnr.save_addr2, addr, 6)) {
                con_info_2tnr.req_con_sta &= ~BIT(1);
            }

            if (status == 0x08 || status == 0x3e) {
                ble_disconnect_req(index);
            }
//        }
    }
}

void wireless_2tnr_info_init(void)
{
    memset((u8 *)&con_info_2tnr, 0, sizeof(struct con_info_t));
    con_info_2tnr.req_con_sta = BIT(0);
}

///wireless adapter断开后，清除对应的连接消息
void wireless_2tnr_info_clr(void)
{
    wireless_2tnr_info_init();
}
#endif

//发送ble_connect_req消息之后，库获取连接地址的回调函数
AT(.text.wireless.cb)
bool ble_get_con_req_addr_cb(uint8_t *addr)
{
    memcpy(addr, con_req_addr, 6);
    return true;
}

//发送ble_scan_set_enable消息之后，底层收到数据包的回调函数
//返回值：0=错误的地址，继续扫描；1=地址正确，自动关闭扫描，可发起连接请求
AT(.text.wireless.cb)
bool ble_scan_rx_rep_cb(uint8_t *addr, uint8_t addr_type, uint8_t *adv_ext_data)
{
    printf("addr: ");
    print_r(addr, 6);

    ///准备发起连接，关闭scan
    memcpy(con_req_addr, addr, 6);

#if WIRELESS_MIC_2TNR_EN
    if (wireless_scan_2tnr_can_deal(addr, addr_type, adv_ext_data)) {
    } else
#endif
   {
        func_device_set_create_con();
   }

    return true;
}

#if WIRELESS_MIC_BROADCAST_EN
AT(.text.wireless.cb)
bool ble_ws_per_scan_rx_cb(uint8_t *addr, uint8_t addr_type, uint8_t *adv_ext_data)
{
    printf("addr: ");
    print_r(addr, 6);

    ble_per_scan_set_enable(0);
    ///发起sync，去同步刚刚scan到的设备
    ble_per_sync_set_enable(1);

    return true;
}
#endif

//addr: 连接的设备地址
//timeout: 超时时间（单位ms，0xffff时一直连接不超时）
AT(.text.wireless.api)
void ble_create_con_for_addr(uint8_t *addr, uint16_t timeout)
{
    memcpy(con_req_addr, addr, 6);
    ble_connect_req(timeout);   //发消息后，通过回调函数ble_get_con_req_addr_cb获取连接地址
}
AT(.text.wireless.init)
void printf_connect_message(void)
{
    my_printf("<TX_INTERVAL>         %d\n",cfg_wireless_tx_interval);
    my_printf("<CON_INTERVAL>        %d\n",cfg_wireless_con_interval);
    my_printf("<WIRELESS_FEAT>       %d\n",cfg_wireless_feat);
    my_printf("<WIRELESS_CODEC>      %d\n",cfg_wireless_codec[0]);
    my_printf("<FREQ_BAND>           %d\n",cfg_bb_rf_freq_bands);
    my_printf("<RETRY>               %d\n",cfg_wireless_tx_retry);
    //my_printf("<DISCON_AUTO_PWROFF>  %d\n",cfg_discon_auto_pwroff);
    my_printf("<CONFIG_RSSI>         %d\n",cfg_le_rssi_thr);
    my_printf("<WS_NAME>             %s\n",xcfg_cb.ws_name);
}

AT(.text.wireless.init)
void wireless_init(void)
{
    //更新配置工具的设置
    cfg_bb_rf_def_txpwr = xcfg_cb.bt_rf_pwrdec;

#if BSP_TBOX_TEST_EN
    u8 vusb_config_rssi = vusb_test_rssi_get();
    if (vusb_config_rssi) {
        cfg_le_rssi_thr = vusb_config_rssi;
    }
#endif

#if WIRELESS_MIC_2TNR_EN
    wireless_2tnr_info_init();
    cfg_wireless_ext_feat |= ((WIRELESS_CON_2TNR_NB << 4)&EXT_FEAT_2TNR_NB);
#endif

    wireless_dump_init();

	wireless_con_adapter_init();
    wireless_con_device_init();
    printf_connect_message();
    bt_setup();
}

AT(.text.wireless.adapter)
void wireless_adapter_init(void)
{
#if (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3S)
#if WIRELESS_CON_FULL_DUPLEX_EN
    lc3s_enc_init(WIRELESS_SPK_SAMPLE_RATE_SELECT, WIRELESS_SPK_SAMPLES_SELECT);
#endif
    lc3s_dec_init(WIRELESS_MIC_SAMPLE_RATE_SELECT,WIRELESS_MIC_SAMPLES_SELECT);
#if !DEVICE_STEREO_EN || WIRELESS_MIC_STEREO                                                               //修包发射端初始化一次即可
    plc_soft_init(0, WIRELESS_MIC_SAMPLES_SELECT);
    plc_soft_init(1, WIRELESS_MIC_SAMPLES_SELECT);
#endif
#elif (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3B)
    lc3b_dec_init(WIRELESS_MIC_SAMPLE_RATE_SELECT,WIRELESS_MIC_SAMPLES_SELECT);
    plc_soft_60_init(0, WIRELESS_MIC_SAMPLES_SELECT);
#endif

#if ADAPTER_MIX_DRC_EN
    mix_drc_init();
#endif

#if WIRELESS_CON_PWR_CTR
    ws_pwr_ctr_init();
#endif

#if WIRELESS_SPK_TX_EN
#if ADAPTER_AUX_TX_EN
    spk_start();
#endif
#if ADAPTER_AUDIO_FADE_IN_EN
    soft_gain_init();
#endif
#endif
}

AT(.text.wireless.device)
void wireless_device_init(void)
{
#if (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3S)
    lc3s_enc_init(WIRELESS_MIC_SAMPLE_RATE_SELECT, WIRELESS_MIC_SAMPLES_SELECT);
#if WIRELESS_CON_FULL_DUPLEX_EN
    lc3s_dec_init(WIRELESS_SPK_SAMPLE_RATE_SELECT, WIRELESS_SPK_SAMPLES_SELECT);

    plc_soft_init(0, WIRELESS_SPK_SAMPLES_SELECT);
    plc_soft_init(1, WIRELESS_SPK_SAMPLES_SELECT);
#endif
#elif (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3B)
    lc3b_enc_init(WIRELESS_MIC_SAMPLE_RATE_SELECT, WIRELESS_MIC_SAMPLES_SELECT);
#endif

#if WIRELESS_MIC_TX_EN && !DEVICE_STEREO_EN && !WIRELESS_MIC_STEREO
    mic_start();
#endif
#if DEVICE_STEREO_EN || WIRELESS_MIC_STEREO
    spk_start();
#endif
}

AT(.text.wireless.adapter)
void wireless_adapter_exit(u8 idx, u8 con_sta)
{
    if(con_sta == 0) {
#if ADAPTER_I2S_IN_OUT_EN
        i2s_audio_in_out_exit();
#endif

#if ADAPTER_AUX_TX_EN || DEVICE_STEREO_EN
        spk_stop();
#endif

#if WIRELESS_MIC_2TNR_EN
        wireless_2tnr_info_clr();
#endif
    }

#if WIRELESS_MIC_TX_EN
    mic_dec_reset(idx);
#endif

#if (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3S)
    plc_soft_exit(idx);
#elif (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3B)
    plc_soft_60_exit(idx);
#endif


    wireless_cmd_reset(idx);
#if WIRELESS_CON_PWR_CTR
    ws_pwr_ctr_reset(idx);
#endif

#if (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3S)
    lc3s_dec_exit(idx);
#elif (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3B)
    lc3b_dec_exit(idx);
#endif
}

AT(.text.wireless.device)
void wireless_device_exit(void)
{
#if WIRELESS_MIC_TX_EN && !WIRELESS_MIC_STEREO
    mic_stop();
    mic_enc_reset();
#endif
#if WIRELESS_MIC_STEREO
    spk_stop();
#endif
#if WIRELESS_CON_FULL_DUPLEX_EN
    plc_soft_exit(0);
    plc_soft_exit(1);
    lc3s_dec_exit(0);
    lc3s_dec_exit(1);
#endif

    wireless_cmd_reset(0);
}

#if DEVICE_INTERPHONE_EN
void wireless_interphone_init(void)
{
    wireless_adapter_init();
    wireless_device_init();
}

void wireless_interphone_exit(u8 idx, u8 con_sta)
{
    wireless_adapter_exit(idx, con_sta);
    wireless_device_exit();
}
#endif
