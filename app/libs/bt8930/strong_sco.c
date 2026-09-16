/**********************************************************************
*
*   strong_sco.c
*   定义库里面通话算法部分WEAK函数的Strong函数，动态关闭库代码
***********************************************************************/
#include "include.h"

/*****************************************************************************
 * Module    : AEC算法强定义
 *****************************************************************************/

#if !BT_ALC_EN
void alc_init(void) {}
void alc_process(void) {}
void alc_fade_in(s16 *buf) {}
void alc_fade_out(s16 *buf) {}
AT(.bt_voice.alc)
bool alc_isr(void) { return false; }
AT(.bt_voice.alc)
void bt_alc_process(u8 *ptr, u32 samples, int ch_mode) {};
#endif


/*****************************************************************************
 * Module    : 通话其他部分强定义
 *****************************************************************************/

#if !BT_SCO_DUMP_EN && !BT_AEC_DUMP_EN && !BT_SCO_FAR_DUMP_EN && !BT_EQ_DUMP_EN
void sco_huart_init(void){}
bool sco_huart_putcs(u8 type, u8 frame_num, const void *buf, uint len){return 0;}
s16 bt_sco_dump_buf[0];
AT(.com_huart.text) WEAK
void sco_huart_tx_done(void){}
#endif

#if !BT_AEC_DUMP_EN && !BT_SCO_FAR_DUMP_EN && !BT_EQ_DUMP_EN
void aec_audio_dump_init(void){}
void aec_audio_dump(u8 type, s16* ptr, u16 len, u8 ch){}
#endif

#if !SDADC_SOFT_GAIN_EN
AT(.com_text.sdadc.soft_gain)
void sdadc_drc_soft_gain_proc(s16 *ptr, u32 samples, int ch_mode) {}
AT(.com_text.sdadc.soft_gain.proc)
bool sdadc_soft_gain_proc(s16 *ptr, u32 samples, int ch_mode){return false;}
bool sdadc_drc_init(u8 *drc_addr, int drc_len){return false;}
void sdadc_set_soft_gain(u16 ch, u32 gain) {}
#else
    void sdadc_set_digital_gain(u16 ch, u16 gain) {}
#endif

#if !SDADC_EQ_EN
AT(.com_text.sdadc.soft_eq)
void sdadc_soft_eq_proc(s16 *ptr, u32 samples, int ch_mode) {}
bool sdadcl_set_soft_eq_by_res(u32 addr, u32 len){return false;}
bool sdadcr_set_soft_eq_by_res(u32 addr, u32 len){return false;}
#endif

#if !BT_SCO_FADE_EN
void bt_voice_fade_init(u32 timeout) {}
AT(.bt_voice.sco.fade)
void bt_voice_fade_do(s16 *ptr, u32 samples, int ch_mode) {}
#else
AT(.bt_voice.sco.skip)
void bt_voice_skip_frame_do(s16 *ptr, u32 samples, int ch_mode) {}
#endif

/*****************************************************************************
 * Module    : 通话上行降噪算法强定义
 *****************************************************************************/
#if !BT_TRUMPET_NR_EN
void trumpet_denoise_init(u8 level) {}
void trumpet_denoise(int *fft_in) {}
AT(.bt_voice.sco.trumpet)
void bt_sco_trumpet_nr(s32 *fft_in) {}
#endif

#if !BT_SNDP_SMIC_EN
void bt_sndp_sm_init(void *alg_cb) {}
void sndp_sm_process(void) {}
void bt_sndp_sm_exit(void) {}
#endif

#if !BT_SNDP_DMIC_EN && !BT_SNDP_DMIC_DNN_EN
void bt_sndp_dm_proc_do(void) {}
void bt_sndp_dm_process(s16 *buf) {}
void bt_sndp_dmic_init(void) {}
void bt_sndp_dmic_exit(void) {}
#endif

#if !BT_SCO_SMIC_AI_EN && !BT_SCO_DMIC_AI_EN
AT(.bt_voice.sco)
bool bt_sco_dnn_en(void) {return 0;}
#endif

#if !BT_SCO_SMIC_AI_EN
void bt_dnn_init(void *alg_cb) {}
void dnn_sm_process(void) {}
void bt_dnn_exit(void) {}
#endif

#if !BT_SCO_DMIC_AI_EN
void bt_dmns_init(void *alg_cb) {}
void dnn_dm_process(void) {}
void bt_dmns_exit(void) {}
#endif

#if !BT_SCO_NR_USER_SMIC_EN && !BT_SCO_NR_USER_DMIC_EN
void bt_nr_user_proc_do(s16 *mic1, s16 *mic2, nr_cb_t *nr_cb) {}
void bt_nr_user_init(void *alg_cb, nr_cb_t *nr_cb) {}
#endif

#if !BT_SCO_AEC_USER_EN
AT(.bt_voice.soc_nr.far)
void bt_sco_far_get(s16 *buf) {}
void bt_sco_far_cache_init(void) {}
#endif

#if !BT_SCO_NR_EN
void bt_ains_init(void *alg_cb) {}
void ains_sm_process(void) {}
void bt_ains_exit(void) {}
#endif

#if !BT_SNDP_FBDM_EN
void bt_sndp_fbdm_init(void *alg_cb) {}
void alg_sndp_fbdm_process(void) {}
void bt_sndp_fbdm_exit(void) {}
#endif

#if !BT_SNDP_DM_EN
void bt_sndp_dm_init(void *alg_cb) {}
void alg_sndp_dm_process(void) {}
void bt_sndp_dm_exit(void) {}
#endif

/*****************************************************************************
 * Module    : 通话下行降噪算法强定义
 *****************************************************************************/
#if !BT_SCO_FAR_NR_EN
AT(.com_text.isr.peri_nr)
void peri_nr_isr(void) {}
AT(.bt_voice.peri_nr)
void bt_sco_far_peri_nr_process(s16 *ptr, u16 samples) {}
void bt_sco_far_nr_int(void *cfg) {}
void bt_peri_nr_exit(void) {}
void bt_peri_nr_init(void) {}
#endif

#if !BT_SCO_CALLING_NR_EN
AT(.bt_voice.sco)
u16 dnr_voice_maxpow_calling(u32 *ptr, u16 len){return 0;}
#endif

#if !BT_SCO_DAC_DRC_EN
bool bt_sco_dac_drc_init(u8 *drc_addr, int drc_len) {return false;}
AT(.bt_voice.dac.drc)
s16 bt_sco_dac_drc_calc(s32 data) {return 0;}
AT(.bt_voice.dac.drc)
void bt_sco_dac_drc_proc(s16 *ptr, u32 samples) {}
#endif

#if !BT_SCO_DAC_DNR_EN
AT(.bt_voice.dac.dnr)
void bt_sco_dac_dnr_proc(s16 *ptr, u32 samples, int dnr_thr) {}
#endif


