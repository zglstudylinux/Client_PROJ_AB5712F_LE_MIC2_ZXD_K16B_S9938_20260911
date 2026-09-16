#include "include.h"

volatile int pwrkey_detect_flag;            //pwrkey 820K用于复用检测的标志。


void plugin_init(void)
{
    CLKGAT0 = BIT(0)|BIT(12);
    CLKGAT1 = BIT(1)|BIT(2)|BIT(3)|BIT(18)|BIT(19)|BIT(20)|BIT(21)|BIT(22)|BIT(23)|BIT(24)|BIT(25)|BIT(26)|BIT(27)|BIT(28)|BIT(29)|BIT(30);
    CLKGAT2 = BIT(2)|BIT(10)|BIT(11)|BIT(12)|BIT(15)|BIT(17)|BIT(23)|BIT(24)|BIT(25)|BIT(26)|BIT(29);
    CLKGAT3 = 0;

#if UART0_PRINTF_SEL != PRINTF_NONE
    CLKGAT0 |= BIT(6);
#endif

#if (LANG_SELECT == LANG_EN_ZH)
    multi_lang_init(sys_cb.lang_id);
#endif

#if USB_SUPPORT_EN
    CLKGAT0 |= BIT(15);
#endif
#if SD_SUPPORT_EN
    CLKGAT0 |= BIT(16);
#endif

}

#if MP3_SPR_CHANGED_JUMP_NEXT
volatile u32 mp3_spr_changed_cnt = 0;

typedef struct {
    volatile u8  sta;
    volatile u8  nslot;             //用于统计main线程是否有时间跑
    u8  type;
    u16 total_time;
    u16 cur_time_ms;
} music_cb_t;
extern music_cb_t music_cb;

AT(.mp3dec.frame)
void mp3_frame_spr_changed(u32 frame_erridx)
{
    mp3_spr_changed_cnt++;
}

AT(.com_text.plugin)
void plugin_check_mp3_spr(void)
{
    if (mp3_spr_changed_cnt >= 3) {
        music_cb.sta = MUSIC_STOP;
        mp3_spr_changed_cnt = 0;
        sys_cb.sd_failed_file_cnt++;
    }
}
#endif

void plugin_var_init(void)
{
    pwrkey_detect_flag = 0;


//    if (!xcfg_cb.led_disp_en) {
//        xcfg_cb.bled_io_sel = 0;
//        xcfg_cb.charge_full_bled = 0;
//        xcfg_cb.led_sta_config_en = 0;
//    }

    if (!xcfg_cb.led_pwr_en) {
        xcfg_cb.rled_io_sel = 0;
    }

    if (xcfg_cb.buck_mode_en) {
        xcfg_cb.vddbt_capless_en = 0;
    }

    if (!xcfg_cb.charge_en) {
        xcfg_cb.chbox_en = 0;
    }

    if (!xcfg_cb.chbox_en) {
        xcfg_cb.ch_box_type_sel            = 3;
        xcfg_cb.ch_out_auto_pwron_en       = 0;
        xcfg_cb.ch_leakage_sel             = 0;
        xcfg_cb.chg_inbox_pwrdwn_en        = 0;
        xcfg_cb.chbox_out_delay            = 0;
    }


#if TKEY_SOFT_PWR_EN
    sys_cb.tkey_pwrdwn_en = 1;
#endif

    //PWRKEY模拟硬开关,需要关闭长按10S复位， 第一次上电开机， 长按5秒进配对等功能。
    if (PWRKEY_2_HW_PWRON) {
        xcfg_cb.powkey_10s_reset = 0;
//        xcfg_cb.pwron_frist_bat_en = 0;
    }

#if LED_DISP_EN
	led_func_init();
#endif // LED_DISP_EN
}

AT(.com_text.plugin)
void plugin_tmr5ms_isr(void)
{
}

AT(.com_text.plugin)
void plugin_tmr1ms_isr(void)
{
}

//蓝牙休眠时，唤醒IO配置
void sleep_wakeup_config(void)
{
#if ADKEY_EN
    wakeup_gpio_config(get_adc_gpio_num(ADKEY_CH), 0, 0);           //配置ADKEY IO下降沿唤醒。
#endif // ADKEY_EN

#if ADKEY2_EN
    wakeup_gpio_config(get_adc_gpio_num(ADKEY2_CH), 0, 0);          //配置ADKEY1 IO下降沿唤醒。
#endif // ADKEY2_EN

#if ADKEY_MUX_SDCLK_EN
    wakeup_gpio_config(get_adc_gpio_num(SDCLK_AD_CH), 0 ,0);        //IO下降沿唤醒。
#endif // ADKEY_MUX_SDCLK_EN

#if IOKEY_EN
    if (xcfg_cb.iokey_config_en) {
        wakeup_gpio_config(xcfg_cb.iokey_io0, 0, 1);                //配置IO下降沿唤醒。
        wakeup_gpio_config(xcfg_cb.iokey_io1, 0, 1);
        wakeup_gpio_config(xcfg_cb.iokey_io2, 0, 1);
        wakeup_gpio_config(xcfg_cb.iokey_io3, 0, 1);
        wakeup_gpio_config(xcfg_cb.iokey_io4, 0, 1);
    } else {
        //不用工具配置IOKEY时，根据实际使用的IOKEY IO进行修改。
        wakeup_gpio_config(IO_PB1, 0, 1);
        wakeup_gpio_config(IO_PB2, 0, 1);
    }
#endif // IOKEY_EN

#if SC7A20_EN
	wakeup_wko_config(); /*配置PB5作为GPIO唤醒功能*/
	//printf("%s config pb5 wakeup\n", __func__);
#endif

#if PWRKEY_EN
    if ((!PWRKEY_2_HW_PWRON) && !bsp_tkey_wakeup_en()) {
        wakeup_wko_config();
    }
#endif // PWRKEY_EN

}

AT(.com_text.bsp.sys)
void lefmic_zero_detect(u8 *ptr, u32 samples)
{
}

bool is_sleep_dac_off_enable(void)
{
    return true;
}

//设置piano提示音播放的数字音量 (0 ~ 0x7fff)
u32 get_piano_digvol(void)
{
    return bsp_volume_convert(WARNING_VOLUME);
}

//设置WAV RES提示音播放的数字音量 (0 ~ 0x7fff)
u32 get_wav_res_digvol(void)
{
    return bsp_volume_convert(WARNING_VOLUME);
}

void maxvol_tone_play(void)
{
	bsp_res_play(TWS_RES_MAX_VOL);
}

void minvol_tone_play(void)
{

}

void plugin_playmode_warning(void)
{

}

void plugin_lowbat_vol_reduce(void)
{
#if LPWR_REDUCE_VOL_EN
    music_src_set_volume(0x50c0);       //设置音乐源音量达到整体降低系统音量 (范围：0~0x7fff)
#endif // LPWR_REDUCE_VOL_EN
}

void plugin_lowbat_vol_recover(void)
{
#if LPWR_REDUCE_VOL_EN
    music_src_set_volume(0x7fff);       //还原音量
#endif // LPWR_REDUCE_VOL_EN
}

AT(.com_text.port.vbat)
void plugin_vbat_filter(u32 *vbat)
{
#if  VBAT_FILTER_USE_PEAK
    //电源波动比较大的音箱方案, 取一定时间内的电量"最大值"或"次大值",更能真实反应电量.
    #define VBAT_MAX_TIME  (3000/5)   //电量峰值判断时间 3S
    static u16 cnt = 0;
	static u16 vbat_max_cnt = 0;
    static u32 vbat_max[2] = {0,0};
    static u32 vbat_ret = 0;
    u32 vbat_cur = *vbat;
    if (cnt++  < VBAT_MAX_TIME) {
        if (vbat_max[0] < vbat_cur) {
            vbat_max[1] = vbat_max[0];  //vbat_max[1] is less max (次大值)
            vbat_max[0] = vbat_cur;     //vbat_max[0] is max(最大值)
            vbat_max_cnt = 0;
        } else if (vbat_max[0] == vbat_cur) {
            vbat_max_cnt ++;
        }
    } else {
        if (vbat_max_cnt >= VBAT_MAX_TIME/5) {  //总次数的(1/5)都采到最大值,则返回最大值.
            vbat_ret = vbat_max[0];
        } else if (vbat_max[1] != 0) {   //最大值次数较少,则返回次大值(舍弃最大值)
            vbat_ret = vbat_max[1];
        }
        vbat_max[0] = 0;
        vbat_max[1] = 0;
        vbat_max_cnt = 0;
        cnt = 0;
    }
    //返回值
    if (vbat_ret != 0) {
        *vbat = vbat_ret;
    }
#endif
}

//初始化完成, 各方案可能还有些不同参数需要初始化,预留接口到各方案
void plugin_sys_init_finish_callback(void)
{
}

bool plugin_func_idle_enter_check(void)
{
    //可以在这里决定否需要进入idle
    return true;
}



