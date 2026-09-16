#include "include.h"
#include "api_alg.h"
#include "freq_shift2.h"

/*
 * 文件名称: freq_shift2.c
 * 功能描述: 本文件为软件dnn_L1处理模块
    AT(.buf.freq_shift2);
    AT(.rodata.freq_shift2)
    AT(.text.freq_shift2_proc)
    AT(.text.freq_shift2_init)

    注意mic_pcm_t 实际配置类型

 ****************************************************************************************
    code + rodata :
    buf           :
    npu           ：
    time          :
 */

#if FREQ_SHIFT2_EN
#define UARTDUMP_FREQ_SHIFT2_EN         0                          //DUMP数据一键配置，需要使用定制上位机
#define FREQ_SHIFT2_INFO_PRINT          0
#define FRAME_LEN                       120                         //算法处理帧长
#define PROCESS_OUT_SAMPLES             120                         //每次存取帧长

static struct tog_bug_tag freq_shift2_tbuf AT(.buf.freq_shift2);                    //乒乓buf控制
static mic_pcm_t freq_shift2_cache_buf[FRAME_LEN*2] AT(.buf.freq_shift2);           //乒乓buf缓存
static mic_pcm_t freq_shift2_tmp_buf[PROCESS_OUT_SAMPLES] AT(.buf.freq_shift2);     //输出buf中转缓存
static void *freq_shift2_proc_ptr = freq_shift2_cache_buf;


static freq_shift2_mic_cfg_t freq_shift2_mic_cfg AT(.buf.freq_shift2);

#if UARTDUMP_FREQ_SHIFT2_EN
static u8 dump_head_buf[14*2]AT(.buf.freq_shift2);
static u8 UartDump_buf[2][14 + FRAME_LEN*2]AT(.buf.freq_shift2);
static u8 UartDump_buf_temp[14 + FRAME_LEN*2]AT(.buf.freq_shift2);
#endif

#if FREQ_SHIFT2_INFO_PRINT
AT(.com_text.freq_shift2)
const char freq_shift2_info[] = "FREQ_SHIFT2_EN samples = %d, isrcnt = %d (SR_%d) %d\n";

AT(.text.freq_shift2_proc)
static void info_printf(void)
{
    static u32 ticks = 0;
    static u32 isr_cnt = 0;
    isr_cnt++;
    if(tick_check_expire(ticks,1000)) {
        my_printf(freq_shift2_info, FRAME_LEN, isr_cnt, FRAME_LEN*isr_cnt, sizeof(mic_pcm_t));
        isr_cnt = 0;
        ticks = tick_get();
    }
}
#endif

AT(.text.freq_shift2_proc)
void freq_shift2_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    if (!freq_shift2_mic_cfg.mute && wireless_cb.alg_en) {

//        while(samples > 0) {
//            uint rlen = (samples > PROCESS_OUT_SAMPLES)? PROCESS_OUT_SAMPLES : samples;
//            if(tog_buf_get(&freq_shift2_tbuf, (u8 *)freq_shift2_tmp_buf, rlen*sizeof(mic_pcm_t))) {
//                tog_buf_rd_toggle(&freq_shift2_tbuf);
//            }
//
//            // 填充block, true表示算法攒帧完毕, kick低优先级线程处理算法
//            if(tog_buf_put(&freq_shift2_tbuf, ptr, rlen*sizeof(mic_pcm_t))) {
//                freq_shift2_proc_ptr = tog_bug_get_w_block(&freq_shift2_tbuf);
//                tog_buf_wr_toggle(&freq_shift2_tbuf);
//                freq_shift2_mic_cfg.kick_proc_done++;
//                freq_shift2_mic_proc_kick_start();
//            }
//
//            memcpy(ptr, freq_shift2_tmp_buf, rlen*sizeof(mic_pcm_t));
//            if(freq_shift2_mic_cfg.callback) {
//                freq_shift2_mic_cfg.callback((void *)ptr, rlen, ch_mode, params);
//            }
//
//            samples -= rlen;
//        }
//        #if FREQ_SHIFT2_INFO_PRINT
//            info_printf();
//        #endif
        howlingex_process(ptr,ptr,samples);
        if(freq_shift2_mic_cfg.callback) {
            freq_shift2_mic_cfg.callback((void *)ptr, samples, ch_mode, params);
        }
    } else {
        if(freq_shift2_mic_cfg.callback) {
            freq_shift2_mic_cfg.callback((void *)ptr, samples, ch_mode, params);
        }
    }

}

//freq_shift2算法启动计算 放在低优先级现场处理
AT(.text.freq_shift2_proc)
void freq_shift2_mic_proc_cb(void)
{
    mic_pcm_t *rptr = freq_shift2_proc_ptr;
#if FREQ_SHIFT2_INFO_PRINT
    info_printf();
#endif
#if UARTDUMP_FREQ_SHIFT2_EN
    s16 *ptr_temp = (s16 *) UartDump_buf_temp;
    dump_dma_wait();
    dump_put2ram(&UartDump_buf[0][0],rptr,FRAME_LEN*2,0); //算法前数据组包

    dump_put2ram(&UartDump_buf[1][0],ptr_temp,FRAME_LEN*2,1);//算法后数据组包
    dump_dma_kick(&UartDump_buf[0][0], (FRAME_LEN*2 + 14) * 2); //最后再一起吐数据，避免前后kick数据打架
#endif

    howlingex_process(rptr,rptr,FRAME_LEN);

#if UARTDUMP_FREQ_SHIFT2_EN
    for(int i = 0; i < FRAME_LEN; i++) {
        ptr_temp[i]  = rptr[i];
    }
#endif

    freq_shift2_mic_cfg.kick_proc_done--;
}

AT(.text.freq_shift2_set)
void freq_shift2_mic_output_callback_set(audio_callback_t callback)
{
    freq_shift2_mic_cfg.callback = callback;
}

AT(.text.freq_shift2_init)
void freq_shift2_mic_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset((uint8_t *)&freq_shift2_mic_cfg, 0, sizeof(freq_shift2_mic_cfg));
    tog_buf_init(&freq_shift2_tbuf, freq_shift2_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));

    freq_shift2_mic_param_set(0);

//    ///算法初始化处先mute，等连上无线麦再打开/接收端发送使能再打开
//    freq_shift2_mic_mute_set(1);

#if UARTDUMP_FREQ_SHIFT2_EN
    memset(dump_head_buf, 0, 14*2);
    memset(UartDump_buf, 0, 2*(14 + FRAME_LEN*2));

    huart_module_init();    //初始化高速串口模块
    dump_buf_init(dump_head_buf,2, huart_putbuf, huart_wait_tx_finish);
#endif
}

AT(.text.freq_shift2_exit)
void freq_shift2_mic_exit(void)
{

}

AT(.text.freq_shift2_set.param)
void freq_shift2_mic_param_set(s16 freq_shift2_nt)
{
//    memset((uint8_t *)&freq_shift2_cb, 0, sizeof(freq_shift2_cb));
    holwing_ex_init(48000, 5, 5);
}

AT(.text.freq_shift2_set.mute)
void freq_shift2_mic_mute_set(uint8_t mute)
{
    freq_shift2_mic_cfg.mute = mute;
    if (mute) {
        while(freq_shift2_mic_cfg.kick_proc_done){
            printf("#");
        }
        tog_buf_init(&freq_shift2_tbuf, freq_shift2_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));
    }
}

AT(.text.freq_shift2_get.mute)
uint8_t freq_shift2_mic_mute_get(void)
{
    return freq_shift2_mic_cfg.mute;
}
#else
void freq_shift2_mic_proc_cb(void){}
#endif
