#ifndef __HOWLING_NOTCH_L2_H
#define __HOWLING_NOTCH_L2_H

///公共
typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *params);
typedef struct {
    cbuf_cfg_t howling_notch_L2_mic_cbuf;
    cbuf_cfg_t howling_notch_L2_mic_cbuf2;
    u8 mute;
    u8 sample_rate;
    u16 samples;
    audio_callback_t callback;
} howling_notch_L2_mic_cfg_t;

extern const s32 notch_tabel[256][5];

///库接口外的模块接口声明
void howling_notch_L2_mic_init(u8 sample_rate, u16 samples, u8 channel);
void howling_notch_L2_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void howling_notch_L2_mic_output_callback_set(audio_callback_t callback);
void howling_notch_L2_mic_param_set(s16 howling_notch_L2_noise_nt);//howling_notch_L2_noise_nt 最好每500一个step，值越大底噪越大，值越小底噪越小
uint8_t howling_notch_L2_mic_mute_get(void);
void howling_notch_L2_mic_mute_set(uint8_t mute);
void howling_notch_L2_mic_exit(void);
void howling_notch_L2_mic_proc_cb(void);

#endif
