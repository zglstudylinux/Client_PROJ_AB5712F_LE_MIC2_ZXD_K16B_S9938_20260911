#include "include.h"
#include "func.h"
#include "usb_device/usb_enum.h"

AT(.text.func.wireless_mic)
void func_adapter_message(u16 msg)
{
    switch (msg) {
#if ADAPTER_USB_STORAGE_AUDIO
    case EVT_SD_INSERT:
        sys_clk_req(INDEX_MUSIC, SYS_120M);                     //先抬高主频
        sys_clk_req(INDEX_MUSIC, ADAPTER_USB_STORAGE_AUDIO_CLK);
        SD_IO_INIT();
        sd_disk_init();
        sd_insert();
        sd_enable_user();
        sd0_init();

        fsdisk_callback_init(0);
        fs_mount();
        break;
    case EVT_SD_REMOVE:
        sys_clk_free(INDEX_MUSIC);
        break;
#endif
    case MSG_SYS_1S:
#if WIRELESS_CON_PWR_CTR
        ws_pwr_ctr_tx_cmd_process();
#endif
#if ADAPTER_HUART_OUTPUT_EN && ADAPTER_HUART_COMMAND_EN
        huart_cmd_sync();
#endif
        break;
#if ADAPTER_USB_MIC_RX_EN
    case EVT_PC_INSERT:
        printf("EVT_PC_INSERT\n");
        ude_info_init();
        usb_device_enter(UDE_ENUM_TYPE);
        break;

    case EVT_PC_REMOVE:
        printf("EVT_PC_REMOVE\n");
        usb_device_exit();
        break;
#endif

#if ADAPTER_HUART_COMMAND_EN
    case EVT_HUART_COMMAND_PROC:
        if(huart_role_audio_out()) {
            huart_rx_msg_process();
        } else {
            uart_command_rx_proc();
        }
        break;
#endif

#if ADAPTER_USB_MIC_RX_EN && ADAPTER_USB_SPK_TX_EN
    case EVT_UDE_SET_VOL://PC向usb speaker设备发送的设置音量指令时，函数ude_set_sys_volume会发送EVT_UDE_SET_VOL消息
        spk_soft_gain_set_level(sys_cb.db_level);
        break;
#endif
    default:
        func_message(msg);
        break;
    }
}
