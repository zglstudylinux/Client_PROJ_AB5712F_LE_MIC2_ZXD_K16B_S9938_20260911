#ifndef __HOWLING_AI_MASK_H
#define __HOWLING_AI_MASK_H

///公共
typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *params);
typedef struct {
//    cbuf_cfg_t howling_ai_mask_mic_cbuf;
//    cbuf_cfg_t howling_ai_mask_mic_cbuf2;
    u8 mute;
    u8 sample_rate;
    u16 samples;
    audio_callback_t callback;
} howling_ai_mask_mic_cfg_t;
///库接口外的模块接口声明
void howling_ai_mask_mic_init(u8 sample_rate, u16 samples, u8 channel);
void howling_ai_mask_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void howling_ai_mask_mic_output_callback_set(audio_callback_t callback);
void howling_ai_mask_mic_param_set(int8_t howling_ai_mask_nt);
uint8_t howling_ai_mask_mic_mute_get(void);
void howling_ai_mask_mic_mute_set(uint8_t mute);
void howling_ai_mask_mic_exit(void);
void howling_ai_mask_mic_proc_cb(void);

#endif
