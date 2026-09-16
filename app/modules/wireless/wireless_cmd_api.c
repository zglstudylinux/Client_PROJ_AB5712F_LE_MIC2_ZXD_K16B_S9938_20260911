#include "include.h"
#include "wireless_cmd.h"



void echo_audio_mute_set(u8 mute);
void plc_adpcm_audio_mute_set(u8 mute);
void sndp_audio_mute_set(u8 mute);
void ains3_audio_mute_set(u8 mute);
void src_audio_mute_set(u8 mute);
void dac_out_audio_mute_set(u8 mute);
void magic_mic_mute_set(u8 mute);
void usb_mic_in_audio_mute_set(u8 mute);
void howling_audio_mute_set(u8 mute);
void magic_mic_mute_set(u8 mute);
void mic_mute_set(u8 mute);
bool bsp_set_volume(u8 vol);
bool usb_device_hid_send(u16 hid_val, int auto_release);


/*****************************************************************************
 * Module    : 作用于发射端代码
 *****************************************************************************/
AT(.text.wireless_cmd.usb)
void wireless_tx_usb_cmd(u8 msg, u8 param)
{
    wireless_cmd_t pdu;

    pdu.cmd = PRIVATE_USB_CMD;
    pdu.buf[0] = msg;
    pdu.buf[1] = param;

    wireless_send_cmd(0, (u8 *)&pdu, 3);
}

AT(.text.wireless_cmd.user)
void wireless_tx_user_cmd(u8 *ptr, u8 len)
{
    wireless_cmd_t pdu;
    len = (len>TX_MAX_BUF_SIZE)? TX_MAX_BUF_SIZE : len;

    pdu.cmd = PRIVATE_USER_DATA;
    memcpy(&pdu.buf[0], ptr, len);

    wireless_send_cmd(0, (u8 *)&pdu, len+1);
}

AT(.text.wireless_cmd.user)
void wireless_tx_pwr_ctr_cmd(u8 index, u8 pwr_level)
{
    wireless_cmd_t pdu;

    pdu.cmd = PRIVATE_PWR_CTR_CMD;
    pdu.buf[0] = pwr_level;

    wireless_send_cmd(index, (u8 *)&pdu, 2);
}

AT(.text.wireless_cmd.mute)
void wireless_tx_ws_mic_cmd(u8 sub_cmd, u8 mute_enable)
{
    wireless_cmd_t pdu;
    pdu.cmd = PRIVATE_WS_MIC_CMD;
    pdu.buf[0] = sub_cmd;
    pdu.buf[1] = mute_enable;

    wireless_send_cmd(0, (u8 *)&pdu, 3);
}
//
// AT(.text.wireless_cmd)
//void mic_mute_enable(u8 mute_enable)
//{
//    if (wireless_role_is_adapter()) {
//        wireless_tx_ws_mic_cmd(MIC_MUTE, mute_enable);
//    } else {
//        mic_mute_set(mute_enable);
//    }
//}
//
//AT(.text.wireless_cmd)
//void ains3_audio_mute_enable(u8 mute_enable)
//{
//    if (wireless_role_is_adapter()) {
//        wireless_tx_ws_mic_cmd(AINS3_AUDIO_MUTE, mute_enable);
//    } else {
//        ains3_audio_mute_set(mute_enable);
//    }
//}
//
//AT(.text.wireless_cmd)
//void echo_audio_mute_enable(u8 mute_enable)
//{
//    if (wireless_role_is_adapter()) {
//        wireless_tx_ws_mic_cmd(ECHO_AUDIO_MUTE, mute_enable);
//    } else {
//        echo_audio_mute_set(mute_enable);
//    }
//}
//
//AT(.text.wireless_cmd)
//void magic_audio_mute_enable(u8 mute_enable)
//{
//    if (wireless_role_is_adapter()) {
//        wireless_tx_ws_mic_cmd(MAGIC_AUDIO_MUTE, mute_enable);
//    } else {
//        magic_mic_mute_set(mute_enable);
//    }
//}
//
//AT(.text.howling_mic_api)
//void howling_audio_mute_enable(u8 mute_enable)
//{
//    if (wireless_role_is_adapter()) {
//        wireless_tx_ws_mic_cmd(HOWLING_AUDIO_MUTE, mute_enable);
//    } else {
//        howling_audio_mute_set(mute_enable);
//    }
//}
//
//AT(.text.wireless_cmd)
//void adpcm_coder_audio_mute_enable(u8 mute_enable)
//{
//
//}

/*****************************************************************************
 * Module    : 作用于接收端代码
 *****************************************************************************/
static const u16 ws_msc_msg[] = {
    MSG_VOL_UP,             //MUSIC_CTL_VOL_UP
    MSG_VOL_DOWN,           //MUSIC_CTL_VOL_DOWN
    MSG_MUSIC_PREV,         //MUSIC_CTL_PREV
    MSG_MUSIC_NEXT,         //MUSIC_CTL_NEXT
    MSG_MUSIC_PLAY_PAUSE,   //MUSIC_CTL_PLAY_PAUSE
    MSG_VOICE_RM,           //MUSIC_CTL_VOICE_REMOVE
};

AT(.text.wireless_cmd)
void wireless_ws_mic_cmd(wireless_cmd_t *pdu)
{
    u8 sub_cmd = pdu->buf[0];
//    u8 mute_enable = pdu->buf[1];

    switch(sub_cmd) {
//        case ECHO_AUDIO_MUTE:
//            echo_audio_mute_enable(mute_enable);
//            break;
//
//        case ADPCM_CODER_MUTE:
//            adpcm_coder_audio_mute_enable(mute_enable);
//            break;
//
//        case ADPCM_DECODER_MUTE:
//            adpcm_decoder_mute_enable(mute_enable);
//            break;
//
//        case PLC_ADPCM_AUDIO_MUTE:
//            plc_adpcm_audio_mute_set(mute_enable);
//            break;
//
//        case SNDP_AUDIO_MUTE:
//            sndp_audio_mute_set(mute_enable);
//            break;
//
//        case SRC_AUDIO_MUTE:
//            src_audio_mute_set(mute_enable);
//            break;
//
//        case DAC_OUT_AUDIO_MUTE:
//            dac_out_audio_mute_set(mute_enable);
//            break;
//
//        case USB_MIC_IN_AUDIO_MUTE:
//            usb_mic_in_audio_mute_set(mute_enable);
//            break;
//
//        case MAGIC_AUDIO_MUTE:
//            magic_mic_mute_set(mute_enable);
//            break;
//
//        case HOWLING_AUDIO_MUTE:
//            howling_audio_mute_set(mute_enable);
//            break;
//
//        case MIC_MUTE:
//            mic_mute_enable(mute_enable);
//            break;
//
//        case AINS3_AUDIO_MUTE:
//            ains3_audio_mute_set(mute_enable);
//            break;

#if ECHO_EN
        case ECHO_DELAY_LEVEL_UP:
            echo_delay_level_up();
            break;

        case ECHO_DELAY_LEVEL_DOWN:
            echo_delay_level_down();
            break;
#endif
#if MAGIC_EN
        case MAGIC_LEVEL_CHANGE:
            magic_effect_level_change();
            break;
#endif

        case MUSIC_CTL_VOL_UP:
        case MUSIC_CTL_VOL_DOWN:
        case MUSIC_CTL_PREV:
        case MUSIC_CTL_NEXT:
        case MUSIC_CTL_PLAY_PAUSE:
        case MUSIC_CTL_VOICE_REMOVE:
            msg_enqueue(ws_msc_msg[(sub_cmd-MUSIC_CTL_1ST)]);
            break;

        default:
            break;
    }
}

#if ADAPTER_USB_SPK_TX_EN || ADAPTER_USB_MIC_RX_EN
AT(.text.wireless_cmd.usb)
void wireless_rx_usb_cmd(wireless_cmd_t *pdu)
{
    u8 sub_cmd = pdu->buf[0];
    u8 param = pdu->buf[1];

    switch(sub_cmd) {
        case USB_SET_SPK_VOLUME:
            bsp_set_volume(param);
            break;

        case USB_CTL_PLAY_PAUSE:
//            printf("UDE_HID_PLAYPAUSE\n");
            usb_device_hid_send(UDE_HID_PLAYPAUSE, 1);
            break;

        case USB_CTL_VOLUME_UP:
//            printf("UDE_HID_VOLUP\n");
            usb_device_hid_send(UDE_HID_VOLUP, 1);
            break;

        case USB_CTL_VOLUME_DOWN:
//            printf("UDE_HID_VOLDOWN\n");
            usb_device_hid_send(UDE_HID_VOLDOWN, 1);
            break;

        case USB_CTL_PREVFILE:
//            printf("UDE_HID_PREVFILE\n");
            usb_device_hid_send(UDE_HID_PREVFILE, 1);
            break;

        case USB_CTL_NEXTFILE:
//            printf("UDE_HID_NEXTFILE\n");
            usb_device_hid_send(UDE_HID_NEXTFILE, 1);
            break;

        case USB_CTL_MIC_STA:
//           printf("wireless_usb_mic_sta %d\n",wireless_usb_mic_sta);
//            wireless_usb_mic_sta = param;
        default:
            break;
    }
}
#endif

/*****************************************************************************
 * Module    : 公共部分代码
 *****************************************************************************/
AT(.text.wireless_cmd)
void wireless_rx_cmd(u8 index, u8 *ptr, u8 len)
{
    wireless_cmd_t *pdu = (void *)ptr;
//    printf("rx_cmd(%d,%d): ", pdu->buf[0], pdu->buf[1]);
//    print_r(ptr, len);

    if (pdu->cmd == PRIVATE_WS_MIC_CMD) {
        wireless_ws_mic_cmd(pdu);
#if ADAPTER_USB_SPK_TX_EN || ADAPTER_USB_MIC_RX_EN
    } else if(pdu->cmd == PRIVATE_USB_CMD) {
        wireless_rx_usb_cmd(pdu);
#endif
    } else if(pdu->cmd == PRIVATE_PWR_CTR_CMD) {
        ble_audio_pwr_ctr_set(index, pdu->buf[0]);
    } else {
        wireless_rx_user_cmd(index, ptr+1, len-1);
    }
}

AT(.text.wireless_cmd)
void wireless_cmd_init(void)
{
    wireless_cmd_buf_init();
}
