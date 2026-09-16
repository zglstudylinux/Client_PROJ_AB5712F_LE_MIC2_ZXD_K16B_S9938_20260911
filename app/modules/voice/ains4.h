#ifndef __AINS4_H
#define __AINS4_H

///公共
typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *params);
typedef struct {
    u8 mute;
    u8 sample_rate;
    u16 samples;
    volatile u8 kick_proc_done;
    volatile u8 ains4_flag;
    uint8_t ains4_in_cnt;
    uint8_t ains4_out_cnt;
    audio_callback_t callback;
} ains4_mic_cfg_t;
///库接口外的模块接口声明
void ains4_mic_init(u8 sample_rate, u16 samples, u8 channel);
void ains4_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void ains4_mic_output_callback_set(audio_callback_t callback);
void ains4_mic_param_set(u8 mode ,s16 ains4_noise_nt);//ains4_noise_nt 最好每500一个step，值越大底噪越大，值越小底噪越小
uint8_t ains4_mic_mute_get(void);
void ains4_mic_mute_set(uint8_t mute);
void ains4_mic_exit(void);
void ains4_mic_proc_cb(void);

#endif
