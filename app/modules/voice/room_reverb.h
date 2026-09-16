#ifndef __ROOM_REVERB_H
#define __ROOM_REVERB_H


typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *params);

typedef struct {
    u8 mute;
    u8 sample_rate;
    u16 samples;
    audio_callback_t callback;
} room_reverb_cfg_t;

void room_reverb_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void room_reverb_audio_output_callback_set(audio_callback_t callback);
void room_reverb_audio_init(u8 sample_rate, u16 samples, u8 channel);
void room_reverb_audio_mute_set(uint8_t mute);
void room_reverb_audio_set_param(u16 room_decay_set, u16 room_dry, u16 room_wet);
#endif
