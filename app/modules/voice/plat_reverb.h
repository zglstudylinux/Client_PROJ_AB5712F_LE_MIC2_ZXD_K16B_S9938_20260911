#ifndef __PLAT_REVERB_H
#define __PLAT_REVERB_H


typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *params);

typedef struct {
    u8 mute;
    u8 sample_rate;
    u16 samples;
    audio_callback_t callback;
} plat_reverb_cfg_t;

void plat_reverb_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void plat_reverb_audio_output_callback_set(audio_callback_t callback);
void plat_reverb_audio_init(u8 sample_rate, u16 samples);
void plat_reverb_audio_mute_set(uint8_t mute);
/* 
pre_dly: 0 ~ 1999   最大值为PREDELAY预延时  值越大回声越大
plat_dry: 0 ~ 1000  干度  值越大原声占比越大
plat_wet: 0 ~ 1000  湿度  值越大混响效果越明显
damping: 0 ~ 999    阻尼    值越大高频衰减越多，声音越干
diffusion: 0 ~ 1000 衰减扩散     控制尾部密度
decay: 0 ~ 1000     衰减率       值越小，衰减越快，尾音少
hp_freq: 0 ~ 8      0:表示不过高通滤波器，1-7：表示150hz-500hz,步进50hz
 */
void plat_reverb_audio_set_param(u16 pre_dly, u16 plat_dry, u16 plat_wet, u16 damping,
                                 u16 diffusion, u16 decay, u8 hp_freq);
#endif
