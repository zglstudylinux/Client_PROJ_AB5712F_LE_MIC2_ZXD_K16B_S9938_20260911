/*
 * pcm线程消息处理，处理硬件usb模块中断触发的消息处理
 *
 ****************************************************************************************
 */
#include "include.h"
#include "os_thread.h"

#if UDE_STORAGE_EN || UDE_SPEAKER_EN || UDE_HID_EN || UDE_MIC_EN

AT(.com_text.thread.aupcm)
void thread_aupcm_proc_msg_cb(u32 msg)
{
    switch (msg) {
        case MSG_UDE_EP_RESET:
            ude_ep_reset();
            break;

        case MSG_UDE_CTL_FLOW:
            ude_control_flow();
            break;

        case MSG_UDE_ISO_IN:
            ude_isoc_tx_process();
            break;

        case MSG_UDE_ISO_OUT:
            ude_isoc_rx_process();  //ude_isoc_rx_process_callback() 回调处理 接收到的数据
            break;

        case MSG_UDE_HID_OUT:
            ude_hid_get_data();    //ude_hid_get_data_callback() 回调hid 接收到的数据
            break;

#if UDE_IAP_PROCESS_EN
        case MSG_UDE_IAP_OUT:
            ude_iap_get_data();
            break;
        case MSG_UDE_IAP_IN:
//            ude_iap_tx_process();
            break;
#endif
    }
}


#endif
