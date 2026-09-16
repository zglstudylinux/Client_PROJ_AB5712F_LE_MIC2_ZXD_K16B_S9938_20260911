#include "include.h"
#include "allpass_filter_change.h"
/*
 * 文件名称: allpass_filter_change.c
 * 功能描述: 本文件为软件随机相位处理模块

 ****************************************************************************************
    code :
    buf  : 10000*2 Bytes
    time : 160M 下 120 个点处理 147us
*/
#if ALLPASS_FILTER_CHANGE_EN
allpass_filter_mic_cfg_t allpass_filter_mic_cfg;

#define CLIP(x,n)   (x>=(1<<n)) ? (1<<n)-1 : (x<(-(1<<n))) ? -(1<<n) : x

AT(.text.allpass_filter_change_proc)
void allpass_filter_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    s32 sample =0;
    s16 *pcm = (s16 *)ptr;

    for(u8 j=0; j<samples;j++) {
        sample = pcm[j];
        sample = allpass_filter_change(sample, 0);
		sample = CLIP(sample, 15);
		pcm[j] = sample;
    }

    //输出到下一级
    if (allpass_filter_mic_cfg.callback) {
        allpass_filter_mic_cfg.callback(ptr, samples, ch_mode, params);
    }
}


AT(.text.allpass_filter_init)
void allpass_filter_audio_output_callback_set(audio_callback_t callback)
{
    allpass_filter_mic_cfg.callback = callback;
}

AT(.text.allpass_filter_init)
void allpass_filter_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&allpass_filter_mic_cfg, 0, sizeof(allpass_filter_mic_cfg));

    allpass_filter_change_init(0, 100);
}

AT(.text.mic_eq_drc.exit)
void mic_allpass_filter_exit(u8 sample_rate, u16 samples)
{

}
#endif
