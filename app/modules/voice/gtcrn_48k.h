#ifndef __GTCRN_48K_H
#define __GTCRN_48K_H

///公共
typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *params);
typedef struct {
    cbuf_cfg_t gtcrn_48k_cbuf;
    cbuf_cfg_t gtcrn_48k_cbuf2;
    u8 mute;
    u8 sample_rate;
    u16 samples;
    audio_callback_t callback;
} gtcrn_48k_mic_cfg_t;
///库接口外的模块接口声明
void gtcrn_48k_mic_init(u8 sample_rate, u16 samples, u8 channel);
void gtcrn_48k_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void gtcrn_48k_mic_output_callback_set(audio_callback_t callback);
void gtcrn_48k_mic_param_set(int8_t gtcrn_48k_nt);
uint8_t gtcrn_48k_mic_mute_get(void);
void gtcrn_48k_mic_mute_set(uint8_t mute);
void gtcrn_48k_mic_exit(void);
void gtcrn_48k_mic_proc_cb(void);

#endif
