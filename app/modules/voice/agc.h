#ifndef __AGC_H_
#define __AGC_H_

typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *params);


typedef struct {
    u8 mute;
    u8 sample_rate;
    u16 samples;
    u16 max_delay_len;
    audio_callback_t callback;

} agc_cfg_t;

void agc_audio_input(u8 *ptr, u32 samples, int ch_mode, void *param);
void agc_audio_output_callback_set(audio_callback_t callback);
void agc_audio_init(u8 sample_rate, u16 samples, u8 channel);
void agc_mic_mute_set(uint8_t mute);
uint8_t agc_mic_mute_get(void);
void agc_mic_proc_cb(void);
#endif
