#include "include.h"
#include "func.h"
#include "func_adapter.h"
#include "bsp_usbfot.h"

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif // TRACE_EN

enum {
    ADAPTER_STA_INIT_IDLE,
    ADAPTER_STA_INIT_W4_CONNECT,
    ADAPTER_STA_START_ACTION,
    ADAPTER_STA_IDLE,

    ADAPTER_STA_BROADCAST_IDLE,
    ADAPTER_STA_BROADCAST_RX,

    ADAPTER_STA_INIT_CONNECT,
    ADAPTER_STA_DELAY,
    ADAPTER_STA_SCAN,
};

static struct {
    uint8_t init_flag;
    uint8_t init_state;
    uint32_t ticks;
    uint tick_delay;
    bool create_con_flag;
} adapter_cb;

//无线MIC适配器处理流程
//ADAPTER_RX -> wireless_d2a_set_rxpkt_cb -> decoder_prio_trans_audio_input -> mic_dec_audio_input --> mic_mix_audio_input-> dac0_out_audio_input -> DAC
//                                                                                                 |-> dac0_out_audio_input -> DAC
#if WIRELESS_MIC_TX_EN
static ws_link_list_t mic_rx_cfg_tbl[] = {
/*  模块类型，                  使能，          初始化，                    输入接口，                  设置输出*/
    {DECODER_PRIO_TRANS_TYPE,   1,              decoder_prio_trans_init,    NULL,                       decoder_prio_trans_audio_output_callback_set},
    {MIC_DEC_TYPE,              1,              mic_dec_init,               mic_dec_audio_input,        mic_dec_audio_output_callback_set},
#if (ADAPTER_ECHO_EN  || ADAPTER_VOICE_CHANGE_V2_EN ||ADAPTER_MAGIC_EN || ADAPTER_AGC_EN || ADAPTER_HOWLING_DNN_EN || ADAPTER_ROOM_REVERB_EN || ADAPTER_SOFT_EQ_EN || ADAPTER_HUART_OUTPUT_EN)
    {DECODER_PRIO_TRANS_TYPE,   1,              alg_prio_trans_init,        alg_prio_trans_audio_input, alg_prio_trans_audio_output_callback_set},
#endif
#if ADAPTER_FREQ_SHIFT2_EN
    {FREQ_SHIFT2_TYPE,          1,              freq_shift2_mic_init,       freq_shift2_mic_audio_input,freq_shift2_mic_output_callback_set},
#endif
#if ADAPTER_LOCAL_MIC_MIX_EN
    {MIC_MIX_TYPE,              1,              mic_mix_init,               mic_mix_audio_input,        mic_mix_output_callback_set},
#endif

#if ADAPTER_SOFT_EQ_EN
    {SOFT_EQ_TYPE,             1,              soft_eq_mic_init,       soft_eq_mic_audio_input,  soft_eq_mic_output_callback_set},
#endif

#if ADAPTER_HOWLING_DNN_EN
    {HOWLING_AIMASK_TYPE,       1,              howling_dnn_mic_init,       howling_dnn_mic_audio_input,  howling_dnn_mic_output_callback_set},
#endif
#if ADAPTER_MIC_EQ_DRC_EN
    {MIC_EQ_DRC_TYPE,           1,              mic_eq_drc_init,            mic_eq_drc_audio_input,     mic_eq_drc_audio_output_callback_set},
#endif

#if ADAPTER_AGC_EN
    {AGC_MIC_TYPE,              1,              agc_audio_init,             agc_audio_input,            agc_audio_output_callback_set},
#endif

#if ADAPTER_ROOM_REVERB_EN
    {ROOM_REVERB_TYPE,          1,              room_reverb_audio_init,   room_reverb_audio_input,         room_reverb_audio_output_callback_set},
#endif

#if ADAPTER_ECHO_EN
    {ECHO_TYPE,                 1,              echo_audio_init,            echo_audio_input,           echo_audio_output_callback_set},
#endif
#if ADAPTER_MAGIC_EN
    {MAGCI_PITCH_SHIFT_TYPE,    1,              magic_audio_init,           magic_audio_input,          magic_audio_output_callback_set},
#endif
#if ADAPTER_VOICE_CHANGE_V2_EN
    {VOICE_CHANGE_V2_TYPE,         1,              voice_change_v2_mic_init,     voice_change_v2_mic_audio_input,voice_change_v2_mic_output_callback_set},
#endif
#if ADAPTER_HUART_OUTPUT_EN
    {HUART_TX_TYPE,             1,              huart_audio_out_init,      huart_audio_out_input,      huart_audio_out_output_callback_set},
#endif
#if ADAPTER_HUART_INPUT_EN
    {HUART_RX_TYPE,             1,              huart_audio_in_init,       huart_audio_in_input,      huart_audio_in_output_callback_set},
#endif
#if ADAPTER_I2S_OUTPUT_EN
    {I2S_OUT_TYPE,             1,               i2s_audio_out_init,        i2s_audio_out_input,         i2s_audio_output_callback_set},               //I2S传输
#endif
#if ADAPTER_I2S_IN_OUT_EN
    {I2S_OUT_TYPE,             1,               i2s_audio_in_out_init,     i2s_audio_in_out_input,      i2s_audio_in_out_output_callback_set},        //I2S双向传输
#endif
#if ADAPTER_DAC_OUTPUT_EN
    {DAC_OUT_TYPE,              1,              dac1_out_init,              dac1_out_audio_input,       dac1_out_audio_output_callback_set},
#endif
#if ADAPTER_HARDWARE_SRC1_EN && (!ADAPTER_USB_SPK_TX_EN) && (!ADAPTER_I2S_TX_EN)
    {SRC_TYPE,                  1,              src1_stereo_init,           src1_audio_input,           src1_audio_output_callback_set},              //USB真立体声调速
#endif
    {USB_MIC_IN_TYPE,           1,              usb_mic_in_init,            usb_mic_in_audio_input,     usb_mic_in_audio_output_callback_set},
};
#endif

#if ADAPTER_USB_SPK_TX_EN
static ws_link_list_t spk_tx_cfg_tbl[] = {
/*  模块类型，                 使能，            初始化，                    输入接口，                         设置输出*/
    {USB_SPK_OUT_TYPE,         1,                usb_audio_out_init,          NULL,                       usb_audio_out_audio_output_callback_set},
#if ADAPTER_SPK_OUTPUT_EN
    {DAC_OUT_TYPE,             1,                dac0_out_init,               dac0_out_audio_input,       dac0_out_audio_output_callback_set},
#endif
    {SRC_TYPE,                 1,                src1_stereo_init,            src1_audio_input,           src1_audio_output_callback_set},                //USB立体声调速
    {SPK_VOL_TYPE,             1,                spk_soft_gain_init,          spk_soft_gain_input,        spk_soft_gain_audio_output_callback_set},
    {SRC_TYPE,                 1,                src_buf_init,                src_buf_audio_input,        src_buf_audio_output_callback_set},
    {SPK_ENC_TYPE,             1,                spk_enc_init,                NULL,                       spk_enc_audio_output_callback_set},             //PCM编码
};
#elif (ADAPTER_AUX_TX_EN || ADAPTER_I2S_TX_EN)
static ws_link_list_t spk_tx_cfg_tbl[] = {
/*  模块类型，                 使能，            初始化，                    输入接口，                         设置输出*/
    {MIC_TYPE,                 1,                spk_init,                    NULL,                       spk_audio_output_callback_set},
#if ADAPTER_I2S_TX_EN
    {I2S_IN_TYPE,              1,                i2s_audio_in_init,           i2s_audio_in_input,         i2s_audio_input_callback_set},                  //I2S传输
#endif
#if ADAPTER_SPK_OUTPUT_EN
    {DAC_OUT_TYPE,             1,                dac0_out_init,               dac0_out_audio_input,       dac0_out_audio_output_callback_set},
#endif
    {SPK_ENC_TYPE,             1,                spk_enc_init,                spk_enc_audio_input,        spk_enc_audio_output_callback_set},             //PCM编码
};
#endif

AT(.text.func.adapter)
void func_adapter_init(void)
{
    if (!wireless_role_is_adapter()) {
        return;
    }
    if (!adapter_cb.init_flag) {
        adapter_cb.init_flag = 1;
#if WIRELESS_MIC_TX_EN
		ws_link_list_init(mic_rx_cfg_tbl, (sizeof(mic_rx_cfg_tbl)/sizeof(ws_link_list_t)), WIRELESS_MIC_SAMPLE_RATE_SELECT, WIRELESS_MIC_SAMPLES_SELECT, WIRELESS_MIC_CHANNEL_SELECT);
#endif

#if WIRELESS_SPK_TX_EN && !DEVICE_INTERPHONE_EN
        ws_link_list_init(spk_tx_cfg_tbl, (sizeof(spk_tx_cfg_tbl)/sizeof(ws_link_list_t)), WIRELESS_SPK_SAMPLE_RATE_SELECT, WIRELESS_SPK_SAMPLES_SELECT, WIRELESS_SPK_CHANNEL_SELECT);
#endif
    }
    adapter_cb.init_state = ADAPTER_STA_INIT_IDLE;
}

#if WIRELESS_MIC_BROADCAST_EN
AT(.text.func.adapter)
static void func_adapter_process_do(void)
{
    if(wireless_cb.change_flag) {
        wireless_cb.change_flag = 0;
        adapter_cb.init_state = ADAPTER_STA_INIT_IDLE;
    }
//    my_printf("adapter_cb.init_state %d\n",adapter_cb.init_state);
    switch(adapter_cb.init_state) {
    case ADAPTER_STA_INIT_IDLE:
        ble_per_scan_set_enable(1);
        adapter_cb.init_state = ADAPTER_STA_BROADCAST_RX;
        break;
    case ADAPTER_STA_BROADCAST_RX:
        adapter_cb.init_state = ADAPTER_STA_BROADCAST_IDLE;
        break;

    default:
        break;
    }
}
#elif WIRELESS_MIC_2TNR_EN
AT(.text.func.adapter)
static void func_adapter_process_do(void)
{
    u8 addr[6];

    if(wireless_cb.change_flag) {
        wireless_cb.change_flag = 0;

        if (adapter_cb.create_con_flag) {
            TRACE("adapter, create_con\n");
            adapter_cb.create_con_flag = 0;
            ble_scan_set_enable(0);
            ble_connect_req(500);
            adapter_cb.init_state  = ADAPTER_STA_INIT_CONNECT;
        } else {
            switch(wireless_cb.change_sta) {
            case 0:     //connect success
                adapter_cb.init_state = ADAPTER_STA_START_ACTION;
                break;

            case 1:     //connect fail
                if(!wireless_mic_is_bonding()) {
                    adapter_cb.ticks      = tick_get();
                    adapter_cb.tick_delay = 100+get_random(0xffff)%1000;
                    adapter_cb.init_state = ADAPTER_STA_DELAY;
                    break;
                }
                //no break

            case 2:     //disconnect
                adapter_cb.ticks      = tick_get();
                adapter_cb.tick_delay = 400+get_random(0xffff)%500;
                adapter_cb.init_state = ADAPTER_STA_DELAY;
                break;
            }
        }
    }

    switch(adapter_cb.init_state) {
    case ADAPTER_STA_INIT_IDLE:
        TRACE("adapter, init(%d)\n", wireless_mic_is_bonding());
        //不允许直接回连,要检查广播信息,先连主机才能连从机(避免直接对TX从机发起连接)
        if(bt_get_link_info_addr(addr, 0) && 0) {
            //有回连信息，开始回连
            TRACE("adapter, con_req: ");
            TRACE_R(addr, 6);
            ble_create_con_for_addr(addr, 1500);
            adapter_cb.init_state  = ADAPTER_STA_INIT_CONNECT;
        } else {
            //没有回连信息，搜索组队
            TRACE("adapter, scan_en\n");
            ble_scan_set_enable(1);
            adapter_cb.init_state  = ADAPTER_STA_SCAN;
            adapter_cb.ticks       = tick_get();
        }
        break;

    case ADAPTER_STA_INIT_CONNECT:
        break;

    case ADAPTER_STA_START_ACTION:
        TRACE("adapter, con_sta(%d): %x\n", wireless_mic_is_bonding(), wireless_cb.connected_sta);

        if(wireless_cb.connected_sta == WIRELESS_CON_STA_MASK) {
            //已连接，关闭扫描
            TRACE("adapter, con_complete\n");
            adapter_cb.init_state = ADAPTER_STA_IDLE;
        } else if(wireless_mic_is_bonding() && bt_get_link_info_addr(addr, 0) && 0) {
            //有回连信息，开始回连
            TRACE("adapter, con_req: ");
            TRACE_R(addr, 6);
            ble_create_con_for_addr(addr, 1500);
            adapter_cb.init_state  = ADAPTER_STA_INIT_CONNECT;
        } else {
            //没有回连信息，搜索组队
            TRACE("adapter, scan_en\n");
            ble_scan_set_enable(1);
            adapter_cb.init_state = ADAPTER_STA_SCAN;
            adapter_cb.ticks      = tick_get();
        }
        break;

    case ADAPTER_STA_SCAN:
//        if(tick_check_expire(adapter_cb.ticks, 400)) {     //省电时可以间歇性扫描
//            ble_scan_set_enable(0);
//            adapter_cb.ticks      = tick_get();
//            adapter_cb.tick_delay = 500+get_random(0xffff)%200;
//            adapter_cb.init_state = ADAPTER_STA_DELAY;
//        }
        break;

    case ADAPTER_STA_DELAY:
        if(tick_check_expire(adapter_cb.ticks, adapter_cb.tick_delay)) {
            adapter_cb.init_state = ADAPTER_STA_START_ACTION;
        }
        break;
    }
}
#else
AT(.text.func.adapter)
static void func_adapter_process_do(void)
{
    u8 link_nb;

    if(wireless_cb.change_flag) {
        wireless_cb.change_flag = 0;
        adapter_cb.init_state = ADAPTER_STA_START_ACTION;
    }

    switch(adapter_cb.init_state) {
    case ADAPTER_STA_INIT_IDLE:
        link_nb = bt_get_link_info_nb();
        TRACE("adapter, init(%d): %x, %d\n", wireless_mic_is_bonding(), wireless_cb.connected_sta, link_nb);

        if(link_nb != 0) {
            //有回连信息，等待被连接（关闭可被发现）
            ble_adv_set_enable(1, 0);
            adapter_cb.init_state  = ADAPTER_STA_INIT_W4_CONNECT;
            adapter_cb.ticks       = tick_get();
        } else {
            //没有回连信息，等待被发现和连接
            ble_adv_set_enable(1, 1);
            adapter_cb.init_state  = ADAPTER_STA_IDLE;
        }
        break;

    case ADAPTER_STA_INIT_W4_CONNECT:
        if(tick_check_expire(adapter_cb.ticks, 2000)) {
            adapter_cb.init_state = ADAPTER_STA_START_ACTION;
        }
        break;

    case ADAPTER_STA_START_ACTION:
        link_nb = bt_get_link_info_nb();
        TRACE("adapter, con_sta(%d): %x, %d\n", wireless_mic_is_bonding(), wireless_cb.connected_sta, link_nb);

        if(wireless_cb.connected_sta == WIRELESS_CON_STA_MASK) {
            //两个都连上了，关闭可被发现和可被连接
            ble_adv_set_enable(0, 0);
        } else if(wireless_mic_is_bonding() && link_nb >= WIRELESS_CON_LINK_NB) {
            //组队绑定时，切配对过两个，等待被连接（关闭可被发现）
            ble_adv_set_enable(1, 0);
        } else {
            //等待被发现和连接
            ble_adv_set_enable(1, 1);
        }
        adapter_cb.init_state = ADAPTER_STA_IDLE;
        break;
    }
}
#endif

AT(.com_text.usb_detect)
void usb_detect(void)
{

    if(!sys_cb.bt_is_inited){
        return;
    }
	u8 usb_sta, usb_sta_old;
#if USB_DET_VER_SEL
    usb_sta = usb_connect();
#else
    usb_sta = usbchk_connect(USBCHK_ONLY_DEVICE);
#endif
    if (usb_sta == 2) {
        if (dev_online_filter(DEV_USBPC)) {
            msg_enqueue(EVT_PC_INSERT);
            usb_sta_old = usbchk_connect(USBCHK_ONLY_DEVICE);
//            my_printf("pc insert:%x\n", usb_sta_old);
        }
    } else {
        if (dev_offline_filter(DEV_USBPC)) {
            usbchk_only_device();                            //没有SOF包切换到旧的拔出检测机制
            usb_sta_old = usbchk_connect(USBCHK_ONLY_DEVICE);
            if(usb_sta_old != 2) {
                msg_enqueue(EVT_PC_REMOVE);
                pc_remove();
            }
//            my_printf("pc remove:%x\n", usb_sta_old);
        }
    }

}

#if WIRELESS_CON_PAIR_MODE
AT(.text.bsp.wireless_cb)
void wireless_con_role(void)          //判断主副麦
{
    if(!wireless_get_status()) {
        if(tick_check_expire(wireless_cb.con_id_switch_ticks, 3000)){     //3s切换一次广播数据包
            wireless_cb.con_id_switch_ticks = tick_get();
            if(wireless_cb.con_role_switch_flag) {
                ble_set_con_id(ADAPTER_SET_TX1_CON_ID);
                sys_cb.con_role = 1;
                wireless_cb.con_role_switch_flag = 0;
                printf("Master ADV\n");
            } else {
                ble_set_con_id(ADAPTER_SET_TX2_CON_ID);
                sys_cb.con_role = 0;
                wireless_cb.con_role_switch_flag = 1;
                printf("Slave ADV\n");
            }
        }
    } else if((wireless_get_status() != 3) && !wireless_cb.con_temp) {
        wireless_cb.con_temp = 1;
        if(sys_cb.con_role) {           //已经连上主麦
            ble_set_con_id(ADAPTER_SET_TX2_CON_ID);
            sys_cb.con_role = 0;
            printf("Slave ADV\n");
        } else {                       //已经连上副麦
            ble_set_con_id(ADAPTER_SET_TX1_CON_ID);
            sys_cb.con_role = 1;
            printf("Master ADV\n");
        }
    }
}
#endif

AT(.text.func.process.adapter)
void func_adapter_process(void)
{
#if TRACE_EN
    static u8 sta = 0xff;
    if(sta != adapter_cb.init_state) {
        sta = adapter_cb.init_state;
        TRACE("adapter, state: %d\n", sta);
    }
#endif
#if WIRELESS_MIC_BROADCAST_EN
    if(adapter_cb.init_state != ADAPTER_STA_BROADCAST_IDLE || wireless_cb.change_flag) {
#elif WIRELESS_MIC_2TNR_EN
    if(((adapter_cb.init_state != ADAPTER_STA_IDLE) && (adapter_cb.init_state != ADAPTER_STA_INIT_CONNECT)) || wireless_cb.change_flag) {
#else
    if(adapter_cb.init_state != ADAPTER_STA_IDLE || wireless_cb.change_flag) {
#endif
        func_adapter_process_do();
    }

#if ADAPTER_USB_MIC_RX_EN
    usb_device_process();
#if ADAPTER_AB_FOT_DEVICE_SUPPORT
    fot_check();
    usb_fot_device_process();
#endif

#endif

//    led_disp_proc();
    wireless_sta_proc();
    func_process();
#if ADAPTER_USB_UPDATE_EN
	usb_dev_update_process();
#endif

#if WIRELESS_CON_PAIR_MODE
    if(!wireless_cb.con_id_switch_ticks) {
        wireless_cb.con_id_switch_ticks = tick_get();
        printf("con_role_init\n");
        ble_set_con_id(0);
    }

    wireless_con_role();
#endif

}

AT(.text.func.adapter)
static void func_adapter_enter(void)
{
    func_adapter_init();
    func_bt_init();
#if ADAPTER_NORMAL_LE_EN
	ble_adv_start_init();
	ble_con_start_init();
#endif

#if WIRELESS_MIC_2TNR_EN
    ble_scan_set_param(64,32);
    ble_set_con_flag(CON_FLAG_T2R, CON_FLAG_T2R);
#endif
}

AT(.text.func.adapter)
static void func_adapter_exit(void)
{
//    le_mic_discon_all();
    bt_off();
    func_cb.last = FUNC_ADAPTER;
    adapter_cb.init_flag = 0;
}

#if WIRELESS_MIC_2TNR_EN
AT(.text.func.adapter)
void func_adapter_set_create_con(void)
{
    adapter_cb.create_con_flag = 1;
    wireless_cb.change_flag = 1;
}
#endif

AT(.text.func.adapter)
void func_adapter(void)
{
    printf("%s\n", __func__);

    func_adapter_enter();

    while (func_cb.sta == FUNC_ADAPTER) {
        func_adapter_process();
        func_adapter_message(msg_dequeue());
        //func_adapter_display();
    }

    func_adapter_exit();
}

