#ifndef __SYSTEM_H
#define __SYSTEM_H

#include "mute.h"
#include "param.h"
#include "device.h"
#include "vbat.h"

#define VOL_MAX                         xcfg_cb.vol_max   //最大音量级数

enum {
    PWROFF_IDLE,
    PWROFF_W4_TIMEOUT,
    PWROFF_END,
};

struct pwroff_tag {
    volatile union {
        struct {
            u8 hw_pdwn_ind      : 1;
            u8 ntc_ind          : 1;
            u8 low_bat_ind      : 1;
            u8 aux_insert_ind   : 1;
            u8 pwr_key_ind      : 1;
            u8 timeout_ind      : 1;
            u8 user_ind         : 1;    //app、tws等操作
        };
        u8 all_flag;
    };
    u32 ticks;                          //PWR键按下的时刻
    u32 delay_ticks;                    //PWR键长按超时
    bool tone_en;                       //是否播放pwroff提示音
    u8 state;
};

typedef struct {
    u32 rst_reason;
    u8  play_mode;
    u8  db_level;
    u8  vol;                    //非蓝牙模式系统音量级数
    u8  a2dp_vol;               //蓝牙音乐音量级数
    u8  hfp_vol;                //蓝牙通话音量级数
    u8  eq_mode;
    u8  cur_dev;
    u8  lang_id;
    u8  lpwr_warning_cnt;
    u8  lpwr_warning_times;     //播报低电次数，0xff表示一直播
    u8  vbat_nor_cnt;
    s8  gain_offset;            //用于动态修改音量表
    u8  hfp2sys_mul;            //系统音量与HFP音量倍数，手机HFP音量只有16级。
    u8  lpwr_cnt;               //低电计数
#if DAC_DNR_EN
    u8  dnr_sta;                //动态降噪状态
#endif
    u8 dac_sta_bck;
    u8 vusb_uart_flag;          //vusb uart打开标志
    u8 discon_reason;
#if CHARGE_LOW_POWER_EN
    u32 charge_timr0_pr;
#endif
volatile u8  incall_flag;

    struct pwroff_tag pwroff;

    u8 fmrx_type;
    u8 inbox_wken_flag;
    u8 outbox_pwron_flag;
    u16 vbat;
    u32 sleep_time;
    u32 pwroff_time;
    u32 sleep_delay;
    u32 pwroff_delay;
    u32 sleep_wakeup_time;
    u32 ms_ticks;               //ms为单位
    u32 rand_seed;
    u8  tws_res_brk;
    u8  tws_res_wait;
    u8  wav_sysclk_bak;
    u8 tkey_pwrdwn_en;
    u8 sw_rst_flag;

    u8 sleep_counter;       //sleepmode，定时计数
    u16 temp_tkcnt;
    u16 temp_tkcnt1;

volatile u8  cm_times;
volatile u8  loudspeaker_mute;  //功放MUTE标志
volatile u8  pwrkey_5s_check;   //是否进行开机长按5S的PWRKEY检测
volatile u8  pwrkey_5s_flag;    //长按5秒PWRKEY开机标志
volatile u8  charge_sta;        //0:充电关闭， 1：充电开启， 2：充满
volatile u8  charge_bled_flag;  //charge充满蓝灯常亮标志
volatile u8  ch_bled_cnt;       //charge充满蓝灯亮时间计数
volatile u8  micl_en;           //MICL enable working标志
volatile u8  poweron_flag;      //pwrkey开机标志
volatile u8  key2unmute_cnt;    //按键操作延时解mute
volatile u8  dc_in_filter;      //DC IN filter

#if REC_FAST_PLAY
    u16 rec_num;                //记录最新的录音文件编号
    u32 ftime;
#endif

#if ANC_ALG_EN
    volatile u8 anc_alg_en;
#endif // ANC_ALG_EN
    ///位变量不要用于需要在中断改值的变量。 请谨慎使用位变量，尽量少定义位变量。
    u8  rtc_first_pwron  : 1,   //RTC是否第一次上电
        mute             : 1,   //系统MUTE控制标志
        cm_factory       : 1,   //是否第一次FLASH上电
        cm_vol_change    : 1,   //音量级数是否需要更新到FLASH
        bthid_mode       : 1,   //独立HID拍照模式标志
        port2led_en      : 1,   //1个IO推两个灯
        voice_evt_brk_en : 1,   //播放提示音时，U盘、SD卡、LINEIN等插入事件是否立即响应.
        karaok_init      : 1;   //karaok初始化标志

    u8  sleep_en         : 1,   //用于控制是否进入sleep
		lowbat_flag      : 1,
        hfp_karaok_en    : 1;   //通话模式是否打开K歌功能。
    u8 led_scan_en;
    u8 sd_failed_file_cnt;      //sd卡内错误mp3格式文件个数
    u8 bt_is_inited;

    u8 led_sta;                 //led状态标志位
    u8 disp_sta;                //led显示状态

    u8 mute_flag;               //mute标志位

    u8 phone_type;              //设备类型标志位
    u8 usb_detec_flag;         //USB检测识别
    u8  ext_powet_flag;

#if WIRELESS_CON_PAIR_MODE
    u8 con_role_data[3];
    u8 con_role;
#endif
    u8 soft_gain_level;
    u8 wireless_id_sta;
#if DISP_BAT_EN
    u8 bat_level;			    //电量级数
    u8 bat_disp;                //显示电量标志
#endif
} sys_cb_t;
extern sys_cb_t sys_cb;

extern volatile int micl2gnd_flag;
extern volatile u32 ticks_50ms;


void sys_init(void);
void sys_update_init(void);
void timer1_irq_init(void);
void bsp_sys_mute(void);
void bsp_sys_unmute(void);
void bsp_clr_mute_sta(void);
void bsp_loudspeaker_mute(void);
void bsp_loudspeaker_unmute(void);
void uart0_mapping_sel(void);
void linein_detect(void);
bool linein_micl_is_online(void);
void get_usb_chk_sta_convert(void);
uint bsp_get_bat_level(void);
void sd_soft_cmd_detect(u32 check_ms);
void bsp_i2s_init(void);
void mic_channel_check(void);

#endif // __SYSTEM_H
