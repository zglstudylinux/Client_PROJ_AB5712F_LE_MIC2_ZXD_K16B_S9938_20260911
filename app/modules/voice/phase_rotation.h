#ifndef __PHASE_ROTATION_H
#define __PHASE_ROTATION_H

typedef struct {
    u8 mute;
    u8 sample_rate;
    volatile u8 kick_proc_done;
    u16 samples;
    audio_callback_t callback;
} phase_rotation_mic_cfg_t;

///库接口外的模块接口声明
void phase_rotation_mic_init(u8 sample_rate, u16 samples, u8 channel);
void phase_rotation_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void phase_rotation_mic_output_callback_set(audio_callback_t callback);
void phase_rotation_mic_param_set(s16 phase_rotation_nt);
uint8_t phase_rotation_mic_mute_get(void);
void phase_rotation_mic_mute_set(uint8_t mute);
void phase_rotation_mic_exit(void);
void phase_rotation_mic_proc_cb(void);
int s_clip16(int x);
void huart_wait_txdone(void);

#endif
