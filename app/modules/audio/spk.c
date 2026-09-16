#include "include.h"

//AUX立体声通路配置
static sdadc_cfg_t spk_cfg = {CH_MIC2|(CH_MIC3<<8),  SPR_48000, 3,  0, 0xFF,  ADC2DAC_EN,  120, NULL};

extern bool sdadc_w4_kick;

AT(.text.mic)
void spk_audio_output_callback_set(audio_callback_t callback)
{
    spk_cfg.callback = (pcm_callback_t)callback;

    //改到蓝牙连接后再初始化，节省未连接功耗
//    sdadc_init(&spk_cfg);
//    sdadc_start(spk_cfg.channel);
}

AT(.text.mic)
void spk_init(u8 sample_rate, u16 samples, u8 channel)
{
    u8 anl_gain = xcfg_cb.aux_anl_gain;
    u8 dig_gain = xcfg_cb.aux_dig_gain;

    spk_cfg.sample_rate = sample_rate;
    spk_cfg.samples     = samples*2;

    spk_cfg.channel = bsp_aux_ch_getcfg();
    spk_cfg.anl_gain = anl_gain | anl_gain<<6;    //双声道模拟增益保持一致
    spk_cfg.dig_gain = dig_gain | dig_gain<<6;    //双声道数字增益保持一致

    //这里先注释掉，在callback_set中设置callback函数后再初始化
//    sdadc_init(&spk_cfg);
//    sdadc_start(spk_cfg.channel);
}

void spk_start(void)
{
    sdadc_init(&spk_cfg);
    sdadc_start(spk_cfg.channel);
    sdadc_w4_kick = true;
}

void spk_stop(void)
{
    sdadc_exit(spk_cfg.channel);
}

AT(.com_text.bsp.wireless_mic)
void wireless_spk_kick(void)
{
    if (sdadc_w4_kick) {
        sdadc_w4_kick = false;
        sdadc_start_kick(spk_cfg.channel);
    }
}
