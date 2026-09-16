#ifndef __ALLPASS_FILTER_CHANGE_H
#define __ALLPASS_FILTER_CHANGE_H

///公共
typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *params);
typedef struct {
    cbuf_cfg_t freq_shift2_mic_cbuf;
    cbuf_cfg_t freq_shift2_mic_cbuf2;
    u8 mute;
    u8 sample_rate;
    u16 samples;
    audio_callback_t callback;
} allpass_filter_mic_cfg_t;


void allpass_filter_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void allpass_filter_audio_output_callback_set(audio_callback_t callback);
void allpass_filter_init(u8 sample_rate, u16 samples, u8 channel);

///allpass_filter_change算法结构体以及相关API声明
void allpass_filter_change_init(u8 idx, s32 change_time);
s32 allpass_filter_change(s32 input, u8 idx);
#endif
