#include "include.h"


//MIC analog gain: 0~14(共15级), step 3DB (0db ~ +42db)
//adadc digital gain: 0~63, step 0.5 DB, 保存在gain的低6bit
const sdadc_cfg_t rec_cfg_tbl[] = {
/*   通道,             采样率,    模拟增益, 数字增益,    BITS,    通路控制,    样点数,   回调函数*/
//    {AUX_CHANNEL_CFG,  SPR_48000,   2,         0,       0xff,     ADC2DAC_EN,    256,    aux_sdadc_callback},            /* AUX     */
//    {MIC_CHANNEL_CFG,  SPR_8000,    12,        0,       1,        ADC2DAC_EN,    480,    bt_sdadc_callback},             /* BTMIC   */
//    {MIC_CHANNEL_CFG,  SPR_48000,   12,        0,       1,        ADC2DAC_EN,    256,    usbmic_sdadc_callback},         /* USBMIC  */
//    {MIC_CHANNEL_CFG,  SPR_44100,   12,        0,       0xff,     ADC2SRC_EN,    256,    karaok_sdadc_callback},         /* KARAOK  */
//    {MIC_CHANNEL_CFG,  SPR_16000,   12,        0,       0xff,     ADC2DAC_EN,    256,    opus_sdadc_callback},           /* opus  */
//    {MIC_CHANNEL_CFG,  SPR_44100,   6,         0,       0xff,     ADC2SRC_EN,    256,    ttp_sdadc_callback},            /* TRANSPARENCY  */
//    {MIC_CHANNEL_CFG,  SPR_16000,   12,        0,       1,        ADC2SANC_EN,   256,    anc_alg_sdadc_callback},        /* ANC ALG */
//    {MIC_CHANNEL_CFG,  SPR_48000,   10,        0,       0xff,     ADC2DAC_EN,    256,    iodm_test_sdadc_callback},      /* IODM TEST */
};

AT(.rodata.mic)
const u16 mic_ch_tbl[5] = {CH_MIC0, CH_MIC1, CH_MIC2, CH_MIC3, CH_MIC4};

#if FUNC_AUX_EN || ADAPTER_AUX_TX_EN || WIRELESS_MIC_STEREO || DEVICE_STEREO_EN || APAPTER_LOCAL_AUX_EN
AT(.rodata.aux)
static const u16 auxl_adc_tbl[5] = {
    0, CH_AUXL0, CH_AUXL1, CH_MIC2, CH_MIC3
};

AT(.rodata.aux)
static const u16 auxr_adc_tbl[5] = {
    0, CH_AUXR0, CH_AUXR1, CH_MIC3, CH_MIC2
};

u16 bsp_aux_ch_getcfg(void);
#endif

/*****************************************************************************
 * 功能   : 初始化时，检查mic在anc、bt_call时选择是否正确
 * 输入   : 无
 * 注意   : anc单馈默认使用mic0、双馈默认使用mic0\mic1
 * 返回   : 无
 *****************************************************************************/
void mic_channel_check(void)
{
    bool mic[5] = {false};

    if(xcfg_cb.mic_en) {
        mic[xcfg_cb.bt_mmic_cfg] = true;
    }

    if ((mic[xcfg_cb.bt_mmic_cfg] && !xcfg_cb.mic_en)) {
        printf("error: mic's configuration error!\n");
        print_r(mic, 5);
        while (1) WDT_CLR();
    }
}

/*****************************************************************************
 * 功能   : 初始化对应AUDIO_PATH
 * 输入   : audio_path_idx 对应不同功能使用的audio通路
 * 注意   : 区分bt_call和其他状态
 * 返回   : 无
 *****************************************************************************/
void audio_path_init(u8 path_idx)
{
    sdadc_cfg_t cfg;
    memcpy(&cfg, &rec_cfg_tbl[path_idx], sizeof(sdadc_cfg_t));

#if FUNC_AUX_EN
    if (path_idx == AUDIO_PATH_AUX) {
        cfg.channel = bsp_aux_ch_getcfg();
        cfg.anl_gain = xcfg_cb.aux_anl_gain | xcfg_cb.aux_anl_gain<<6;      //双声道模拟增益保持一致
        cfg.dig_gain = xcfg_cb.aux_dig_gain | xcfg_cb.aux_dig_gain<<6;      //双声道数字增益保持一致
    }
#endif // FUNC_AUX_EN

    sdadc_init(&cfg);
}

/*****************************************************************************
 * 功能   : 启动AUDIO采集和DAC数据处理
 * 输入   : audio_path_idx 对应不同功能使用的audio通路
 * 注意   : channel需要和init时保持一致，否则通路会启动失败
 * 返回   : 无
 *****************************************************************************/
void audio_path_start(u8 path_idx)
{
    sdadc_cfg_t cfg;
    memcpy(&cfg, &rec_cfg_tbl[path_idx], sizeof(sdadc_cfg_t));
#if FUNC_AUX_EN
    if (path_idx == AUDIO_PATH_AUX) {
        cfg.channel = bsp_aux_ch_getcfg();
    }
#endif // FUNC_AUX_EN

    sdadc_start(cfg.channel);
}

/*****************************************************************************
 * 功能   : 关闭对应AUDIO_PATH
 * 输入   : audio_path_idx 对应不同功能使用的audio通路
 * 注意   : 关闭audio后，功耗要和打开audio之前保持一致
 * 返回   : 无
 *****************************************************************************/
void audio_path_exit(u8 path_idx)
{
    sdadc_cfg_t cfg;
    memcpy(&cfg, &rec_cfg_tbl[path_idx], sizeof(sdadc_cfg_t));

#if FUNC_AUX_EN
    if (path_idx == AUDIO_PATH_AUX) {
        cfg.channel = bsp_aux_ch_getcfg();
    }
#endif // FUNC_AUX_EN

    {
        adpll_spr_set(DAC_OUT_SPR);
    }
}

u16 bsp_aux_ch_getcfg(void)
{
    if(!xcfg_cb.adapter_aux_en || (!xcfg_cb.auxl_sel && !xcfg_cb.auxr_sel)) {        //需要打开AUX配置对应的IO才能使用AUX相关功能
        printf("AUX IO CONFIG ERROR\n");
        WDT_RST();
    }
#if FUNC_AUX_EN || ADAPTER_AUX_TX_EN || WIRELESS_MIC_STEREO || DEVICE_STEREO_EN || APAPTER_LOCAL_AUX_EN
    u16 aux_sel = auxl_adc_tbl[xcfg_cb.auxl_sel] | (auxr_adc_tbl[xcfg_cb.auxr_sel] << 8);
    return aux_sel;
#else
    return 0;
#endif // FUNC_AUX_EN
}
