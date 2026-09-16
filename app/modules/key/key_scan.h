#ifndef _BSP_KEY_H
#define _BSP_KEY_H

#define KEY_SCAN_TIMES          6           //按键防抖的扫描次数
#define KEY_UP_TIMES            6           //抬键防抖的扫描次数
#define KEY_LONG_TIMES          150         //长按键的次数
#define KEY_HOLD_TIMES          35          //连按的频率次数
#define KEY_LONG_HOLD_TIMES     (KEY_LONG_TIMES + KEY_HOLD_TIMES)

//key_type
#define KEY_TYPE_POS            6
#define KEY_TYPE_MASK           (0x3 << KEY_TYPE_POS)
#define KEY_TYPE_UNUSE          (0x0 << KEY_TYPE_POS)
#define KEY_TYPE_SINGLE         (0x1 << KEY_TYPE_POS)    //单击
#define KEY_TYPE_NORMAL         (0x2 << KEY_TYPE_POS)    //单击、长按
#define KEY_TYPE_MULTI          (0x3 << KEY_TYPE_POS)    //单击、长按、多击

//key_id
#define KEY_UID_MASK            0xff
#define KEY_ID_MASK             0x3f
#define KEY_SINGLE(key_id)      (key_id | KEY_TYPE_SINGLE)  //仅支持单击按键
#define KEY_NORMAL(key_id)      (key_id | KEY_TYPE_NORMAL)  //支持单击、长按的按键
#define KEY_MULTI(key_id)       (key_id | KEY_TYPE_MULTI)   //支持多击的按键

//key_val
#define KEY_EVT_POS             12
#define KEY_EVT_MASK            (0xF << KEY_EVT_POS)
#define KEY_SHORT               (0x0 << KEY_EVT_POS)
#define KEY_SHORT_UP            (0x1 << KEY_EVT_POS)
#define KEY_LONG                (0x2 << KEY_EVT_POS)
#define KEY_LONG_UP             (0x3 << KEY_EVT_POS)
#define KEY_HOLD                (0x4 << KEY_EVT_POS)
#define KEY_LHOLD               (0x5 << KEY_EVT_POS)
#define KEY_DOUBLE              (0x6 << KEY_EVT_POS)        //2击
#define KEY_THREE               (0x7 << KEY_EVT_POS)        //3击
#define KEY_FOUR                (0x8 << KEY_EVT_POS)        //4击
#define KEY_FIVE                (0x9 << KEY_EVT_POS)        //5击

#define KEY_EVT_IDX(n)          ((n) >> KEY_EVT_POS)



//POWER KEY相关0xe0打头，便于开关机按键判断 (0xe0 ~ 0xef)
#define K_PWR_MASK              0xF0
#define K_PWR                   0xe0
#define KEY_PWR0                0xe0
#define KEY_PWR1                0xe1
#define KEY_PWR2                0xe2
#define KEY_PWR3                0xe3

#define NO_KEY                  0

//key_id: 0~0x3f
enum {
    KEY_1       = 0x00,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_10,
    KEY_11,
    KEY_12,
    KEY_13,
    KEY_14,
    KEY_15,

    KEY_NUM_0,
    KEY_NUM_1,
    KEY_NUM_2,
    KEY_NUM_3,
    KEY_NUM_4,
    KEY_NUM_5,
    KEY_NUM_6,
    KEY_NUM_7,
    KEY_NUM_8,
    KEY_NUM_9,
    KEY_NUM_10,
};

//Message
enum {
    //Key Message
    MSG_NO                  = 0,
    MSG_PWR_OFF,                    //按键关机
    MSG_PWR_HOLD,                   //PWR键按住
    MSG_PWR_RELEASE,                //PWR键释放
    MSG_CHANGE_MODE,                //切换模式
    MSG_CHANGE_LANGUAGE,            //切换提示音语言
    MSG_ENTER_DUT,                  //进入DUT测试
    MSG_EXIT_DUT,                   //退出DUT测试
    MSG_VOL_UP,                     //音量加
    MSG_VOL_DOWN,                   //音量减
    MSG_VOL_MUTE,                   //音量MUTE
    MSG_CHANGE_ANC,                 //切换ANC效果
    MSG_CHANGE_EQ,                  //切换EQ效果
    MSG_CHANGE_MAGIC,               //切换魔音效果
    MSG_ECHO_LEVEL_UP,
    MSG_ECHO_LEVEL_DOWN,
    MSG_VOICE_RM,                   //消除原声

    MSG_RECORD_START,               //开始录音
    MSG_RECORD_PAUSE,               //暂停录音
    MSG_RECORD_STOP,                //结束录音
    MSG_DELETE_FILE,

    MSG_MUSIC_PLAY          = 0x80, //播放
    MSG_MUSIC_STOP,                 //停止
    MSG_MUSIC_PAUSE,                //暂停
    MSG_MUSIC_PLAY_PAUSE,           //切换播放/暂停
    MSG_MUSIC_PREV,                 //上一曲
    MSG_MUSIC_NEXT,                 //下一曲
    MSG_MUSIC_FB_START,         //快退开始
    MSG_MUSIC_FB_END,           //快退结束
    MSG_MUSIC_FF_START,             //快进开始
    MSG_MUSIC_FF_END,               //快进结束
    MSG_MUSIC_CHANGE_DEV,           //切换播放设备
    MSG_MUSIC_NEXT_FOLDER,          //播放下一个文件夹
    MSG_MUSIC_PREV_FOLDER,          //播放上一个文件夹
    MSG_MUSIC_PLAY_MODE,
    MSG_MUSIC_PLAY_RECORD,
    MSG_MUSIC_AB_REPEAT,            //设置AB重复
    MSG_MUSIC_LOW_LATENCY,          //设置低延迟模式

    MSG_CALL_HANG_UP,               //挂断
    MSG_CALL_ANSWER,                //接听
    MSG_CALL_SWITCH_PRIV,           //切换私密接听
    MSG_CALL_SWITCH_3WAY_CALL,      //切换三方通话
    MSG_CALL_REDIAL_LAST,           //回拨最后一个号码
    MSG_SWITCH_SIRI,                //呼叫/退出SIRI

    MSG_BT_CONNECT,
    MSG_BT_DISCONNECT,
    MSG_HID_CONNECT,
    MSG_HID_DISCONNECT,
    MSG_HID_KEY,
    MSG_HID_HOME,
    MSG_HID_ENTER,
    MSG_HID_VOL_UP,
    MSG_HID_VOL_DOWN,

    MSG_DENOISE_ON          =0x90,  //开启降噪
    MSG_DENOISE_OFF,                //关闭降噪
    MSG_TX_POWER_OFF,               //关机


    MSG_KEY_PP_KU           = 0xE0, //多功能键短按抬起（单击）
    MSG_KEY_PP_KL,                  //多功能键长按
    MSG_KEY_PP_HOLD,                //多功能键按住
    MSG_KEY_PP_KLU,                 //多功能键长按抬起
    MSG_KEY_PP_DOUBLE,              //多功能键双击
    MSG_KEY_PP_THREE,               //多功能键三击
    MSG_KEY_PP_FOUR,                //多功能键四击
    MSG_KEY_PP_FIVE,                //多功能键五击

    MSG_KEY_USER1_DOUBLE,
    MSG_KEY_USER2_DOUBLE,
    MSG_KEY_USER_THREE,

    MSG_KEY_NUM0,
    MSG_KEY_NUM1,
    MSG_KEY_NUM2,
    MSG_KEY_NUM3,
    MSG_KEY_NUM4,
    MSG_KEY_NUM5,
    MSG_KEY_NUM6,
    MSG_KEY_NUM7,
    MSG_KEY_NUM8,
    MSG_KEY_NUM9,
    MSG_KEY_NUM10,


    //System Event, 范围：0x700 ~ 0x7ff
    MSG_SYS_1S            = 0x7ff,
    MSG_SYS_500MS         = 0x7fe,
    EVT_SD_INSERT         = 0x7fd,
    EVT_SD_REMOVE         = 0x7fc,
    EVT_SD1_INSERT        = 0x7fb,
    EVT_SD1_REMOVE        = 0x7fa,
    EVT_UDISK_INSERT      = 0x7f9,
    EVT_UDISK_REMOVE      = 0x7f8,
    EVT_PC_INSERT         = 0x7f7,
    EVT_PC_REMOVE         = 0x7f6,
    EVT_LINEIN_INSERT     = 0x7f5,
    EVT_LINEIN_REMOVE     = 0x7f4,
    EVT_HFP_SET_VOL       = 0x7f3,
    EVT_A2DP_SET_VOL      = 0x7f2,       //手机端调音量
    EVT_A2DP_MUSIC_PLAY   = 0x7f1,
    EVT_A2DP_MUSIC_STOP   = 0x7f0,
    EVT_ONLINE_SET_EFFECT = 0x7ef,
    EVT_ONLINE_SET_EQ     = 0x7ee,
    EVT_KEY_2_UNMUTE      = 0x7ec,
    EVT_TWS_HID_SHUTTER   = 0x7eb,
    EVT_BT_SET_LANG_ID    = 0x7ea,
    EVT_BT_SET_EQ         = 0x7e9,
    EVT_MIC_INSERT        = 0x7e8,
    EVT_MIC_REMOVE        = 0x7e7,
    EVT_ECHO_LEVEL        = 0x7e6,
    EVT_MIC_VOL           = 0x7e5,
    EVT_MUSIC_VOL         = 0x7e4,
    EVT_CHARGE_INBOX      = 0x7e3,
    EVT_ONLINE_SET_ANC    = 0x7e2,
    EVT_ONLINE_SET_ENC    = 0x7e1,
    EVT_ONLINE_SET_SMIC   = 0x7e0,
    EVT_CHARGE_DCIN       = 0x7df,

    EVT_BLE_ADV0_BAT      = 0x7de,
    EVT_AUDIO_BY_PASS     = 0x7dd,
    EVT_AUDIO_ENABLE      = 0x7dc,

    EVT_TWS_INIT_VOL      = 0x7db,       //TWS连接上同步音量
    EVT_TWS_SET_VOL       = 0x7da,       //TWS按键调音量
    EVT_MSG_SYS_30S       = 0x7d9,
    EVT_MSG_DMA_TWS_SYNC  = 0x7d8,       //TWS连接上，用于主机发同步数据给附机
    EVT_TWS_LED_SYNC      = 0x7d7,
    EVT_LED_BT_IDLE       = 0x7d6,

    EVT_BT_TWS_EAR_STA    = 0x7d5,
    EVT_BT_MENU_VA_EN     = 0x7d4,
    EVT_BT_MENU_VA_DIS    = 0x7d3,
    EVT_NR_STA_CHANGE     = 0x7d2,
    EVT_QTEST_PICKUP_PWROFF = 0x7d1,
    EVT_BT_UPDATE_STA     = 0x7d0,

    EVT_A2DP_LLTY_EN      = 0x7cf,
    EVT_A2DP_LLTY_DIS     = 0x7ce,
    EVT_DEV_SAVE_VOL      = 0x7cd,

    EVT_AUTO_PWFOFF_EN    = 0x7cc,
    EVT_AUTO_PWFOFF_DIS   = 0x7cb,
    EVT_IODM_TEST         = 0x7ca,

    EVT_A2DP_SAVE_DEV_VOL = 0x7c9,
    EVT_A2DP_SYNC_CLOSE   = 0x7c8,

    EVT_UDE_SET_VOL       = 0x7c7,
    EVT_DBB_VOL_SET       = 0x7c6,

    EVT_DENOISE_SYNC      = 0x7c5,
    EVT_HCI_CMD           = 0x7c4,

    EVT_CHARGE_DCOUT      = 0x7c3,

    EVT_HUART_COMMAND_PROC = 0x7c2,
    EVT_ONLINE_SET_BTMIC_DUMP = 0x7c1,
};




extern const u8 key_evt_idx_tbl[];
extern const u8 music_key_msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX];
extern const u8 aux_key_msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX];
extern const u8 speaker_key_msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX];
extern const u8 bt_music_key_msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX];
extern const u8 bt_call_key_msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX];
extern const u8 usbdev_key_msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX];
extern const u8 wireless_mic_key_msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX];
void key_set_msg_tbl(const void *msg_tbl);



//按键消抖参数
typedef struct {
    u16 scan_cnt;
    u16 up_cnt;
    u16 long_cnt;
    u16 hold_cnt;
} key_shake_tbl_t;

typedef struct {
    u8 adc_val;
    u8 usage_id;
} adkey_tbl_t;

extern const adkey_tbl_t adkey_table[];
extern const adkey_tbl_t adkey2_table[];
extern const adkey_tbl_t pwrkey_table[];

void io_key_init(void);
u8 get_iokey(void);
void key_var_init(void);
void key_init(void);
void key_set_ignore(u8 usage_id);
u8 bsp_key_scan(void);
void power_on_check(void);

void pwr_down_flag_clr(void);
void pwr_down_flag_set(void);
bool pwrkey_get_status(void);
u8 get_double_key_time(void);
void adkey_mux_sdclk_w4_convert(void);

u8 bsp_key_scan_do(void);
u8 bsp_key_pwr_scan(void);

#include "pwrkey.h"
#include "adkey.h"
#include "tkey.h"
#include "irkey.h"

#endif // _BSP_KEY_H


