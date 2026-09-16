#include "include.h"

#if SYS_KARAOK_EN

karaok_vol_t karaok_vol;

#if SYS_ECHO_EN
static s16 echo_cache_buf[0x2800] AT(.echo_buf);
#if BT_HFP_CALL_KARAOK_EN
static s16 bt_echo_cache_buf[0x2400] AT(.sco_echo_buf);
#endif
#endif // SYS_ECHO_EN

#if SYS_MAGIC_VOICE_EN
static s16 buf_1[2048] AT(.mav_cache);
static s16 buf_2[128] AT(.mav_cache);
static s16 buf_3[128] AT(.mav_cache);
static s16 buf_4[512] AT(.mav_cache);
static s16 buf_5[64] AT(.mav_cache);
static s16 buf_6[64] AT(.mav_cache);
static char buf_7[512] AT(.mav_cache);

#if BT_HFP_CALL_KARAOK_EN
static s16 buf_11[2048] AT(.sco_mav_cache);
static s16 buf_21[128] AT(.sco_mav_cache);
static s16 buf_31[128] AT(.sco_mav_cache);
static s16 buf_41[512] AT(.sco_mav_cache);
static s16 buf_51[64] AT(.sco_mav_cache);
static s16 buf_61[64] AT(.sco_mav_cache);
static char buf_71[512] AT(.sco_mav_cache);
#endif

#endif

AT(.text.bsp.dac.table)
const u16 karaok_dvol_table_12[12 + 1] = {
    DIG_N60DB,  DIG_N43DB,  DIG_N32DB,  DIG_N26DB,  DIG_N22DB,  DIG_N18DB,  DIG_N14DB,  DIG_N12DB,  DIG_N10DB,
    DIG_N6DB,   DIG_N4DB,   DIG_N2DB,   DIG_N0DB,
};

AT(.text.bsp.dac.table)
const u16 karaok_dvol_table_16[16 + 1] = {
    DIG_N60DB,  DIG_N43DB,  DIG_N32DB,  DIG_N26DB,  DIG_N24DB,  DIG_N22DB,  DIG_N20DB,  DIG_N18DB, DIG_N16DB,
    DIG_N14DB,  DIG_N12DB,  DIG_N10DB,  DIG_N8DB,   DIG_N6DB,   DIG_N4DB,   DIG_N2DB,   DIG_N0DB,
};

//低通滤波器参数， 3KHz截至频率
const s32 echo_filter_coef[2][8] = {
    {   //44.1KHz 3K
        //分子
        FIX_BIT(0.006644395788557),
        FIX_BIT(0.019933187365671),
        FIX_BIT(0.019933187365671),
        FIX_BIT(0.006644395788557),
        //分母
        FIX_BIT(1.000000000000000),
        FIX_BIT(-2.150944711976045),
        FIX_BIT(1.626545208636282),
        FIX_BIT(-0.422445330351779),
    }, {//48KHz
        //分子
        FIX_BIT(0.005300409794526),
        FIX_BIT(0.015901229383577),
        FIX_BIT(0.015901229383577),
        FIX_BIT(0.005300409794526),

        //分母
        FIX_BIT(1.000000000000000),
        FIX_BIT(-2.219168618311665),
        FIX_BIT(1.715117830033402),
        FIX_BIT(-0.453545933365530),
    }
};

////低通滤波器参数， 4KHz截至频率
//const s32 echo_filter_coef[2][8] = {
//    {   //44.1KHz 4K
//        //分子
//        FIX_BIT(0.014099708769044),
//        FIX_BIT(0.042299126307133),
//        FIX_BIT(0.042299126307133),
//        FIX_BIT(0.014099708769044),
//        //分母
//        FIX_BIT(1.000000000000000),
//        FIX_BIT(-1.873027248422390),
//        FIX_BIT(1.300326954651049),
//        FIX_BIT(-0.314502036076304),
//    }, {//48KHz
//        //分子
//        FIX_BIT(0.011324865405187),
//        FIX_BIT(0.033974596215561),
//        FIX_BIT(0.033974596215561),
//        FIX_BIT(0.011324865405187),
//        //分母
//        FIX_BIT(1.000000000000000),
//        FIX_BIT(-1.962990915244728),
//        FIX_BIT(1.400000000000000),
//        FIX_BIT(-0.346410161513776),
//    }
//};

#if SYS_ECHO_EN
//混响间隔：(echo_delay[0, 87] * 128 / 44100)ms
const u16 echo_delay_12[12 + 1] = {
    0,
    30,       35,        38,         40,
    45,       48,        55,         60,
    75,       80,        85,         87,
};

//混响间隔：(echo_delay[0, 87] * 128 / 44100)ms
const u16 echo_delay_16[16 + 1] = {
     0,
     3,          5,        10,        15,
    20,         25,        40,        48,
    55,         60,        65,        70,
    74,         78,        83,        87,
};
#endif

#if SYS_MAGIC_VOICE_EN
mav_cfg_t mav_cfg = {
    .res[0]         = 0,
    .res[1]         = 0,
    .buf_1          = buf_1,
    .buf_2          = buf_2,
    .buf_3          = buf_3,
    .buf_4          = buf_4,
    .buf_5          = buf_5,
    .buf_6          = buf_6,
    .buf_7          = buf_7,
};
#endif

kara_cfg_t kara_cfg = {
    .lp_coef        = &echo_filter_coef,
#if SYS_MAGIC_VOICE_EN
    .mav            = &mav_cfg,
#endif
    .res[0]         = 0,
    .res[1]         = 0,
    .echo_en        = SYS_ECHO_EN,
    .hifi4_echo_en  = HIFI4_ECHO_EN,
    .reverb_en      = HIFI4_REVERB_EN,
    .mic_dnr        = MIC_DNR_EN,
    .rec_en         = KARAOK_REC_EN,
    .magic_voice    = SYS_MAGIC_VOICE_EN,
    .howling_en     = SYS_HOWLING_EN,
    .hifi_howling_en = HIFI4_HOWLING_EN,
    .pitch_shift    = HIFI4_PITCH_SHIFT_EN,
    .bass_treble    = HIFI4_BASS_TREBLE_EN,
    .drc_en         = HIFI4_MIC_DRC_EN,
};

#if SYS_ECHO_EN
AT(.text.bsp.karaok)
void bsp_echo_set_level(void)
{
    echo_set_level(karaok_vol.echo_level_gain[sys_cb.echo_level]);
}

AT(.text.bsp.karaok)
void bsp_echo_set_delay(void)
{
    echo_set_delay(karaok_vol.echo_delay[sys_cb.echo_delay]);
}
#endif

AT(.text.bsp.karaok)
void bsp_karaok_set_mic_volume(void)
{
    karaok_set_mic_volume(karaok_vol.micvol_table[sys_cb.mic_vol]);
}

AT(.text.bsp.karaok)
void bsp_karaok_set_music_volume(void)
{
    karaok_set_music_volume(karaok_vol.dvol_table[sys_cb.music_vol]);
}

void bsp_karaok_music_vol_adjust(u8 up)
{
    if (up) {
        if (sys_cb.music_vol <  KNOB_KEY_LEVEL -1) {
            sys_cb.music_vol++;
        } else {
            sys_cb.music_vol = KNOB_KEY_LEVEL -1;
        }
    } else {
        if (sys_cb.music_vol > 0) {
            sys_cb.music_vol--;
        }
    }
    printf("karaok music_vol = %d\n", sys_cb.music_vol);
    bsp_karaok_set_music_volume();
}

void bsp_karaok_mic_vol_adjust(u8 up)
{
    if (up) {
        if (sys_cb.mic_vol <  KNOB_KEY_LEVEL -1) {
            sys_cb.mic_vol++;
        } else {
            sys_cb.mic_vol = KNOB_KEY_LEVEL -1;
        }
    } else {
        if (sys_cb.mic_vol > 0) {
            sys_cb.mic_vol--;
        }
    }
    printf("karaok mic_vol = %d\n", sys_cb.mic_vol);
    bsp_karaok_set_mic_volume();
}

AT(.text.bsp.karaok)
void bsp_karaok_echo_reset_buf(u8 func)
{
    if (func == FUNC_NULL) {
        return;
    }
#if SYS_ECHO_EN
    if (func == FUNC_BT || func == FUNC_BTHID || BT_BACKSTAGE_EN) {
#if BT_HFP_CALL_KARAOK_EN
        if (sys_cb.hfp_karaok_en) {
            if (kara_cfg.echo_cache == bt_echo_cache_buf) {
                return;
            }
            kara_cfg.echo_cache = bt_echo_cache_buf;
            kara_cfg.echo_cache_size = sizeof(bt_echo_cache_buf) / 2;
        } else
#endif
        {
            if (kara_cfg.echo_cache == echo_cache_buf) {
                return;
            }
            kara_cfg.echo_cache = echo_cache_buf;
            kara_cfg.echo_cache_size = sizeof(echo_cache_buf) / 2;
        }
    } else {
        if (kara_cfg.echo_cache == echo_cache_buf) {
            return;
        }
        kara_cfg.echo_cache = echo_cache_buf;
        kara_cfg.echo_cache_size = sizeof(echo_cache_buf) / 2;
    }
#endif
    karaok_reset_buf(&kara_cfg);
#if SYS_ECHO_EN
    echo_set_delay(karaok_vol.echo_delay[sys_cb.echo_delay]);
    echo_set_level(karaok_vol.echo_level_gain[sys_cb.echo_level]);
#endif
}

AT(.text.bsp.karaok)
void bsp_karaok_init(u8 path, u8 func)
{
//    printf("%s %d\n", __func__, func);
    if (sys_cb.karaok_init) {
        return;
    }
    if (func == FUNC_NULL) {
        return;
    }
#if SYS_ECHO_EN
    kara_cfg.echo_cache = echo_cache_buf;
    kara_cfg.echo_cache_size = sizeof(echo_cache_buf) / 2;
    kara_cfg.echo_cache1 = NULL;
    kara_cfg.echo_cache_size1 = 0;
#endif

    if (func == FUNC_BT || func == FUNC_BTHID) {
#if SYS_MAGIC_VOICE_EN
    #if BT_HFP_CALL_KARAOK_EN
        if (sys_cb.hfp_karaok_en) {
            mav_cfg.buf_1 = buf_11;
            mav_cfg.buf_2 = buf_21;
            mav_cfg.buf_3 = buf_31;
            mav_cfg.buf_4 = buf_41;
            mav_cfg.buf_5 = buf_51;
            mav_cfg.buf_6 = buf_61;
            mav_cfg.buf_7 = buf_71;
        } else
    #endif
        {
            mav_cfg.buf_1 = buf_1;
            mav_cfg.buf_2 = buf_2;
            mav_cfg.buf_3 = buf_3;
            mav_cfg.buf_4 = buf_4;
            mav_cfg.buf_5 = buf_5;
            mav_cfg.buf_6 = buf_6;
            mav_cfg.buf_7 = buf_7;
        }
#endif

#if BT_HFP_CALL_KARAOK_EN && SYS_ECHO_EN
        if (sys_cb.hfp_karaok_en) {
            kara_cfg.echo_cache = bt_echo_cache_buf;
            kara_cfg.echo_cache_size = sizeof(bt_echo_cache_buf) / 2;
        }
#endif
    }

    if (kara_cfg.rec_en) {
        sys_clk_req(INDEX_KARAOK, SYS_160M);
    } else {
        sys_clk_req(INDEX_KARAOK, SYS_120M);
    }

    sys_cb.echo_delay = SYS_ECHO_DELAY;
#if (KNOB_KEY_LEVEL == 12)
#if SYS_ECHO_EN
    karaok_vol.echo_delay = echo_delay_12;
    karaok_vol.echo_level_gain = echo_level_gain_12;
#endif // SYS_ECHO_EN
    karaok_vol.dvol_table = karaok_dvol_table_12;
    karaok_vol.micvol_table = karaok_dvol_table_12;
#elif (KNOB_KEY_LEVEL == 16)
#if SYS_ECHO_EN
    karaok_vol.echo_delay = echo_delay_16;
    karaok_vol.echo_level_gain = echo_level_gain_16;
#endif // SYS_ECHO_EN
    karaok_vol.dvol_table = karaok_dvol_table_16;
    karaok_vol.micvol_table = karaok_dvol_table_16;
#endif
    audio_path_init(path);
    karaok_init(&kara_cfg);
    plugin_karaok_init();
    karaok_bufmode(SYS_KARAOK_BUFMODE);     //使用降采样来加倍混响深度
#if HIFI4_MIC_DRC_EN
    hifi4_drc_set_param_by_res(RES_BUF_EQ_KARA_MIC0_DRC, RES_LEN_EQ_KARA_MIC0_DRC, 0);
    hifi4_drc_set_param_by_res(RES_BUF_EQ_KARA_MIC1_DRC, RES_LEN_EQ_KARA_MIC1_DRC, 1);
#endif

#if MIC_DNR_EN
    mic_dnr_init(2, 500, 100, 1000);        //10ms检测一次 //连续超过2次大于500就认为有声，连续超过100次低于 1000 就认为无声
#endif
    //karaok_enable_mic_first(65535/10);    //(数字音量最大值为0xFFFF_65535)
    //mic_first_init(2, 700, 30, 1200);     //连续超过2次大于700 就开启mic_first，连续超过 100 次低于 1200 就关闭mic_first
    //karaok_mic_first_music_vol_fade_enable(100, 1000);   //淡入步进100, 淡出步进1000.   (数字音量最大值为0xFFFF_65535)

#if SYS_ECHO_EN
    echo_set_delay(karaok_vol.echo_delay[sys_cb.echo_delay]);
    echo_set_level(karaok_vol.echo_level_gain[sys_cb.echo_level]);
#endif
    karaok_set_mic_volume(karaok_vol.micvol_table[sys_cb.mic_vol]);
    karaok_set_music_volume(karaok_vol.dvol_table[sys_cb.music_vol]);

#if MIC_DETECT_EN
    if (!dev_is_online(DEV_MIC)) {
        karaok_mic_mute();
    }
#endif
    audio_path_start(path);

#if SYS_MAGIC_VOICE_EN
    karaok_set_mav(sys_cb.magic_type, sys_cb.magic_level);
#endif
    sys_cb.karaok_init = 1;
}

AT(.text.bsp.karaok)
void bsp_karaok_exit(u8 path)
{
    if (sys_cb.karaok_init == 0) {
        return;
    }
    karaok_exit();
    audio_path_exit(path);
    sys_clk_free(INDEX_KARAOK);

    sys_cb.karaok_init = 0;
}

#if KARAOK_REC_EN
AT(.text.bsp.karaok)
void bsp_karaok_rec_init(void)
{
    rec_src.spr = SPR_44100;
    if (DACDIGCON0 & BIT(1)) {
        rec_src.spr = SPR_48000;
    }
    rec_src.nchannel = 0x01;
    rec_src.source_start = karaok_rec_start;
    rec_src.source_stop  = karaok_rec_stop;
    karaok_rec_init(rec_cb.src->nchannel & 0x03);
}

AT(.text.bsp.karaok)
void bsp_karaok_rec_exit(void)
{
//    bsp_karaok_echo_reset_buf(func_cb.sta);
}
#endif
#endif

bool bt_sco_karaok_is_en(u8 enter)
{
#if SYS_KARAOK_EN
    if (enter) {
        bsp_karaok_exit(AUDIO_PATH_KARAOK);
        sys_cb.hfp_karaok_en = BT_HFP_CALL_KARAOK_EN;       //通话是否支持KARAOK
        plugin_hfp_karaok_configure();
        if (sys_cb.hfp_karaok_en) {
            bsp_karaok_init(AUDIO_PATH_BTMIC, FUNC_BT);
            kara_sco_start();
            return true;
        } else {
            return false;
        }
    } else {
        if (sys_cb.hfp_karaok_en) {
            kara_sco_stop();
            bsp_karaok_exit(AUDIO_PATH_BTMIC);
            sys_cb.hfp_karaok_en = 0;
            return true;
        } else {
            return false;
        }
    }
#else
    return false;
#endif
}
