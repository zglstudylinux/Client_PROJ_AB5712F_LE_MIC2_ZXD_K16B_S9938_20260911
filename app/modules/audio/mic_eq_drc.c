/*
 * 文件名称: mic_eq_drc.c
 * 功能描述: EQ_DRC主要用来处理音效,主频160M，120个点,EQ和DRC算法时间分别为250us和180us左右;
 *           MIX_DRC主要用来一拖二音频混合,主频160M，120个点,算法时间为180us左右;
 ****************************************************************************************
 */
#include "include.h"
#include "mic_eq_drc.h"


#if ADAPTER_MIC_EQ_DRC_EN || WIRELESS_MIC_EQ_DRC_EN

#define NBYTE       2

u16 mic_eq_buff[WIRELESS_MIC_SAMPLES_SELECT] AT(.eq_buf);

static mic_eq_drc_cfg_t wireless_mic_eq_drc_cfg;

//------------------------------------------------------------------------------------------
//EQ_DRC处理

AT(.com_text.mic_eq_drc.input)
void mic_eq_drc_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    //硬件EQ_DRC
    if(wireless_mic_eq_drc_cfg.eq_drc_en && samples != 0) {
        mic_eq_drc_proc((u16 *)ptr, mic_eq_buff, samples);
        memcpy(ptr, (u8 *)mic_eq_buff, samples*NBYTE);
    }

    //输出到下一级
    if (wireless_mic_eq_drc_cfg.callback) {
        wireless_mic_eq_drc_cfg.callback(ptr, samples, ch_mode, params);
    }
}

AT(.text.mic_eq_drc.set)
void mic_eq_drc_audio_output_callback_set(audio_callback_t callback)
{
    wireless_mic_eq_drc_cfg.callback = callback;
}

AT(.text.mic_eq_drc.init)
void mic_eq_drc_init(u8 sample_rate, u16 samples, u8 channel)
{

    memset(&wireless_mic_eq_drc_cfg, 0, sizeof(wireless_mic_eq_drc_cfg));

    mic_eq_drc_cfg(0);  //16BIT

    u32 eq_len, drc_len;
    u32 eq_addr, drc_addr;

    ///device和adapter使用不同的资源文件
//    if (wireless_role_is_adapter()) {
//        eq_addr  = (u32)effect_res_addr_get(EFFECT_IDX_LOC_MIC_EQ);
//        eq_len   = effect_res_len_get(EFFECT_IDX_LOC_MIC_EQ);
//
//        drc_addr = (u32)effect_res_addr_get(EFFECT_IDX_LOC_MIC_DRC);
//        drc_len  = effect_res_len_get(EFFECT_IDX_LOC_MIC_DRC);
//    } else {
        eq_addr  = (u32)effect_res_addr_get(EFFECT_IDX_LOC_MIC_EQ);
        eq_len   = effect_res_len_get(EFFECT_IDX_LOC_MIC_EQ);

        drc_addr = (u32)effect_res_addr_get(EFFECT_IDX_LOC_MIC_DRC);
        drc_len  = effect_res_len_get(EFFECT_IDX_LOC_MIC_DRC);
//    }

    if (mic_eq_set_by_res(eq_addr, eq_len) && mic_drc_set_by_res(drc_addr, drc_len)) {
        wireless_mic_eq_drc_cfg.eq_drc_en = 1;
    }
}

AT(.text.mic_eq_drc.exit)
void mic_eq_drc_exit(u8 sample_rate, u16 samples)
{

}
#endif

#if ADAPTER_MIX_DRC_EN
static drc_v3_cb_t mix_drc_cb;

//------------------------------------------------------------------------------------------
//MIX_DRC处理
//若output = NULL, MIX_DRC处理输出到pcm0
AT(.com_text.mix_drc.input)
void mix_drc_audio_input(s16 *pcm0, s16 *pcm1, s16 *output, u16 samples)
{
    s32 val = 0;
    s16 *out = (output == NULL)? pcm0 : output;

    for(uint i=0; i<samples; i++) {

        #if ADAPTER_SOFT_EQ_EN
            val = soft_eq_mic_audio_input2(pcm0[i] + pcm1[i]);
        #else
            val = pcm0[i] + pcm1[i];
        #endif

        out[i] = drc_v3_calc(val, &mix_drc_cb, 0);

        #if ADAPTER_SOFT_GAIN_AFTER_MIX_EN
            val  = __builtin_muls_shift15(out[i] , SOFT_GAIN_P0_0DB);

            out[i] =(s16)s_clip16( val);
        #endif
    }
}

AT(.text.mix_drc.init)
void mix_drc_init(void)
{
    memset(&mix_drc_cb, 0, sizeof(mix_drc_cb));
    u32 res_addr = (u32)effect_res_addr_get(EFFECT_IDX_MIX_DRC);
    u32 res_len  = effect_res_len_get(EFFECT_IDX_MIX_DRC);
    drc_v3_init_toolkit((const void *)res_addr, res_len, &mix_drc_cb);
}

AT(.text.mix_drc)
void mix_drc_set_param(u32 res_addr, u32 res_len)
{
    drc_v3_init_toolkit((const void *)res_addr, res_len, &mix_drc_cb);
}

AT(.text.mix_drc.exit)
void mix_drc_exit(void)
{

}
#endif

#if MIC_DRC_EN
static const u8 soft_gain_range[16+1] = {63,20,18,16,14,12,10,9,8,7,6,5,4,3,2,1,0};

AT(.rodata.sdadc.gain_tbl)
const u32 sdadc_soft_gain_tbl_64[64] = {
    //负增益
    SOFT_GAIN_N0DB,SOFT_GAIN_N1DB,SOFT_GAIN_N2DB,SOFT_GAIN_N3DB,SOFT_GAIN_N4DB,
    SOFT_GAIN_N5DB,SOFT_GAIN_N6DB,SOFT_GAIN_N7DB,SOFT_GAIN_N8DB,SOFT_GAIN_N9DB,
    SOFT_GAIN_N10DB,SOFT_GAIN_N11DB,SOFT_GAIN_N12DB,SOFT_GAIN_N13DB,SOFT_GAIN_N14DB,
    SOFT_GAIN_N15DB,SOFT_GAIN_N16DB,SOFT_GAIN_N17DB,SOFT_GAIN_N18DB,SOFT_GAIN_N19DB,
    SOFT_GAIN_N20DB,SOFT_GAIN_N21DB,SOFT_GAIN_N22DB,SOFT_GAIN_N23DB,SOFT_GAIN_N24DB,
    SOFT_GAIN_N25DB,SOFT_GAIN_N26DB,SOFT_GAIN_N27DB,SOFT_GAIN_N28DB,SOFT_GAIN_N29DB,
    SOFT_GAIN_N30DB,SOFT_GAIN_N31DB,SOFT_GAIN_N32DB,SOFT_GAIN_N33DB,SOFT_GAIN_N34DB,
    SOFT_GAIN_N35DB,SOFT_GAIN_N36DB,SOFT_GAIN_N37DB,SOFT_GAIN_N38DB,SOFT_GAIN_N39DB,
    SOFT_GAIN_N40DB,SOFT_GAIN_N41DB,SOFT_GAIN_N42DB,SOFT_GAIN_N43DB,SOFT_GAIN_N44DB,
    SOFT_GAIN_N45DB,SOFT_GAIN_N46DB,SOFT_GAIN_N47DB,SOFT_GAIN_N48DB,SOFT_GAIN_N49DB,
    SOFT_GAIN_N50DB,SOFT_GAIN_N51DB,SOFT_GAIN_N52DB,SOFT_GAIN_N53DB,SOFT_GAIN_N54DB,
    SOFT_GAIN_N55DB,SOFT_GAIN_N56DB,SOFT_GAIN_N57DB,SOFT_GAIN_N58DB,SOFT_GAIN_N59DB,
    SOFT_GAIN_N60DB,SOFT_GAIN_N65DB,SOFT_GAIN_N110DB,SOFT_GAIN_N110DB,
};

//软件数字增益
AT(.rodata.sdadc.gain_tbl)
const u32 sdadc_soft_gain_tbl_128[128] = {
    SOFT_GAIN_P0_0DB, SOFT_GAIN_P0_5DB, SOFT_GAIN_P1_0DB, SOFT_GAIN_P1_5DB, SOFT_GAIN_P2_0DB,
    SOFT_GAIN_P2_5DB, SOFT_GAIN_P3_0DB, SOFT_GAIN_P3_5DB, SOFT_GAIN_P4_0DB, SOFT_GAIN_P4_5DB,
    SOFT_GAIN_P5_0DB, SOFT_GAIN_P5_5DB, SOFT_GAIN_P6_0DB, SOFT_GAIN_P6_5DB, SOFT_GAIN_P7_0DB,
    SOFT_GAIN_P7_5DB, SOFT_GAIN_P8_0DB, SOFT_GAIN_P8_5DB, SOFT_GAIN_P9_0DB, SOFT_GAIN_P9_5DB,
    SOFT_GAIN_P10_0DB, SOFT_GAIN_P10_5DB, SOFT_GAIN_P11_0DB, SOFT_GAIN_P11_5DB, SOFT_GAIN_P12_0DB,
    SOFT_GAIN_P12_5DB, SOFT_GAIN_P13_0DB, SOFT_GAIN_P13_5DB, SOFT_GAIN_P14_0DB, SOFT_GAIN_P14_5DB,
    SOFT_GAIN_P15_0DB, SOFT_GAIN_P15_5DB, SOFT_GAIN_P16_0DB, SOFT_GAIN_P16_5DB, SOFT_GAIN_P17_0DB,
    SOFT_GAIN_P17_5DB, SOFT_GAIN_P18_0DB, SOFT_GAIN_P18_5DB, SOFT_GAIN_P19_0DB, SOFT_GAIN_P19_5DB,
    SOFT_GAIN_P20_0DB, SOFT_GAIN_P20_5DB, SOFT_GAIN_P21_0DB, SOFT_GAIN_P21_5DB, SOFT_GAIN_P22_0DB,
    SOFT_GAIN_P22_5DB, SOFT_GAIN_P23_0DB, SOFT_GAIN_P23_5DB, SOFT_GAIN_P24_0DB, SOFT_GAIN_P24_5DB,
    SOFT_GAIN_P25_0DB, SOFT_GAIN_P25_5DB, SOFT_GAIN_P26_0DB, SOFT_GAIN_P26_5DB, SOFT_GAIN_P27_0DB,
    SOFT_GAIN_P27_5DB, SOFT_GAIN_P28_0DB, SOFT_GAIN_P28_5DB, SOFT_GAIN_P29_0DB, SOFT_GAIN_P29_5DB,
    SOFT_GAIN_P30_0DB, SOFT_GAIN_P30_5DB, SOFT_GAIN_P31_0DB, SOFT_GAIN_P31_5DB,
    //负增益
    SOFT_GAIN_N0DB,SOFT_GAIN_N1DB,SOFT_GAIN_N2DB,SOFT_GAIN_N3DB,SOFT_GAIN_N4DB,
    SOFT_GAIN_N5DB,SOFT_GAIN_N6DB,SOFT_GAIN_N7DB,SOFT_GAIN_N8DB,SOFT_GAIN_N9DB,
    SOFT_GAIN_N10DB,SOFT_GAIN_N11DB,SOFT_GAIN_N12DB,SOFT_GAIN_N13DB,SOFT_GAIN_N14DB,
    SOFT_GAIN_N15DB,SOFT_GAIN_N16DB,SOFT_GAIN_N17DB,SOFT_GAIN_N18DB,SOFT_GAIN_N19DB,
    SOFT_GAIN_N20DB,SOFT_GAIN_N21DB,SOFT_GAIN_N22DB,SOFT_GAIN_N23DB,SOFT_GAIN_N24DB,
    SOFT_GAIN_N25DB,SOFT_GAIN_N26DB,SOFT_GAIN_N27DB,SOFT_GAIN_N28DB,SOFT_GAIN_N29DB,
    SOFT_GAIN_N30DB,SOFT_GAIN_N31DB,SOFT_GAIN_N32DB,SOFT_GAIN_N33DB,SOFT_GAIN_N34DB,
    SOFT_GAIN_N35DB,SOFT_GAIN_N36DB,SOFT_GAIN_N37DB,SOFT_GAIN_N38DB,SOFT_GAIN_N39DB,
    SOFT_GAIN_N40DB,SOFT_GAIN_N41DB,SOFT_GAIN_N42DB,SOFT_GAIN_N43DB,SOFT_GAIN_N44DB,
    SOFT_GAIN_N45DB,SOFT_GAIN_N46DB,SOFT_GAIN_N47DB,SOFT_GAIN_N48DB,SOFT_GAIN_N49DB,
    SOFT_GAIN_N50DB,SOFT_GAIN_N51DB,SOFT_GAIN_N52DB,SOFT_GAIN_N53DB,SOFT_GAIN_N54DB,
    SOFT_GAIN_N55DB,SOFT_GAIN_N56DB,SOFT_GAIN_N57DB,SOFT_GAIN_N58DB,SOFT_GAIN_N59DB,
    SOFT_GAIN_N60DB,SOFT_GAIN_N65DB,SOFT_GAIN_N110DB,SOFT_GAIN_N110DB,
};

typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *params);

typedef struct {
    u8 mute;
    u8 sample_rate;
    u16 samples;
    audio_callback_t callback;
} drc_mic_cfg_t;

static drc_mic_cfg_t drc_mic_cfg;
WEAK uint8_t drc_mic_en = 1;
WEAK uint8_t drc_post_gain_level = SOFT_GAIN_DEFAULT_LEVEL - 1;
drc_v3_cb_t soft_drc_cb;

#define UARTDUMP_SOFT_DRC_EN                    0
#if UARTDUMP_SOFT_DRC_EN
static u8 dump_head_buf[14*2]AT(.buf.howling_dnn);

static u8 UartDump_buf[2][14 + 120*2]AT(.buf.howling_dnn);
static u8 UartDump_buf_temp[14 + 120*2]AT(.buf.howling_dnn);
#endif

AT(.text.sdadc.gain)
void soft_gain_up(void)
{
    if (sys_cb.soft_gain_level < (SOFT_GAIN_MAX_LEVEL - 1)) {
        sys_cb.soft_gain_level += 1;
    }else if (sys_cb.soft_gain_level > SOFT_GAIN_MAX_LEVEL - 1){
        sys_cb.soft_gain_level = SOFT_GAIN_MAX_LEVEL - 1;
    }
    printf("sys_cb.soft_gain_level = %d soft_gain_range[sys_cb.soft_gain_level] = %d\n",sys_cb.soft_gain_level,soft_gain_range[sys_cb.soft_gain_level]);
    wireless_mic_set_soft_gain(soft_gain_range[sys_cb.soft_gain_level]);
}

AT(.text.sdadc.gain)
void soft_gain_down(void)
{
    if (sys_cb.soft_gain_level) {
        sys_cb.soft_gain_level -= 1;
    }
    printf("sys_cb.soft_gain_level = %d soft_gain_range[sys_cb.soft_gain_level] = %d\n",sys_cb.soft_gain_level,soft_gain_range[sys_cb.soft_gain_level]);
    wireless_mic_set_soft_gain(soft_gain_range[sys_cb.soft_gain_level]);
}

AT(.text.sdadc.gain)
bool soft_gain_level_is_max_min(void)
{
    if (sys_cb.soft_gain_level == (SOFT_GAIN_MAX_LEVEL - 1) || sys_cb.soft_gain_level == 0) {
        return true;
    }
    return false;
}

AT(.text.sdadc.gain)
void wireless_mic_set_soft_gain(u16 gain_r)
{
    drc_post_gain_level = gain_r;
}

AT(.com_text.mic_drc)
void drc_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    s16 * rptr = (s16 *)ptr;
#if UARTDUMP_SOFT_DRC_EN
    s16 *ptr_temp = (s16 *) UartDump_buf_temp;
    dump_dma_wait();
    dump_put2ram(&UartDump_buf[0][0],ptr,120*2,0); //算法前数据组包

    dump_put2ram(&UartDump_buf[1][0],ptr_temp,120*2,1);//算法后数据组包
    dump_dma_kick(&UartDump_buf[0][0], (120*2 + 14) * 2); //最后再一起吐数据，避免前后kick数据打架
#endif
#if SOFT_DRC_DUMP_EN
    audio_dump_input(rptr, samples, 0, 0);
#endif
    if(drc_mic_en) {
        //32bit 拓展
        s32 p_left;
        for(u16 i = 0; i < samples; ++i) {
            p_left = rptr[i];
            p_left = __builtin_muls_shift15(p_left, sdadc_soft_gain_tbl_64[drc_post_gain_level]);
            //ADC DRC
//            if(p_left > 32767) {
//                p_left = 32767;
//            } else if(p_left < -32768) {
//                p_left = -32768;
//            }
//                rptr[i] = (s16)p_left;
             rptr[i]  = drc_v3_calc(p_left, &soft_drc_cb, 0);

        }
    }
#if SOFT_DRC_DUMP_EN
    audio_dump_input(rptr, samples, 1, 0);
#endif
#if UARTDUMP_SOFT_DRC_EN
    for(int i = 0; i < 120; i++) {
        ptr_temp[i]  = rptr[i];
    }
#endif // UARTDUMP_SOFT_DRC_EN
    if (drc_mic_cfg.callback) {
        drc_mic_cfg.callback((u8 *)rptr, samples, ch_mode, params);
    }
}

AT(.text.mic_drc)
void drc_mic_audio_output_callback_set(audio_callback_t callback)
{
    drc_mic_cfg.callback = callback;
}

//extern drc_v3_cb_t soft_drc_cb;
AT(.text.soft_drc.init)
void drc_mic_init(u8 sample_rate, u16 samples, u8 channel)
{
    printf("drc_mic_init\n");
    memset(&soft_drc_cb, 0, sizeof(soft_drc_cb));

    u32 res_addr = (u32)effect_res_addr_get(EFFECT_IDX_LOC_MIC_DRC);
    u32 res_len  = effect_res_len_get(EFFECT_IDX_LOC_MIC_DRC);
    drc_v3_init_toolkit((const void *)res_addr, res_len, &soft_drc_cb);

#if UARTDUMP_SOFT_DRC_EN
    memset(dump_head_buf, 0, 14*2);
    memset(UartDump_buf, 0, 2*(14 + 120*2));

    huart_module_init();    //初始化高速串口模块
    dump_buf_init(dump_head_buf,2, huart_putbuf, huart_wait_tx_finish);
#endif

#if SOFT_DRC_DUMP_EN
    audio_dump_init();
#endif
}

AT(.text.soft_drc)
void drc_mic_drc_set_param(u32 res_addr, u32 res_len)
{
    drc_v3_init_toolkit((const void *)res_addr, res_len, &soft_drc_cb);
}
#endif
