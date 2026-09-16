#include "include.h"
#include "api_alg.h"
#include "ylcrn_16k.h"

/*
 * 文件名称: ylcrn_16k.c
 * 功能描述: 本文件为软件dnn_L1处理模块
    AT(.buf.ylcrn_16k);
    AT(.rodata.ylcrn_16k)
    AT(.text.ylcrn_16k_proc)
    AT(.text.ylcrn_16k_init)

    注意mic_pcm_t 实际配置类型

 ****************************************************************************************
    code + rodata :
    buf           :
    time          :  6.3/15ms
 */

#if YLCRN_16K_EN
typedef s16 mic_pcm_t;
#define TOG_BUF_EN                   1
#define YLCRN_16K_INFO_PRINT         0
#define FRAME_LEN                    240                         //算法处理帧长
#define PROCESS_OUT_SAMPLES          40                          //每次存取帧长

#if TOG_BUF_EN
static struct tog_bug_tag ylcrn_16k_tbuf AT(.buf.ylcrn_16k);                    //乒乓buf控制
static mic_pcm_t ylcrn_16k_cache_buf[FRAME_LEN*2] AT(.buf.ylcrn_16k);           //乒乓buf缓存
static mic_pcm_t ylcrn_16k_tmp_buf[PROCESS_OUT_SAMPLES] AT(.buf.ylcrn_16k);     //输出buf中转缓存
static void *ylcrn_16k_proc_ptr = ylcrn_16k_cache_buf;
#endif

static dnn_plus_cb_t dnn_plus_cb AT(.buf.ylcrn_16k);
static ylcrn_16k_mic_cfg_t ylcrn_16k_mic_cfg AT(.buf.ylcrn_16k);

static u8 ylcrn_16k_src_in[WIRELESS_MIC_SAMPLES_SELECT*2 + 30] AT(.buf.ylcrn_16k);

#if YLCRN_16K_INFO_PRINT
AT(.com_text.ylcrn_16k)
const char ylcrn_16k_info[] = "YLCRN_16K_EN samples = %d, isrcnt = %d (SR_%d) %d\n";

AT(.text.ylcrn_16k_proc)
static void info_printf(void)
{
    static u32 ticks = 0;
    static u32 isr_cnt = 0;
    isr_cnt++;
    if (tick_check_expire(ticks,1000)) {
        my_printf(ylcrn_16k_info, FRAME_LEN, isr_cnt, FRAME_LEN*isr_cnt, sizeof(mic_pcm_t));
        isr_cnt = 0;
        ticks = tick_get();
    }
}
#endif

AT(.text.ylcrn_16k_proc)
void ylcrn_16k_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    uint resamples = 0;
    if (!ylcrn_16k_mic_cfg.mute /*&& wireless_cb.alg_en*/) {
#if TOG_BUF_EN
        while(samples > 0) {
            uint rlen = (samples > PROCESS_OUT_SAMPLES)? PROCESS_OUT_SAMPLES : samples;
            if(tog_buf_get(&ylcrn_16k_tbuf, (u8 *)ylcrn_16k_tmp_buf, rlen*sizeof(mic_pcm_t))) {
                tog_buf_rd_toggle(&ylcrn_16k_tbuf);
            }

            // 填充block, true表示算法攒帧完毕, kick低优先级线程处理算法
            if(tog_buf_put(&ylcrn_16k_tbuf, ptr, rlen*sizeof(mic_pcm_t))) {
                ylcrn_16k_proc_ptr = tog_bug_get_w_block(&ylcrn_16k_tbuf);
                tog_buf_wr_toggle(&ylcrn_16k_tbuf);
                ylcrn_16k_mic_cfg.kick_proc_done++;
                ylcrn_16k_mic_proc_kick_start();
            }

            //16k 重采样回 48k送进编码
            resamples = src_process(ylcrn_16k_tmp_buf, ylcrn_16k_src_in, rlen);
            if (resamples != WIRELESS_MIC_SAMPLES_SELECT) {
                return;
            }

            if(ylcrn_16k_mic_cfg.callback) {
                ylcrn_16k_mic_cfg.callback((void *)ylcrn_16k_src_in, resamples, ch_mode, params);
            }

            samples -= rlen;
        }
#else

        if (samples == FRAME_LEN) {
        #if YLCRN_16K_INFO_PRINT
            info_printf();
        #endif


        }
        if(ylcrn_16k_mic_cfg.callback) {
            ylcrn_16k_mic_cfg.callback((void *)ptr, samples, ch_mode, params);
        }
#endif
    } else {
        uint rlen = (samples > PROCESS_OUT_SAMPLES)? PROCESS_OUT_SAMPLES : samples;
        //16k 重采样回 48k送进编码
        resamples = src_process(ptr, ylcrn_16k_src_in, rlen);
        if (resamples != WIRELESS_MIC_SAMPLES_SELECT) {
            return;
        }

        if(ylcrn_16k_mic_cfg.callback) {
            ylcrn_16k_mic_cfg.callback((void *)ylcrn_16k_src_in, resamples, ch_mode, params);
        }
    }

}

//ylcrn_16k算法启动计算 放在低优先级现场处理
AT(.text.ylcrn_16k_proc)
void ylcrn_16k_mic_proc_cb(void)
{
#if TOG_BUF_EN
    mic_pcm_t *rptr = ylcrn_16k_proc_ptr;
#if YLCRN_16K_INFO_PRINT
    info_printf();
#endif
    dnn_plus_ns_process(rptr);

    ylcrn_16k_mic_cfg.kick_proc_done--;
#endif
}

AT(.text.ylcrn_16k_set)
void ylcrn_16k_mic_output_callback_set(audio_callback_t callback)
{
    ylcrn_16k_mic_cfg.callback = callback;
}

AT(.text.ylcrn_16k_init)
void ylcrn_16k_mic_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset((uint8_t *)&ylcrn_16k_mic_cfg, 0, sizeof(ylcrn_16k_mic_cfg));
#if TOG_BUF_EN
    tog_buf_init(&ylcrn_16k_tbuf, ylcrn_16k_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));
#endif
    soft_src_init(1,0);
    ylcrn_16k_mic_param_set(0);

}

AT(.text.ylcrn_16k_exit)
void ylcrn_16k_mic_exit(void)
{

}

AT(.text.ylcrn_16k_set.param)
void ylcrn_16k_mic_param_set(s16 ylcrn_16k_nt)
{
    dnn_plus_ns_init(&dnn_plus_cb);
}

AT(.text.ylcrn_16k_set.mute)
void ylcrn_16k_mic_mute_set(uint8_t mute)
{
    ylcrn_16k_mic_cfg.mute = mute;
    if (mute) {
        while(ylcrn_16k_mic_cfg.kick_proc_done){
            printf("#");
        }
#if TOG_BUF_EN
        tog_buf_init(&ylcrn_16k_tbuf, ylcrn_16k_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));
#endif
    }
}

AT(.text.ylcrn_16k_get.mute)
uint8_t ylcrn_16k_mic_mute_get(void)
{
    return ylcrn_16k_mic_cfg.mute;
}
#else
void ylcrn_16k_mic_proc_cb(void){}
#endif
