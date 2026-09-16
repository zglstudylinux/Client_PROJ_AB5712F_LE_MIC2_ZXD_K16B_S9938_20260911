#ifndef __AINS3_H
#define __AINS3_H

///公共
typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *params);
typedef struct {
    cbuf_cfg_t ains3_mic_cbuf;
    cbuf_cfg_t ains3_mic_cbuf2;
    u8 mute;
    u8 sample_rate;
    u16 samples;
    audio_callback_t callback;
} ains3_mic_cfg_t;
///库接口外的模块接口声明
void ains3_mic_init(u8 sample_rate, u16 samples, u8 channel);
void ains3_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void ains3_mic_output_callback_set(audio_callback_t callback);
void ains3_mic_param_set(int8_t ains3_nt,u8 ains3_idx);
uint8_t ains3_mic_mute_get(void);
void ains3_mic_mute_set(uint8_t mute);
void ains3_mic_exit(void);
void ains3_mic_proc_cb(void);

#endif
