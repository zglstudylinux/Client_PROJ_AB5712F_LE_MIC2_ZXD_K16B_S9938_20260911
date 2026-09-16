#ifndef __GRCRN_QMF_HOWLING_H
#define __GRCRN_QMF_HOWLING_H

typedef struct {
    u8 mute;
    u8 sample_rate;
    volatile u8 kick_proc_done;
    u16 samples;
    audio_callback_t callback;
} gtcrn_qmf_howling_mic_cfg_t;

///库接口外的模块接口声明
void gtcrn_qmf_howling_mic_init(u8 sample_rate, u16 samples, u8 channel);
void gtcrn_qmf_howling_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void gtcrn_qmf_howling_mic_output_callback_set(audio_callback_t callback);
uint8_t gtcrn_qmf_howling_mic_mute_get(void);
void gtcrn_qmf_howling_mic_mute_set(uint8_t mute);
void gtcrn_qmf_howling_mic_exit(void);
void gtcrn_qmf_howling_mic_proc_cb(void);
void npu_exit(void);
#endif
