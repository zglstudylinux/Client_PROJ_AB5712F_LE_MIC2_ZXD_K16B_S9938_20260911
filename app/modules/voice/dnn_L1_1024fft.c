#include "include.h"
#include "api_alg.h"
#include "dnn_L1_1024fft.h"

/*
 * 文件名称: dnn_L1_api.c
 * 功能描述: 本文件为软件dnn_L1处理模块
    AT(.buf.dnn_48k_1024fft);
    AT(.rodata.dnn_L1)
    AT(.text.dnn_L1_proc)
    AT(.text.dnn_L1_init)

 ****************************************************************************************
    code + rodata : 7k
    buf           : 20K
    npu           ：100k
    time          : 5.3ms
 */

#if DNN_L1_1024FFT_EN

#define UARTDUMP_DNN_EN          0                         //DUMP数据一键配置，需要使用定制上位机
#define DNN_L1_RDFT_HW           0                         //是否使用硬件FFT处理,软件fft底层处理若是关闭则无法编译通过
#define FRAME_LEN		        480                       //算法处理帧长
#define PROCESS_OUT_SAMPLES     120                       //每次存取帧长
s16 dnn_L1_1024fft_pingpangcache_temp[FRAME_LEN*2] AT(.buf.dnn_48k_1024fft); //乒乓buf缓存
u8 dnn_L1_1024fft_out_buf[PROCESS_OUT_SAMPLES*2] AT(.buf.dnn_48k_1024fft);   //输出buf中转缓存
WEAK volatile u8 dnn_L1_1024fft_flag = 0;                           //乒乓buf 切换标志位
uint8_t dnn_L1_1024fft_in_cnt = 0;                                 //写指针
uint8_t dnn_L1_1024fft_out_cnt = 0;                                //读指针

static dnn_48k_cb_t dnn_L1_1024fft_cb AT(.buf.dnn_48k_1024fft);
static dnn_L1_1024fft_mic_cfg_t dnn_L1_1024fft_mic_cfg AT(.buf.dnn_48k_1024fft);


#if UARTDUMP_DNN_EN
static u8 dump_head_buf[14*2]AT(.buf.dnn_48k_1024fft);

static u8 UartDump_buf[2][14 + FRAME_LEN*2]AT(.buf.dnn_48k_1024fft);
static u8 UartDump_buf_temp[14 + FRAME_LEN*2]AT(.buf.dnn_48k_1024fft);
#endif
void source_data_in(u8 *ptr,u8 len);
AT(.text.dnn_L1_1024fft_proc)WEAK
void dnn_L1_1024fft_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
#if UARTDUMP_DNN_EN
	if (!dnn_L1_1024fft_mic_cfg.mute && 1) {
#else
    if (!dnn_L1_1024fft_mic_cfg.mute && wireless_get_status()) {
#endif
        uint8_t process_num = samples/PROCESS_OUT_SAMPLES;
        s16 *process_pcm_ptr = (s16 *)ptr;
        if (process_num*PROCESS_OUT_SAMPLES != samples) {
            printf("dnn_L1_1024fft input samples err\n");
            return;
        }

        while (process_num) {
            ///OUT取 PING-PONG BUFFER
            memcpy(dnn_L1_1024fft_out_buf,&dnn_L1_1024fft_pingpangcache_temp[dnn_L1_1024fft_out_cnt * PROCESS_OUT_SAMPLES],PROCESS_OUT_SAMPLES*2);
            dnn_L1_1024fft_out_cnt++;
            if(dnn_L1_1024fft_out_cnt>=FRAME_LEN*2/PROCESS_OUT_SAMPLES){
                dnn_L1_1024fft_out_cnt = 0;
            }
            ///IN存 PING-PONG BUFFER
            memcpy(&dnn_L1_1024fft_pingpangcache_temp[dnn_L1_1024fft_in_cnt * PROCESS_OUT_SAMPLES], process_pcm_ptr, PROCESS_OUT_SAMPLES * 2);
            dnn_L1_1024fft_in_cnt++;
            ///kick 低优先级线程处理
            if(dnn_L1_1024fft_in_cnt == FRAME_LEN/PROCESS_OUT_SAMPLES){
                dnn_L1_1024fft_flag = 1;
                ///发射端处理kick 低现场 ，接收端处理攒够点数直接处理
                if (!wireless_role_is_adapter()) {//device
                    dnn_L1_1024fft_mic_proc_kick_start();
                }else{//adapter
                    dnn_L1_1024fft_mic_proc_cb();
                }
            }else if(dnn_L1_1024fft_in_cnt >= FRAME_LEN*2/PROCESS_OUT_SAMPLES){
                dnn_L1_1024fft_in_cnt = 0;
                dnn_L1_1024fft_flag = 2;
                ///发射端处理kick 低现场 ，接收端处理攒够点数直接处理
                if (!wireless_role_is_adapter()) {//device
                    dnn_L1_1024fft_mic_proc_kick_start();
                }else{//adapter
                    dnn_L1_1024fft_mic_proc_cb();
                }
            }
            memcpy(process_pcm_ptr,dnn_L1_1024fft_out_buf,PROCESS_OUT_SAMPLES*2);
            /// break while()
            process_num--;

            if (dnn_L1_1024fft_mic_cfg.callback) {
                dnn_L1_1024fft_mic_cfg.callback((void *)ptr, samples, ch_mode, params);
            }
        }

    }else{

        if (dnn_L1_1024fft_mic_cfg.callback) {
            dnn_L1_1024fft_mic_cfg.callback((void *)ptr, samples, ch_mode, params);
        }
    }

}

//dnn_L1_1024fft算法启动计算 放在低优先级现场处理
AT(.text.dnn_L1_1024fft_proc)WEAK
void dnn_L1_1024fft_mic_proc_cb(void)
{
    s16 *ptr = &dnn_L1_1024fft_pingpangcache_temp[0];
    if(dnn_L1_1024fft_flag == 2){
        ptr = &dnn_L1_1024fft_pingpangcache_temp[FRAME_LEN];
    }
#if UARTDUMP_DNN_EN
    s16 *ptr_temp = (s16 *) UartDump_buf_temp;
    dump_dma_wait();
    dump_put2ram(&UartDump_buf[0][0],ptr,FRAME_LEN*2,0); //算法前数据组包

    dump_put2ram(&UartDump_buf[1][0],ptr_temp,FRAME_LEN*2,1);//算法后数据组包
    dump_dma_kick(&UartDump_buf[0][0], (FRAME_LEN*2 + 14) * 2); //最后再一起吐数据，避免前后kick数据打架
#endif

#if DNN_L1_RDFT_HW
    dnn_process_ext_window(ptr,NULL,0,dnn_L1_rdft.dnn_L1_fft_in);
    fft_hw(&dnn_L1_rdft.fft_cft);
    dnn_fre_process(dnn_L1_rdft.dnn_L1_fft_out, 0, 0);
    ifft_hw(&dnn_L1_rdft.ifft_cft);
    dnn_process_ext_next(dnn_L1_rdft.dnn_L1_ifft_out,NULL,0,ptr);
#if UARTDUMP_DNN_EN
    for(int i = 0; i < FRAME_LEN; i++) {
        ptr_temp[i]  = ptr[i];
    }
#endif // UARTDUMP_DNN_EN

#else

  //  GPIOASET |= BIT(6);
    dnn_48k_ns_process(ptr);
#if UARTDUMP_DNN_EN
	for(int i = 0; i < FRAME_LEN; i++) {
	  ptr_temp[i]  = ptr[i];
	}
#endif // UARTDUMP_DNN_EN


  //  GPIOACLR |= BIT(6);

#endif

//#if UARTDUMP_DNN_EN
//    dump_put2ram(&UartDump_buf[1][0],ptr,FRAME_LEN*2,1);//算法后数据组包
//    dump_dma_kick(&UartDump_buf[0][0], (FRAME_LEN*2 + 14) * 2); //最后再一起吐数据，避免前后kick数据打架
//#endif
//    memset(dnn_L1_rdft.dnn_L1_fft_in,0,256*2);
//    memcpy(dnn_L1_rdft.dnn_L1_fft_in,ptr,FRAME_LEN*2);
//    for(i = 0; i < 240; i++) {
//        dnn_L1_rdft.dnn_L1_fft_in[i] = ptr[i];
//    }
//    fft_hw(&dnn_L1_rdft.fft_cft);
//
//    dnn_fre_process((s32 *)dnn_L1_rdft.dnn_L1_fft_out,0,0);
//
//    ifft_hw(&dnn_L1_rdft.ifft_cft);
//
//    for(i = 0; i < 240; i++) {
//        ptr[i] = (s16)dnn_L1_rdft.dnn_L1_ifft_out[i];
//    }

    dnn_L1_1024fft_flag = 0;

}

AT(.text.dnn_L1_1024fft_set)WEAK
void dnn_L1_1024fft_mic_output_callback_set(audio_callback_t callback)
{
    dnn_L1_1024fft_mic_cfg.callback = callback;
}

AT(.text.dnn_L1_1024fft_init)WEAK
void dnn_L1_1024fft_mic_init(u8 sample_rate, u16 samples, u8 channel)
{
    my_printf("%s %d\n",__func__,__LINE__);
#if DNN_L1_RDFT_HW
    dnn_L1_rdft.fft_cft.size            = RDFT_512;
	dnn_L1_rdft.fft_cft.input_type      = 1;
    dnn_L1_rdft.fft_cft.window_en       = 1;
    dnn_L1_rdft.fft_cft.isr_en          = 0;
    dnn_L1_rdft.fft_cft.in_addr         = dnn_L1_rdft.dnn_L1_fft_in;
    dnn_L1_rdft.fft_cft.out_addr        = dnn_L1_rdft.dnn_L1_fft_out;

    dnn_L1_rdft.ifft_cft.size           = RDFT_512;
    dnn_L1_rdft.ifft_cft.output_type    = 1;
    dnn_L1_rdft.ifft_cft.window_en      = 1;
    dnn_L1_rdft.ifft_cft.isr_en         = 0;
    dnn_L1_rdft.ifft_cft.overlap_en     = 1;
    dnn_L1_rdft.ifft_cft.overlap_len    = 0;
    dnn_L1_rdft.ifft_cft.in_addr        = dnn_L1_rdft.dnn_L1_fft_out;
    dnn_L1_rdft.ifft_cft.out_addr       = dnn_L1_rdft.dnn_L1_ifft_out;

    memset(dnn_L1_rdft.dnn_L1_fft_in, 0, sizeof(dnn_L1_rdft.dnn_L1_fft_in));
    memset(dnn_L1_rdft.dnn_L1_fft_out, 0, sizeof(dnn_L1_rdft.dnn_L1_fft_out));
    memset(dnn_L1_rdft.dnn_L1_ifft_out, 0, sizeof(dnn_L1_rdft.dnn_L1_ifft_out));
#endif

#if UARTDUMP_DNN_EN
    memset(dump_head_buf, 0, 14*2);
    memset(UartDump_buf, 0, 2*(14 + FRAME_LEN*2));

    huart_module_init();    //初始化高速串口模块
    dump_buf_init(dump_head_buf,2, huart_putbuf, huart_wait_tx_finish);
#endif

    memset((uint8_t *)&dnn_L1_1024fft_cb, 0, sizeof(dnn_L1_1024fft_cb));
    memset((uint8_t *)&dnn_L1_1024fft_mic_cfg, 0, sizeof(dnn_L1_1024fft_mic_cfg));

    memset(dnn_L1_1024fft_pingpangcache_temp, 0, FRAME_LEN*2*2);
    memset(dnn_L1_1024fft_out_buf, 0, PROCESS_OUT_SAMPLES*2);


    dnn_L1_1024fft_cb.overdrive			= 32768;
	dnn_L1_1024fft_cb.denoiseBound			= 3000;
	dnn_L1_1024fft_cb.smooth_en			= 1;
	dnn_L1_1024fft_cb.modelUpdatePars0		= 2;
	dnn_L1_1024fft_cb.quan_gap				= 3277;
	dnn_L1_1024fft_cb.quan_gap_low_len		= 6;
	dnn_L1_1024fft_cb.quan_gap_low			= 3277;
	dnn_L1_1024fft_cb.lquantile_sm			= 26214;
	dnn_L1_1024fft_cb.factor				= 30*32768;
	dnn_L1_1024fft_cb.delta_k_up		    = 0;
	dnn_L1_1024fft_cb.sin_dnn_en			= 1;
	dnn_L1_1024fft_cb.sin_all_en			= 0;
	dnn_L1_1024fft_cb.sin_all_len			= 0;
	dnn_L1_1024fft_cb.prior_opt_idx		= 10;
	dnn_L1_1024fft_cb.low_noise_range		= 16;
	dnn_L1_1024fft_cb.spp_fre_p			= 128;

    dnn_48k_ns_init(&dnn_L1_1024fft_cb);
}

AT(.text.dnn_L1_1024fft_exit)WEAK
void dnn_L1_1024fft_mic_exit(void)
{

}

AT(.text.dnn_L1_1024fft_set.param)WEAK
void dnn_L1_mic_param_set(int8_t dnn_L1_1024fft_nt)
{
    dnn_L1_1024fft_cb.overdrive			= 32768;
	dnn_L1_1024fft_cb.denoiseBound			= 3000; //dnn_L1_1024fft_nt;
	dnn_L1_1024fft_cb.smooth_en			= 1;
	dnn_L1_1024fft_cb.modelUpdatePars0		= 2;
	dnn_L1_1024fft_cb.quan_gap				= 3277;
	dnn_L1_1024fft_cb.quan_gap_low_len		= 6;
	dnn_L1_1024fft_cb.quan_gap_low			= 3277;
	dnn_L1_1024fft_cb.lquantile_sm			= 26214;
	dnn_L1_1024fft_cb.factor				= 30*32768;
	dnn_L1_1024fft_cb.delta_k_up		    = 0;
	dnn_L1_1024fft_cb.sin_dnn_en			= 1;
	dnn_L1_1024fft_cb.sin_all_en			= 0;
	dnn_L1_1024fft_cb.sin_all_len			= 0;
	dnn_L1_1024fft_cb.prior_opt_idx		= 10;
	dnn_L1_1024fft_cb.low_noise_range		= 16;
	dnn_L1_1024fft_cb.spp_fre_p			= 128;
    dnn_48k_ns_init(&dnn_L1_1024fft_cb);
}

AT(.text.dnn_L1_1024fft_set.mute)WEAK
void dnn_L1_1024fft_mic_mute_set(uint8_t mute)
{
    dnn_L1_1024fft_mic_cfg.mute = mute;
}

AT(.text.dnn_L1_1024fft_get.mute)WEAK
uint8_t dnn_L1_1024fft_mic_mute_get(void)
{
    return dnn_L1_1024fft_mic_cfg.mute;
}

#endif
