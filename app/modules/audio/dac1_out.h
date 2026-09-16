#ifndef _DAC1_OUT_H
#define _DAC1_OUT_H

typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *params);

typedef struct {
    u8  mute;
    u8  sample_rate;
    u16 samples;
    u8  low_thr;
    u8  high_thr;
    audio_callback_t callback;
} dac_out_cfg_t;

typedef struct {
    u8  sample_rate;
    u8  low_thr;
    u8  high_thr;
} dac1_out_param_cfg_t;

void dac1_out_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void usb_speaker_dac1_out_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void dac1_out_audio_output_callback_set(audio_callback_t callback);
void dac1_out_init(u8 sample_rate, u16 samples, u8 channel);
void dac1_out_audio_mute_set(uint8_t mute);

void dac0_out_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void dac0_out_audio_output_callback_set(audio_callback_t callback);
void dac0_out_init(u8 sample_rate, u16 samples, u8 channel);

void usb_speaker_dac1_put_sample_32bit(s32 right);
#endif
