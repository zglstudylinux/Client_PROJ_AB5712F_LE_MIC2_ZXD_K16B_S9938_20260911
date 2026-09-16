#ifndef _HUART_AUDIO_IN_MIX_H
#define _HUART_AUDIO_IN_MIX_H

#define HUART_RX_MAX_LEN    247
#define HUART_CMD_MAX_LEN   247

typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *params);

typedef struct {
    uint8_t rx_type;
    uint8_t rx_buf[HUART_RX_MAX_LEN];
} huart_rx_str_t;

typedef struct {
    u8 kick_flag;
    u8 mute;
    u8 sample_rate;
    u16 samples;
    cbuf_cfg_t huart_audio_in_cbuf;
    audio_callback_t callback;
} huart_audio_in_cfg_t;

void huart_audio_in_input(u8 *ptr, u32 samples, int ch_mode, void *param);
void huart_audio_in_output_callback_set(audio_callback_t callback);
void huart_audio_in_init(u8 sample_rate, u16 samples);
void huart_audio_in_mute_set(uint8_t mute);
#endif //_WIRELESS_H
