#ifndef _MIC_MIX_H
#define _MIC_MIX_H

typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *params);


typedef struct {
    u8 kick_flag;
    u8 mute;
    u8 sample_rate;
    u16 samples;
    audio_callback_t callback;
} mic_mix_t;

void mic_mix_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void mic_mix_output_callback_set(audio_callback_t callback);
void mic_mix_init(u8 sample_rate, u16 samples, u8 channel);

void mic_mix_process_cb(u8 *ptr, u32 samples, int ch_mode, void *params);
void local_mic_output(void);
#endif
