#ifndef _I2S_AUDIO_H
#define _I2S_AUDIO_H

#include "bsp_i2s.h"

#define I2S_INCACHE_SIZE            2048
#define I2S_OUTCACHE_SIZE           512

typedef struct {
    uint8_t dmabuf[I2S_DMABUF_LEN];
    uint8_t incache[I2S_INCACHE_SIZE];
    uint8_t outcache[I2S_OUTCACHE_SIZE];

    cbuf_cfg_t in_cbuf;
} i2s_audio_buf_t;

typedef struct {
    u16 samples;
    u8  ch_mode;
    s8  speed;

    audio_callback_t callback;
    i2s_cfg_t i2s_cfg;
} i2s_in_ctl_t;

//I2S INPUT
void i2s_audio_in_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void i2s_audio_input_callback_set(audio_callback_t callback);
void i2s_audio_in_init(u8 sample_rate, u16 samples, u8 channel);
void src1_audio_adj_process(void);

//I2S OUTPUT
void i2s_audio_out_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void i2s_audio_output_callback_set(audio_callback_t callback);
void i2s_audio_out_init(u8 sample_rate, u16 samples, u8 channel);

void i2s_audio_in_out_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void i2s_audio_in_out_output_callback_set(audio_callback_t callback);
void i2s_audio_in_out_init(u8 sample_rate, u16 samples, u8 channel);
void i2s_audio_in_out_exit(void);
#endif
