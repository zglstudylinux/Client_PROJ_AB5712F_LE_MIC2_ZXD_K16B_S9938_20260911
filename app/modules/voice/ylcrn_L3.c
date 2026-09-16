#include "include.h"
#include "api_alg.h"
#include "ylcrn_L3.h"

/*
 * 文件名称: gtcrn_L3.c
 * 功能描述: 本文件为软件dnn_L1处理模块
    AT(.buf.gtcrn_L3);
    AT(.rodata.gtcrn_L3)
    AT(.text.gtcrn_L3_proc)
    AT(.text.gtcrn_L3_init)

    注意mic_pcm_t 实际配置类型

 ****************************************************************************************
    code + rodata :
    buf           :
    npu           ：100k
    time          : 12.8ms/20ms   no hw_fft
 */

#if YLCRN_L3_EN
#define UARTDUMP_YLCRN_L3_EN          0                          //DUMP数据一键配置，需要使用定制上位机
#define GTCRN_L3_INFO_PRINT           0
#define FRAME_LEN                    960                         //算法处理帧长
#define PROCESS_OUT_SAMPLES          120                         //每次存取帧长

static struct tog_bug_tag gtcrn_L3_tbuf AT(.buf.gtcrn_L3);                    //乒乓buf控制
static mic_pcm_t gtcrn_L3_cache_buf[FRAME_LEN*2] AT(.buf.gtcrn_L3);           //乒乓buf缓存
static mic_pcm_t gtcrn_L3_tmp_buf[PROCESS_OUT_SAMPLES] AT(.buf.gtcrn_L3);     //输出buf中转缓存
static void *gtcrn_L3_proc_ptr = gtcrn_L3_cache_buf;

static gtcrn_L3_cb_t gtcrn_L3_cb AT(.buf.gtcrn_L3);
static gtcrn_L3_mic_cfg_t gtcrn_L3_mic_cfg AT(.buf.gtcrn_L3);

#if UARTDUMP_YLCRN_L3_EN
static u8 dump_head_buf[14*2]AT(.buf.gtcrn_L3);
static u8 UartDump_buf[2][14 + FRAME_LEN*2]AT(.buf.gtcrn_L3);
static u8 UartDump_buf_temp[14 + FRAME_LEN*2]AT(.buf.gtcrn_L3);
#endif

#if GTCRN_L3_INFO_PRINT
AT(.com_text.gtcrn_L3)
const char gtcrn_L3_info[] = "YLCRN_L3_EN samples = %d, isrcnt = %d (SR_%d) %d\n";

AT(.text.gtcrn_L3_proc)
static void info_printf(void)
{
    static u32 ticks = 0;
    static u32 isr_cnt = 0;
    isr_cnt++;
    if(tick_check_expire(ticks,1000)) {
        my_printf(gtcrn_L3_info, FRAME_LEN, isr_cnt, FRAME_LEN*isr_cnt, sizeof(mic_pcm_t));
        isr_cnt = 0;
        ticks = tick_get();
    }
}
#endif

AT(.text.gtcrn_L3_proc)
void ylcrn_L3_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    if (!gtcrn_L3_mic_cfg.mute && wireless_cb.alg_en) {

        while(samples > 0) {
            uint rlen = (samples > PROCESS_OUT_SAMPLES)? PROCESS_OUT_SAMPLES : samples;
            if(tog_buf_get(&gtcrn_L3_tbuf, (u8 *)gtcrn_L3_tmp_buf, rlen*sizeof(mic_pcm_t))) {
                tog_buf_rd_toggle(&gtcrn_L3_tbuf);
            }

            // 填充block, true表示算法攒帧完毕, kick低优先级线程处理算法
            if(tog_buf_put(&gtcrn_L3_tbuf, ptr, rlen*sizeof(mic_pcm_t))) {
                gtcrn_L3_proc_ptr = tog_bug_get_w_block(&gtcrn_L3_tbuf);
                tog_buf_wr_toggle(&gtcrn_L3_tbuf);
                gtcrn_L3_mic_cfg.kick_proc_done++;
                ylcrn_L3_mic_proc_kick_start();
            }

            memcpy(ptr, gtcrn_L3_tmp_buf, rlen*sizeof(mic_pcm_t));
            if(gtcrn_L3_mic_cfg.callback) {
                gtcrn_L3_mic_cfg.callback((void *)ptr, rlen, ch_mode, params);
            }

            samples -= rlen;
        }
    } else {
        if(gtcrn_L3_mic_cfg.callback) {
            gtcrn_L3_mic_cfg.callback((void *)ptr, samples, ch_mode, params);
        }
    }

}

//gtcrn_L3算法启动计算 放在低优先级现场处理
AT(.text.gtcrn_L3_proc)
void ylcrn_L3_mic_proc_cb(void)
{
    mic_pcm_t *rptr = gtcrn_L3_proc_ptr;

#if UARTDUMP_YLCRN_L3_EN
    s16 *ptr_temp = (s16 *) UartDump_buf_temp;
    dump_dma_wait();
    dump_put2ram(&UartDump_buf[0][0],rptr,FRAME_LEN*2,0); //算法前数据组包

    dump_put2ram(&UartDump_buf[1][0],ptr_temp,FRAME_LEN*2,1);//算法后数据组包
    dump_dma_kick(&UartDump_buf[0][0], (FRAME_LEN*2 + 14) * 2); //最后再一起吐数据，避免前后kick数据打架
#endif

#if GTCRN_L3_INFO_PRINT
    info_printf();
#endif

    gtcrn_L3_ns_process(rptr);

#if UARTDUMP_YLCRN_L3_EN
    for(int i = 0; i < FRAME_LEN; i++) {
        ptr_temp[i]  = rptr[i];
    }
#endif
    gtcrn_L3_mic_cfg.kick_proc_done--;
}

AT(.text.gtcrn_L3_set)
void ylcrn_L3_mic_output_callback_set(audio_callback_t callback)
{
    gtcrn_L3_mic_cfg.callback = callback;
}

AT(.text.gtcrn_L3_init)
void ylcrn_L3_mic_init(u8 sample_rate, u16 samples, u8 channel)
{
    tog_buf_init(&gtcrn_L3_tbuf, gtcrn_L3_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));
    memset((uint8_t *)&gtcrn_L3_mic_cfg, 0, sizeof(gtcrn_L3_mic_cfg));
    ylcrn_L3_mic_param_set(1800);

#if UARTDUMP_YLCRN_L3_EN
    memset(dump_head_buf, 0, 14*2);
    memset(UartDump_buf, 0, 2*(14 + FRAME_LEN*2));

    huart_module_init();    //初始化高速串口模块
    dump_buf_init(dump_head_buf,2, huart_putbuf, huart_wait_tx_finish);
#endif

}

AT(.text.gtcrn_L3_exit)
void ylcrn_L3_mic_exit(void)
{

}

/*
 * 接口名称: void ylcrn_L3_mic_param_set(s16 ylcrn_L3_nt);
 * 主要调试参数描述:
    ylcrn_L3_cb.denoiseBound(ylcrn_L3_nt): 降噪量，范围为1~32768，数值越小降噪越明显;
                           计算公式 20*log10(denoiseBound/32768)，16384 就相当于噪声降6.02db。
*/
AT(.text.gtcrn_L3_set.param)
void ylcrn_L3_mic_param_set(s16 ylcrn_L3_nt)
{
    memset((uint8_t *)&gtcrn_L3_cb, 0, sizeof(gtcrn_L3_cb));
    if (ylcrn_L3_nt<1) {
        ylcrn_L3_nt = 1;
    } else if (ylcrn_L3_nt>32768) {
        ylcrn_L3_nt = 32768;
    }
	gtcrn_L3_cb.overdrive			= 32768;
	gtcrn_L3_cb.adaptive_floor		= 0;
	gtcrn_L3_cb.denoiseBound			= ylcrn_L3_nt;
	gtcrn_L3_cb.denoiseBoundLow		= 1800;
	gtcrn_L3_cb.denoiseBoundHigh		= 1800;
	gtcrn_L3_cb.denoiseBoundProb		= 8000;
	gtcrn_L3_cb.smooth_en			= 1;
	gtcrn_L3_cb.modelUpdatePars0		= 2;
	gtcrn_L3_cb.prior_opt_idx		= 3;
	gtcrn_L3_cb.prior_opt_ada_en		= 1;
	gtcrn_L3_cb.quan_gap				= 3277;
	gtcrn_L3_cb.quan_gap_low_len		= 6;
	gtcrn_L3_cb.quan_gap_low			= 3277;
	gtcrn_L3_cb.lquantile_sm			= 26214;
	gtcrn_L3_cb.factor				= 30*32768;

	gtcrn_L3_cb.delta_k_up		    = 1;
	gtcrn_L3_cb.sin_dnn_en			= 0;
	gtcrn_L3_cb.sin_all_en			= 0;
	gtcrn_L3_cb.sin_all_len			= 512;

	gtcrn_L3_cb.low_noise_range		= 16;
	gtcrn_L3_cb.spp_fre_p			= 6000;
	gtcrn_L3_cb.spp_fre_len			= 64;

	gtcrn_L3_cb.high_gain_len        = 64;
	gtcrn_L3_cb.gain_assign_len		= 512;
	gtcrn_L3_cb.gain_assign			= 26666;
	gtcrn_L3_cb.nn_only_len			= 0;
	gtcrn_L3_cb.music_lev			= 9;
	gtcrn_L3_cb.intensity			= 0;
	gtcrn_L3_cb.mask_vad_fre_thr		= 0;
	gtcrn_L3_cb.spp_max_en			= 1;

    gtcrn_L3_ns_init(&gtcrn_L3_cb);

}

AT(.text.gtcrn_L3_set.mute)
void ylcrn_L3_mic_mute_set(uint8_t mute)
{
    gtcrn_L3_mic_cfg.mute = mute;
    if(mute) {
        while(gtcrn_L3_mic_cfg.kick_proc_done){
            printf("#");
        }
        npu_exit();
        tog_buf_init(&gtcrn_L3_tbuf, gtcrn_L3_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));
    } else {
       // gtcrn_L3_ns_init(&gtcrn_L3_cb);
    }
}

AT(.text.gtcrn_L3_get.mute)
uint8_t ylcrn_L3_mic_mute_get(void)
{
    return gtcrn_L3_mic_cfg.mute;
}
#else
void ylcrn_L3_mic_proc_cb(void){}
#endif
