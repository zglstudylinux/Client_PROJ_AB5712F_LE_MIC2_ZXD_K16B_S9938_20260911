#ifndef __SOFT_EQ_H
#define __SOFT_EQ_H

///公共
typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *params);
typedef struct {
//    cbuf_cfg_t soft_eq_mic_cbuf;
//    cbuf_cfg_t soft_eq_mic_cbuf2;
    u8 mute;
    u8 sample_rate;
    u16 samples;
    audio_callback_t callback;
} soft_eq_mic_cfg_t;
///库接口外的模块接口声明
void soft_eq_mic_init(u8 sample_rate, u16 samples, u8 channel);
void soft_eq_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void soft_eq_mic_output_callback_set(audio_callback_t callback);
void soft_eq_mic_param_set(int8_t soft_eq_nt);
uint8_t soft_eq_mic_mute_get(void);
void soft_eq_mic_mute_set(uint8_t mute);
void soft_eq_mic_exit(void);
void soft_eq_mic_proc_cb(void);

#define EQ_VERSION                              1   //支持的版本:V2

///soft_eq 模块管理
#define BAND_NUM 8

typedef struct{
	u8  outsat_en;
	s32 coef_ptr[BAND_NUM][5];
	s32 zx[BAND_NUM+1][2];
} soft_eq_t;

typedef struct {
    u8 band_cnt;
    u8 version;
    u8 sample;
    u8 rfu[1];
    const u32 *param;
} eq_param_t;

void soft_eq_init(const u32 *coef, u8 band_cnt, soft_eq_t *biquad_sb);
s16 soft_eq_biquad(s16 input, soft_eq_t *iir_filter_sb);
s32 soft_eq_biquad_s32(s32 input, soft_eq_t *iir_filter_sb);
#endif
