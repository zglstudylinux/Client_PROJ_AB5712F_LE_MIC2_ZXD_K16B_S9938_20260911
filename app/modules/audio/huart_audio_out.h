#ifndef _HUART_AUDIO_OUT_H
#define _HUART_AUDIO_OUT_H

#define HUART_TX_MAX_LEN      247

typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *params);

enum{
    TX_TYPE_AUDIO           = 0xb0,
    TX_TYPE_CMD,
    TX_TYPE_AUDIO_CMD,
};

typedef struct {
    uint8_t tx_type;
    uint8_t tx_buf[HUART_TX_MAX_LEN];
    uint8_t tx_buf_len;
} huart_tx_str_t;


typedef struct {
    u8 mute;
    u8 sample_rate;
    u16 samples;
    audio_callback_t callback;
} huart_audio_out_cfg_t;

void huart_audio_out_input(u8 *ptr, u32 samples, int ch_mode, void *param);
void huart_audio_out_output_callback_set(audio_callback_t callback);
void huart_audio_out_init(u8 sample_rate, u16 samples);
void huart_audio_out_mute_set(uint8_t mute);
#endif //_WIRELESS_H
