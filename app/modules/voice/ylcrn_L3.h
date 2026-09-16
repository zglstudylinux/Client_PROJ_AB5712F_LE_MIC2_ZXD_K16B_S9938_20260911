#ifndef __YLCRN_L3_H
#define __YLCRN_L3_H

typedef s16			mic_pcm_t;

typedef struct {
    u8 mute;
    u8 sample_rate;
    volatile u8 kick_proc_done;
    u16 samples;
    audio_callback_t callback;
} gtcrn_L3_mic_cfg_t;

///库接口外的模块接口声明
void ylcrn_L3_mic_init(u8 sample_rate, u16 samples, u8 channel);
void ylcrn_L3_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void ylcrn_L3_mic_output_callback_set(audio_callback_t callback);
void ylcrn_L3_mic_param_set(s16 ylcrn_L3_nt);
uint8_t ylcrn_L3_mic_mute_get(void);
void ylcrn_L3_mic_mute_set(uint8_t mute);
void ylcrn_L3_mic_exit(void);
void ylcrn_L3_mic_proc_cb(void);

#endif
