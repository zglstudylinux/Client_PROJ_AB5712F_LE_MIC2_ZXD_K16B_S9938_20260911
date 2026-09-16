#ifndef __AEC_H
#define __AEC_H

///公共
typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *params);
typedef struct {
    u8 mute;
    u8 sample_rate;
    u16 samples;
    volatile u8 kick_proc_done;
    volatile u8 aec_flag;
    uint8_t aec_in_cnt;
    uint8_t aec_out_cnt;
    audio_callback_t callback;
} aec_mic_cfg_t;


#endif
