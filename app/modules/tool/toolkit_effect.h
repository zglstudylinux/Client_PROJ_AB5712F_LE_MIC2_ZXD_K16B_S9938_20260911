#ifndef __TOOLKIT_EFFECT_H
#define __TOOLKIT_EFFECT_H


// MAGIC
typedef struct __attribute__((packed)){
    int8_t  channel;
    int8_t  enable;
    int16_t shfit_hz;
//    int16_t crc;
}tool_magic_pitch_coef_t;

////////////////////////////////////////////////////////////////////////////////////
// ECHO
typedef struct __attribute__((packed)){
    int8_t  channel;
    int8_t  enable;
    int8_t  lp_filter_en;
    int8_t  attention;
    int16_t delay;
    int16_t cutoff_freq;
    int16_t dry;
    int16_t wet;
}tool_echo_pitch_coef_t;

////////////////////////////////////////////////////////////////////////////////////
// REVERB
typedef struct __attribute__((packed)){
    int8_t  channel;
    int8_t  enable;
    int8_t  level;
    int16_t dry;
    int16_t wet;
}tool_reverb_pitch_coef_t;

////////////////////////////////////////////////////////////////////////////////////
// EQ
typedef struct __attribute__((packed)){
    int8_t  channel;
    int8_t  enable;
    int8_t  sample_rate;
    int8_t  max_bandnum;
    int8_t  param_size;
    int8_t  param_cnt;
    int8_t  band_value_start;
    int8_t  band_value_cnt;
    int8_t  reverse : 1;
    int16_t RSVD    : 15;
    int32_t gain;
    int32_t coef[6][5];//EQ_BAND_MAX_NB = 6
//    int16_t crc;
}tool_eq_coef_t;

////////////////////////////////////////////////////////////////////////////////////
//dyeq
typedef struct __attribute__((packed)){
    int8_t  enable;
    int8_t  mode;
    int32_t gain;
    int32_t threshold1;
    int32_t threshold2;
    int32_t slop;
    int32_t in_attack;
    int32_t in_release;
    int32_t coef[4][5];
//    int16_t crc;
}tool_dyeq_coef_t;

////////////////////////////////////////////////////////////////////////////////////
// total drc_v3
typedef struct __attribute__((packed)){
    int8_t  channel;
    int8_t  enable;
    int16_t  delay;
    int8_t  sample_rate;
    int8_t coeff_cnt;
    int16_t RSVD;
	int32_t in_attack_coeff;
	int32_t in_release_coeff;
    int32_t out_attack_coeff;
	int32_t out_release_coeff;
    int32_t tav_coeff;
    int32_t drc_coef[4][3];
//    int16_t crc;
}tool_drc_coef_t;

////////////////////////////////////////////////////////////////////////////////////
// multi_drc_v3 (NEW_DRC)
typedef struct __attribute__((packed)){
    int8_t  channel;
    int8_t  enable;
    int8_t type; //<0:Low-DRC,1:Mid-DRC,2:High-DRC,3:Low-EQ,4:Mid-EQ,5:High-EQ>
    int16_t delay;

    union {
        tool_eq_coef_t  xeq;
        tool_drc_coef_t xdrc;
    };
}tool_multi_drc_v3_coef_t;


//voice
void mic_magic_effect_update_callback(u8 *buf, u32 len, u8 params);
void mic_echo_effect_update_callback(u8 *buf, u32 len, u8 params);
void local_mic_eq_effect_update_callback(u8 *buf, u32 len, u8 params);
void local_mic_drc_effect_update_callback(u8 *buf, u32 len, u8 params);
void mix_mic_eq_effect_update_callback(u8 *buf, u32 len, u8 params);
void mix_mic_drc_effect_update_callback(u8 *buf, u32 len, u8 params);
void msbc_eq_effect_update_callback(u8 *buf, u32 len, u8 params);
void msbc_drc_effect_update_callback(u8 *buf, u32 len, u8 params);
void csvd_eq_effect_update_callback(u8 *buf, u32 len, u8 params);
void csvd_drc_effect_update_callback(u8 *buf, u32 len, u8 params);
void usb_mic_eq_effect_update_callback(u8 *buf, u32 len, u8 params);
void usb_mic_drc_effect_update_callback(u8 *buf, u32 len, u8 params);
void usb_mic1_eq_effect_update_callback(u8 *buf, u32 len, u8 params);
void usb_mic1_drc_effect_update_callback(u8 *buf, u32 len, u8 params);
void usb_mic0_eq_effect_update_callback(u8 *buf, u32 len, u8 params);
void usb_mic0_drc_effect_update_callback(u8 *buf, u32 len, u8 params);

//effect
void music_dac_eq_effect_update_callback(u8 *buf, u32 len, u8 params);
void music_dac_drc_effect_update_callback(u8 *buf, u32 len, u8 params);

#endif // __TOOLKIT_EFFECT_H
