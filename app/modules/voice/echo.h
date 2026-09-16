#ifndef __ECHO_API_H
#define __ECHO_API_H

typedef struct {
    u8 mute;
    u8 sample_rate;
    u16 samples;
    u16 max_delay_len;
    u16 delay_len_step;
    u16 delay_level;
    audio_callback_t callback;
} echo_cfg_t;

void echo_audio_input(u8 *ptr, u32 samples, int ch_mode, void *param);
void echo_audio_output_callback_set(audio_callback_t callback);
void echo_audio_process(s16 *ptr, u32 samples);
void echo_audio_mute_set(uint8_t mute);
void echo_audio_init(u8 sample_rate, u16 samples, u8 channel);

/// attenuation:    混响次数  0~90
/// delay_length:   每一声混响之间的间隔  0~400
/// cutoffFreq_set: 低通滤波器的截止频率  1~4000
/// lp_filter_en:   低通滤波器的开关      0：关 1：开
/// dry_set:        原始声音               0~49152（Q15即0~1.5）
/// wet_set:        回声湿度，越大所占比例越大  0~32768（Q15 即0~1.0）
/// 默认配置 echo_audio_param_set(55,250,1000,1,32768,15000)
void echo_audio_param_set(u16 attenuation ,s32 delay_length ,u32 cutoffFreq_set ,u32 lp_filter_en , u16 dry_set ,u16 wet_set);

void echo_delay_level_change(void);
void echo_delay_level_up(void);
void echo_delay_level_down(void);
void echo_delay_level_set(u8 delay_len_level);
u8 echo_delay_level_get(void);
bool echo_delay_level_is_max_min(void);
#endif
