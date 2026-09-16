#ifndef _WIRELESS_CMD_API_H
#define _WIRELESS_CMD_API_H

#define TX_MAX_BUF_SIZE         8

//内部定义
typedef enum{
    PRIVATE_WS_MIC_CMD      = 0,
    PRIVATE_USB_CMD,
    PRIVATE_SYNC_CMD,           ///tx端发送变化信息给rx端，rx端再把信息发给另一个tx端实现两个tx同步
    PRIVATE_USER_DATA,
    PRIVATE_PWR_CTR_CMD,
} cmd_t;

typedef struct wireless_cmd{
    u8 cmd;
    u8 buf[TX_MAX_BUF_SIZE];
} wireless_cmd_t;

//无线麦控制
typedef enum{
    ECHO_AUDIO_MUTE         = 0,
    ADPCM_CODER_MUTE,

    ADPCM_DECODER_MUTE,
    PLC_ADPCM_AUDIO_MUTE,
    SNDP_AUDIO_MUTE,
    AINS3_AUDIO_MUTE,
    SRC_AUDIO_MUTE,
    DAC_OUT_AUDIO_MUTE,
    USB_MIC_IN_AUDIO_MUTE,
    MAGIC_AUDIO_MUTE,
    HOWLING_AUDIO_MUTE,
    MIC_MUTE,

    ECHO_DELAY_LEVEL_UP     = 0x40,     //混响等级加
    ECHO_DELAY_LEVEL_DOWN,              //混响等级减
    MAGIC_LEVEL_CHANGE,                 //切换魔音音效

    MUSIC_CTL_1ST           = 0x60,
    MUSIC_CTL_VOL_UP = MUSIC_CTL_1ST,   //音乐音量加
    MUSIC_CTL_VOL_DOWN,                 //音乐音量减
    MUSIC_CTL_PREV,                     //上一曲
    MUSIC_CTL_NEXT,                     //下一曲
    MUSIC_CTL_PLAY_PAUSE,               //播放/暂停
    MUSIC_CTL_VOICE_REMOVE,             //人声消除切换

    /// add
    WIRLESS_MSG_MAGIC_NORMAL    =0x70,
    WIRLESS_MSG_MAGIC_ECHO,
    WIRLESS_MSG_MAGIC_MAN,
    WIRLESS_MSG_MAGIC_WOMAN,
    WIRLESS_MSG_MAGIC_KIDS,
    WIRLESS_MSG_MAGIC_MONSTER,

    WIRLESS_MSG_MIC_ON,
    WIRLESS_MSG_MIC_OFF,
    WIRLESS_MSG_MAGIC_KEY,//从麦克风过来的magic按键消息

    WIRLESS_MSG_SET_MUSIC,
    WIRLESS_MSG_SET_MIC,
    WIRLESS_MSG_SET_ECHO,

    WIRLESS_MSG_MIC_VOL_MAX,
    WIRLESS_MSG_MIC_VOL_MIN,
    WIRLESS_MSG_MIC_VOL_UP,
    WIRLESS_MSG_MIC_VOL_DOWN,

    WIRLESS_MSG_ECHO_MAX,
    WIRLESS_MSG_ECHO_MIN,
    WIRLESS_MSG_ECHO_UP,
    WIRLESS_MSG_ECHO_DOWN,
    WIRLESS_MSG_DU,
    WIRLESS_MSG_MIC_LOW_BAT,

} sub_ws_mic_cmd_t;

//USB播放控制
typedef enum{
    USB_SET_SPK_VOLUME      = 0,
    USB_CTL_MIC_STA,
    USB_CTL_PLAY_PAUSE,
    USB_CTL_VOLUME_UP,
    USB_CTL_VOLUME_DOWN,
    USB_CTL_PREVFILE,
    USB_CTL_NEXTFILE,
    USB_CTL_WDT_RESET,
} sub_usb_cmd_t;


//device <==> adapter私有命令api
void wireless_cmd_init(void);
void wireless_rx_cmd(u8 index, u8 *ptr, u8 len);
void wireless_tx_usb_cmd(u8 msg, u8 param);
void wireless_tx_user_cmd(u8 *ptr, u8 len);
void wireless_tx_ws_mic_cmd(u8 sub_cmd, u8 mute_enable);

//device <==> adpater USB播放控制api
#if !ADAPTER_USB_SPK_TX_EN && !ADAPTER_USB_MIC_RX_EN
    #define wireless_music_play_pause()
    #define wireless_music_volup()
    #define wireless_music_voldown()
    #define wireless_music_prev()
    #define wireless_music_next()
    #define wireless_set_usbspk_vol(vol)
#else
    #define wireless_music_play_pause()             wireless_tx_usb_cmd(USB_CTL_PLAY_PAUSE, 0)      //切换播放/暂停
    #define wireless_music_volup()                  wireless_tx_usb_cmd(USB_CTL_VOLUME_UP, 0)       //音量加
    #define wireless_music_voldown()                wireless_tx_usb_cmd(USB_CTL_VOLUME_DOWN, 0)     //音量减
    #define wireless_music_prev()                   wireless_tx_usb_cmd(USB_CTL_PREVFILE, 0)        //上一曲
    #define wireless_music_next()                   wireless_tx_usb_cmd(USB_CTL_NEXTFILE, 0)        //下一曲
    #define wireless_set_usbspk_vol(vol)            wireless_tx_usb_cmd(USB_SET_SPK_VOLUME, vol)    //spk_vol
#endif

#if !ADAPTER_USB_MIC_RX_EN
    #define wireless_set_usbmic_status(start)
#else
    #define wireless_set_usbmic_status(start)       wireless_tx_usb_cmd(USB_SET_SPK_VOLUME, start)  //mic_status
#endif

//device <==> adapter无线麦命令
#define wirless_echo_level_up()                     wireless_tx_ws_mic_cmd(ECHO_DELAY_LEVEL_UP, 1)
#define wirless_echo_level_up_max()                 wireless_tx_ws_mic_cmd(ECHO_DELAY_LEVEL_UP, 2)
#define wirless_echo_level_down()                   wireless_tx_ws_mic_cmd(ECHO_DELAY_LEVEL_DOWN, 1)
#define wirless_echo_level_down_min()               wireless_tx_ws_mic_cmd(ECHO_DELAY_LEVEL_DOWN, 2)

//#define wirless_magic_change()                      wireless_tx_ws_mic_cmd(MAGIC_LEVEL_CHANGE, 1)
#define wirless_music_vol_up()                      wireless_tx_ws_mic_cmd(MUSIC_CTL_VOL_UP, 1)
#define wirless_music_vol_down()                    wireless_tx_ws_mic_cmd(MUSIC_CTL_VOL_DOWN, 1)
#define wirless_music_prev()                        wireless_tx_ws_mic_cmd(MUSIC_CTL_PREV, 1)
#define wirless_music_next()                        wireless_tx_ws_mic_cmd(MUSIC_CTL_NEXT, 1)
//#define wirless_music_play_pause()                  wireless_tx_ws_mic_cmd(MUSIC_CTL_PLAY_PAUSE, 1)
//#define wirless_music_voice_remove()                wireless_tx_ws_mic_cmd(MUSIC_CTL_VOICE_REMOVE, 1)

#define wirless_mic_vol_up()                        wireless_tx_ws_mic_cmd(WIRLESS_MSG_MIC_VOL_UP, 1)
#define wirless_mic_vol_up_max()                    wireless_tx_ws_mic_cmd(WIRLESS_MSG_MIC_VOL_MAX, 1)
#define wirless_mic_vol_down()                      wireless_tx_ws_mic_cmd(WIRLESS_MSG_MIC_VOL_DOWN, 1)
#define wirless_mic_vol_down_min()                  wireless_tx_ws_mic_cmd(WIRLESS_MSG_MIC_VOL_MIN, 1)
#define wirless_music_prev()                        wireless_tx_ws_mic_cmd(MUSIC_CTL_PREV, 1)
#define wirless_music_next()                        wireless_tx_ws_mic_cmd(MUSIC_CTL_NEXT, 1)
#define wirless_music_play_pause()                  wireless_tx_ws_mic_cmd(MUSIC_CTL_PLAY_PAUSE, 1)
#define wirless_music_voice_remove()                wireless_tx_ws_mic_cmd(MUSIC_CTL_VOICE_REMOVE, 1)
#define wirless_music_mic_mute()                    wireless_tx_ws_mic_cmd(MIC_MUTE, 1)
#define wirless_magic_change()                      wireless_tx_ws_mic_cmd(WIRLESS_MSG_MAGIC_KEY, 1)
#define wirless_mic_mute()                          wireless_tx_ws_mic_cmd(WIRLESS_MSG_MIC_OFF, 2)
#define wirless_mic_unmute()                        wireless_tx_ws_mic_cmd(WIRLESS_MSG_MIC_ON, 2)

//#define wirless_magic_normal()                      wireless_tx_ws_mic_cmd(WIRLESS_MSG_MAGIC_NORMAL, 1)
//#define wirless_magic_echo()                         wireless_tx_ws_mic_cmd(WIRLESS_MSG_MAGIC_ECHO, 1)
//#define wirless_magic_man()                         wireless_tx_ws_mic_cmd(WIRLESS_MSG_MAGIC_MAN, 1)
//#define wirless_magic_woman()                         wireless_tx_ws_mic_cmd(WIRLESS_MSG_MAGIC_WOMAN, 1)
//#define wirless_magic_kids()                         wireless_tx_ws_mic_cmd(WIRLESS_MSG_MAGIC_KIDS, 1)
//#define wirless_magic_monster()                         wireless_tx_ws_mic_cmd(WIRLESS_MSG_MAGIC_MONSTER, 1)

#define wirless_set_music()                         wireless_tx_ws_mic_cmd(WIRLESS_MSG_SET_MUSIC, 1)
#define wirless_set_mic()                           wireless_tx_ws_mic_cmd(WIRLESS_MSG_SET_MIC, 1)
#define wirless_set_echo()                          wireless_tx_ws_mic_cmd(WIRLESS_MSG_SET_ECHO, 1)
#define wirless_music_vol_up()                      wireless_tx_ws_mic_cmd(MUSIC_CTL_VOL_UP, 1)
#define wirless_music_vol_down()                    wireless_tx_ws_mic_cmd(MUSIC_CTL_VOL_DOWN, 1)
#define wirless_set_voice_remove()                  wireless_tx_ws_mic_cmd(MUSIC_CTL_VOICE_REMOVE, 1)
#define wirless_mic_low_bat()                       wireless_tx_ws_mic_cmd(WIRLESS_MSG_MIC_LOW_BAT, 1)

#define wireless_tx_user_discon_cmd()               wireless_tx_user_link_cmd(DISCONNECT_NUM,1);

#if ADAPTER_USB_MIC_RX_EN
    #define wireless_wdt_reset()                    wireless_tx_usb_cmd(USB_CTL_WDT_RESET, 0)       //控制看门狗复位
#endif

//外部函数声明
void wireless_cmd_buf_init(void);
bool wireless_send_cmd(u8 index, u8 *cmd, u8 len);
void wireless_rx_user_cmd(u8 index, u8 *ptr, u8 len);
void wireless_tx_pwr_ctr_cmd(u8 index, u8 pwr_level);


#endif
