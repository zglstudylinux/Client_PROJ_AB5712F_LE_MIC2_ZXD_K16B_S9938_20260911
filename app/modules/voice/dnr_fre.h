#ifndef __DNR_FRE_H
#define __DNR_FRE_H

typedef struct {
    u8 mute;
    u8 sample_rate;
    volatile u8 kick_proc_done;
    u16 samples;
    audio_callback_t callback;
} dnr_fre_mic_cfg_t;

///库接口外的模块接口声明
void dnr_fre_mic_init(u8 sample_rate, u16 samples, u8 channel);
void dnr_fre_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void dnr_fre_mic_output_callback_set(audio_callback_t callback);
void dnr_fre_mic_param_set(s16 dnr_fre_nt);
uint8_t dnr_fre_mic_mute_get(void);
void dnr_fre_mic_mute_set(uint8_t mute);
void dnr_fre_mic_exit(void);
void dnr_fre_mic_proc_cb(void);
int s_clip16(int x);
#endif
