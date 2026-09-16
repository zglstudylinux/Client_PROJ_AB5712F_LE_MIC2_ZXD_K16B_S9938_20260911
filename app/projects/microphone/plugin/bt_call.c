#include "include.h"

#if BT_SNDP_SMIC_EN
    static sndp_sm_cb_t sndp_sm_cb AT(.sndp_buf);
#elif BT_SNDP_DM_EN
    static sndp_dm_cb_t sndp_dm_cb AT(.sndp_dm_buf);
#elif BT_SNDP_FBDM_EN
    static sndp_fbdm_cb_t sndp_fbdm_cb AT(.sndp_fbdm_buf);
#elif BT_SCO_SMIC_AI_EN
    static dnn_cb_t dnn_cb AT(.dnn_buf);
#elif BT_SCO_DMIC_AI_EN
    static dmns_cb_t dmns_cb AT(.dmdnn_buf);
#elif BT_SCO_NR_EN
    static ains_cb_t ains_cb AT(.ains_buf);
#endif

#if BT_SCO_FAR_NR_EN
static peri_nr_cfg_t far_cfg AT(.nr_buf.far);
#endif

void bt_sco_rec_exit(void);

AT(.text.sco.gain.tbl)
const int mic_gain_tbl[16] = {
    SOFT_DIG_P0DB,
    SOFT_DIG_P1DB,
    SOFT_DIG_P2DB,
    SOFT_DIG_P3DB,
    SOFT_DIG_P4DB,
    SOFT_DIG_P5DB,
    SOFT_DIG_P6DB,
    SOFT_DIG_P7DB,
    SOFT_DIG_P8DB,
    SOFT_DIG_P9DB,
    SOFT_DIG_P10DB,
    SOFT_DIG_P11DB,
    SOFT_DIG_P12DB,
    SOFT_DIG_P13DB,
    SOFT_DIG_P14DB,
    SOFT_DIG_P15DB,
};

void bt_sco_dump_init(u8 *sysclk, call_cfg_t *p)
{
    nr_cb_t *nr = &p->nr;
#if BT_SCO_DUMP_EN
    #if BT_SCO_SMIC_EN
    nr->dump_en = DUMP_SNDP_TALK | DUMP_SNDP_NR;
    #elif BT_SCO_DMIC_EN
    nr->dump_en = DUMP_SNDP_TALK | DUMP_SNDP_FF | DUMP_SNDP_NR;
    #endif
#elif BT_AEC_DUMP_EN
    nr->dump_en = DUMP_AEC_INPUT | DUMP_AEC_FAR | DUMP_AEC_OUTPUT;
#elif BT_SCO_FAR_DUMP_EN
    nr->dump_en = DUMP_FAR_NR_INPUT | DUMP_FAR_NR_OUTPUT;
#elif BT_EQ_DUMP_EN
    nr->dump_en = DUMP_SNDP_TALK | DUMP_SNDP_NR | DUMP_EQ_OUTPUT;
#endif
    if (nr->dump_en != 0) {
        if (!xcfg_cb.huart_en) {
            printf("dump huart xcfg init err!\n");
        }
        //*sysclk = *sysclk < SYS_120M ? SYS_120M : *sysclk;
    }
}

void bt_sco_aec_init(u8 *sysclk, aec_cfg_t *aec, alc_cb_t *alc)
{
    if (xcfg_cb.bt_aec_en) {
    #if BT_AEC_EN
        aec->type           = AEC_TYPE_DEFAULT;
        aec->mode           = 1;
        aec->nlp_bypass     = 0;
        aec->nlp_only       = 0;
        aec->nlp_level      = BT_ECHO_LEVEL;
        aec->nlp_part       = 1;
        aec->comforn_level  = 10;
        aec->comforn_floor  = 300;
        aec->comforn_en     = 0;
        aec->xe_add_corr    = 16384;
        aec->upbin          = 63;
        aec->lowbin         = 7;
        aec->diverge_th     = 3;
        aec->echo_th        = 666;
        aec->gamma          = 29490;
        aec->aggrfact       = 28180;
        aec->depre_mode     = 1;
        aec->mic_ch         = 0;
        aec->dig_gain       = mic_gain_tbl[xcfg_cb.aec_dig_gain] >> 8;
        aec->far_offset     = 301;
        *sysclk = *sysclk < SYS_48M ? SYS_48M : *sysclk;
    #endif
    } else {
        aec->type         = AEC_TYPE_NONE;
    }
}

void bt_sco_far_nr_init(u8 *sysclk, nr_cb_t *nr)
{
#if BT_SCO_FAR_NR_EN
    nr->nr_cfg_en |= NR_CFG_FAR_EN;
    memset(&far_cfg, 0, sizeof(peri_nr_cfg_t));
    nr->far_nr = &far_cfg;
    far_cfg.level = BT_SCO_FAR_NOISE_LEVEL;
    far_cfg.min_range1 = 30;
    far_cfg.min_range2 = 50;
    far_cfg.min_range3 = 80;
    far_cfg.min_range4 = 120;
    far_cfg.nera_val = 0x7eb8;
#endif

#if BT_SCO_DAC_DRC_EN
    nr->nr_cfg_en |= NR_CFG_DAC_DRC_EN;
    bt_sco_dac_drc_init((u8 *)RES_BUF_EQ_CALL_DAC_DRC, RES_LEN_EQ_CALL_DAC_DRC);
#endif

#if BT_SCO_DAC_DNR_EN
    nr->nr_cfg_en |= NR_CFG_DAC_DNR_EN;
    nr->dac_dnr_thr = BT_SCO_DAC_DNR_THR;
#endif

#if BT_SCO_CALLING_NR_EN
    nr->calling_voice_cnt = BT_SCO_CALLING_VOICE_CNT;
    nr->calling_voice_pow = BT_SCO_CALLING_VOICE_POW;
#endif
}

void bt_sco_nr_init(u8 *sysclk, nr_cb_t *nr)
{
#if BT_TRUMPET_NR_EN
    nr->nr_cfg_en |= NR_CFG_TRUMPET_EN;
    trumpet_denoise_init(BT_TRUMPET_NR_LEVEL);
#endif

#if BT_SCO_FADE_EN
    nr->nr_cfg_en |= NR_CFG_SCO_FADE_EN;
#endif

    bt_sco_near_nr_init(sysclk, nr);

    if (!xcfg_cb.bt_nr_en) {
        nr->nr_type = ((nr->nr_type & ~NR_CFG_TYPE_MASK) | NR_TYPE_NONE);
    }
}

void bt_sco_nr_exit(void)
{
    bt_sco_near_nr_exit();
}

#if BT_SNDP_DMIC_EN
void bt_sco_sndp_dm_init(u8 *sysclk, nr_cb_t *nr)
{
}
#endif

#if BT_SNDP_SMIC_EN
void bt_sco_sndp_sm_init(u8 *sysclk, nr_cb_t *nr)
{
    if (!xcfg_cb.bt_sndp_smic_en) {
        printf("SNDP DNN xcfg init warning!\n");
        return;
    }
    nr->nr_type             = NR_TYPE_SNDP_SM;

    memset(&sndp_sm_cb, 0, sizeof(sndp_sm_cb_t));
    sndp_sm_cb.param_printf = 1;
    sndp_sm_cb.level        = xcfg_cb.bt_sndp_level;
    sndp_sm_cb.dnn_level    = xcfg_cb.bt_sndp_dnn_level;
#ifdef RES_BUF_SNDP_BLOCK_BIN
    if (RES_LEN_SNDP_BLOCK_BIN > 0) {
        sndp_dm_cb.coef = (const u32 *)RES_BUF_SNDP_BLOCK_BIN;
    }
#endif

    bt_sndp_sm_init(&sndp_sm_cb);

    *sysclk = *sysclk < SYS_48M ? SYS_48M : *sysclk;
}
#endif

#if BT_SNDP_FBDM_EN
void bt_sco_sndp_fbdm_init(u8 *sysclk, nr_cb_t *nr)
{
    if (!xcfg_cb.bt_sndp_fbdm_en) {
        printf("SNDP FBDM xcfg init warning!\n");
        return;
    }
    nr->nr_type             = NR_TYPE_SNDP_FBDM;
    memset(&sndp_fbdm_cb, 0, sizeof(sndp_fbdm_cb_t));
    sndp_fbdm_cb.param_printf   = 1;
    sndp_fbdm_cb.level          = xcfg_cb.sndp_fbdm_level;
    sndp_fbdm_cb.dnn_level      = xcfg_cb.sndp_fbdm_dnn_level;
#ifdef RES_BUF_SNDP_BLOCK_BIN
    if (RES_LEN_SNDP_BLOCK_BIN > 0) {
        sndp_fbdm_cb.coef = (const u32 *)RES_BUF_SNDP_BLOCK_BIN;
    }
#endif
    bt_sndp_fbdm_init(&sndp_fbdm_cb);

    *sysclk = *sysclk < SYS_80M ? SYS_80M : *sysclk;
}
#endif

#if BT_SNDP_DM_EN
void bt_sco_sndp_dm_init(u8 *sysclk, nr_cb_t *nr)
{
    if (!xcfg_cb.bt_sndp_dm_en) {
        printf("SNDP DM xcfg init warning!\n");
        return;
    }
    nr->nr_type             = NR_TYPE_SNDP_DM;

    memset(&sndp_dm_cb, 0, sizeof(sndp_dm_cb_t));
    sndp_dm_cb.param_printf = 1;
    sndp_dm_cb.level        = xcfg_cb.sndp_dm_level;
    sndp_dm_cb.dnn_level    = xcfg_cb.sndp_dm_dnn_level;
    sndp_dm_cb.distance     = xcfg_cb.sndp_dm_distance;
    sndp_dm_cb.degree       = xcfg_cb.sndp_dm_degree;
    sndp_dm_cb.degree1      = xcfg_cb.sndp_dm_degree1;
    sndp_dm_cb.bf_upper     = 128;

#ifdef RES_BUF_SNDP_BLOCK_BIN
    if (RES_LEN_SNDP_BLOCK_BIN > 0) {
        sndp_dm_cb.coef = (const u32 *)RES_BUF_SNDP_BLOCK_BIN;
    }
#endif
    bt_sndp_dm_init(&sndp_dm_cb);

    *sysclk = *sysclk < SYS_100M ? SYS_100M : *sysclk;
}
#endif

#if BT_SCO_DMIC_AI_EN
void bt_sco_dmns_init(u8 *sysclk, nr_cb_t *nr)
{
    if (!xcfg_cb.bt_dmns_en) {  //配置上面的DMNS双麦降噪功能未打开
        printf("DMNS xcfg init warning!\n");
    }
	memset(&dmns_cb, 0, sizeof(dmns_cb_t));
	nr->nr_type             = NR_TYPE_DMIC_AI;

	dmns_cb.param_printf       		= 0;
	dmns_cb.distance           		= xcfg_cb.bt_dmns_distance;
	dmns_cb.degree             		= 0;
	dmns_cb.nt                 		= xcfg_cb.bt_dmns_level;
	dmns_cb.noise_ps_rate      		= 1;
	//dmns_cb.gamma					= 0;
	//dmns_cb.bf_type				= 1;
	dmns_cb.low_fre_ns0_range		= 36;   //range:1-36
	dmns_cb.comp_mic_fre_L     		= 0;
	dmns_cb.comp_mic_fre_H     	    = 136;
	dmns_cb.prior_opt_idx			= 10;
	dmns_cb.wind_nr_en         		= 1;
	dmns_cb.wind_range         		= 0;
	dmns_cb.wind_level         		= 0;
	dmns_cb.tf_en					= 1;
	dmns_cb.tf_len					= 136;  //range:1-256
	dmns_cb.tf_norm_en				= 1;
	dmns_cb.tf_norm_only_en			= 0;
	dmns_cb.opt_limit				= 32666;
	dmns_cb.bf_choose				= 1;
	dmns_cb.wind_sig_choose			= 1;//0时代表副mic受风小  1代表主mic受风小
	dmns_cb.exp_range_H				= 0;
	dmns_cb.exp_range_L				= 0;
	dmns_cb.music_lev				= 6;
	dmns_cb.gain_expand				= 1024;
	dmns_cb.nn_only					= 0;
	dmns_cb.nn_only_len				= 36;

    bt_dmns_init(&dmns_cb);

    *sysclk = *sysclk < SYS_60M ? SYS_60M : *sysclk;
}
#endif

void bt_sco_near_nr_dft_init(u8 *sysclk, nr_cb_t *nr)
{
    nr->nr_type     = NR_TYPE_NONE;
    *sysclk = *sysclk < SYS_48M ? SYS_48M : *sysclk;

#if BT_SCO_NR_EN
    nr->nr_type     = NR_TYPE_AINS;
    memset(&ains_cb, 0, sizeof(ains_cb_t));
    ains_cb.nt              = BT_SCO_NR_LEVEL;
    ains_cb.prior_opt_idx   = 10;
    ains_cb.ns_ps_rate		= 1;
    ains_cb.trumpet_en		= 0;
	ains_cb.nt_post         = 0; //0-6 >0才起效 0为不开gain指数化 开的话默认为3
	ains_cb.exp_range	    = 90;
	bt_ains_init(&ains_cb);
#endif
}

#if BT_SCO_SMIC_AI_EN
void bt_sco_dnn_init(u8 *sysclk, nr_cb_t *nr)
{
    if (!xcfg_cb.bt_dnn_en) {
        printf("DNN xcfg init warning!\n");
        return;
    }
    nr->nr_type = NR_TYPE_SMIC_AI;
    memset(&dnn_cb, 0, sizeof(dnn_cb_t));

	dnn_cb.param_printf           	= 0;
	dnn_cb.nt                     	= BT_SCO_SMIC_AI_LEVEL;
	dnn_cb.nt_post             		= 0; //0-6 >0才起效 0为不开gain指数化 开的话默认为3
	dnn_cb.exp_range_H			   	= 1;
	dnn_cb.exp_range_L			   	= 0;
	dnn_cb.noise_ps_rate          	= 1;
	dnn_cb.prior_opt_idx	       	= 3;
	dnn_cb.wind_level			   	= 1;
	dnn_cb.wind_range			   	= 0;
	dnn_cb.low_fre_range          	= 40;
//	dnn_cb.low_fre_range0         	= 1;
//	dnn_cb.pitch_filter_en		   	= 1;
	dnn_cb.mask_floor			   	= 1600;
	dnn_cb.mask_floor_r		   		= 0;
	dnn_cb.music_lev			   	= 6;
	dnn_cb.comforN_level		   	= 1;
	dnn_cb.gain_expand			   	= 1024;
	dnn_cb.nn_only				   	= 1;
	dnn_cb.nn_only_len			   	= 40;

	bt_dnn_init(&dnn_cb);
    *sysclk = *sysclk < SYS_48M ? SYS_48M : *sysclk;
}
#endif

#if BT_SCO_NR_USER_SMIC_EN || BT_SCO_NR_USER_DMIC_EN
AT(.bt_voice.sco_nr.user)
void bt_nr_user_proc_do(s16 *mic1, s16 *mic2, nr_cb_t *nr_cb)
{
    //define your nr proc func here, and define nr_input by yourself

    //if you need aec ref signal, you can refer to ↓↓↓
//    if (!bt_sco_far_cache_read((u8*)&nr_input[nr_cb->nr_samples*2], nr_cb->nr_samples*2)) {
//        memset(&nr_input[nr_samples*2], 0, nr_samples * 2);
//    }
}

AT(.bt_voice.sco_nr.user)
void bt_nr_user_init(void *alg_cb, nr_cb_t *nr_cb)
{
    //define your nr init func here
    printf("nr user\n");
}

void bt_sco_nr_user_init(u8 *sysclk, nr_cb_t *nr)
{
    nr->nr_type = NR_TYPE_USER;

    *sysclk = *sysclk < SYS_60M ? SYS_60M : *sysclk;
}
#endif
