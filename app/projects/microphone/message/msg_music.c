#include "include.h"
#include "func.h"
#include "func_music.h"

#if FUNC_MUSIC_EN
AT(.text.func.music.msg)
void func_music_message(u16 msg)
{
    switch (msg) {
        case MSG_MUSIC_PLAY_PAUSE:
#if IR_NUMKEY_EN
            #if (GUI_SELECT != GUI_NO)
            if (box_cb.disp_sta == GUI_BOX_INPUTNUM) {
                box_cb.times = 0;           //确认数字选曲
                return;
            }
            #endif
#endif // IR_NUMKEY_EN

            if (f_msc.pause) {
                f_msc.pause = 0;
                music_control(MUSIC_MSG_PLAY);
                led_music_play();
                printf("music continue playing\n");
            } else {
                f_msc.pause = 1;
                bsp_clr_mute_sta();
                music_control(MUSIC_MSG_PAUSE);
                led_idle();
                printf("music pause\n");
            }
            break;

        case MSG_MUSIC_PREV:
            bsp_clr_mute_sta();
            func_music_switch_file(0);
            break;

        case MSG_MUSIC_NEXT:
            bsp_clr_mute_sta();
            func_music_switch_file(1);
            break;

#if MUSIC_QSKIP_EN
        case MSG_MUSIC_FF_START:
            bsp_clr_mute_sta();
            music_qskip(QSKIP_BACKWARD, 3);             //快进
            break;

        case MSG_MUSIC_FB_START:
            bsp_clr_mute_sta();
            music_qskip(QSKIP_FORWARD, 3);               //快退
            break;

        case MSG_MUSIC_FB_END:
        case MSG_MUSIC_FF_END:
            music_qskip_end();
            break;
#endif // MUSIC_QSKIP_EN

#if MUSIC_FOLDER_SELECT_EN
        case MSG_MUSIC_PREV_FOLDER:
            func_music_switch_dir(0);
            break;

        case MSG_MUSIC_NEXT_FOLDER:
            func_music_switch_dir(1);
            break;
#endif // MUSIC_FOLDER_SELECT_EN

        case MSG_MUSIC_PLAY_MODE:
            music_playmode_next();
            break;

        case MSG_CHANGE_MODE:
            bsp_clr_mute_sta();
            if ((f_msc.cur_dev == DEV_UDISK) && (dev_is_online(DEV_SDCARD) || dev_is_online(DEV_SDCARD1))) {
                f_msc.dev_change = 1;
                music_breakpoint_save();
            } else if ((f_msc.cur_dev == DEV_SDCARD1) && (dev_is_online(DEV_SDCARD))) {
                f_msc.dev_change = 1;
                music_breakpoint_save();
            } else {
                if (dev_is_online(DEV_UDISK)) {
                    sys_cb.cur_dev = DEV_UDISK;
                } else if (dev_is_online(DEV_SDCARD1)) {
                    sys_cb.cur_dev = DEV_SDCARD1;
                }
                func_cb.sta = FUNC_NULL;
            }
        break;

        case MSG_MUSIC_CHANGE_DEV:
            //双设备切换
            if (dev_is_online(DEV_UDISK) && (dev_is_online(DEV_SDCARD) || dev_is_online(DEV_SDCARD1))) {
                bsp_clr_mute_sta();
                f_msc.dev_change = 1;
                music_breakpoint_save();
            }
            break;

#if IR_NUMKEY_EN
        case MSG_KEY_NUM0:
        case MSG_KEY_NUM1:
        case MSG_KEY_NUM2:
        case MSG_KEY_NUM3:
        case MSG_KEY_NUM4:
        case MSG_KEY_NUM5:
        case MSG_KEY_NUM6:
        case MSG_KEY_NUM7:
        case MSG_KEY_NUM8:
        case MSG_KEY_NUM9:
            gui_box_show_inputnum(msg - KEY_NUM_0);
            break;
#endif // IR_NUMKEY_EN

        case EVT_UDISK_REMOVE:
            func_music_remove_device(DEV_UDISK);
            break;

        case EVT_SD_REMOVE:
            func_music_remove_device(DEV_SDCARD);
            break;

        case EVT_SD1_REMOVE:
            func_music_remove_device(DEV_SDCARD1);
            break;

        case EVT_UDISK_INSERT:
            //防止插入苹果手机打断SD卡播放
            if (!dev_udisk_activation_try(0)) {
                break;
            }
            func_music_insert_device(DEV_UDISK);
            break;

        case EVT_SD1_INSERT:
            func_music_insert_device(DEV_SDCARD1);
            break;

        case EVT_SD_INSERT:
            func_music_insert_device(DEV_SDCARD);
            break;

#if REC_FAST_PLAY
        case MSG_MUSIC_PLAY_RECORD:    //播放最新的录音.
            if ((sys_cb.rec_num) && (sys_cb.rec_num != f_msc.file_num)) {
                f_msc.file_change = 1;
                f_msc.file_num = sys_cb.rec_num;
            }
            break;
#endif

#if MUSIC_REC_FILE_FILTER
        case KL_MODE:
            if (f_msc.rec_type) {
                if (func_music_filter_switch(0)) {
                    f_msc.rec_type = 0;
                }
            } else {
                if (func_music_filter_switch(1)) {
                    f_msc.rec_type = 1;
                }
            }
            break;
#endif // MUSIC_REC_FILE_FILTER

        default:
            func_message(msg);
            break;
    }
}
#endif // FUNC_MUSIC_EN

