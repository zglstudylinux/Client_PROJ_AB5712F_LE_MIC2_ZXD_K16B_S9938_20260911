/*
 * 文件名称: soft_gain.c
 * 功能描述: 本文件为软件增益处理模块
 ****************************************************************************************
 */
#include "include.h"
#include "soft_gain.h"

#if ADAPTER_AUDIO_FADE_IN_EN || ADAPTER_USB_SPK_TX_EN
#define SOFT_GAIN_MAX_LEVEL     64
AT(.com_rodata.soft_gain_tbl)
static const u32 soft_gain_tbl_64[SOFT_GAIN_MAX_LEVEL+1] = {
    //负增益
    SOFT_GAIN_N0DB,  SOFT_GAIN_N1DB,  SOFT_GAIN_N2DB,  SOFT_GAIN_N3DB,  SOFT_GAIN_N4DB,
    SOFT_GAIN_N5DB,  SOFT_GAIN_N6DB,  SOFT_GAIN_N7DB,  SOFT_GAIN_N8DB,  SOFT_GAIN_N9DB,
    SOFT_GAIN_N10DB, SOFT_GAIN_N11DB, SOFT_GAIN_N12DB, SOFT_GAIN_N13DB, SOFT_GAIN_N14DB,
    SOFT_GAIN_N15DB, SOFT_GAIN_N16DB, SOFT_GAIN_N17DB, SOFT_GAIN_N18DB, SOFT_GAIN_N19DB,
    SOFT_GAIN_N20DB, SOFT_GAIN_N21DB, SOFT_GAIN_N22DB, SOFT_GAIN_N23DB, SOFT_GAIN_N24DB,
    SOFT_GAIN_N25DB, SOFT_GAIN_N26DB, SOFT_GAIN_N27DB, SOFT_GAIN_N28DB, SOFT_GAIN_N29DB,
    SOFT_GAIN_N30DB, SOFT_GAIN_N31DB, SOFT_GAIN_N32DB, SOFT_GAIN_N33DB, SOFT_GAIN_N34DB,
    SOFT_GAIN_N35DB, SOFT_GAIN_N36DB, SOFT_GAIN_N37DB, SOFT_GAIN_N38DB, SOFT_GAIN_N39DB,
    SOFT_GAIN_N40DB, SOFT_GAIN_N41DB, SOFT_GAIN_N42DB, SOFT_GAIN_N43DB, SOFT_GAIN_N44DB,
    SOFT_GAIN_N45DB, SOFT_GAIN_N46DB, SOFT_GAIN_N47DB, SOFT_GAIN_N48DB, SOFT_GAIN_N49DB,
    SOFT_GAIN_N50DB, SOFT_GAIN_N51DB, SOFT_GAIN_N52DB, SOFT_GAIN_N53DB, SOFT_GAIN_N54DB,
    SOFT_GAIN_N55DB, SOFT_GAIN_N56DB, SOFT_GAIN_N57DB, SOFT_GAIN_N58DB, SOFT_GAIN_N59DB,
    SOFT_GAIN_N60DB, SOFT_GAIN_N65DB, SOFT_GAIN_N110DB, SOFT_GAIN_N110DB, SOFT_GAIN_N110DB
};
#endif

#if ADAPTER_AUDIO_FADE_IN_EN
static struct {
    u8 curr;
    u8 level;
} soft_gain;

AT(.text.soft_gain)
void soft_gain_init(void)
{
    soft_gain.curr = SOFT_GAIN_MAX_LEVEL;
    soft_gain.level = 0;
}

///软件淡入处理(主频120M，240个点，算法时间在28us左右)
AT(.com_text.soft_gain)
void soft_gain_proc(mic_pcm_t *ptr, u32 samples)
{
    if(soft_gain.level != soft_gain.curr) {
        if(soft_gain.curr > soft_gain.level) {
            soft_gain.curr--;       //步进为1
        } else if(soft_gain.curr < soft_gain.level) {
            soft_gain.curr++;
        }
    }

    if(soft_gain.curr != 0) {
        u32 gain = soft_gain_tbl_64[soft_gain.curr];

        mic_pcm_t *rptr = (mic_pcm_t *)ptr;
        mic_pcm_t temp = 0;

        for(int i = 0; i < samples; i++) {
            temp = __builtin_muls_shift15(rptr[i], gain);
            if(temp > PCM_MAX_V) {
                rptr[i] = PCM_MAX_V;
            } else if(temp < PCM_MIN_V) {
                rptr[i] = PCM_MIN_V;
            } else {
                rptr[i] = temp;
            }
        }
    }
}

AT(.text.soft_gain)
void soft_gain_set_level(uint level)
{
    soft_gain.level = (level > SOFT_GAIN_MAX_LEVEL)? SOFT_GAIN_MAX_LEVEL : level;
}
#endif

//----------------------------------------------------------------------------------
#if ADAPTER_USB_SPK_TX_EN
typedef struct {
    u8 curr;
    u8 level;
    audio_callback_t callback;
} spk_soft_gain_t;

static spk_soft_gain_t spk_gain_cb;

AT(.com_text.soft_gain.spk)
void spk_soft_gain_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    if(spk_gain_cb.level != spk_gain_cb.curr) {
        if(spk_gain_cb.curr > spk_gain_cb.level) {
            spk_gain_cb.curr--;       //步进为1
        } else if(spk_gain_cb.curr < spk_gain_cb.level) {
            spk_gain_cb.curr++;
        }
    }

    if(spk_gain_cb.curr != 0) {
        u32 gain = soft_gain_tbl_64[spk_gain_cb.curr];

        mic_pcm_t *rptr = (mic_pcm_t *)ptr;
        mic_pcm_t temp = 0;

        for(int i = 0; i < samples*ch_mode; i++) {
            temp = __builtin_muls_shift15(rptr[i], gain);
            if(temp > PCM_MAX_V) {
                rptr[i] = PCM_MAX_V;
            } else if(temp < PCM_MIN_V) {
                rptr[i] = PCM_MIN_V;
            } else {
                rptr[i] = temp;
            }
        }
    }

    if(spk_gain_cb.callback) {
        spk_gain_cb.callback(ptr, samples, ch_mode, params);
    }
}

AT(.text.soft_gain.spk)
void spk_soft_gain_audio_output_callback_set(audio_callback_t callback)
{
    spk_gain_cb.callback = callback;
}

AT(.text.soft_gain.spk)
void spk_soft_gain_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&spk_gain_cb, 0, sizeof(spk_gain_cb));
    spk_gain_cb.curr = SOFT_GAIN_MAX_LEVEL;
    spk_gain_cb.level = 0;
}

AT(.text.soft_gain.spk)
void spk_soft_gain_set_level(uint level)
{
    //PC端下发的音量等级, level对应64个等级, 并通过查表设置对应的GAIN值
    if(level >= SOFT_GAIN_MAX_LEVEL) {
        spk_gain_cb.level = 0;
    } else {
        spk_gain_cb.level = SOFT_GAIN_MAX_LEVEL - level;
    }
    printf("spk_level:%d\n", level);
}
#endif

