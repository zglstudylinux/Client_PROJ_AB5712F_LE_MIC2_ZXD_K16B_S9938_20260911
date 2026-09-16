#ifndef __API_SRC_H
#define __API_SRC_H

typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *param);

void src0_init(u8 sample_rate, u16 samples, u8 channel);
void src0_audio_input(u8 *buf, u32 samples, int nch, void *params);
void src0_audio_output_callback_set(audio_callback_t callback);
void src0_adjust_speed(int speed);
void src0_stop(void);

void src1_init(u8 sample_rate, u16 samples, u8 channel);
void src1_stereo_init(u8 sample_rate, u16 samples, u8 channel);
void src1_audio_input(u8 *buf, u32 samples, int nch, void *params);
void src1_audio_output_callback_set(audio_callback_t callback);
void src1_adjust_speed(int speed);
void src1_stop(void);

void src_buf_init(u8 sample_rate, u16 samples, u8 channel);
void src_buf_audio_output_callback_set(audio_callback_t callback);
void src_buf_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void src_buf_kick(uint samples, uint ch_mode, audio_callback_t handle);

///软件src
void soft_src_init(u8 spr_in, s16 phase);
void src_phase_comp_set(int phase);
int src_process(u8 *din, u8 *dout, int in_cnt);

///------------------------------------------------------------------------------------------
///软件src算法结构体以及相关API声明
void soft_src_alg_init(u32 ch_index, u32 spr_in, u32 spr_out);
int src_frame_resample(u32 ch_index, short *src_in, short *src_out, int in_cnt);
#endif // __API_SRC_H
