#include "include.h"
#include "func.h"
#include "func_bt.h"

u16 user_def_convert_vol_msg(u8 func_sel)
{
    u16 msg = NO_MSG;

    if(func_sel == UDK_VOL_UP) {            //VOL+
#if BT_TWS_EN
        if(xcfg_cb.user_def_lr_en && xcfg_cb.bt_tws_en) {
            msg = func_bt_tws_get_channel()? MSG_VOL_UP : MSG_VOL_DOWN;
        } else
#endif
        {
            msg = MSG_VOL_UP;
        }
    } else if(func_sel == UDK_VOL_DOWN) {   //VOL-
#if BT_TWS_EN
        if(xcfg_cb.user_def_lr_en && xcfg_cb.bt_tws_en) {
            msg = func_bt_tws_get_channel()? MSG_VOL_DOWN : MSG_VOL_UP;
        } else
#endif
        {
            msg = MSG_VOL_DOWN;
        }
    }
    return msg;
}

void user_def_track_msg(u16 msg)
{
    if (msg == MSG_MUSIC_PREV) {
        bt_music_prev();
    } else {
        bt_music_next();
    }
}

/////检查USER_DEF按键消息处理
//bool user_def_key_msg(u8 func_sel)
//{
//    u16 msg = NO_MSG;
//
//    if (!user_def_func_is_ready(func_sel)) {
//        return false;
//    }
//
//    if (func_sel == UDK_REDIALING) {
//        if(bsp_res_play(TWS_RES_REDIALING) == RES_ERR_INVALID) {
//            bt_call_redial_last_number();               //回拨电话
//        }
//    } else if (func_sel == UDK_SIRI) {                  //SIRI
//        bt_hfp_siri_switch();
//    } else if (func_sel == UDK_NR) {                    //NR
//        bt_ctl_nr_sta_change();                         //发消息通知手机
//    } else if (func_sel == UDK_PREV) {                  //PREV
//#if BT_TWS_EN
//        if(xcfg_cb.user_def_lr_en && xcfg_cb.bt_tws_en) {
//            msg = func_bt_tws_get_channel()? MSG_MUSIC_PREV : MSG_MUSIC_NEXT;
//        } else
//#endif
//        {
//            msg = MSG_MUSIC_PREV;
//        }
//        user_def_track_msg(msg);
//    } else if (func_sel == UDK_NEXT) {                  //NEXT
//#if BT_TWS_EN
//        if(xcfg_cb.user_def_lr_en && xcfg_cb.bt_tws_en) {
//            msg = func_bt_tws_get_channel()? MSG_MUSIC_NEXT : MSG_MUSIC_PREV;
//        } else
//#endif
//        {
//            msg = MSG_MUSIC_NEXT;
//        }
//        user_def_track_msg(msg);
//    } else if (func_sel == UDK_MODE) {                  //MODE
//        func_message(MSG_CHANGE_MODE);
//    } else if (func_sel == UDK_PHOTO) {
//        return bsp_bt_hid_photo(HID_KEY_VOL_UP);        //拍照
//    } else if (func_sel == UDK_HOME) {
//        return bt_hid_consumer(HID_KEY_IOS_HOME);       //IOS Home按键功能
//    } else if (func_sel == UDK_LANG) {
//        func_bt_switch_voice_lang();                    //中英文切换
//    } else if (func_sel == UDK_PLAY_PAUSE) {
//        bt_music_play_pause();
//    } else if (func_sel == UDK_DUT) {                  //CBT 测试模式
//        if(func_cb.sta != FUNC_BT_DUT){
//            func_cb.sta = FUNC_BT_DUT;
//            sys_cb.discon_reason = 0;
//        }
//    } else if (func_sel == UDK_LOW_LATENCY) {
//        bool low_latency = bt_is_low_latency();
//        if (low_latency) {
//            bsp_res_play(TWS_RES_MUSIC_MODE);
//        } else {
//            bsp_res_play(TWS_RES_GAME_MODE);
//        }
//    }else {                                            //VOL+, VOL-
//        func_message(user_def_convert_vol_msg(func_sel));
//    }
//    return true;
//}

#if BT_HID_MANU_EN
//双击VOL-, VOL+的功能处理
void bt_hid_vol_msg(u8 sel)
{
    if (sel == 1) {
        bsp_bt_hid_photo(HID_KEY_VOL_UP);
    } else if (sel == 2) {
        bsp_bt_hid_photo(HID_KEY_VOL_DOWN);
    } else if (sel == 3) {
        bsp_bt_hid_tog_conn();
    }
}
#endif

bool bt_tws_pair_mode(u8 method)
{
#if BT_TWS_EN
    if ((xcfg_cb.bt_tws_en) && (xcfg_cb.bt_tws_pair_mode == method) && (!bt_nor_is_connected())) {
        if(bt_tws_is_connected()) {
            bt_tws_disconnect();
        } else {
            bt_tws_search_slave(15000);
        }
        return true;
    }
#endif
    return false;
}

AT(.text.func.bt.msg)
void func_bt_message_do(u16 msg)
{
    switch (msg) {
    case MSG_MUSIC_PLAY:
        bt_music_play();
        break;
    case MSG_MUSIC_PAUSE:
        bt_music_pause();
        break;
    case MSG_MUSIC_PLAY_PAUSE:
        bt_music_play_pause();
        break;
    case MSG_MUSIC_PREV:
        bt_music_prev();
        sys_cb.key2unmute_cnt = 15 * sys_cb.mute;
        break;
    case MSG_MUSIC_NEXT:
        bt_music_next();
        sys_cb.key2unmute_cnt = 15 * sys_cb.mute;
        break;

    case MSG_MUSIC_FB_START:
        bt_music_rewind();
        break;
    case MSG_MUSIC_FB_END:
        bsp_clr_mute_sta();
        bt_music_rewind_end();
        break;
    case MSG_MUSIC_FF_START:
        bt_music_fast_forward();
        break;
    case MSG_MUSIC_FF_END:
        bsp_clr_mute_sta();
        bt_music_fast_forward_end();
        break;

    case MSG_CALL_REDIAL_LAST:
        if (bt_nor_is_connected()) {
            if(bsp_res_play(TWS_RES_REDIALING) == RES_ERR_INVALID) {
                bt_call_redial_last_number();       //回拨电话
            }
        }
        break;

    case MSG_MUSIC_LOW_LATENCY:
        break;

    case MSG_CHANGE_LANGUAGE:
        func_bt_switch_voice_lang();
        break;

//    ///双击按键处理
//    case MSG_KEY_PP_DOUBLE:
//        if (user_def_key_msg(xcfg_cb.user_def_kd_sel)) {
//        } else if (xcfg_cb.user_def_kd_lang_en) {
//            func_bt_switch_voice_lang();
//        }
//        break;
//
//    ///三击按键处理
//    case MSG_KEY_PP_THREE:
//        user_def_key_msg(xcfg_cb.user_def_kt_sel);
//        break;
//
//    ///四击按键处理
//    case MSG_KEY_PP_FOUR:
//        user_def_key_msg(xcfg_cb.user_def_kfour_sel);
//        break;
//
//    ///五击按键处理
//    case MSG_KEY_PP_FIVE:
//        if (xcfg_cb.user_def_kfive_sel) {
//            user_def_key_msg(xcfg_cb.user_def_kfive_sel);
//        }
//        break;

    case MSG_SWITCH_SIRI:
        if (bt_nor_is_connected()) {
            bt_hfp_siri_switch();
        }
        break;

    case MSG_HID_HOME:
        bt_hid_consumer(HID_KEY_IOS_HOME);
        break;

    case MSG_SYS_1S:
        bt_hfp_report_bat();
#if BT_TWS_DBG_EN
        bt_tws_report_dgb();
#endif
        break;

    case EVT_A2DP_MUSIC_PLAY:
        if (!sbc_is_bypass()) {
            dac_fade_in();
        }
        if (f_bt.pp_2_unmute) {
            f_bt.pp_2_unmute = 0;
            bsp_clr_mute_sta();
        }
        break;

    case EVT_A2DP_MUSIC_STOP:
        if (!sbc_is_bypass() && !bsp_res_is_playing()) {
            dac_fade_out();
        }
        break;

    case EVT_KEY_2_UNMUTE:
        bsp_clr_mute_sta();
        break;

    default:
        func_message(msg);
        break;
    }
}

AT(.text.func.bt.msg)
void func_bt_message(u16 msg)
{
    if(msg){
        func_bt_message_do(msg);
    }
}

