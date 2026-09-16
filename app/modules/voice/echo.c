#include "include.h"
#include "echo.h"
#include "api_alg.h"
/*
 * 文件名称: echo.c
 * 功能描述: 本文件为软件ECHO处理模块

 ****************************************************************************************
    code :
    buf  : 10000*2 Bytes
    time : 160M 下 120 个点处理 147us
 */
#if ECHO_EN

#define SAMPLE_48k_RATE         48000
#define SAMPLE_32k_RATE         32000
#define DELAY_BUF_LENGTH        10000 //8600
#define MAX_DELAY_LENGTH_48K    (DELAY_BUF_LENGTH/((SAMPLE_48k_RATE >> 1) * 0.001f))  //48K最大delay时间
#define MAX_DELAY_LENGTH_32K    (DELAY_BUF_LENGTH/((SAMPLE_32k_RATE >> 1) * 0.001f))  //48K最大delay时间
#define PERCENT                 327  //PERCENT

static echo_cb_t echo_cb  AT(.buf.echo);
static echo_cfg_t echo_cfg AT(.buf.echo);
s16 delay_lbuf[DELAY_BUF_LENGTH] AT(.buf.echo.delay);


AT(.text.echo_proc.input)
void echo_audio_input(u8 *ptr, u32 samples, int ch_mode, void *param)
{
    s16 *rptr = (s16 *)ptr;
    if(!echo_cfg.mute){
        if(samples){
            for(u8 i=0;i<samples;i++){
                echo_process(rptr+i);
            }
        }else{

        }
    }

    if (echo_cfg.callback) {
        echo_cfg.callback((u8 *)rptr, samples, ch_mode, param);
    }
}

AT(.text.echo_set.callback)
void echo_audio_output_callback_set(audio_callback_t callback)
{
    echo_cfg.callback = callback;
}

//AT(.text.echo_proc.input)
//void echo_audio_process(s16 *ptr, u32 samples)
//{
//    s16 *rptr = ptr;
//    if(!echo_cfg.mute){
//        for(u8 i=0;i<samples;i++){
//            echo_process(rptr+i);
//        }
//    }
//}

AT(.text.echo_set.mute)
void echo_audio_mute_set(uint8_t mute)
{
    echo_cfg.mute = mute;

    if(echo_cfg.mute){
        echo_clear_buf();
    }
}

/// attenuation:    混响次数  0~90
/// delay_length:   每一声混响之间的间隔  0~400
/// cutoffFreq_set: 低通滤波器的截止频率  1~4000
/// lp_filter_en:   低通滤波器的开关      0：关 1：开
/// dry_set:        原始声音               0~49152（Q15即0~1.5）
/// wet_set:        回声湿度，越大所占比例越大  0~32768（Q15 即0~1.0）
/// 默认配置 echo_audio_param_set(55,250,1000,1,32768,15000)
AT(.text.echo_set.param)
void echo_audio_param_set(u16 attenuation ,s32 delay_length ,u32 cutoffFreq_set ,u32 lp_filter_en , u16 dry_set ,u16 wet_set)
{
    echo_cb_t *st =  &echo_cb;
    echo_clear_buf();
    if(attenuation >= 90){
        attenuation = 90;
    }
    if(delay_length >= echo_cfg.max_delay_len){
        delay_length = echo_cfg.max_delay_len;
    }

    st->dry = dry_set;                            //原始声音
	st->wet = wet_set;                            //回声湿度，越大所占比例越大，不建议修改
	st->attenuation = attenuation * PERCENT;       // percent
	st->filter_en = lp_filter_en;		          //低通滤波器的开关
	if(echo_cfg.sample_rate == 3){
        st->delay_length = delay_length * (SAMPLE_32k_RATE >> 1) * 0.001f;//500ms, 最大delay 500ms
	}else{
        st->delay_length = delay_length * (SAMPLE_48k_RATE >> 1) * 0.001f;//500ms, 最大delay 500ms
	}
    iir2_filter_init(cutoffFreq_set);   //低通滤波器的截止频率

    echo_set_param(st);
}

AT(.text.echo_init)
void echo_delay_len_set(u16 delay_len)
{
#if WIRELESS_MIC_PARAM_MEMORY_EN
    param_echo_delay_level_write((u8 *)&echo_cfg.delay_level);
#endif
    echo_audio_param_set(ECHO_LEVEL, delay_len, 1000, 0, ECHO_DRY_USER, ECHO_WET_USER);
}

AT(.text.echo_set)
void echo_delay_level_change(void)
{
    echo_cfg.delay_level += 1;
    if (echo_cfg.delay_level >= ECHO_DELAY_MAX_LEVEL) {
        echo_cfg.delay_level = 0;
    }
    echo_delay_len_set(echo_cfg.delay_level*echo_cfg.delay_len_step);
}

AT(.text.echo_set)
void echo_delay_level_up(void)
{
    if (echo_cfg.delay_level < (ECHO_DELAY_MAX_LEVEL - 1)) {
        echo_cfg.delay_level += 1;
    }
    echo_delay_len_set(echo_cfg.delay_level*echo_cfg.delay_len_step);
}

AT(.text.echo_set)
void echo_delay_level_down(void)
{
    if (echo_cfg.delay_level) {
        echo_cfg.delay_level -= 1;
    }
    echo_delay_len_set(echo_cfg.delay_level*echo_cfg.delay_len_step);
}

AT(.text.echo_get)
u8 echo_delay_level_get(void)
{
    return echo_cfg.delay_level;
}

AT(.text.echo_get)
bool echo_delay_level_is_max_min(void)
{
    if (echo_cfg.delay_level == (ECHO_DELAY_MAX_LEVEL - 1) || echo_cfg.delay_level == 0) {
        return true;
    }
    return false;
}

AT(.text.echo_init) WEAK
void echo_delay_level_set(u8 delay_level)
{
    if (delay_level > (ECHO_DELAY_MAX_LEVEL - 1)) {
        delay_level = ECHO_DELAY_MAX_LEVEL - 1;
    }
    echo_cfg.delay_level = delay_level;
    echo_delay_len_set(echo_cfg.delay_level*echo_cfg.delay_len_step);
}

AT(.text.echo_init)
void echo_audio_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&echo_cfg, 0, sizeof(echo_cfg_t));
    memset(&echo_cb, 0, sizeof(echo_cb));
    echo_cfg.sample_rate = sample_rate;
    if (echo_cfg.sample_rate == 3) {
        echo_cfg.max_delay_len = MAX_DELAY_LENGTH_32K;
    } else {
        echo_cfg.max_delay_len = MAX_DELAY_LENGTH_48K;
    }
    echo_cfg.delay_len_step = echo_cfg.max_delay_len/(ECHO_DELAY_MAX_LEVEL - 1);
    echo_cfg.delay_level = ECHO_DELAY_DEFAULT_LEVEL - 1;
#if WIRELESS_MIC_PARAM_MEMORY_EN
    param_echo_delay_level_read((u8 *)&echo_cfg.delay_level);
#endif
    echo_init_mode(sample_rate, delay_lbuf, DELAY_BUF_LENGTH, echo_cfg.max_delay_len);

    echo_audio_param_set(ECHO_LEVEL, echo_cfg.delay_level*echo_cfg.delay_len_step, 1000, 0, ECHO_DRY_USER, ECHO_WET_USER);
    echo_audio_mute_set(0);
}

AT(.text.echo_exit)
void echo_audio_exit(void)
{
}



#endif //ECHO_EN
