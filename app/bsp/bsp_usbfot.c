#include "include.h"

#if ADAPTER_AB_FOT_DEVICE_SUPPORT
typedef struct _fot_usbdev_t
{
    u8 fot_cnt;
    u8 fot_flag;
    u8 fot_exit_cnt;
    u8 fot_update_en_flag;            //是否进入升级模式
    u8 fot_had_update_flag;           //是否进入过升级模式
}fot_usbdev_t;
fot_usbdev_t fot_usbdev;

AT(.text.func.usbdev)
bool ude_fot_sta(void)
{
    return fot_usbdev.fot_flag;
}

AT(.text.func.usbdev)
u8 fot_update_en_flag_get(void)
{
    return fot_usbdev.fot_update_en_flag;
}

AT(.text.func.usbdev)
void fot_update_en_switch(void)
{
    fot_usbdev.fot_update_en_flag ^= 1;               //按键进入升级模式，此处先这样，后面在修改
}

AT(.text.func.usbdev)
void fot_check(void)
{
    static u32 ticks = 0;
    if(fot_update_en_flag_get()){                     //是否进入升级模式
        fot_usbdev.fot_exit_cnt = 0;                  //清空退出升级模式的cnt，确保下一次可以成功枚举audio
        fot_usbdev.fot_had_update_flag = 1;           //此标志位表示已进去过升级模式，完成storage的枚举
        if(tick_check_expire(ticks,200)){
            ticks = tick_get();
            if(fot_usbdev.fot_cnt < 60)
            {
                fot_usbdev.fot_cnt++;
            }
        }
        if(fot_usbdev.fot_cnt < 50){
            fot_usbdev.fot_flag = 1;
            if(ude_fot_sta()){
                usb_device_enter(UDE_ONLY_STORAGE_TYPE);
            }else{
                usb_device_enter(UDE_ON_STORAGE_TYPE);
            }
        }
    } else {
        fot_usbdev.fot_cnt = 0;                         //清空进入升级模式的cnt，确保下一次可以成功枚举成storage
        if(fot_usbdev.fot_had_update_flag){
            if(tick_check_expire(ticks,200)){
                ticks = tick_get();
                if(fot_usbdev.fot_exit_cnt < 60)
                {
                    fot_usbdev.fot_exit_cnt++;
                }
            }
            if(fot_usbdev.fot_exit_cnt < 50){
                fot_usbdev.fot_flag = 0;
                if(ude_fot_sta()){
                    usb_device_enter(UDE_ONLY_STORAGE_TYPE);
                }else{
                    usb_device_enter(UDE_ON_STORAGE_TYPE);
                }
                return;
            }
            fot_usbdev.fot_had_update_flag = 0;            //枚举完才清零
        }
    }
}
#endif // ADAPTER_AB_FOT_DEVICE_SUPPORT
