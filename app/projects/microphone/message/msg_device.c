#include "include.h"
#include "func.h"
#include "func_device.h"

AT(.text.func.device)
void func_device_message(u16 msg)
{
    switch (msg) {
    case MSG_KEY_PP_DOUBLE:
        sys_cb.wireless_id_sta = !sys_cb.wireless_id_sta;
        if(sys_cb.wireless_id_sta)
        {
            ble_set_con_id(WIRELESS_CON_CON_ID_SEL2);
        }
        else
        {
            ble_set_con_id(WIRELESS_CON_CON_ID_SEL);
        }
        break;
//    case 0xe0:
//        mic_enc_mute_set(1);
//        ylcrn_L3_mic_mute_set(1);
//        ains4_mic_param_set(0,1000);
//        ains4_mic_mute_set(0);
//        delay_5ms(10);
//        mic_enc_mute_set(0);
//
//        my_printf("AINS4 EN\n");
//        break;
//
//    case 0x08:
//        mic_enc_mute_set(1);
//        ains4_mic_mute_set(1);
//        ylcrn_L3_mic_param_set(1000);
//        ylcrn_L3_mic_mute_set(0);
//        delay_5ms(10);
//        mic_enc_mute_set(0);
//        my_printf("AI EN\n");
//        break;

//    case 0x09:
//        my_printf("KEY2\n");
//        break;

    case MSG_SYS_1S:
#if WIRELESS_MIC_BROADCAST_EN
        ble_per_ch_map_update();
#endif
        break;

    default:
        if(wireless_get_status()) {
            func_mic_emit_user_key_message(msg);
        }
        else
        {
            func_message(msg);
        }
        break;
    }
}

static const u16 magic_wirless_msg[] = {
    WIRLESS_MSG_MAGIC_NORMAL,
    WIRLESS_MSG_MAGIC_MAN,
    WIRLESS_MSG_MAGIC_WOMAN,
    WIRLESS_MSG_MAGIC_KIDS,
    WIRLESS_MSG_MAGIC_MONSTER,
};

enum {
    MAGIC_EFFECT_NORMAL,
    MAGIC_EFFECT_MAN,
    MAGIC_EFFECT_WOMAN,
    MAGIC_EFFECT_KIDS,
    MAGIC_EFFECT_MONSTER,
};
    AT(.text.func.mic_emit)
    void func_mic_emit_user_key_message(u16 msg)
    {
        switch (msg) {

        case MSG_VOL_UP:         // K1短按抬起
            soft_gain_up();
            gui_box_show_vol();
            if(soft_gain_level_is_max_min()) {
                wirless_mic_vol_up_max();
            } else {
                wirless_mic_vol_up();
            }
            printf("K1 key short key up\n");
            break;

        case MSG_KEY_USER1_DOUBLE:
            echo_delay_level_up();
            gui_box_show_echo();
            if(echo_delay_level_is_max_min())
            {
                wirless_echo_level_up_max();
            }else
            {
                wirless_echo_level_up();
            }
//            wirless_music_next();
//            printf("K1 key double up\n");
            break;

        case MSG_VOL_DOWN:         // K2短按抬起
            soft_gain_down();
            gui_box_show_vol();
            if(soft_gain_level_is_max_min()){
                wirless_mic_vol_down_min();
            } else {
                wirless_mic_vol_down();
            }
            printf("K2 key short key up\n");
            break;

        case MSG_KEY_USER2_DOUBLE:           // K2双击
            echo_delay_level_down();
//            wirless_echo_level_down();
            gui_box_show_echo();
            if(echo_delay_level_is_max_min())
            {
                wirless_echo_level_down_min();
            }else
            {
                wirless_echo_level_down();
            }
            printf("K2 key double up\n");
            break;

        case MSG_KEY_PP_KU:         // PP按键短按抬起
        magic_audio_mute_set(1);             //mute 魔音，防连续切换导致复位关机
        magic_effect_level_change();         //设置魔音效果
        magic_audio_mute_set(0);
        wireless_tx_ws_mic_cmd(magic_wirless_msg[magic_effect_level_get()], 1);

            break;

        default:
            func_message(msg);
            break;
        }
}
