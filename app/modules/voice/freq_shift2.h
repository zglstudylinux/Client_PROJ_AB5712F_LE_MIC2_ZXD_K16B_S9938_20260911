#ifndef __FREQ_SHIFT2_H
#define __FREQ_SHIFT2_H

typedef struct {
    u8 mute;
    u8 sample_rate;
    volatile u8 kick_proc_done;
    u16 samples;
    audio_callback_t callback;
} freq_shift2_mic_cfg_t;

///库接口外的模块接口声明
void freq_shift2_mic_init(u8 sample_rate, u16 samples, u8 channel);
void freq_shift2_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void freq_shift2_mic_output_callback_set(audio_callback_t callback);
void freq_shift2_mic_param_set(s16 freq_shift2_nt);
uint8_t freq_shift2_mic_mute_get(void);
void freq_shift2_mic_mute_set(uint8_t mute);
void freq_shift2_mic_exit(void);
void freq_shift2_mic_proc_cb(void);

#endif
