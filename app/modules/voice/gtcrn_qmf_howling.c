#include "include.h"
#include "api_alg.h"
#include "gtcrn_qmf_howling.h"

/*
 * 文件名称: gtcrn_qmf_howling.c
 * 功能描述: 本文件为软件gtcrn_qmf_howling处理模块
    AT(.buf.gtcrn_qmf_howling);
    AT(.rodata.gtcrn_qmf_howling)
    AT(.text.gtcrn_qmf_howling_proc)
    AT(.text.gtcrn_qmf_howling_init)

    注意mic_pcm_t 实际配置类型

 ****************************************************************************************
    code + rodata : 14k
    buf           : 18k
    npu           ：100k
    time          : 6.2ms /10ms npu+float+hwfft
 */

#if GTCRN_QMF_HOWLING_EN
#define UARTDUMP_GTCRN_QMF_HOWLING_EN         0                          //DUMP数据一键配置，需要使用定制上位机
#define GTCRN_QMF_HOWLING_INFO_PRINT          0
#define FRAME_LEN                    480                         //算法处理帧长
#define PROCESS_OUT_SAMPLES          120                         //每次存取帧长

static struct tog_bug_tag gtcrn_qmf_howling_tbuf AT(.buf.gtcrn_qmf_howling);                    //乒乓buf控制
static mic_pcm_t gtcrn_qmf_howling_cache_buf[FRAME_LEN*2] AT(.buf.gtcrn_qmf_howling);           //乒乓buf缓存
static mic_pcm_t gtcrn_qmf_howling_tmp_buf[PROCESS_OUT_SAMPLES] AT(.buf.gtcrn_qmf_howling);     //输出buf中转缓存
static void *gtcrn_qmf_howling_proc_ptr = gtcrn_qmf_howling_cache_buf;

static gtcrn_48k_qmf_10ms_cb_t gtcrn_qmf_howling_cb AT(.buf.gtcrn_qmf_howling);
static gtcrn_qmf_howling_mic_cfg_t gtcrn_qmf_howling_mic_cfg AT(.buf.gtcrn_qmf_howling);

#if UARTDUMP_GTCRN_QMF_HOWLING_EN
static u8 dump_head_buf[14*2]AT(.buf.gtcrn_qmf_howling);
static u8 UartDump_buf[2][14 + FRAME_LEN*2]AT(.buf.gtcrn_qmf_howling);
static u8 UartDump_buf_temp[14 + FRAME_LEN*2]AT(.buf.gtcrn_qmf_howling);
#endif

void gtcrn_qmf_howling_mic_param_init();

#if GTCRN_QMF_HOWLING_INFO_PRINT
AT(.com_text.gtcrn_qmf_howling)
const char gtcrn_qmf_howling_info[] = "GTCRN_QMF_HOWLING_EN samples = %d, isrcnt = %d (SR_%d) %d\n";

AT(.text.gtcrn_qmf_howling_proc)
static void info_printf(void)
{
    static u32 ticks = 0;
    static u32 isr_cnt = 0;
    isr_cnt++;
    if(tick_check_expire(ticks,1000)) {
        my_printf(gtcrn_qmf_howling_info, FRAME_LEN, isr_cnt, FRAME_LEN*isr_cnt, sizeof(mic_pcm_t));
        isr_cnt = 0;
        ticks = tick_get();
    }
}
#endif

AT(.text.gtcrn_qmf_howling_proc)
void gtcrn_qmf_howling_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    if (!gtcrn_qmf_howling_mic_cfg.mute && wireless_cb.alg_en) {

        while(samples > 0) {
            uint rlen = (samples > PROCESS_OUT_SAMPLES)? PROCESS_OUT_SAMPLES : samples;
            if(tog_buf_get(&gtcrn_qmf_howling_tbuf, (u8 *)gtcrn_qmf_howling_tmp_buf, rlen*sizeof(mic_pcm_t))) {
                tog_buf_rd_toggle(&gtcrn_qmf_howling_tbuf);
            }

            // 填充block, true表示算法攒帧完毕, kick低优先级线程处理算法
            if(tog_buf_put(&gtcrn_qmf_howling_tbuf, ptr, rlen*sizeof(mic_pcm_t))) {
                gtcrn_qmf_howling_proc_ptr = tog_bug_get_w_block(&gtcrn_qmf_howling_tbuf);
                tog_buf_wr_toggle(&gtcrn_qmf_howling_tbuf);
                gtcrn_qmf_howling_mic_cfg.kick_proc_done++;
                gtcrn_qmf_howling_mic_proc_kick_start();
            }

            memcpy(ptr, gtcrn_qmf_howling_tmp_buf, rlen*sizeof(mic_pcm_t));
            if(gtcrn_qmf_howling_mic_cfg.callback) {
                gtcrn_qmf_howling_mic_cfg.callback((void *)ptr, rlen, ch_mode, params);
            }

            samples -= rlen;
        }
    } else {
        if(gtcrn_qmf_howling_mic_cfg.callback) {
            gtcrn_qmf_howling_mic_cfg.callback((void *)ptr, samples, ch_mode, params);
        }
    }

}

//gtcrn_qmf_howling算法启动计算 放在低优先级现场处理
AT(.text.gtcrn_qmf_howling_proc)
void gtcrn_qmf_howling_mic_proc_cb(void)
{
    mic_pcm_t *rptr = gtcrn_qmf_howling_proc_ptr;
#if GTCRN_QMF_HOWLING_INFO_PRINT
    info_printf();
#endif
#if UARTDUMP_GTCRN_QMF_HOWLING_EN
    s16 *ptr_temp = (s16 *) UartDump_buf_temp;
    dump_dma_wait();
    dump_put2ram(&UartDump_buf[0][0],rptr,FRAME_LEN*2,0); //算法前数据组包

    dump_put2ram(&UartDump_buf[1][0],ptr_temp,FRAME_LEN*2,1);//算法后数据组包
    dump_dma_kick(&UartDump_buf[0][0], (FRAME_LEN*2 + 14) * 2); //最后再一起吐数据，避免前后kick数据打架
#endif

    gtcrn_48k_qmf_10ms_howling_512_process(rptr);

#if UARTDUMP_GTCRN_QMF_HOWLING_EN
    for(int i = 0; i < FRAME_LEN; i++) {
        ptr_temp[i]  = rptr[i];
    }
#endif

    gtcrn_qmf_howling_mic_cfg.kick_proc_done--;
}

AT(.text.gtcrn_qmf_howling_set)
void gtcrn_qmf_howling_mic_output_callback_set(audio_callback_t callback)
{
    gtcrn_qmf_howling_mic_cfg.callback = callback;
}

AT(.text.gtcrn_qmf_howling_init)
void gtcrn_qmf_howling_mic_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset((uint8_t *)&gtcrn_qmf_howling_mic_cfg, 0, sizeof(gtcrn_qmf_howling_mic_cfg));
    tog_buf_init(&gtcrn_qmf_howling_tbuf, gtcrn_qmf_howling_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));

//    ///算法初始化处先mute，等连上无线麦再打开/接收端发送使能再打开
//    gtcrn_qmf_howling_mic_mute_set(1);
    gtcrn_qmf_howling_mic_param_init();

#if UARTDUMP_GTCRN_QMF_HOWLING_EN
    memset(dump_head_buf, 0, 14*2);
    memset(UartDump_buf, 0, 2*(14 + FRAME_LEN*2));

    huart_module_init();    //初始化高速串口模块
    dump_buf_init(dump_head_buf,2, huart_putbuf, huart_wait_tx_finish);
#endif
}

AT(.text.gtcrn_qmf_howling_exit)
void gtcrn_qmf_howling_mic_exit(void)
{

}

AT(.text.gtcrn_qmf_howling_set.param)
void gtcrn_qmf_howling_mic_param_init()
{
    memset((uint8_t *)&gtcrn_qmf_howling_cb, 0, sizeof(gtcrn_qmf_howling_cb));
    gtcrn_48k_qmf_10ms_cb_t *p = &gtcrn_qmf_howling_cb;
	p->overdrive			= 32768;
	p->denoiseBound			= 0;
	p->smooth_en			= 1;
	p->modelUpdatePars0		= 2;
	p->prior_opt_idx		= 10;
	p->prior_opt_ada_en		= 1;
	p->quan_gap				= 3277;
	p->quan_gap_low_len		= 6;
	p->quan_gap_low			= 3277;
	p->lquantile_sm			= 26214;
	p->factor				= 30*32768;
	p->delta_k_up		    = 0;
	p->sin_dnn_en			= 1;
	p->sin_all_en			= 0;
	p->sin_all_len			= 0;
	p->gain_assign			= 26666;
	p->low_noise_range		= 26;
	p->spp_fre_p			= 8000;
	p->spp_fre_len			= 16;
	p->high_gain_len        = 64;
	p->howling_enh_en		= 0;
	p->howling_thr_low		= 8192;
	p->howling_thr_hi		= 1024;

	p->change_range			= 8192*3;
	p->ang_cg_mode			= 3;
	p->fs					= 48000;
	p->freq_step			= 1000;
	p->warping_coef			= 29491;

	// time domain post process to simulate nonlinear distortion
	p->trad_position		= 1;
	p->process_type			= 0; // 0:������, 1:tanh, 2:exponential saturation
	p->tanh_threshold       = 54613;

	p->gain_thr_en			= 0;
	p->gain_thr				= 20000;

    gtcrn_48k_qmf_10ms_howling_512_init(p);

}

AT(.text.gtcrn_qmf_howling_set.mute)
void gtcrn_qmf_howling_mic_mute_set(uint8_t mute)
{
    gtcrn_qmf_howling_mic_cfg.mute = mute;
    if (mute) {
        while(gtcrn_qmf_howling_mic_cfg.kick_proc_done){
            printf("#");
        }
        npu_exit();
        tog_buf_init(&gtcrn_qmf_howling_tbuf, gtcrn_qmf_howling_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));
    }
}

AT(.text.gtcrn_qmf_howling_get.mute)
uint8_t gtcrn_qmf_howling_mic_mute_get(void)
{
    return gtcrn_qmf_howling_mic_cfg.mute;
}
#else
void gtcrn_qmf_howling_mic_proc_cb(void){}
#endif
