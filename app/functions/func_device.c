#include "include.h"
#include "func.h"
#include "func_device.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif // TRACE_EN

enum {
    DEVICE_STA_INIT_IDLE,
    DEVICE_STA_INIT_CONNECT,
    DEVICE_STA_START_ACTION,
    DEVICE_STA_DELAY,

    DEVICE_STA_IDLE,
    DEVICE_STA_SCAN,

    DEVICE_STA_BROADCAST_TX,
    DEVICE_STA_BROADCAST_IDLE,

    DEVICE_STA_INTERPHONE_SCAN,
    DEVICE_STA_INTERPHONE_SCAN_TO,
    DEVICE_STA_INTERPHONE_ADV,
    DEVICE_STA_INTERPHONE_ADV_TO,
    DEVICE_STA_INTERPHONE_DELAY,

    DEVICE_STA_INIT_W4_CONNECT,
    DEVICE_STA_ADV,
    DEVICE_STA_W4_TO,
};

static struct {
    uint8_t init_flag;
    uint8_t init_state;
    uint8_t con_state;
    uint32_t ticks;
    uint tick_delay;
    bool create_con_flag;
    uint8_t adv_en;
    bool con_cmp_flag;
    uint8_t delay_state;

#if WIRELESS_MIC_2TNR_EN
    bool adv_type_flag;
    uint adv_tick_delay;
    uint32_t adv_ticks;
#endif
} device_cb;

//无线MIC发射端处理流程
//MIC_ADC -> encoder_prio_trans_audio_input --> mic_enc_audio_input -> wireless_d2a_put_tx_frame -> memcpy(txbuf, enc_buf, size) -> DEVICE_TX
//                                          |-> dac0_out_audio_input -> DAC
#if WIRELESS_MIC_TX_EN
static ws_link_list_t mic_tx_cfg_tbl[] = {
/*  模块类型，                  使能，          初始化，                  输入接口，                       设置输出*/
#if DEVICE_STEREO_EN || WIRELESS_MIC_STEREO
    {MIC_TYPE,                  1,              spk_init,                 NULL,                            spk_audio_output_callback_set},
#else
    {MIC_TYPE,                  1,              mic_init,                 NULL,                            mic_audio_output_callback_set},
#endif
#if WIRELESS_MIC_I2S_INPUT_EN
    {I2S_IN_TYPE,              1,              i2s_audio_in_init,        i2s_audio_in_input,              i2s_audio_input_callback_set},                    //I2S传输
#endif
    {ENCODER_PRIO_TRANS_TYPE,   1,              encoder_prio_trans_init,  encoder_prio_trans_audio_input,  encoder_prio_trans_audio_output_callback_set},    //缓存到编码缓冲区，并转到低优先级处理
#if WIRELESS_MIC_ECHO_EN
    {ECHO_TYPE,                 1,              echo_audio_init,          echo_audio_input,                echo_audio_output_callback_set},
#endif
#if MIC_DRC_EN
    {MIC_DRC,                   1,              drc_mic_init,             drc_mic_audio_input,             drc_mic_audio_output_callback_set},
#endif
#if WIRELESS_MIC_EQ_DRC_EN
    {MIC_EQ_DRC_TYPE,           1,              mic_eq_drc_init,          mic_eq_drc_audio_input,          mic_eq_drc_audio_output_callback_set},
#endif
#if WIRELESS_MIC_DNR_FRE_EN
    {DNR_FRE_TYPE,              1,              dnr_fre_mic_init,         dnr_fre_mic_audio_input,          dnr_fre_mic_output_callback_set},               //EQ
#endif
#if WIRELESS_MIC_MAGIC_EN
    {MAGCI_PITCH_SHIFT_TYPE,    1,              magic_audio_init,         magic_audio_input,               magic_audio_output_callback_set},
#endif
#if WIRELESS_VOICE_CHANGE_V2_EN
    {VOICE_CHANGE_V2_TYPE,      1,              voice_change_v2_mic_init,     voice_change_v2_mic_audio_input,voice_change_v2_mic_output_callback_set},
#endif
#if WIRELESS_MIC_DNN_L1_EN
    {DNN_L1_TYPE,               1,              dnn_L1_mic_init,          dnn_L1_mic_audio_input,          dnn_L1_mic_output_callback_set},
#endif
#if WIRELESS_MIC_YLCRN_L3_EN
    {YLCRN_L3_TYPE,             1,              ylcrn_L3_mic_init,          ylcrn_L3_mic_audio_input,       ylcrn_L3_mic_output_callback_set},
#endif
#if WIRELESS_MIC_YLCRN_L2_EN
    {YLCRN_L2_TYPE,             1,              ylcrn_L2_mic_init,          ylcrn_L2_mic_audio_input,       ylcrn_L2_mic_output_callback_set},
#endif
#if WIRELESS_MIC_YLCRN_HOWLING_EN
    {YLCRN_HOWLING_TYPE,         1,              ylcrn_howling_mic_init,     ylcrn_howling_mic_audio_input,  ylcrn_howling_mic_output_callback_set},
#endif
#if WIRELESS_MIC_PHASE_ROTATION_EN
    {PHASE_ROTATION_TYPE,       1,              phase_rotation_mic_init,     phase_rotation_mic_audio_input,  phase_rotation_mic_output_callback_set},
#endif
#if WIRELESS_MIC_GTCRN_QMF_HOWLING_EN
    {GTCRN_QMF_HOWLING_TYPE,    1,              gtcrn_qmf_howling_mic_init, gtcrn_qmf_howling_mic_audio_input, gtcrn_qmf_howling_mic_output_callback_set},
#endif
#if WIRELESS_MIC_HOWLING_DNN_EN
    {HOWLING_AIMASK_TYPE,       1,              howling_dnn_mic_init,       howling_dnn_mic_audio_input,  howling_dnn_mic_output_callback_set},
#endif
#if WIRELESS_MIC_AGC_EN
    {AGC_MIC_TYPE,              1,              agc_audio_init,           agc_audio_input,                 agc_audio_output_callback_set},
#endif
#if WIRELESS_MIC_DNN_L3_EN
    {DNN_L3_TYPE,               1,              dnn_L3_mic_init,          dnn_L3_mic_audio_input,          dnn_L3_mic_output_callback_set},
#endif

#if WIRELESS_MIC_AINS4_EN
    {AINS_TYPE,                 1,              ains4_mic_init,           ains4_mic_audio_input,           ains4_mic_output_callback_set},
#endif

#if WIRELESS_MIC_NOTCH_L2_EN
    {HOWLING_NOTCH_TYPE,        1,              howling_notch_L2_mic_init,howling_notch_L2_mic_audio_input,howling_notch_L2_mic_output_callback_set},
#endif

#if WIRELESS_MIC_ROOM_REVERB_EN
    {ROOM_REVERB_TYPE,          1,              room_reverb_audio_init,   room_reverb_audio_input,         room_reverb_audio_output_callback_set},
#endif

#if WIRELESS_MIC_PLAT_REVERB_EN
    {PLAT_REVERB_TYPE,          1,              plat_reverb_audio_init,   plat_reverb_audio_input,         plat_reverb_audio_output_callback_set},
#endif

#if WIRELESS_MIC_FREQ_SHIFT2_EN
    {FREQ_SHIFT2_TYPE,          1,              freq_shift2_mic_init,     freq_shift2_mic_audio_input,     freq_shift2_mic_output_callback_set},
#endif

#if WIRELESS_ALLPASS_FILTER_CHANGE_EN
    {ALLPASS_FILTER_TYPE,          1,           allpass_filter_init,      allpass_filter_audio_input,      allpass_filter_audio_output_callback_set},
#endif
#if WIRELESS_INTERPHONE_AEC_EN
    {AEC_ALG_TYPE,              1,              wireless_mic_aec_init,    aec_mic_audio_input,             aec_output_callback_set},
#endif
#if WIRELESS_MIC_SRC_EN
    {SRC_TYPE,                  1,              src0_init,                src0_audio_input,                src0_audio_output_callback_set},
#endif
#if WIRELESS_MIC_YLCRN_16K_EN
    {YLCRN_16K_TYPE,            1,              ylcrn_16k_mic_init,       ylcrn_16k_mic_audio_input,          ylcrn_16k_mic_output_callback_set},
#endif
#if WIRELESS_MIC_DAC_OUTPUT_EN && !DEVICE_INTERPHONE_EN
    {DAC_OUT_TYPE,              1,              dac1_out_init,            dac1_out_audio_input,            dac1_out_audio_output_callback_set},
#endif
    {MIC_ENC_TYPE,              1,              mic_enc_init,             mic_enc_audio_input,             mic_enc_audio_output_callback_set},
};
#endif

#if WIRELESS_SPK_TX_EN && !DEVICE_INTERPHONE_EN
static ws_link_list_t spk_rx_cfg_tbl[] = {
/*  模块类型，                  使能，          初始化，                  输入接口，                       设置输出*/
    {DECODER_PRIO_TRANS_TYPE,   1,              decoder_prio_trans_init,  NULL,                            decoder_prio_trans_audio_output_callback_set},
    {SPK_DEC_TYPE,              1,              spk_dec_init,             spk_dec_audio_input,             spk_dec_audio_output_callback_set},             //PCM解码
#if WIRELESS_SPK_DAC_OUTPUT_EN
    {DAC_OUT_TYPE,              1,              dac0_out_init,            dac0_out_audio_input,            dac0_out_audio_output_callback_set},
#endif
};
#endif

#if DEVICE_INTERPHONE_EN
//无线MIC对讲机处理流程
static ws_link_list_t mic_rx_cfg_tbl[] = {
/*  模块类型，                  使能，          初始化，                    输入接口，                  设置输出*/
    {DECODER_PRIO_TRANS_TYPE,   1,              decoder_prio_trans_init,    NULL,                       decoder_prio_trans_audio_output_callback_set},
    {MIC_DEC_TYPE,              1,              mic_dec_init,               mic_dec_audio_input,        mic_dec_audio_output_callback_set},
#if WIRELESS_INTERPHONE_AEC_EN
    {AEC_ALG_TYPE,              1,              interphone_aec_far_init,    interphone_aec_far_audio_input,   interphone_aec_far_audio_output_callback_set},
#endif
#if WIRELESS_MIC_DAC_OUTPUT_EN
    {DAC_OUT_TYPE,              1,              dac1_out_init,              dac1_out_audio_input,       dac1_out_audio_output_callback_set},
#endif
};
#endif


AT(.text.func.device_cb)
void func_device_init(void)
{
    if (!device_cb.init_flag) {
        device_cb.init_flag = 1;
#if WIRELESS_MIC_TX_EN
        ws_link_list_init(mic_tx_cfg_tbl, (sizeof(mic_tx_cfg_tbl)/sizeof(ws_link_list_t)), WIRELESS_MIC_SAMPLE_RATE_SELECT, WIRELESS_MIC_SAMPLES_SELECT, WIRELESS_MIC_CHANNEL_SELECT);
#endif

#if WIRELESS_SPK_TX_EN && !DEVICE_INTERPHONE_EN
        ws_link_list_init(spk_rx_cfg_tbl, (sizeof(spk_rx_cfg_tbl)/sizeof(ws_link_list_t)), WIRELESS_SPK_SAMPLE_RATE_SELECT, WIRELESS_SPK_SAMPLES_SELECT, WIRELESS_SPK_CHANNEL_SELECT);
#endif

#if DEVICE_INTERPHONE_EN
        ws_link_list_init(mic_rx_cfg_tbl, (sizeof(mic_rx_cfg_tbl)/sizeof(ws_link_list_t)), WIRELESS_MIC_SAMPLE_RATE_SELECT, WIRELESS_MIC_SAMPLES_SELECT, WIRELESS_MIC_CHANNEL_SELECT);
        device_cb.adv_en = 1;
#endif
    }

    device_cb.init_state = DEVICE_STA_INIT_IDLE;
}
#if WIRELESS_MIC_BROADCAST_EN
AT(.text.func.device)
static void func_device_process_do(void)
{
    switch(device_cb.init_state) {
    case DEVICE_STA_INIT_IDLE:
        ble_per_adv_set_enable(1);
        device_cb.init_state = DEVICE_STA_BROADCAST_TX;
        break;
    case DEVICE_STA_BROADCAST_TX:
        device_cb.init_state = DEVICE_STA_BROADCAST_IDLE;
        break;

    default:
        break;
    }
}

#elif DEVICE_INTERPHONE_EN
AT(.text.func.device)
static void func_device_process_do(void)
{
    if(wireless_cb.change_flag) {
        wireless_cb.change_flag = 0;
        device_cb.init_state = DEVICE_STA_START_ACTION;
        if (device_cb.create_con_flag) {
            TRACE("interphone, create_con\n");
            device_cb.create_con_flag = 0;
            ble_scan_set_enable(0);
            ble_connect_req(500);
            device_cb.init_state = DEVICE_STA_INIT_CONNECT;
        }

        if (device_cb.con_cmp_flag) {
            TRACE("interphone, con_cmp\n");
            device_cb.con_cmp_flag = 0;
            ble_scan_set_enable(0);
#if WIRELESS_CON_LINK_NB == 1
            ble_adv_set_enable(0, 0);
#endif
        }
    }

    switch(device_cb.init_state) {
        case DEVICE_STA_INIT_IDLE:
            device_cb.init_state = DEVICE_STA_START_ACTION;
            break;
        case DEVICE_STA_START_ACTION:
            TRACE("interphone, con_sta(%d): %x\n", wireless_mic_is_bonding(), wireless_cb.connected_sta);
            if(wireless_cb.connected_sta) {
                //已连接，关闭扫描
                device_cb.init_state = DEVICE_STA_IDLE;
            } else {
                //无连接，搜索组队
                device_cb.init_state = DEVICE_STA_INTERPHONE_SCAN;
            }
            break;

        case DEVICE_STA_INTERPHONE_SCAN:
            TRACE("interphone, scan_en\n");
            cfg_wireless_role = false;
            ble_scan_set_enable(1);
            device_cb.init_state = DEVICE_STA_INTERPHONE_DELAY;
            device_cb.ticks      = tick_get();
            device_cb.tick_delay = 100 + get_random(0xffff)%300;
            device_cb.delay_state = DEVICE_STA_INTERPHONE_SCAN_TO;
            break;
        case DEVICE_STA_INTERPHONE_SCAN_TO:
            ble_scan_set_enable(0);
            device_cb.init_state = DEVICE_STA_INTERPHONE_DELAY;
            device_cb.ticks      = tick_get();
            device_cb.tick_delay = 100 + get_random(0xffff)%200;
            if (device_cb.adv_en) {
                ///使能adv的话，就在scan跟adv之间切换，否则一直scan
                device_cb.tick_delay = get_random(0xffff)%100;
                device_cb.delay_state = DEVICE_STA_INTERPHONE_ADV;
            } else {
                device_cb.delay_state = DEVICE_STA_INTERPHONE_SCAN;
            }
            break;
        case DEVICE_STA_INTERPHONE_ADV:
            TRACE("interphone, adv en\n");
#if WIRELESS_CON_LINK_NB == 1
            cfg_wireless_role = true;
            ble_adv_set_enable(1, 1);
#endif
            device_cb.init_state = DEVICE_STA_INTERPHONE_DELAY;
            device_cb.ticks      = tick_get();
            device_cb.tick_delay = 100 + get_random(0xffff)%200;
            device_cb.delay_state = DEVICE_STA_INTERPHONE_ADV_TO;
            break;
        case DEVICE_STA_INTERPHONE_ADV_TO:
            TRACE("interphone, adv dis\n");
#if WIRELESS_CON_LINK_NB == 1
            ble_adv_set_enable(0, 0);
#endif
            device_cb.init_state = DEVICE_STA_INTERPHONE_DELAY;
            device_cb.ticks      = tick_get();
            device_cb.tick_delay = get_random(0xffff)%100;
            device_cb.delay_state = DEVICE_STA_INTERPHONE_SCAN;
            break;
        case DEVICE_STA_INTERPHONE_DELAY:
            if(tick_check_expire(device_cb.ticks, device_cb.tick_delay)) {
                device_cb.init_state = device_cb.delay_state;
            }
            break;

        case DEVICE_STA_INIT_CONNECT:
            break;

        case DEVICE_STA_IDLE:
            break;
    }
}
#elif WIRELESS_MIC_2TNR_EN
#define WL_SYNC_TO              4400
#define WL_D2A_ADV_TO           600
AT(.text.func.device)
static void func_device_process_do(void)
{
    u8 link_nb;

    if(wireless_cb.change_flag) {
        wireless_cb.change_flag = 0;
        ble_adv_set_enable(0, 0);
        ble_scan_set_enable(0);
        delay_5ms(2);
        device_cb.init_state = DEVICE_STA_START_ACTION;
        if (device_cb.create_con_flag) {
            TRACE("device, create_con\n");
            device_cb.create_con_flag = 0;
            ble_connect_req(500);
            device_cb.init_state = DEVICE_STA_INIT_CONNECT;
        }
#if WIRELESS_CON_AND_BT_PAIRING_EN || WIRELESS_CON_BONDING_EN
        wireless_adapter_bond_update();
#endif
    }

    ///wl_sycn_scan/wl_sync_adv&wl_adv交替，一旦有无线麦连接就只adv，关闭scan
    switch(device_cb.init_state) {
    case DEVICE_STA_INIT_IDLE:
        device_cb.init_state = DEVICE_STA_START_ACTION;
        break;

    case DEVICE_STA_INIT_CONNECT:
        break;

    case DEVICE_STA_START_ACTION:
        link_nb = bt_get_link_info_nb();
        TRACE("device, con_sta(%d): %x, %d, %d\n", wireless_mic_is_bonding(), wireless_cb.connected_sta|(wireless_cb.device_con_sta << 4), link_nb, wireless_cb.device_con_role);

        if ((wireless_cb.connected_sta == ((1 << WIRELESS_CON_2TNR_NB)-1)) && wireless_cb.device_con_sta) {
            device_cb.init_state = DEVICE_STA_IDLE;
        } else {
            device_cb.init_state = DEVICE_STA_ADV;
            if (wireless_cb.device_con_sta) {
                device_cb.adv_type_flag = 0;
            } else {
                device_cb.adv_type_flag = !device_cb.adv_type_flag;
            }
            if (device_cb.adv_type_flag) {
                device_cb.adv_ticks       = tick_get();
                device_cb.adv_tick_delay  = WL_SYNC_TO;
            } else {
                device_cb.adv_ticks       = tick_get();
                device_cb.adv_tick_delay  = WL_D2A_ADV_TO;
            }
        }
        break;

    case DEVICE_STA_SCAN:
        TRACE("device, scan\n");
        ble_set_con_flag(CON_FLAG_NONE, CON_FLAG_NONE);
        ble_scan_set_enable(1);
        if(tick_check_expire(device_cb.adv_ticks, device_cb.adv_tick_delay)) {
            device_cb.delay_state = DEVICE_STA_START_ACTION;
        } else {
            device_cb.delay_state = DEVICE_STA_ADV;
        }
        device_cb.init_state  = DEVICE_STA_DELAY;
        device_cb.ticks       = tick_get();
        device_cb.tick_delay  = 100+get_random(0xffff)%200;
        break;

    case DEVICE_STA_ADV:
        TRACE("device, adv_type:%d\n", device_cb.adv_type_flag);
        if (device_cb.adv_type_flag) {
            ble_set_con_flag(CON_FLAG_NONE, CON_FLAG_NONE);
        } else {
            ble_set_con_flag(CON_FLAG_T2R, CON_FLAG_NONE);
        }
        ble_adv_set_enable(1, 1);
        if(tick_check_expire(device_cb.adv_ticks, device_cb.adv_tick_delay) || wireless_cb.connected_sta) {
            device_cb.delay_state = DEVICE_STA_START_ACTION;
        } else {
            if (device_cb.adv_type_flag) {
                device_cb.delay_state = DEVICE_STA_SCAN;
            } else {
                device_cb.delay_state = DEVICE_STA_ADV;
            }
        }
        device_cb.init_state  = DEVICE_STA_DELAY;
        device_cb.ticks       = tick_get();
        device_cb.tick_delay  = 100+get_random(0xffff)%200;
        break;

    case DEVICE_STA_DELAY:
        if(tick_check_expire(device_cb.ticks, device_cb.tick_delay)) {
            device_cb.init_state = device_cb.delay_state;
            ble_adv_set_enable(0, 0);
            ble_scan_set_enable(0);
            delay_5ms(2);
        }
        break;
    }
}
#else
AT(.text.func.device)
static void func_device_process_do(void)
{
    u8 addr[6];

    if(wireless_cb.change_flag) {
        wireless_cb.change_flag = 0;

        if (device_cb.create_con_flag) {
            TRACE("emit, create_con\n");
            device_cb.create_con_flag = 0;
            ble_scan_set_enable(0);
            ble_connect_req(500);
            device_cb.init_state  = DEVICE_STA_INIT_CONNECT;
        } else {
            switch(wireless_cb.change_sta) {
            case 0:     //connect success
                device_cb.init_state = DEVICE_STA_START_ACTION;
                break;

            case 1:     //connect fail
                if(!wireless_mic_is_bonding()) {
                    device_cb.ticks      = tick_get();
                    device_cb.tick_delay = 100+bt_get_rand()%500;
                    device_cb.init_state = DEVICE_STA_DELAY;
                    break;
                }
                //no break

            case 2:     //disconnect
                device_cb.ticks      = tick_get();
                device_cb.tick_delay = 500+bt_get_rand()%500;
                device_cb.init_state = DEVICE_STA_DELAY;
                break;
            }
        }
    }

    switch(device_cb.init_state) {
    case DEVICE_STA_INIT_IDLE:
        TRACE("emit, init(%d)\n", wireless_mic_is_bonding());
        if(bt_get_link_info_addr(addr, 0)) {
            //有回连信息，开始回连
            TRACE("emit, con_req: ");
            TRACE_R(addr, 6);
            ble_create_con_for_addr(addr, 2500);
            device_cb.init_state  = DEVICE_STA_INIT_CONNECT;
        } else {
            //没有回连信息，搜索组队
            TRACE("emit, scan_en\n");
            ble_scan_set_enable(1);
            device_cb.init_state  = DEVICE_STA_SCAN;
            device_cb.ticks       = tick_get();
        }
        break;

    case DEVICE_STA_INIT_CONNECT:
        break;

    case DEVICE_STA_START_ACTION:
        TRACE("emit, con_sta(%d): %x\n", wireless_mic_is_bonding(), wireless_cb.connected_sta);

        if(wireless_cb.connected_sta) {
            //已连接，关闭扫描
            TRACE("emit, con_complete\n");
            device_cb.init_state = DEVICE_STA_IDLE;
        } else if(wireless_mic_is_bonding() && bt_get_link_info_addr(addr, 0)) {
            //有回连信息，开始回连
            TRACE("emit, con_req: ");
            TRACE_R(addr, 6);
            ble_create_con_for_addr(addr, 2500);
            device_cb.init_state  = DEVICE_STA_INIT_CONNECT;
        } else {
            //没有回连信息，搜索组队
            TRACE("emit, scan_en\n");
            ble_scan_set_enable(1);
            device_cb.init_state = DEVICE_STA_SCAN;
            device_cb.ticks      = tick_get();
        }
        break;

    case DEVICE_STA_SCAN:
        if(tick_check_expire(device_cb.ticks, 4000)) {     //省电时可以间歇性扫描
            ble_scan_set_enable(0);
            device_cb.ticks      = tick_get();
            device_cb.tick_delay = 1000+bt_get_rand()%500;
            device_cb.init_state = DEVICE_STA_DELAY;
        }
        break;

    case DEVICE_STA_DELAY:
        if(tick_check_expire(device_cb.ticks, device_cb.tick_delay)) {
            device_cb.init_state = DEVICE_STA_START_ACTION;
        }
        break;
    }
}
#endif


AT(.text.func.process.device)
static void func_device_process(void)
{
#if TRACE_EN
    static u8 sta = 0xff;
    if(sta != device_cb.init_state) {
        sta = device_cb.init_state;
        TRACE("emit, state: %d\n", sta);
    }
#endif
#if WIRELESS_MIC_BROADCAST_EN
    if(device_cb.init_state != DEVICE_STA_BROADCAST_IDLE) {
#elif WIRELESS_MIC_2TNR_EN
    if((device_cb.init_state != DEVICE_STA_IDLE) || wireless_cb.change_flag) {
#else
    if(((device_cb.init_state != DEVICE_STA_IDLE) && (device_cb.init_state != DEVICE_STA_INIT_CONNECT)) || wireless_cb.change_flag) {
#endif
        func_device_process_do();
    }
//    led_disp_proc();
    wireless_sta_proc();
    func_process();
}

AT(.text.func.device)
static void func_device_enter(void)
{
    func_device_init();
    func_bt_init();

#if WIRELESS_MIC_2TNR_EN
    ble_scan_set_param(64,56);
    ble_adv_set_interval(WIRELESS_MIC_TX_INTERVAL*8);
#endif

#if UARTDUMP_DNN_EN
    sys_clk_req(INDEX_KARAOK, SYS_160M);     //本地麦测试   //先抬高主频
    wireless_device_init();
    wireless_mic_kick();
    while(1){
        WDT_CLR();
    }
#endif
    gui_box_flicker_set(5, 0xff, 0xff);
    printf("gui_box_flicker_set \n");
}

#if DEVICE_INTERPHONE_EN
AT(.text.func.device)
void func_device_interphone_set_con_cmp(void)
{
    device_cb.con_cmp_flag = 1;
    wireless_cb.change_flag = 1;
}
#endif

AT(.text.func.device)
void func_device_set_create_con(void)
{
    device_cb.create_con_flag = 1;
    wireless_cb.change_flag = 1;
}

AT(.text.func.device)
static void func_device_exit(void)
{
//    le_mic_discon_all();
    mic_stop();
    bt_off();
    func_cb.last = FUNC_DEVICE;
    sys_cb.bt_is_inited = 0;
    device_cb.init_flag = 0;
}

AT(.text.func.device_cb.display)
void func_device_display(void)
{
    gui_box_process();

    switch (box_cb.disp_sta) {
    case GUI_BOX_NULL:
        gui_display(DISP_DEVICE);
        break;
    default:
        gui_box_display();
        break;
    }
}

AT(.text.func.device_cb)
void func_device(void)
{
    printf("%s\n", __func__);
    func_device_enter();

    while (func_cb.sta == FUNC_DEVICE) {
        func_device_process();
        func_device_message(msg_dequeue());
        func_device_display();
    }

    func_device_exit();
}

