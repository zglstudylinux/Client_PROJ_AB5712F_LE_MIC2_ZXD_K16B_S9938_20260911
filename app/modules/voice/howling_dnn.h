#ifndef __HOWLING_DNN_H
#define __HOWLING_DNN_H

///公共
typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *params);
typedef struct {
    u8 mute;
    u8 sample_rate;
    u16 samples;
    audio_callback_t callback;
} howling_dnn_mic_cfg_t;
///库接口外的模块接口声明
void howling_dnn_mic_init(u8 sample_rate, u16 samples, u8 channel);
void howling_dnn_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void howling_dnn_mic_output_callback_set(audio_callback_t callback);
void howling_dnn_mic_param_set(int8_t howling_dnn_nt);
uint8_t howling_dnn_mic_mute_get(void);
void howling_dnn_mic_mute_set(uint8_t mute);
void howling_dnn_mic_exit(void);
void howling_dnn_mic_proc_cb(void);
#endif
