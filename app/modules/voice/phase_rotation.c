#include "include.h"
#include "api_alg.h"
#include "phase_rotation.h"

/*
 * 文件名称: phase_rotation.c
 * 功能描述: 本文件为软件dnn_L1处理模块
    AT(.buf.phase_rotation);
    AT(.rodata.phase_rotation)
    AT(.text.phase_rotation_proc)
    AT(.text.phase_rotation_init)

    注意mic_pcm_t 实际配置类型

 ****************************************************************************************
    code + rodata :
    buf           :
    npu           ：
    time          :
 */

#if PHASE_ROTATION_EN

typedef s16                     mic_pcm_t;

#define UARTDUMP_PHASE_ROTATION_EN   0                           //DUMP数据一键配置，需要使用定制上位机
#define PHASE_ROTATION_INFO_PRINT    0
#define FRAME_LEN                    120                         //算法处理帧长
#define PROCESS_OUT_SAMPLES          120                         //每次存取帧长

static struct tog_bug_tag phase_rotation_tbuf AT(.buf.phase_rotation);                    //乒乓buf控制
static mic_pcm_t phase_rotation_cache_buf[FRAME_LEN*2] AT(.buf.phase_rotation);           //乒乓buf缓存
//static mic_pcm_t phase_rotation_tmp_buf[PROCESS_OUT_SAMPLES] AT(.buf.phase_rotation);     //输出buf中转缓存
static void *phase_rotation_proc_ptr = phase_rotation_cache_buf;

//static phase_rotation_cb_t phase_rotation_cb AT(.buf.phase_rotation);
static phase_rotation_mic_cfg_t phase_rotation_mic_cfg AT(.buf.phase_rotation);

#if UARTDUMP_PHASE_ROTATION_EN
static u8 dump_head_buf[14*2] AT(.buf.phase_rotation);
static u8 UartDump_buf[2][14 + FRAME_LEN*2] AT(.buf.phase_rotation);
static u8 UartDump_buf_temp[14 + FRAME_LEN*2] AT(.buf.phase_rotation);
//static u8 huart_buffer_rx[128] AT(.buf.phase_rotation);
#endif

#if PHASE_ROTATION_INFO_PRINT
AT(.com_text.phase_rotation)
const char phase_rotation_info[] = "PHASE_ROTATION_EN samples = %d, isrcnt = %d (SR_%d) %d\n";

AT(.text.phase_rotation_proc)
static void info_printf(void)
{
    static u32 ticks = 0;
    static u32 isr_cnt = 0;
    isr_cnt++;
    if (tick_check_expire(ticks,1000)) {
        my_printf(phase_rotation_info, FRAME_LEN, isr_cnt, FRAME_LEN*isr_cnt, sizeof(mic_pcm_t));
        isr_cnt = 0;
        ticks = tick_get();
    }
}
#endif

AT(.text.phase_rotation_proc)
void phase_rotation_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    if (!phase_rotation_mic_cfg.mute && wireless_cb.alg_en) {

//        while(samples > 0) {
//            uint rlen = (samples > PROCESS_OUT_SAMPLES)? PROCESS_OUT_SAMPLES : samples;
//            if(tog_buf_get(&phase_rotation_tbuf, (u8 *)phase_rotation_tmp_buf, rlen*sizeof(mic_pcm_t))) {
//                tog_buf_rd_toggle(&phase_rotation_tbuf);
//            }
//
//            // 填充block, true表示算法攒帧完毕, kick低优先级线程处理算法
//            if(tog_buf_put(&phase_rotation_tbuf, ptr, rlen*sizeof(mic_pcm_t))) {
//                phase_rotation_proc_ptr = tog_bug_get_w_block(&phase_rotation_tbuf);
//                tog_buf_wr_toggle(&phase_rotation_tbuf);
//                phase_rotation_mic_cfg.kick_proc_done++;
//                phase_rotation_mic_proc_kick_start();
//            }
//
//            memcpy(ptr, phase_rotation_tmp_buf, rlen*sizeof(mic_pcm_t));
//            if(phase_rotation_mic_cfg.callback) {
//                phase_rotation_mic_cfg.callback((void *)ptr, rlen, ch_mode, params);
//            }
//
//            samples -= rlen;
//        }
        s16 *rptr = (s16 *)ptr;

        audio_phase_rot2_process(rptr, 120, 0);

        if(phase_rotation_mic_cfg.callback) {
            phase_rotation_mic_cfg.callback((void *)ptr, samples, ch_mode, params);
        }
    } else {
        if(phase_rotation_mic_cfg.callback) {
            phase_rotation_mic_cfg.callback((void *)ptr, samples, ch_mode, params);
        }
    }

}

//phase_rotation算法启动计算 放在低优先级现场处理
AT(.text.phase_rotation_proc)
void phase_rotation_mic_proc_cb(void)
{
    mic_pcm_t *rptr = phase_rotation_proc_ptr;
#if PHASE_ROTATION_INFO_PRINT
    info_printf();
#endif

#if UARTDUMP_PHASE_ROTATION_EN
    s16 *ptr_temp = (s16 *) UartDump_buf_temp;
    dump_dma_wait();
    dump_put2ram(&UartDump_buf[0][0],rptr,FRAME_LEN*2,0); //算法前数据组包

    dump_put2ram(&UartDump_buf[1][0],ptr_temp,FRAME_LEN*2,1);//算法后数据组包
    dump_dma_kick(&UartDump_buf[0][0], (FRAME_LEN*2 + 14) * 2); //最后再一起吐数据，避免前后kick数据打架
#endif

    audio_phase_rot2_process(rptr, 120, 0);

#if UARTDUMP_PHASE_ROTATION_EN
    for (int i = 0; i < FRAME_LEN; i++) {
        ptr_temp[i]  = rptr[i];
    }
#endif

    phase_rotation_mic_cfg.kick_proc_done--;
}

AT(.text.phase_rotation_set)
void phase_rotation_mic_output_callback_set(audio_callback_t callback)
{
    phase_rotation_mic_cfg.callback = callback;
}

AT(.text.phase_rotation_init)
void phase_rotation_mic_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset((uint8_t *)&phase_rotation_mic_cfg, 0, sizeof(phase_rotation_mic_cfg));
    tog_buf_init(&phase_rotation_tbuf, phase_rotation_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));

    phase_rotation_mic_param_set(0);

//    ///算法初始化处先mute，等连上无线麦再打开/接收端发送使能再打开
//    phase_rotation_mic_mute_set(1);

#if UARTDUMP_PHASE_ROTATION_EN
    memset(dump_head_buf, 0, 14*2);
    memset(UartDump_buf, 0, 2*(14 + FRAME_LEN*2));

    huart_module_init();    //初始化高速串口模块
    dump_buf_init(dump_head_buf,2, huart_putbuf, huart_wait_tx_finish);
#endif
}

AT(.text.phase_rotation_exit)
void phase_rotation_mic_exit(void)
{

}

AT(.text.phase_rotation_set.param)
void phase_rotation_mic_param_set(s16 phase_rotation_nt)
{
//    memset((uint8_t *)&phase_rotation_cb, 0, sizeof(phase_rotation_cb));
    audio_phase_rot2_init(0);

}

AT(.text.phase_rotation_set.mute)
void phase_rotation_mic_mute_set(uint8_t mute)
{
    phase_rotation_mic_cfg.mute = mute;
    if (mute) {
        while (phase_rotation_mic_cfg.kick_proc_done){
            printf("#");
        }
        tog_buf_init(&phase_rotation_tbuf, phase_rotation_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));
    }
}

AT(.text.phase_rotation_get.mute)
uint8_t phase_rotation_mic_mute_get(void)
{
    return phase_rotation_mic_cfg.mute;
}
#else
void phase_rotation_mic_proc_cb(void){}
#endif
