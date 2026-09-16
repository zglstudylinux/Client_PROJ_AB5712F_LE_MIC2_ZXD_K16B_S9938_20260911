#ifndef __VOICE_CHANGE_V2_H
#define __VOICE_CHANGE_V2_H

///公共
typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *params);
typedef struct {
    u8 mute;
    u8 sample_rate;
    u16 samples;
    audio_callback_t callback;
} voice_change_v2_mic_cfg_t;
///库接口外的模块接口声明
void voice_change_v2_mic_init(u8 sample_rate, u16 samples, u8 channel);
void voice_change_v2_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void voice_change_v2_mic_output_callback_set(audio_callback_t callback);
uint8_t voice_change_v2_mic_mute_get(void);
void voice_change_v2_mic_mute_set(uint8_t mute);
void voice_change_v2_mic_exit(void);
void voice_change_v2_mic_proc_cb(void);
void voice_change_v2_mic_param_set(u16 pitch_rate_set, u16 formant_set);

enum {
    VOICE_CHANGE_NONE,          //原声
    VOICE_CHANGE_ROBOT,         //机器人
    VOICE_CHANGE_MINIONS,       //小黄人
    VOICE_CHANGE_WOMAN,         //女神
    VOICE_CHANGE_MAN,           //男神
    VOICE_CHANGE_MONSTER,       //怪兽音
    VOICE_CHANGE_BABY,          //娃娃音
    VOICE_CHANGE_MAX,
};

#endif
