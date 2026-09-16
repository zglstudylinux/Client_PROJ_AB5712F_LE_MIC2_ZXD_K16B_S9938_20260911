#include "include.h"
#include "magic.h"
#include "api_alg.h"
/*
 * 文件名称: magic.c
 * 功能描述: 本文件为软件魔音处理模块

 ****************************************************************************************
    code :
    buf  : 3132Bytes
    time : 160M 下 120 个点处理 229us
 */
#if MAGIC_EN
static magic_cfg_t magic_cfg AT(.buf.pitch_shift.cache);
static pitch_shift2_cb_t pitch_shift2_cb AT(.buf.pitch_shift.cache);

const s32 magic_effect_tbl[] = {
    0,              ///原音
    5000,           ///男神
    -13500,         ///女神
    -30000,         ///娃娃音
    15000,          ///魔兽
};

//AT(.text.pitch_shift_proc)
//const char magic_info[] = "AINS3 samples = %d, isrcnt = %d (SR_%d)\n";


AT(.text.pitch_shift2_proc.input)WEAK
void magic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    s16 *rptr = (s16 *)ptr;
    if (samples == MAGIC_PROC_SAMPLES) {
        if (!magic_cfg.mute) {
            pitch_shift2_process(rptr, &pitch_shift2_cb);
        }
    }else {
        printf("magic_input_samples error\n");
    }
    if (magic_cfg.callback) {
        magic_cfg.callback((u8 *)ptr, samples, ch_mode, params);
    }
}

AT(.text.pitch_shift_set.callback)
void magic_audio_output_callback_set(audio_callback_t callback)
{
    magic_cfg.callback = callback;
}


AT(.text.pitch_shift2_proc.input)WEAK
void magic_audio_process(s16 *ptr, u32 samples)
{
    s16 *rptr = (s16 *)ptr;

    if (samples == MAGIC_PROC_SAMPLES) {
        if (!magic_cfg.mute) {
            pitch_shift2_process(rptr, &pitch_shift2_cb);
        }
    } else {
        printf("magic_input_samples error\n");
    }
}

AT(.text.pitch_shift_init)WEAK
void magic_audio_init(u8 sample_rate, u16 samples, u8 channel)
{
    u16 sample_rete_b[5] = {48000,44100,38000,32000,24000};

    memset(&magic_cfg, 0, sizeof(magic_cfg_t));
    magic_cfg.sample_rate = sample_rate;
    magic_cfg.samples = samples;
    magic_cfg.magic_effect_level = MAGIC_EFFECT_DEFAULT_LEVEL;
#if WIRELESS_MIC_PARAM_MEMORY_EN
    param_magic_effect_level_read((u8 *)&magic_cfg.magic_effect_level);
#endif
    pitch_shift2_init(sample_rete_b[magic_cfg.sample_rate], magic_effect_tbl[magic_cfg.magic_effect_level], &pitch_shift2_cb);
}

AT(.text.pitch_shift_set.param)
void magic_audio_param_set(s32 magic_param)
{
    u8 magic_sta = magic_audio_mute_get();
    magic_audio_mute_set(1);
    u16 sample_rete_b[5] = {48000,44100,38000,32000,24000};
#if WIRELESS_MIC_PARAM_MEMORY_EN
    param_magic_effect_level_write((u8 *)&magic_cfg.magic_effect_level);
#endif

    pitch_shift2_init(sample_rete_b[magic_cfg.sample_rate],magic_param, &pitch_shift2_cb);
    magic_audio_mute_set(magic_sta);
}

AT(.text.pitch_shift_set.param)
void magic_effect_level_change(void)
{
    magic_cfg.magic_effect_level += 1;
    if (magic_cfg.magic_effect_level >= (sizeof(magic_effect_tbl)/4)) {
        magic_cfg.magic_effect_level = 0;
    }
    magic_audio_param_set(magic_effect_tbl[magic_cfg.magic_effect_level]);
}

AT(.text.pitch_shift_set.param) WEAK
void magic_effect_level_set(u8 effect_level)
{
    if (effect_level >= (sizeof(magic_effect_tbl)/4)) {
        effect_level = 0;
    }
    magic_cfg.magic_effect_level = effect_level;
    magic_audio_param_set(magic_effect_tbl[magic_cfg.magic_effect_level]);
}

AT(.text.pitch_shift_get)
u8 magic_effect_level_get(void)
{
    return magic_cfg.magic_effect_level;
}

AT(.text.pitch_shift_set.mute)
void magic_audio_mute_set(uint8_t mute)
{
    magic_cfg.mute = mute;
}

AT(.text.pitch_shift_get.mute)
u8 magic_audio_mute_get(void)
{
    return magic_cfg.mute;
}

AT(.text.pitch_shift_exit)
void magic_audio_exit(void)
{
}

#endif //MAGIC_EN
