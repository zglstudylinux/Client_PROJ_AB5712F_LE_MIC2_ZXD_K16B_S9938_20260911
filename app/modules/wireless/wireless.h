#ifndef _WIRELESS_H
#define _WIRELESS_H

#include "wireless_txrx.h"
#include "wireless_proc.h"
#include "wireless_cmd.h"
#include "wireless_pwr_ctr.h"
#include "../../os/os_thread.h"


#if WIRELESS_CON_COMB_BUF_EN
    #define WIRELESS_MIC_RX_BUFFER_SIZE     (WIRELESS_MIC_FRAME_SIZE*WIRELESS_MIC_COMB_NB)*WIRELESS_MIC_RETRY_NB
    #define WIRELESS_MIC_TX_BUFFER_SIZE     (WIRELESS_MIC_FRAME_SIZE*WIRELESS_MIC_COMB_NB)*WIRELESS_MIC_RETRY_NB
#else
    #define WIRELESS_MIC_RX_BUFFER_SIZE     (WIRELESS_MIC_FRAME_SIZE*WIRELESS_MIC_COMB_NB)
    #define WIRELESS_MIC_TX_BUFFER_SIZE     (WIRELESS_MIC_FRAME_SIZE*WIRELESS_MIC_COMB_NB)
    #define WIRELESS_SPK_RX_BUFFER_SIZE     (WIRELESS_SPK_FRAME_SIZE*WIRELESS_SPK_COMB_NB)
    #define WIRELESS_SPK_TX_BUFFER_SIZE     (WIRELESS_SPK_FRAME_SIZE*WIRELESS_SPK_COMB_NB)
#endif

#if DEVICE_INTERPHONE_EN
void wireless_interphone_init(void);
void wireless_interphone_exit(u8 idx, u8 con_sta);
#endif

void mic_enc_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void mic_enc_audio_output_callback_set(audio_callback_t callback);
void mic_enc_init(u8 sample_rate, u16 samples, u8 channel);
void mic_enc_reset(void);

void mic_dec_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void mic_dec_audio_output_callback_set(audio_callback_t callback);
void mic_dec_init(u8 sample_rate, u16 samples, u8 channel);
void mic_dec_reset(u8 idx);

void spk_dec_audio_input(u8 *ptr, u32 len, int ch_mode, void *params);
void spk_dec_audio_output_callback_set(audio_callback_t callback);
void spk_dec_init(u8 sample_rate, u16 samples, u8 channel);
void spk_enc_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void spk_enc_audio_output_callback_set(audio_callback_t callback);
void spk_enc_init(u8 sample_rate, u16 samples, u8 channel);

void wireless_init(void);
void wireless_adapter_init(void);
void wireless_device_init(void);
void wireless_device_exit(void);
void wireless_adapter_exit(u8 idx, u8 con_sta);
void wireless_channel_status(u8 idx, u8 chidx, s8 rssi, u8 bfi, u8 chstatus);

#if WIRELESS_DUMP_EN
void wireless_dump_init(void);
void wireless_dump_proc(void);
void wireless_dump_reset(u8 idx);
void wireless_dump_set_rx_status(u8 idx, u8 chidx, s8 rssi, u8 bfi);
void wireless_dump_set_chmap_cb(u8 idx, const u8 *chmap);
#else
    #define wireless_dump_init()
    #define wireless_dump_proc()
    #define wireless_dump_reset(idx)
    #define wireless_dump_set_rx_status(idx, chidx, rssi, bfi)
    #define wireless_dump_set_chmap_cb(idx, chmap)
#endif

void load_code_wl_ains4(void);

#endif //_WIRELESS_H
