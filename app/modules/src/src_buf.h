#ifndef __SRC_BUF_H
#define __SRC_BUF_H

#define INCACHE_SIZE        2048
#define OUTCACHE_SIZE       2048

typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *params);
//
//typedef struct {
//    volatile u16 len;
//    u8 *obuf;
//    u8 *wptr;
//    u8 *rptr;
//    u8 *init_buf;
//    u8 enable;
//    u16 buf_len;
//} cbuf_cfg_t;

typedef struct {
    cbuf_cfg_t cbuf;
    u16 input_min;
    u16 input_min1;
    u8  input_cnt;
    s8  speed;
    bool start;

    u8  mute;
    u8  sample_rate;
    u16 samples;
    u8  ch_mode;
    audio_callback_t callback;
    audio_callback_t handle;
} src_buf_t;

///外部变量及api
void cbuf_input_audio(u8 *ptr, u16 samples, cbuf_cfg_t *emt);
bool cbuf_output_audio(u8 *buf, u16 samples, cbuf_cfg_t *emt);
u16  cbuf_total_samples_get(cbuf_cfg_t *emt);
void cbuf_init(cbuf_cfg_t *cbuf, u8 *buf, u16 buf_size);


#endif

