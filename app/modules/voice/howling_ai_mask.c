#include "include.h"
#include "api_alg.h"
#include "howling_ai_mask.h"

/*
 * 文件名称: howling_ai_mask_api.c
 * 功能描述: 本文件为软件howling_ai_mask处理模块
    AT(.buf.howling_ai_mask);
    AT(.rodata.howling_ai_mask)
    AT(.text.howling_ai_mask_proc)
    AT(.text.howling_ai_mask_init)

 ****************************************************************************************
    code + rodata : 5K
    buf           : 16K
    time          : 2.5ms
 */

#if HOWLING_AI_MASK_EN

#define UARTDUMP_AI_MASK_EN          0                         //DUMP数据一键配置，需要使用定制上位机
#define HOWLING_AI_MASK_RDFT_HW           1                         //是否使用硬件FFT处理,软件fft底层处理若是关闭则无法编译通过
#define FRAME_LEN		        240                       //算法处理帧长
#define PROCESS_OUT_SAMPLES     120                       //每次存取帧长
s16 howling_ai_mask_pingpangcache_temp[FRAME_LEN*2] AT(.buf.howling_ai_mask); //乒乓buf缓存
u8 howling_ai_mask_out_buf[PROCESS_OUT_SAMPLES*2] AT(.buf.howling_ai_mask);   //输出buf中转缓存
WEAK volatile u8 howling_ai_mask_flag = 0;                           //乒乓buf 切换标志位
uint8_t howling_ai_mask_in_cnt = 0;                                 //写指针
uint8_t howling_ai_mask_out_cnt = 0;                                //读指针

static dnn_cb_t howling_ai_mask_cb AT(.buf.howling_ai_mask);
static howling_ai_mask_mic_cfg_t howling_ai_mask_mic_cfg AT(.buf.howling_ai_mask);

#if HOWLING_AI_MASK_RDFT_HW
typedef struct{
    fft_cfg_t fft_cft;
    ifft_cfg_t ifft_cft;
    s32 howling_ai_mask_fft_in[512];
    s32 howling_ai_mask_fft_out[512];
    s32 howling_ai_mask_ifft_out[512];
}howling_ai_mask_rdft_t;
static howling_ai_mask_rdft_t howling_ai_mask_rdft AT(.data.howling_ai_mask);
#endif

#if UARTDUMP_AI_MASK_EN
static u8 dump_head_buf[14*2]AT(.buf.howling_ai_mask);

static u8 UartDump_buf[2][14 + FRAME_LEN*2]AT(.buf.howling_ai_mask);
static u8 UartDump_buf_temp[14 + FRAME_LEN*2]AT(.buf.howling_ai_mask);
#endif

AT(.text.howling_ai_mask_proc)WEAK
void howling_ai_mask_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    if (!howling_ai_mask_mic_cfg.mute && wireless_get_status()) {
        uint8_t process_num = samples/PROCESS_OUT_SAMPLES;
        s16 *process_pcm_ptr = (s16 *)ptr;
        if (process_num*PROCESS_OUT_SAMPLES != samples) {
            printf("howling_ai_mask input samples err\n");
            return;
        }

        while (process_num) {
            ///OUT取 PING-PONG BUFFER
            memcpy(howling_ai_mask_out_buf,&howling_ai_mask_pingpangcache_temp[howling_ai_mask_out_cnt * PROCESS_OUT_SAMPLES],PROCESS_OUT_SAMPLES*2);
            howling_ai_mask_out_cnt++;
            if(howling_ai_mask_out_cnt>=FRAME_LEN*2/PROCESS_OUT_SAMPLES){
                howling_ai_mask_out_cnt = 0;
            }
            ///IN存 PING-PONG BUFFER
            memcpy(&howling_ai_mask_pingpangcache_temp[howling_ai_mask_in_cnt * PROCESS_OUT_SAMPLES], process_pcm_ptr, PROCESS_OUT_SAMPLES * 2);
            howling_ai_mask_in_cnt++;
            ///kick 低优先级线程处理
            if(howling_ai_mask_in_cnt == FRAME_LEN/PROCESS_OUT_SAMPLES){
                howling_ai_mask_flag = 1;
                ///发射端处理kick 低现场 ，接收端处理攒够点数直接处理
                if (!wireless_role_is_adapter()) {//device
//                    howling_ai_mask_mic_proc_kick_start();
                }else{//adapter
                    howling_ai_mask_mic_proc_cb();
                }
            }else if(howling_ai_mask_in_cnt >= FRAME_LEN*2/PROCESS_OUT_SAMPLES){
                howling_ai_mask_in_cnt = 0;
                howling_ai_mask_flag = 2;
                ///发射端处理kick 低现场 ，接收端处理攒够点数直接处理
                if (!wireless_role_is_adapter()) {//device
//                    howling_ai_mask_mic_proc_kick_start();
                }else{//adapter
                    howling_ai_mask_mic_proc_cb();
                }
            }
            memcpy(process_pcm_ptr,howling_ai_mask_out_buf,PROCESS_OUT_SAMPLES*2);
            /// break while()
            process_num--;

            if (howling_ai_mask_mic_cfg.callback) {
                howling_ai_mask_mic_cfg.callback((void *)ptr, samples, ch_mode, params);
            }
        }

    }else{

        if (howling_ai_mask_mic_cfg.callback) {
            howling_ai_mask_mic_cfg.callback((void *)ptr, samples, ch_mode, params);
        }
    }

}

//howling_ai_mask算法启动计算 放在低优先级现场处理
AT(.text.howling_ai_mask_proc)WEAK
void howling_ai_mask_mic_proc_cb(void)
{
    s16 *ptr = &howling_ai_mask_pingpangcache_temp[0];
    if(howling_ai_mask_flag == 2){
        ptr = &howling_ai_mask_pingpangcache_temp[FRAME_LEN];
    }
#if UARTDUMP_AI_MASK_EN
    s16 *ptr_temp = (s16 *) UartDump_buf_temp;
    dump_dma_wait();
    dump_put2ram(&UartDump_buf[0][0],ptr,FRAME_LEN*2,0); //算法前数据组包

    dump_put2ram(&UartDump_buf[1][0],ptr_temp,FRAME_LEN*2,1);//算法后数据组包
    dump_dma_kick(&UartDump_buf[0][0], (FRAME_LEN*2 + 14) * 2); //最后再一起吐数据，避免前后kick数据打架
#endif

#if HOWLING_AI_MASK_RDFT_HW
//    dnn_process_ext_window(ptr,NULL,0,howling_ai_mask_rdft.howling_ai_mask_fft_in);
//    fft_hw(&howling_ai_mask_rdft.fft_cft);
//    dnn_fre_process(howling_ai_mask_rdft.howling_ai_mask_fft_out, 0, 0);
//    ifft_hw(&howling_ai_mask_rdft.ifft_cft);
//    dnn_process_ext_next(howling_ai_mask_rdft.howling_ai_mask_ifft_out,NULL,0,ptr);
#if UARTDUMP_AI_MASK_EN
    for(int i = 0; i < FRAME_LEN; i++) {
        ptr_temp[i]  = ptr[i];
    }
#endif // UARTDUMP_AI_MASK_EN
#endif

    howling_ai_mask_flag = 0;

}

AT(.text.howling_ai_mask_set)WEAK
void howling_ai_mask_mic_output_callback_set(audio_callback_t callback)
{
    howling_ai_mask_mic_cfg.callback = callback;
}

AT(.text.howling_ai_mask_init)WEAK
void howling_ai_mask_mic_init(u8 sample_rate, u16 samples, u8 channel)
{
#if HOWLING_AI_MASK_RDFT_HW
    howling_ai_mask_rdft.fft_cft.size            = RDFT_512;
	howling_ai_mask_rdft.fft_cft.input_type      = 1;
    howling_ai_mask_rdft.fft_cft.window_en       = 1;
    howling_ai_mask_rdft.fft_cft.isr_en          = 0;
    howling_ai_mask_rdft.fft_cft.in_addr         = howling_ai_mask_rdft.howling_ai_mask_fft_in;
    howling_ai_mask_rdft.fft_cft.out_addr        = howling_ai_mask_rdft.howling_ai_mask_fft_out;

    howling_ai_mask_rdft.ifft_cft.size           = RDFT_512;
    howling_ai_mask_rdft.ifft_cft.output_type    = 1;
    howling_ai_mask_rdft.ifft_cft.window_en      = 1;
    howling_ai_mask_rdft.ifft_cft.isr_en         = 0;
    howling_ai_mask_rdft.ifft_cft.overlap_en     = 1;
    howling_ai_mask_rdft.ifft_cft.overlap_len    = 0;
    howling_ai_mask_rdft.ifft_cft.in_addr        = howling_ai_mask_rdft.howling_ai_mask_fft_out;
    howling_ai_mask_rdft.ifft_cft.out_addr       = howling_ai_mask_rdft.howling_ai_mask_ifft_out;

    memset(howling_ai_mask_rdft.howling_ai_mask_fft_in, 0, sizeof(howling_ai_mask_rdft.howling_ai_mask_fft_in));
    memset(howling_ai_mask_rdft.howling_ai_mask_fft_out, 0, sizeof(howling_ai_mask_rdft.howling_ai_mask_fft_out));
    memset(howling_ai_mask_rdft.howling_ai_mask_ifft_out, 0, sizeof(howling_ai_mask_rdft.howling_ai_mask_ifft_out));
#endif

#if UARTDUMP_AI_MASK_EN
    memset(dump_head_buf, 0, 14*2);
    memset(UartDump_buf, 0, 2*(14 + FRAME_LEN*2));

    huart_module_init();    //初始化高速串口模块
    dump_buf_init(dump_head_buf,2, huart_putbuf, huart_wait_tx_finish);
#endif

//    memset((uint8_t *)&howling_ai_mask_cb, 0, sizeof(howling_ai_mask_cb));
    memset((uint8_t *)&howling_ai_mask_mic_cfg, 0, sizeof(howling_ai_mask_mic_cfg));

    memset(howling_ai_mask_pingpangcache_temp, 0, FRAME_LEN*2*2);
    memset(howling_ai_mask_out_buf, 0, PROCESS_OUT_SAMPLES*2);


}

AT(.text.howling_ai_mask_exit)WEAK
void howling_ai_mask_mic_exit(void)
{

}

AT(.text.howling_ai_mask_set.param)WEAK
void howling_ai_mask_mic_param_set(int8_t howling_ai_mask_nt)
{

}

AT(.text.howling_ai_mask_set.mute)WEAK
void howling_ai_mask_mic_mute_set(uint8_t mute)
{
    howling_ai_mask_mic_cfg.mute = mute;
}

AT(.text.howling_ai_mask_get.mute)WEAK
uint8_t howling_ai_mask_mic_mute_get(void)
{
    return howling_ai_mask_mic_cfg.mute;
}

#endif
