#ifndef  __MIC_H_
#define  __MIC_H_


void mic_init(u8 sample_rate, u16 samples, u8 channel);
void mic_audio_output_callback_set(audio_callback_t callback);
void mic_start(void);
void mic_stop(void);
void wireless_mic_reset(void);

void local_mic_init(void);

#endif
