/**********************************************************************
*
*   strong_symbol.c
*   定义库里面部分WEAK函数的Strong函数，动态关闭库代码
***********************************************************************/
#include "include.h"

uint32_t cfg_sdk_version = SDK_VERSION;

#if !ADAPTER_USB_MIC_RX_EN
void usb_dev_isr(void){}
void ude_ep_reset(void){}
void ude_control_flow(void){}
void ude_isoc_tx_process(void){}
void ude_isoc_rx_process(void){}
void lock_code_usbdev(void){}
#endif

#if !UDE_IAP_EN
void ude_iap_init(void *tcb){}
void ude_iap_run_loop_execute(void *tcb){}
WEAK void uds_iap_reset(void){}
AT(.usbdev.com)
WEAK void get_mfi_serial_number(u8 *serial_number){}
#endif

#if !FUNC_REC_EN
bool puts_rec_encbuf(u8 *buf, u16 len) {return false;}
bool gets_rec_obuf(u8 *buf, u16 len) {return false;}
#endif
#if (REC_TYPE_SEL != REC_MP3)
int mpa_encode_frame(void) {return 0;}
#endif //(REC_TYPE_SEL != REC_MP3)

#if (REC_TYPE_SEL != REC_SBC)
bool sbc_encode_init(u8 spr, u8 nch){return false;}
void sbc_encode_process(void){}
#endif

#if (REC_TYPE_SEL != REC_ADPCM && !BT_HFP_REC_EN)
void adpcm_encode_process(void){}
#endif //(REC_TYPE_SEL != REC_ADPCM)

#if !MUSIC_WAV_SUPPORT
int wav_dec_init(void){return 0;}
bool wav_dec_frame(void){return false;}
void lock_code_wavdec(void){}
int wav_decode_init(void){return 0;}
#endif // MUSIC_WAV_SUPPORT

#if !MUSIC_WMA_SUPPORT
int wma_dec_init(void){return 0;}
bool wma_dec_frame(void){return false;}
void lock_code_wmadec(void){}
int wma_decode_init(void){return 0;}
#endif // MUSIC_WMA_SUPPORT

#if !MUSIC_APE_SUPPORT
int ape_dec_init(void){return 0;}
bool ape_dec_frame(void){return false;}
void lock_code_apedec(void){}
int ape_decode_init(void){return 0;}
#else
AT(.com_text.dac)
void obuf_put_one_sample(u32 left, u32 right)
{
    dac_put_sample_16bit(left, right);
}
#endif // MUSIC_APE_SUPPORT

#if !MUSIC_FLAC_SUPPORT
int flac_dec_init(void){return 0;}
bool flac_dec_frame(void){return false;}
void lock_code_flacdec(void){}
int flac_decode_init(void){return 0;}
#endif // MUSIC_FLAC_SUPPORT

#if !MUSIC_SBC_SUPPORT
int sbcio_dec_init(void){return 0;}
bool sbcio_dec_frame(void){return false;}
int sbcio_decode_init(void){return 0;}

AT(.sbcdec.code)
void codec_sbcdec_update(void){}
AT(.sbcdec.code)
void codec_sbcdec_init(void *param){}
#endif // MUSIC_SBC_SUPPORT

#if !MUSIC_AAC_SUPPORT
int aacio_dec_init(void) {return 0;}
int aacio_decode_init(void) {return 0;}
#endif

#if !MUSIC_M4A_SUPPORT
int m4a_decode_init(void){return 0;}
int m4a_dec_init(void){return 0;}
bool aacio_sub_process(void){return true;}
#endif

#if !MUSIC_AAC_SUPPORT && !MUSIC_M4A_SUPPORT
bool aacio_dec_frame(void) {return false;}
#endif

#if (!(MUSIC_WMA_SUPPORT | MUSIC_APE_SUPPORT | MUSIC_FLAC_SUPPORT))
void msc_stream_start(u8 *ptr) {}
void msc_stream_end(void) {}
int msc_stream_read(void *buf, unsigned int size) {return 0;}
bool msc_stream_seek(unsigned int ofs, int whence) {return false;}
void os_stream_fill(void) {}
void os_stream_read(void) {}
void os_stream_seek(void) {}
void os_stream_end(void) {}
AT(.com_text.stream)
void msc_stream_fill(void) {}
#endif

#if !FUNC_AUX_EN && !ADAPTER_AUX_TX_EN && !WIRELESS_SPK_TX_EN && (!WIRELESS_MIC_STEREO) && (!APAPTER_LOCAL_AUX_EN)
void sdadc_analog_aux_start(u8 adc_ch_get, u8 str_ch, u8 ana_gain){}
void sdadc_analog_aux_exit(u8 adc_ch_get, u8 str_ch){}
#endif

#if !BT_MUSIC_EFFECT_EN
void msc_pcm_out_var_init(void) {}
void msc_pcm_out_start(void) {}
void msc_pcm_out_stop(void) {}
u8 codecs_pcm_is_start(void) {return 0;}
AT(.com_text.codecs.pcm)
void msc_pcm_effect_process(u8 *buf, u32 samples, u32 nch, u32 is_24bit, u32 pcm_info) {}
AT(.com_text.codecs.pcm)
void msc_pcm_out_24bit_process(u32 left, u32 right) {}
AT(.com_text.codecs.pcm)
void msc_pcm_out_16bit_process(u32 left, u32 right) {}
AT(.com_text.codecs.alg)
void alg_music_effect_process(void) {}

void mp3_pcm_out_start(void) {}
AT(.mp3dec.pcm)
void mp3_pcm_out_process(u32 samples) {}
AT(.sbcdec.pcm)
void sbc_pcm_out_start(void) {}
AT(.sbcdec.pcm)
bool sbc_pcm_out_process(u32 samples, bool is_tws) { return false; }
AT(.aacdec.pcm)
void aac_pcm_out_start(void) {}
AT(.aacdec.pcm)
void aac_pcm_out_process(u32 samples, bool is_tws) {}
AT(.aacdec.pcm.clear)
void aac_tws_obuf_clear(void) {}

AT(.wavdec.code)
void obuf_put_16bit_sample_wav(u32 left, u32 right)
{
    dac_put_sample_16bit_w(left, right);
}
AT(.wavdec.code)
void obuf_put_24bit_sample_wav(u32 left, u32 right)
{
    dac_put_sample_24bit_w(left, right);
}
AT(.apedec.code)
void obuf_put_one_sample_wma(u32 left, u32 right)
{
    dac_put_sample_16bit_w(left, right);
}
AT(.flacdec.code)
void obuf_put_one_sample_flac(u32 left, u32 right)
{
    dac_put_sample_16bit_w(left, right);
}
AT(.apedec.code)
void obuf_put_one_sample_ape(u32 left, u32 right)
{
    dac_put_sample_16bit_w(left, right);
}
#endif

#if !BT_MUSIC_EFFECT_DBB_EN
u8 music_dbb_get_level_cnt(void) {return 0;}
u8 music_dbb_get_bass_level(void) {return -1;}
int music_dbb_stop(void) {return 0;}
int music_dbb_update_param(u8 vol_level, u8 bass_level) {return 0;}
int music_dbb_init(dbb_param_cb_t* p) {return 0;}
#endif

//是否支持BT AAC音频
#if !BT_A2DP_AAC_AUDIO_EN
void aac_dec_init(void) {}
void aac_decode_init(void) {}
bool aac_dec_frame(void) {return false;}
bool aac_nor_dec_frame(void) {return false;};
bool aac_tws_dec_frame(void) {return false;};
void aac_cache_free_do(void) {}
size_t aac_cache_read_do(uint8_t *buf, uint max_size) {return 0;}
uint16_t tws_pack_aac(uint8_t *buf) {
    return 0;
}
AT(.com_text.aac.obuf)
void aac_fill_tws_obuf(void) {}
AT(.aacdec.text)
void aac_obuf_tws_cpy(void) {}
AT(.aacdec.text)
bool aac_decode(void) { return false; }
#else
void aac_decode_init_do(void);
bool aac_dec_frame_do(void);
void aac_cache_free_do(void);
size_t aac_cache_read_do(uint8_t *buf, uint max_size);
AT(.text.music.init.aac)
void aac_decode_init(void) {
    aac_decode_init_do();
}
AT(.aacdec.text)
bool aac_dec_frame(void) {
    return aac_dec_frame_do();
}
AT(.aacdec.text)
void aac_cache_free(void) {
}
AT(.aacdec.text)
size_t aac_cache_read(uint8_t *buf, uint max_size) {
    return 0;
}
#endif

#if !BT_A2DP_LHDC_AUDIO_EN
void bt_lhdc_dec_init(void) {}
void lhdc_decode_init(u16 codec_id, u8 *value){}
bool lhdc_dec_init(u16 codec_id, u8 *value) {return false;}
bool lhdc_dec_frame(void) {return false;}
int lhdc_decode_frame(bool is_tws){return 0;}
void lhdc_dec_destory(void){}

void au0_dmain_start(void){}
void au0_dmain_stop(void){}
void au0_dmain_kick(s32 *ptr, u32 samples){}
AT(.com_text.au0dma)
void au0_dmain_isr(void){}
void au0_dmain_sem_init(void){}
void spiflash_security_uid_read(void){}

void lhdc_obuf_tws_cpy(void) {}
void lhdc_fill_tws_obuf(void) {}
void lhdc_cpy_tws_obuf(void) {}
AT(.com_text.sbc.play)
void lhdc_dec_kick_isr(uint32_t ticks){}
u8 avdtp_tws_get_lhdc_spr(uint16_t codec_id, uint8_t spec_val){return 0x01;}
bool lhdc_nor_dec_frame(void){return false;}
bool lhdc_tws_dec_frame(void){return false;}
void lhdc_pcm_out_process(u8 *buf, u32 samples, u32 nch, u32 in_24bits, bool is_tws){}
void gpdma_lhdc_kick(u32 *ptr, u32 samples, u32 is_24bit) {}
void lhdc_kick_copy_tws_obuf(void){}
void lhdc_gpdma_done(void) {}
#else
size_t lhdc_cache_read_do(uint8_t *buf, uint max_size);
void lhdc_cache_free_do(void);

AT(.text.lhdc.dec.cache)
void lhdc_cache_free(void) {
}
AT(.text.lhdc.dec.cache)
size_t lhdc_cache_read(uint8_t *buf, uint max_size) {
    return 0;
}
#endif

#if !BT_A2DP_LDAC_AUDIO_EN
void bt_ldac_dec_init(void) {}
void ldac_decode_init(u16 codec_id, u8 *value){}
bool ldac_dec_init(u16 codec_id, u8 *value) {return false;}
bool ldac_dec_frame(void) {return false;}
int ldac_decode_frame(bool is_tws){return 0;}
void ldac_dec_destory(void){}

void ldac_obuf_tws_cpy(void) {}
void ldac_fill_tws_obuf(void) {}
void ldac_cpy_tws_obuf(void) {}
AT(.com_text.sbc.play)
void ldac_dec_kick_isr(uint32_t ticks){}
u8 avdtp_tws_get_ldac_spr_idx(uint8_t spr_bitmap){return 0x01;}
bool ldac_nor_dec_frame(void){return false;}
bool ldac_tws_dec_frame(void){return false;}
void ldac_kick_copy_tws_obuf(void) {}
bool ldac_tws_dec_waiting(void){return true;}

void ldac_pcm_out_process(u8 *buf, u32 samples, u32 nch, u32 in_24bits, bool is_tws) {}
void ldac_gpdma_done(void) {}
void gpdma_ldac_kick(u32 *ptr, u32 samples, u32 is_24bit) {}
#else
size_t ldac_cache_read_do(uint8_t *buf, uint max_size);
void ldac_cache_free_do(void);

AT(.text.ldac.dec.cache)
void ldac_cache_free(void) {
}
AT(.text.ldac.dec.cache)
size_t ldac_cache_read(uint8_t *buf, uint max_size) {
    return 0;
}
#endif

#if !FMRX_REC_EN
void fmrx_rec_start(void){}
void fmrx_rec_stop(void){}
#endif // FMRX_REC_EN

#if !BT_REC_EN && !KARAOK_REC_EN
void bt_music_rec_start(void) {}
void bt_music_rec_stop(void) {}
void au0_dma_isr(void){}
#endif

#if !USB_SUPPORT_EN
void usb_isr(void){}
void usb_init(void){}
#endif

#if ((!SD_SUPPORT_EN) && (!FUNC_USBDEV_EN) && (!ADAPTER_USB_STORAGE_AUDIO))
void sd_disk_init(void){}
void sdctl_isr(void){}
void sd_disk_switch(u8 index){}
bool sd0_stop(bool type){return false;}

bool sd0_init(void){return false;}
bool sd0_read(void *buf, u32 lba){return false;}
bool sd0_write(void* buf, u32 lba){return false;}

#endif

#if !SPI_HW_EN
void set_spi1_baud(u32 baud){}
void update_spi1baud(void){}
#endif

#if (!LE_FCC_TEST_EN && !FUNC_BT_FCC_EN)
uint8_t vs_ble_test(void const *cmd) {
    return 0x11;
}

#if !LE_BQB_RF_EN
uint8_t ble_test_start(void* params) {
    return 0x0c;
}
uint8_t ble_test_stop(void) {
    return 0x0c;
}
#endif
#endif

#if LE_FCC_TEST_EN || FUNC_BT_FCC_EN
uint8_t vs_ble_test_do(void const *cmd);
uint8_t vs_ble_test(void const *cmd) {
    return vs_ble_test_do(cmd);
}
#else
AT(.com_text.bt21.isr.test.fcc)
bool bt_acl_test_rx_end(uint8_t index, void *par) {
    return false;
}
#if !IODM_TEST_EN
uint8_t vs_fcc_test_cmd(void const *param) {
    return 0x11;
}
#endif

#if !LE_BQB_RF_EN
void hci_h3c_init(void) {}
void bt_uart_init(void){}
AT(.com_text.stack.uart_isr)
bool bt_uart_isr(void) {
    return false;
}
#endif
#endif

#if !BT_HFP_REC_EN
AT(.com_text.bt_rec)
void bt_sco_rec_mix_do(u8 *buf, u32 samples) {}
void bt_sco_fill_remote_buf(u16 *buf, u16 samples) {}
#endif


#if !FUNC_SPDIF_EN
void spdif_pcm_process(void){}
bool spdif_smprate_detect(void) {    return false;}
AT(.com_text.isr.spdif)
void spdif_isr(u32 spfrx_pnd){}
#endif

#if !FUNC_SPDIF_TX_EN
AT(.com_text.isr.spdif)
void spdif_tx_isr(u32 spftx_pnd) {}
#endif


#if !KARAOK_REC_EN
AT(.com_text.karaok.rec)
bool karaok_rec_fill_buf(u8 *buf, u16 len) {return false;}
#endif

#if !I2S_DMA_EN
void i2s_isr(void) {}
void i2s_process(void) {}
#endif

void voice_assistant_enc_process(void);
AT(.com_text.opus)
void opus_enc_process(void)
{

}

#if !BT_HID_EN
void hid_device_init(void) {}
int bt_hid_is_connected(void) { return 0;}
void hid_establish_service_level_connection(void* bd_addr) {}
void hid_release_service_level_connection(void* bd_addr) {}
void *get_hid_device_context_for_bd_addr(void *bd_addr) {return NULL;}
void remove_hid_device_context(void * hid_device) {}
void btstack_hid_api(uint param) {}
const void *btstack_hid_tbl[0];
void btstack_hid_send(void) {}
bool bt_hid_send(void *buf, uint len, bool auto_release) { return false;}
bool bt_hid_send_key(uint type, uint keycode) {return false;}
bool bt_hid_is_send_complete(void) { return true;}
void hid_report_set(void *buf, uint len, bool auto_release) {}
#endif


#if !BT_AVDTP_DYN_LATENCY_EN
void a2dp_set_latency_ms(uint32_t a2dp_latency_ms) {}
void a2dp_latency_check(void) {}
uint16_t a2dp_calc_latency(int8_t rssi, uint16_t rxpkt_ok, uint8_t rxpkt_fail, uint8_t rxpkt_total) {return 0;}
AT(.com_text.sbc.play)
void a2dp_switch_latency(void) {}
#endif

#if !BT_A2DP_EXCEPT_RESTORE_PLAY_EN
void noload_set_play_state(uint8_t play_state){}
uint16_t noload_get_play_state(uint8_t play_state){return 0;}
void noload_clear_play_state(uint8_t play_state){}
void noload_reset_play_state() {}
#endif

WEAK uint8_t bt_get_connected_num(void) {return 0;}

#if (UART0_PRINTF_SEL == PRINTF_NONE)
void wdt_irq_init(void) {}
#endif

#if SYS_SLEEP_LEVEL > 2
void sys_sleep_restore(void);
AT(.com_text.ret.bb)
void nanos_sleep_restore(void)
{
    sys_sleep_restore();
}

AT(.sleep_text.sleep.cb)
void sys_sleep_proc_cb(u8 lpclk_type)
{
    sys_sleep_proc_lv(lpclk_type, 0x84);
}
#endif

#if !TKEY_EN
AT(.com_text.tkey.isr)
void tkey_isr(void) {}
#endif

#if !ANC_ALG_EN
void alg_anc_process(void) {}
void anc_alg_sdadc_process(u8* ptr, u32 samples, int ch_mode) {}
void anc_alg_init(anc_alg_param_cb* p) {}
#endif

#if !WIRELESS_MIC_SRC_EN && !ADAPTER_HARDWARE_SRC1_EN && !WIRELESS_MIC_HARDWARE_SRC1_EN
void src_sem_init(void) {}
AT(.com_text.src_isr)
void src_isr(void) {}
#endif

//不够flash空间时可去掉差分或VCMBUF模式
#if DAC_DIFF_DIS
void dac_diff_ang_power_on(u32 restart) { printk("diff error\n");}
void dac_diff_power_off(void) {}
#endif
#if DAC_VCMBUF_DIS
void dac_vcmbuf_ang_power_on(u32 restart) { printk("vcmbuf error\n");}
void dac_vcmbuf_power_off(void) {}
#endif

#if !FOT_EN && !AB_MATE_APP_EN
u8 fot_checksum_cal(u8 *buf) {return 0;}
#endif

#if !DNN_L1_EN
void dnn_L1_mic_proc_cb(void){}
void dnn_L1_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params){}
void dnn_L1_mic_init(u8 sample_rate, u16 samples, u8 channel){}
void dnn_L1_mic_mute_set(uint8_t mute){}
uint8_t dnn_L1_mic_mute_get(void){return 0;}
#endif

#if !DNN_L3_EN
void dnn_L3_mic_proc_cb(void){}
void dnn_L3_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params){}
void dnn_L3_mic_init(u8 sample_rate, u16 samples, u8 channel){}
void dnn_L3_mic_mute_set(uint8_t mute){}
uint8_t dnn_L3_mic_mute_get(void){return 0;}
#endif

#if !AINS4_EN
void ains4_mic_proc_cb(void){}
//void ains4_process(s16 *data){}
void ains4_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params){}
void ains4_mic_init(u8 sample_rate, u16 samples, u8 channel){}
void ains4_mic_mute_set(uint8_t mute){}
uint8_t ains4_mic_mute_get(void){return 0;}
void ains4_mic_param_set(u8 mode ,s16 ains4_noise_nt){}
#endif

#if !GTCRN_48K_QMF_EN
void gtcrn_48k_qmf_mic_proc_cb(void){}
int gtcrn_48k_qmf_ns_process(s16* data) { return 0;}
void gtcrn_48k_qmf_ns_init(gtcrn_48k_qmf_cb_t *p) {}
#endif

#if !NOTCH_L2_EN
void howling_notch_L2_mic_proc_cb(void){}
//void howling_notch_L2_process(s16 *data){}
void howling_notch_L2_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params){}
void howling_notch_L2_mic_init(u8 sample_rate, u16 samples, u8 channel){}
void howling_notch_L2_mic_mute_set(uint8_t mute){}
uint8_t howling_notch_L2_mic_mute_get(void){return 0;}
void howling_notch_L2_mic_param_set(s16 howling_notch_L2_noise_nt){}
#endif

#if !AGC_EN
void agc_mic_proc_cb(void){}
#endif

#if !HOWLING_DNN_EN
void howling_dnn_mic_proc_cb(void){}
#endif
