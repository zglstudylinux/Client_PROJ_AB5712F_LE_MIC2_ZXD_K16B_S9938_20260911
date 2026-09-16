#include "include.h"

// 5705主控只有MIC4
/*                            通道,       采样率,    模拟增益, 数字增益, BITS, 通路控制,     样点数,   回调函数*/
static sdadc_cfg_t mic_cfg = {CH_MIC4,  SPR_48000,  3,       1,       1,     ADC2DAC_EN,  120,      NULL};
volatile bool sdadc_w4_kick;

AT(.text.bsp.wireless_mic)
void mic_init(u8 sample_rate, u16 samples, u8 channel)
{
    mic_cfg.sample_rate = sample_rate;
    mic_cfg.samples = samples*2;

#if WIRELESS_MIC_SRC_EN
    //32k采样率 控制2.5ms一个周期的话就是80点kick一次
    mic_cfg.sample_rate = SPR_32000;
    mic_cfg.samples = 80*2;
#endif

#if WIRELESS_MIC_YLCRN_16K_EN
    //16k采样率 控制2.5ms一个周期的话就是40点kick一次
    mic_cfg.sample_rate = SPR_16000;
    mic_cfg.samples = 40*2;
#endif

    mic_cfg.anl_gain = xcfg_cb.mic_anl_gain;      //模拟增益0~13
    mic_cfg.dig_gain = xcfg_cb.bt_mic_dig_gain;   //数字增益0~63
    if(xcfg_cb.bt_mmic_cfg < 5) {
        mic_cfg.channel = mic_ch_tbl[xcfg_cb.bt_mmic_cfg];
    }
#if APAPTER_LOCAL_AUX_EN
    mic_cfg.channel =  CH_AUXL0;  //默认PE6输入
    mic_cfg.anl_gain = 0; //默认0db
    mic_cfg.dig_gain = 0; //默认0db
    if(xcfg_cb.adapter_aux_en) {
        u8 anl_gain = xcfg_cb.aux_anl_gain;
        u8 dig_gain = xcfg_cb.aux_dig_gain;
        mic_cfg.channel = bsp_aux_ch_getcfg();
        mic_cfg.anl_gain = anl_gain | anl_gain<<6;    //双声道模拟增益保持一致
        mic_cfg.dig_gain = dig_gain | dig_gain<<6;    //双声道数字增益保持一致

    }
#endif // APAPTER_LOCAL_AUX_EN

//    mic_cfg.callback = mic_mix_process_cb;

}

AT(.text.bsp.wireless_mic)
void mic_audio_output_callback_set(audio_callback_t callback)
{
    mic_cfg.callback = callback;

    //改到蓝牙连接后再初始化，节省未连接功耗
//    sdadc_init(&mic_cfg);
//    sdadc_start(mic_cfg.channel);
}



AT(.text.bsp.wireless_mic)
void mic_start(void)
{
    sdadc_init(&mic_cfg);
    sdadc_start(mic_cfg.channel);

    /// DC 去直流 关闭高通滤波器 恢复一些低频信号
    #if DEVICE_MIC_DC_EN
    #define SDADCDIGCON     SFR_RW (SFR1_BASE + 0x37*4)
    SDADCDIGCON &= ~(BIT(29) | BIT(30));
    SDADCDIGCON |= BIT(22) | BIT(23);
    SDADCDIGCON |= (BIT(1) | BIT(9));
    #endif

    sdadc_w4_kick = true;
}

AT(.com_text.bsp.wireless_mic)
void wireless_mic_kick(void)
{
    if (sdadc_w4_kick) {
        sdadc_w4_kick = false;
        sdadc_start_kick(mic_cfg.channel);
    }
}

AT(.text.bsp.wireless_mic)
void mic_stop(void)
{
    sdadc_exit(mic_cfg.channel);
}

#if ADAPTER_LOCAL_MIC_MIX_EN
AT(.text.bsp.wireless_mic)
void local_mic_init(void)
{
    mic_init(WIRELESS_MIC_SAMPLE_RATE_SELECT, WIRELESS_MIC_SAMPLES_SELECT, WIRELESS_MIC_CHANNEL_SELECT);
    mic_cfg.callback = mic_mix_process_cb;

    mic_start();

    sdadc_start_kick(mic_cfg.channel);
}
#endif
