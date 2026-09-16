#include "include.h"
#include "api_alg.h"
#include "ylcrn_L2.h"

/*
 * 文件名称: ylcrn_L2.c
 * 功能描述: 本文件为软件dnn_L1处理模块
    AT(.buf.ylcrn_L2);
    AT(.rodata.ylcrn_L2)
    AT(.text.ylcrn_L2_proc)
    AT(.text.ylcrn_L2_init)

    注意mic_pcm_t 实际配置类型

 ****************************************************************************************
    code + rodata : 14k
    buf           : 18k
    npu           ：100k
    time          : 6.2ms /10ms npu+float+hwfft
 */

#if YLCRN_L2_EN
#define UARTDUMP_YLCRN_L2_EN         0                          //DUMP数据一键配置，需要使用定制上位机
#define YLCRN_L2_INFO_PRINT          0
#define FRAME_LEN                    480                         //算法处理帧长
#define PROCESS_OUT_SAMPLES          120                         //每次存取帧长

static struct tog_bug_tag ylcrn_L2_tbuf AT(.buf.ylcrn_L2);                    //乒乓buf控制
static mic_pcm_t ylcrn_L2_cache_buf[FRAME_LEN*2] AT(.buf.ylcrn_L2);           //乒乓buf缓存
static mic_pcm_t ylcrn_L2_tmp_buf[PROCESS_OUT_SAMPLES] AT(.buf.ylcrn_L2);     //输出buf中转缓存
static void *ylcrn_L2_proc_ptr = ylcrn_L2_cache_buf;

static ylcrn_L2_cb_t ylcrn_L2_cb AT(.buf.ylcrn_L2);
static ylcrn_L2_mic_cfg_t ylcrn_L2_mic_cfg AT(.buf.ylcrn_L2);

#if UARTDUMP_YLCRN_L2_EN
static u8 dump_head_buf[14*2]AT(.buf.ylcrn_L2);
static u8 UartDump_buf[2][14 + FRAME_LEN*2]AT(.buf.ylcrn_L2);
static u8 UartDump_buf_temp[14 + FRAME_LEN*2]AT(.buf.ylcrn_L2);
#endif

#if YLCRN_L2_INFO_PRINT
AT(.com_text.ylcrn_L2)
const char ylcrn_L2_info[] = "YLCRN_L2_EN samples = %d, isrcnt = %d (SR_%d) %d\n";

AT(.text.ylcrn_L2_proc)
static void info_printf(void)
{
    static u32 ticks = 0;
    static u32 isr_cnt = 0;
    isr_cnt++;
    if(tick_check_expire(ticks,1000)) {
        my_printf(ylcrn_L2_info, FRAME_LEN, isr_cnt, FRAME_LEN*isr_cnt, sizeof(mic_pcm_t));
        isr_cnt = 0;
        ticks = tick_get();
    }
}
#endif

AT(.text.ylcrn_L2_proc)
void ylcrn_L2_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    if (!ylcrn_L2_mic_cfg.mute && wireless_cb.alg_en) {

        while(samples > 0) {
            uint rlen = (samples > PROCESS_OUT_SAMPLES)? PROCESS_OUT_SAMPLES : samples;
            if(tog_buf_get(&ylcrn_L2_tbuf, (u8 *)ylcrn_L2_tmp_buf, rlen*sizeof(mic_pcm_t))) {
                tog_buf_rd_toggle(&ylcrn_L2_tbuf);
            }

            // 填充block, true表示算法攒帧完毕, kick低优先级线程处理算法
            if(tog_buf_put(&ylcrn_L2_tbuf, ptr, rlen*sizeof(mic_pcm_t))) {
                ylcrn_L2_proc_ptr = tog_bug_get_w_block(&ylcrn_L2_tbuf);
                tog_buf_wr_toggle(&ylcrn_L2_tbuf);
                ylcrn_L2_mic_cfg.kick_proc_done++;
                ylcrn_L2_mic_proc_kick_start();
            }

            memcpy(ptr, ylcrn_L2_tmp_buf, rlen*sizeof(mic_pcm_t));
            if(ylcrn_L2_mic_cfg.callback) {
                ylcrn_L2_mic_cfg.callback((void *)ptr, rlen, ch_mode, params);
            }

            samples -= rlen;
        }
    } else {
        if(ylcrn_L2_mic_cfg.callback) {
            ylcrn_L2_mic_cfg.callback((void *)ptr, samples, ch_mode, params);
        }
    }

}

//ylcrn_L2算法启动计算 放在低优先级现场处理
AT(.text.ylcrn_L2_proc)
void ylcrn_L2_mic_proc_cb(void)
{
    mic_pcm_t *rptr = ylcrn_L2_proc_ptr;
#if YLCRN_L2_INFO_PRINT
    info_printf();
#endif
#if UARTDUMP_YLCRN_L2_EN
    s16 *ptr_temp = (s16 *) UartDump_buf_temp;
    dump_dma_wait();
    dump_put2ram(&UartDump_buf[0][0],rptr,FRAME_LEN*2,0); //算法前数据组包

    dump_put2ram(&UartDump_buf[1][0],ptr_temp,FRAME_LEN*2,1);//算法后数据组包
    dump_dma_kick(&UartDump_buf[0][0], (FRAME_LEN*2 + 14) * 2); //最后再一起吐数据，避免前后kick数据打架
#endif

    ylcrn_L2_ns_process(rptr);

#if UARTDUMP_YLCRN_L2_EN
    for(int i = 0; i < FRAME_LEN; i++) {
        ptr_temp[i]  = rptr[i];
    }
#endif

    ylcrn_L2_mic_cfg.kick_proc_done--;
}

AT(.text.ylcrn_L2_set)
void ylcrn_L2_mic_output_callback_set(audio_callback_t callback)
{
    ylcrn_L2_mic_cfg.callback = callback;
}

AT(.text.ylcrn_L2_init)
void ylcrn_L2_mic_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset((uint8_t *)&ylcrn_L2_mic_cfg, 0, sizeof(ylcrn_L2_mic_cfg));
    tog_buf_init(&ylcrn_L2_tbuf, ylcrn_L2_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));

    ylcrn_L2_mic_param_set(0);

//    ///算法初始化处先mute，等连上无线麦再打开/接收端发送使能再打开
//    ylcrn_L2_mic_mute_set(1);

#if UARTDUMP_YLCRN_L2_EN
    memset(dump_head_buf, 0, 14*2);
    memset(UartDump_buf, 0, 2*(14 + FRAME_LEN*2));

    huart_module_init();    //初始化高速串口模块
    dump_buf_init(dump_head_buf,2, huart_putbuf, huart_wait_tx_finish);
#endif
}

AT(.text.ylcrn_L2_exit)
void ylcrn_L2_mic_exit(void)
{

}

AT(.text.ylcrn_L2_set.param)
void ylcrn_L2_mic_param_set(s16 ylcrn_L2_nt)
{
    memset((uint8_t *)&ylcrn_L2_cb, 0, sizeof(ylcrn_L2_cb));

	ylcrn_L2_cb.overdrive			= 32768;
	ylcrn_L2_cb.adaptive_floor		= 0;
	ylcrn_L2_cb.denoiseBound			= 0;
	ylcrn_L2_cb.denoiseBoundLow		= 0;
	ylcrn_L2_cb.denoiseBoundHigh		= 0;
	ylcrn_L2_cb.denoiseBoundProb		= 11000;
	ylcrn_L2_cb.smooth_en			= 1;
	ylcrn_L2_cb.modelUpdatePars0		= 2;
	ylcrn_L2_cb.prior_opt_idx		= 10;
	ylcrn_L2_cb.prior_opt_ada_en		= 1;

	ylcrn_L2_cb.sin_dnn_en			= 1;
	ylcrn_L2_cb.sin_all_en			= 0;
	ylcrn_L2_cb.sin_all_len			= 0;

	ylcrn_L2_cb.low_noise_range		= 11;
	ylcrn_L2_cb.spp_fre_p			= 12000;
	ylcrn_L2_cb.spp_fre_len			= 16;
	ylcrn_L2_cb.high_gain_len        = 32;
	ylcrn_L2_cb.music_lev			= 16;
	ylcrn_L2_cb.prev_noise_len		= 11;
	ylcrn_L2_cb.gain_assign			= 21666;
	ylcrn_L2_cb.hi_gain_mode			= 0;
    ylcrn_L2_ns_init(&ylcrn_L2_cb);

}

AT(.text.ylcrn_L2_set.mute)
void ylcrn_L2_mic_mute_set(uint8_t mute)
{
    ylcrn_L2_mic_cfg.mute = mute;
    if (mute) {
        while(ylcrn_L2_mic_cfg.kick_proc_done){
            printf("#");
        }
        npu_exit();
        tog_buf_init(&ylcrn_L2_tbuf, ylcrn_L2_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));
    }
}

AT(.text.ylcrn_L2_get.mute)
uint8_t ylcrn_L2_mic_mute_get(void)
{
    return ylcrn_L2_mic_cfg.mute;
}
#else
void ylcrn_L2_mic_proc_cb(void){}
#endif
