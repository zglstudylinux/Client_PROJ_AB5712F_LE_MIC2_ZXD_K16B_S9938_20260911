#include "include.h"
#include "func.h"
#include "func_idle.h"

#if FUNC_USBDEV_EN
AT(.text.func.usbdev.msg)
void func_usbdev_message(u16 msg)
{
    switch (msg) {
#if USB_IPHONE_PREMOTE_EN
        //短按，播放/暂停/接听
        case MSG_KEY_PP_KU:
            usb_device_hid_send(UDE_HID_PLAYPAUSE, 1);
            break;

        //长按PP, 拒接电话/语音助手
        case MSG_KEY_PP_KL:
            usb_device_hid_send(UDE_HID_PLAYPAUSE, 0);
            usb_device_hid_send(UDE_HID_PLAYPAUSE, 0);
            break;

        //PP抬键
        case MSG_KEY_PP_KLU:
            usb_device_hid_send(0, 0);
            break;
#else
        //短按，播放/暂停/接听
        case MSG_MUSIC_PLAY_PAUSE:
            usb_device_hid_send(UDE_HID_PLAYPAUSE, 1);
            break;
#endif

        case MSG_VOL_MUTE:
            if (sys_cb.mute) {
                bsp_sys_unmute();
            } else {
                bsp_sys_mute();
            }
            usb_device_hid_send(UDE_HID_MUTE, 1);
            break;

        //windows上一曲
        case MSG_MUSIC_PREV:
            if(!typec_computer_mobile_sta()){
                usb_device_hid_send(UDE_HID_PREVFILE, 1);
            }else{
                usb_device_hid_send(UDE_HID_PLAYPAUSE, 1);
                delay_5ms(1);
                usb_device_hid_send(UDE_HID_PLAYPAUSE, 1);
                delay_5ms(1);
                usb_device_hid_send(UDE_HID_PLAYPAUSE, 1);
            }
            break;

        //windows下一曲
        case MSG_MUSIC_NEXT:
            if(!typec_computer_mobile_sta()){
                usb_device_hid_send(UDE_HID_NEXTFILE, 1);
            }else{
                usb_device_hid_send(UDE_HID_PLAYPAUSE, 1);
                delay_5ms(1);
                usb_device_hid_send(UDE_HID_PLAYPAUSE, 1);
            }
            break;

        case MSG_VOL_DOWN:
            usb_device_hid_send(UDE_HID_VOLDOWN, 1);
            break;

        case MSG_VOL_UP:
            usb_device_hid_send(UDE_HID_VOLUP, 1);
            break;

        case MSG_MUSIC_STOP:
            //usb_device_hid_send(UDE_HID_STOP, 1);
            break;

#if UDE_STORAGE_EN
        case EVT_SD_INSERT:
            ude_sdcard_switch(DEV_SDCARD);
            break;

        case EVT_SD_REMOVE:
            break;
#endif // UDE_STORAGE_EN

        case EVT_PC_REMOVE:
            func_cb.sta = func_cb.last;
            break;

        default:
            func_message(msg);
            break;
    }
}
#endif
