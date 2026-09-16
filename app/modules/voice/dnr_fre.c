#include "include.h"
#include "api_alg.h"
#include "dnr_fre.h"

/*
 * 文件名称: dnr_fre.c
 * 功能描述: 本文件为软件dnn_L1处理模块
    AT(.buf.dnr_fre);
    AT(.rodata.dnr_fre)
    AT(.text.dnr_fre_proc)
    AT(.text.dnr_fre_init)

    注意mic_pcm_t 实际配置类型

 ****************************************************************************************
    code + rodata : 8.4k
    buf           : 6.7k
    time          : 500us /2.5ms   240M
 */

#if DNR_FRE_EN
typedef s16 mic_pcm_t;
#define TOG_BUF_EN                   0
#define DNR_FRE_INFO_PRINT           1
#define FRAME_LEN                    120                         //算法处理帧长
#define PROCESS_OUT_SAMPLES          120                         //每次存取帧长

#if TOG_BUF_EN
static struct tog_bug_tag dnr_fre_tbuf AT(.buf.dnr_fre);                    //乒乓buf控制
static mic_pcm_t dnr_fre_cache_buf[FRAME_LEN*2] AT(.buf.dnr_fre);           //乒乓buf缓存
static mic_pcm_t dnr_fre_tmp_buf[PROCESS_OUT_SAMPLES] AT(.buf.dnr_fre);     //输出buf中转缓存
static void *dnr_fre_proc_ptr = dnr_fre_cache_buf;
#endif

static dnr_fre_cb_t dnr_fre_cb AT(.buf.dnr_fre);
static dnr_fre_mic_cfg_t dnr_fre_mic_cfg AT(.buf.dnr_fre);

#if DNR_FRE_INFO_PRINT
AT(.com_text.dnr_fre)
const char dnr_fre_info[] = "DNR_FRE_EN samples = %d, isrcnt = %d (SR_%d) %d\n";

AT(.text.dnr_fre_proc)
static void info_printf(void)
{
    static u32 ticks = 0;
    static u32 isr_cnt = 0;
    isr_cnt++;
    if (tick_check_expire(ticks,1000)) {
        my_printf(dnr_fre_info, FRAME_LEN, isr_cnt, FRAME_LEN*isr_cnt, sizeof(mic_pcm_t));
        isr_cnt = 0;
        ticks = tick_get();
    }
}
#endif

AT(.text.dnr_fre_proc)
void dnr_fre_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{

    if (!dnr_fre_mic_cfg.mute /*&& wireless_cb.alg_en*/) {
#if TOG_BUF_EN
        while(samples > 0) {
            uint rlen = (samples > PROCESS_OUT_SAMPLES)? PROCESS_OUT_SAMPLES : samples;
            if(tog_buf_get(&dnr_fre_tbuf, (u8 *)dnr_fre_tmp_buf, rlen*sizeof(mic_pcm_t))) {
                tog_buf_rd_toggle(&dnr_fre_tbuf);
            }

            // 填充block, true表示算法攒帧完毕, kick低优先级线程处理算法
            if(tog_buf_put(&dnr_fre_tbuf, ptr, rlen*sizeof(mic_pcm_t))) {
                dnr_fre_proc_ptr = tog_bug_get_w_block(&dnr_fre_tbuf);
                tog_buf_wr_toggle(&dnr_fre_tbuf);
                dnr_fre_mic_cfg.kick_proc_done++;
                dnr_fre_mic_proc_kick_start();
            }

            memcpy(ptr, dnr_fre_tmp_buf, rlen*sizeof(mic_pcm_t));
            if(dnr_fre_mic_cfg.callback) {
                dnr_fre_mic_cfg.callback((void *)ptr, rlen, ch_mode, params);
            }

            samples -= rlen;
        }
#else

        if (samples == FRAME_LEN) {
        #if DNR_FRE_INFO_PRINT
            info_printf();
        #endif
            dnr_fre_process((s16 *)ptr, NULL, 0);

        }
        if(dnr_fre_mic_cfg.callback) {
            dnr_fre_mic_cfg.callback((void *)ptr, samples, ch_mode, params);
        }
#endif
    } else {
        if(dnr_fre_mic_cfg.callback) {
            dnr_fre_mic_cfg.callback((void *)ptr, samples, ch_mode, params);
        }
    }

}

//dnr_fre算法启动计算 放在低优先级现场处理
AT(.text.dnr_fre_proc)
void dnr_fre_mic_proc_cb(void)
{
#if TOG_BUF_EN
    mic_pcm_t *rptr = dnr_fre_proc_ptr;
#if DNR_FRE_INFO_PRINT
    info_printf();
#endif
//    s32 p_left;
//    for (u16 i = 0; i < FRAME_LEN; ++i) {
//        p_left = rptr[i];
//        p_left = __builtin_muls_shift15(p_left, SOFT_GAIN_P16_0DB);
//        rptr[i] = s_clip16(p_left);
//    }

    dnr_fre_process(rptr, NULL, 0);

//    for (u16 i = 0; i < FRAME_LEN; ++i) {
//        p_left = rptr[i];
//        p_left = __builtin_muls_shift15(p_left, SOFT_GAIN_N16DB);
//        rptr[i] = s_clip16(p_left);
//    }

    dnr_fre_mic_cfg.kick_proc_done--;
#endif
}

AT(.text.dnr_fre_set)
void dnr_fre_mic_output_callback_set(audio_callback_t callback)
{
    dnr_fre_mic_cfg.callback = callback;
}

AT(.text.dnr_fre_init)
void dnr_fre_mic_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset((uint8_t *)&dnr_fre_mic_cfg, 0, sizeof(dnr_fre_mic_cfg));
#if TOG_BUF_EN
    tog_buf_init(&dnr_fre_tbuf, dnr_fre_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));
#endif
    dnr_fre_mic_param_set(0);

//    ///算法初始化处先mute，等连上无线麦再打开/接收端发送使能再打开
//    dnr_fre_mic_mute_set(1);

}

AT(.text.dnr_fre_exit)
void dnr_fre_mic_exit(void)
{

}

/*
 * 接口名称: void dnr_fre_mic_param_set(s16 dnr_fre_nt);
 * 主要调试参数描述:
    dnr_fre_cb.denoiseBound(dnr_fre_nt): 降噪量，范围为1~32768，数值越小降噪越明显;
                           计算公式 20*log10(denoiseBound/32768)，16384 就相当于噪声降6.02db。

    dnr_fre_cb.enr_nr_thr：噪声门，低于此门限为噪声，直接设置为算法输入数字信号底噪大小（dbfs）即可。
*/
AT(.text.dnr_fre_set.param)
void dnr_fre_mic_param_set(s16 dnr_fre_nt)
{
    //边界保护
    if (dnr_fre_nt<1) {
        dnr_fre_nt = 1;
    } else if (dnr_fre_nt>32768) {
        dnr_fre_nt = 32768;
    }
    memset((uint8_t *)&dnr_fre_cb, 0, sizeof(dnr_fre_cb));
	dnr_fre_cb.overdrive			    = 32768;
	dnr_fre_cb.smooth_en			    = 1;
	dnr_fre_cb.enr_thres				= 0;
	dnr_fre_cb.prior_opt_idx			= 10;
	dnr_fre_cb.low_fre_range			= 0;
	dnr_fre_cb.smooth_v				    = 31129; // 0.9f
	dnr_fre_cb.music_lev				= 11;

	dnr_fre_cb.denoiseBound		        = dnr_fre_nt;  //降噪量：20*log10(denoiseBound/32768) . 16384就相当于噪声降6db
	dnr_fre_cb.enr_nr_thr				= -56;//dB  门限：低于此门限为噪声
	dnr_fre_cb.in_attack				= 32768;//Q15 ms 检测的att时间，peak， 32768=1ms
	dnr_fre_cb.in_release				= 3276800;//Q15 ms 检测的release时间，peak， 32768=1ms
	dnr_fre_cb.fs						= 48000;
	dnr_fre_cb.noise_init				= 1;
    dnr_fre_init(&dnr_fre_cb);
}

AT(.text.dnr_fre_set.mute)
void dnr_fre_mic_mute_set(uint8_t mute)
{
    dnr_fre_mic_cfg.mute = mute;
    if (mute) {
        while(dnr_fre_mic_cfg.kick_proc_done){
            printf("#");
        }
#if TOG_BUF_EN
        tog_buf_init(&dnr_fre_tbuf, dnr_fre_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));
#endif
    }
}

AT(.text.dnr_fre_get.mute)
uint8_t dnr_fre_mic_mute_get(void)
{
    return dnr_fre_mic_cfg.mute;
}
#else
void dnr_fre_mic_proc_cb(void){}
#endif
