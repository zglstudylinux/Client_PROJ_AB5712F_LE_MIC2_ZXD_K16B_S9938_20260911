#include "include.h"
#include "api_alg.h"
#include "howling_dnn.h"

/*
 * 文件名称: howling_dnn_api.c
 * 功能描述: 本文件为软件howling_dnn处理模块
    AT(.buf.howling_dnn);
    AT(.rodata.howling_dnn)
    AT(.text.howling_dnn_proc)
    AT(.text.howling_dnn_init)

 ****************************************************************************************
    code + rodata : 0x6bb8 27576
    buf           : 0x2ee0  12000
    time          : 2.498ms (HW_MATH + 汇编)  ; 1.67ms (HW_MATH + 汇编 + HW_FFT)
 */

#if HOWLING_DNN_EN


#define UARTDUMP_HOWLING_DNN_EN      1                         //DUMP数据一键配置，需要使用定制上位机
#define HOWLING_DNN_INFO_PRINT       0
#define HOWLING_DNN_RDFT_HW          1                         //是否使用硬件FFT处理,软件fft底层处理若是关闭则无法编译通过
#define FRAME_LEN		             240                       //算法处理帧长
#define PROCESS_OUT_SAMPLES          120                       //每次存取帧长
s16 howling_dnn_pingpangcache_temp[FRAME_LEN*2] AT(.buf.howling_dnn); //乒乓buf缓存
u8 howling_dnn_out_buf[PROCESS_OUT_SAMPLES*2] AT(.buf.howling_dnn);   //输出buf中转缓存
WEAK volatile u8 howling_dnn_flag = 0;                           //乒乓buf 切换标志位
uint8_t howling_dnn_in_cnt = 0;                                 //写指针
uint8_t howling_dnn_out_cnt = 0;                                //读指针

static howling_dnn_cb_t howling_dnn_cb AT(.buf.howling_dnn);
static howling_dnn_mic_cfg_t howling_dnn_mic_cfg AT(.buf.howling_dnn);

#if HOWLING_DNN_RDFT_HW
typedef struct{
    fft_cfg_t fft_cft;
    ifft_cfg_t ifft_cft;
    s32 howling_dnn_fft_in[512];
    s32 howling_dnn_fft_out[512];
    s32 howling_dnn_ifft_out[512];
}howling_dnn_rdft_t;
static howling_dnn_rdft_t howling_dnn_rdft AT(.data.howling_dnn);
#endif
#if HOWLING_DNN_INFO_PRINT
AT(.com_text.ains4)
const char dnn_L3_info[] = "HOWLING_DNN samples = %d, isrcnt = %d (SR_%d)\n";
#endif
#if UARTDUMP_HOWLING_DNN_EN
static u8 dump_head_buf[14*2]AT(.buf.howling_dnn);

static u8 UartDump_buf[2][14 + FRAME_LEN*2]AT(.buf.howling_dnn);
static u8 UartDump_buf_temp[14 + FRAME_LEN*2]AT(.buf.howling_dnn);
#endif

AT(.text.howling_dnn_proc)WEAK
void howling_dnn_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    if (!howling_dnn_mic_cfg.mute && wireless_get_status()) {
        uint8_t process_num = samples/PROCESS_OUT_SAMPLES;
        s16 *process_pcm_ptr = (s16 *)ptr;
        if (process_num*PROCESS_OUT_SAMPLES != samples) {
            printf("howling_dnn input samples err\n");
            return;
        }

        while (process_num) {
            ///OUT取 PING-PONG BUFFER
            memcpy(howling_dnn_out_buf,&howling_dnn_pingpangcache_temp[howling_dnn_out_cnt * PROCESS_OUT_SAMPLES],PROCESS_OUT_SAMPLES*2);
            howling_dnn_out_cnt++;
            if(howling_dnn_out_cnt>=FRAME_LEN*2/PROCESS_OUT_SAMPLES){
                howling_dnn_out_cnt = 0;
            }
            ///IN存 PING-PONG BUFFER
            memcpy(&howling_dnn_pingpangcache_temp[howling_dnn_in_cnt * PROCESS_OUT_SAMPLES], process_pcm_ptr, PROCESS_OUT_SAMPLES * 2);
            howling_dnn_in_cnt++;
            ///kick 低优先级线程处理
            if(howling_dnn_in_cnt == FRAME_LEN/PROCESS_OUT_SAMPLES){
                howling_dnn_flag = 1;
                ///发射端处理kick 低现场 ，接收端处理攒够点数直接处理
                if (!wireless_role_is_adapter()) {//device
                    howling_dnn_mic_proc_kick_start();
                }else{//adapter
                    howling_dnn_mic_proc_cb();
                }
            }else if(howling_dnn_in_cnt >= FRAME_LEN*2/PROCESS_OUT_SAMPLES){
                howling_dnn_in_cnt = 0;
                howling_dnn_flag = 2;
                ///发射端处理kick 低现场 ，接收端处理攒够点数直接处理
                if (!wireless_role_is_adapter()) {//device
                    howling_dnn_mic_proc_kick_start();
                }else{//adapter
                    howling_dnn_mic_proc_cb();
                }
            }
            memcpy(process_pcm_ptr,howling_dnn_out_buf,PROCESS_OUT_SAMPLES*2);
            /// break while()
            process_num--;

            if (howling_dnn_mic_cfg.callback) {
                howling_dnn_mic_cfg.callback((void *)ptr, samples, ch_mode, params);
            }
        }

    }else{

        if (howling_dnn_mic_cfg.callback) {
            howling_dnn_mic_cfg.callback((void *)ptr, samples, ch_mode, params);
        }
    }

}

//howling_dnn算法启动计算 放在低优先级现场处理
AT(.text.howling_dnn_proc)WEAK
void howling_dnn_mic_proc_cb(void)
{
    s16 *ptr = &howling_dnn_pingpangcache_temp[0];
    if(howling_dnn_flag == 2){
        ptr = &howling_dnn_pingpangcache_temp[FRAME_LEN];
    }
#if UARTDUMP_HOWLING_DNN_EN
    s16 *ptr_temp = (s16 *) UartDump_buf_temp;
    dump_dma_wait();
    dump_put2ram(&UartDump_buf[0][0],ptr,FRAME_LEN*2,0); //算法前数据组包

    dump_put2ram(&UartDump_buf[1][0],ptr_temp,FRAME_LEN*2,1);//算法后数据组包
    dump_dma_kick(&UartDump_buf[0][0], (FRAME_LEN*2 + 14) * 2); //最后再一起吐数据，避免前后kick数据打架
#endif
#if HOWLING_DNN_INFO_PRINT
    static u32 ticks = 0;
    static u32 isr_cnt = 0;
    isr_cnt++;
    if (tick_check_expire(ticks,1000)) {
        my_printf(dnn_L3_info, FRAME_LEN, isr_cnt, FRAME_LEN*isr_cnt);
        isr_cnt = 0;
        ticks = tick_get();
    }
#endif

#if HOWLING_DNN_RDFT_HW
    howling_dnn_process_ext_window(ptr,NULL,0,howling_dnn_rdft.howling_dnn_fft_in);
    fft_hw(&howling_dnn_rdft.fft_cft);
    howling_dnn_fre_process(howling_dnn_rdft.howling_dnn_fft_out, 0, 0);
    ifft_hw(&howling_dnn_rdft.ifft_cft);
    howling_dnn_process_ext_next(howling_dnn_rdft.howling_dnn_ifft_out,NULL,0,ptr);
#else
    howling_dnn_process(ptr,NULL,0);
#endif

#if UARTDUMP_HOWLING_DNN_EN
    for(int i = 0; i < FRAME_LEN; i++) {
        ptr_temp[i]  = ptr[i];
    }
#endif // UARTDUMP_HOWLING_DNN_EN
    howling_dnn_flag = 0;

}

AT(.text.howling_dnn_set)WEAK
void howling_dnn_mic_output_callback_set(audio_callback_t callback)
{
    howling_dnn_mic_cfg.callback = callback;
}
#if 0
AT(.rodata.howling_dnn)
static u8 test_data[480]={
	0x23, 0x00, 0x3c, 0xa8, 0xdb, 0x51, 0xf9, 0x95, 0xc8, 0x22, 0xe0, 0xbc, 0x65, 0xc0, 0x51, 0x0a, 0x9e, 0x81,
	0x11, 0x07, 0xfd, 0x01, 0x63, 0xdd, 0x89, 0xcd, 0x74,
	0x2a, 0x00, 0x81, 0x87, 0xa4, 0x6f, 0x0c, 0xfa, 0x20, 0xeb, 0x90, 0x24, 0xb7, 0x1d, 0xd8, 0x00, 0x00, 0x2d,
	0x67, 0x2b, 0x17, 0x78, 0x6d, 0xb7, 0x91, 0xc0, 0xdc,
	0xa1, 0x0d, 0x89, 0xd2, 0x2c, 0xcd, 0x89, 0xdd, 0x7c,
	0xc1, 0x00, 0xc9, 0x1b, 0xc3, 0xf4, 0xe2, 0xc0, 0xc7, 0x75, 0x62, 0xab, 0x18, 0xe1, 0xc3, 0xb3, 0x00, 0x01,
	0xa1, 0xc7, 0xc6, 0x0e, 0xb4, 0x4d, 0x91, 0xd5, 0x74,
	0xd3, 0x00, 0xb1, 0x2c, 0x0d, 0xd8, 0xaf, 0xaa, 0xd4, 0x2a, 0xb2, 0x5b, 0xfe, 0x15, 0x74, 0x98, 0xc9, 0x00,
	0xd1, 0xa6, 0xd2, 0xa6, 0x7a, 0x4d, 0x89, 0xc9, 0x84,
	0x23, 0x00, 0x3c, 0xa8, 0xdb, 0x51, 0xf9, 0x95, 0xc8, 0x22, 0xe0, 0xbc, 0x65, 0xc0, 0x51, 0x0a, 0x9e, 0x81,
	0x11, 0x07, 0xfd, 0x01, 0x63, 0xdd, 0x89, 0xcd, 0x74,
	0x2a, 0x00, 0x81, 0x87, 0xa4, 0x6f, 0x0c, 0xfa, 0x20, 0xeb, 0x90, 0x24, 0xb7, 0x1d, 0xd8, 0x00, 0x00, 0x2d,
	0x67, 0x2b, 0x17, 0x78, 0x6d, 0xb7, 0x91, 0xc0, 0xdc,
	0xa1, 0x0d, 0x89, 0xd2, 0x2c, 0xcd, 0x89, 0xdd, 0x7c,
	0xc1, 0x00, 0xc9, 0x1b, 0xc3, 0xf4, 0xe2, 0xc0, 0xc7, 0x75, 0x62, 0xab, 0x18, 0xe1, 0xc3, 0xb3, 0x00, 0x01,
	0xa1, 0xc7, 0xc6, 0x0e, 0xb4, 0x4d, 0x91, 0xd5, 0x74,
	0xd3, 0x00, 0xb1, 0x2c, 0x0d, 0xd8, 0xaf, 0xaa, 0xd4, 0x2a, 0xb2, 0x5b, 0xfe, 0x15, 0x74, 0x98, 0xc9, 0x00,
	0xd1, 0xa6, 0xd2, 0xa6, 0x7a, 0x4d, 0x89, 0xc9, 0x84,
	0x80, 0x00, 0xb1, 0x84, 0x25, 0x8f, 0xd7, 0xbe, 0x16, 0x0e, 0x97, 0x20, 0x55, 0x2c, 0x8b, 0xcd, 0x34, 0x80,
	0x55, 0xe8, 0x3b, 0x0c, 0x8f, 0x5d, 0x21, 0xd1, 0x8c,
	0x66, 0x00, 0x85, 0x6d, 0x32, 0x03, 0xed, 0x99, 0xd7, 0x30, 0x1b, 0x49, 0x89, 0x08, 0xf2, 0xb3, 0x8a, 0x0f,
	0x80, 0xb5, 0xfb, 0x3d, 0xf6, 0x4d,
	0x30, 0x00, 0x68, 0x2b, 0x9a, 0x66, 0x75, 0x4c, 0x1b, 0xb1, 0x99, 0x0d, 0xd7, 0xdf, 0x95, 0x38, 0x00, 0x01,
	0xc2, 0xcb, 0x8c, 0x39, 0xf0, 0x3d, 0x91, 0xb9, 0x8c,
	0x11, 0x00, 0xba, 0x9d, 0xc5, 0xdb, 0x11, 0xb1, 0x87, 0x57, 0x17, 0xec, 0x44, 0x63, 0xe8, 0xd0, 0x80, 0x03,
	0x83, 0x75, 0x82, 0x87, 0x02, 0xcd, 0x89, 0xdd, 0x78,
	0xb9, 0x00, 0x7b, 0xee, 0xe3, 0x70, 0x1b, 0xdb, 0x1e, 0x5f, 0x34, 0x39, 0x74, 0x25, 0xf6, 0x2a, 0x46, 0x9e,

	0x80, 0x00, 0xb1, 0x84, 0x25, 0x8f, 0xd7, 0xbe, 0x16, 0x0e, 0x97, 0x20, 0x55, 0x2c, 0x8b, 0xcd, 0x34, 0x80,
	0x55, 0xe8, 0x3b, 0x0c, 0x8f, 0x5d, 0x21, 0xd1, 0x8c,
	0x66, 0x00, 0x85, 0x6d, 0x32, 0x03, 0xed, 0x99, 0xd7, 0x30, 0x1b, 0x49, 0x89, 0x08, 0xf2, 0xb3, 0x8a, 0x0f,
	0x80, 0xb5, 0xfb, 0x3d, 0xf6, 0x4d,
	0x30, 0x00, 0x68, 0x2b, 0x9a, 0x66, 0x75, 0x4c, 0x1b, 0xb1, 0x99, 0x0d, 0xd7, 0xdf, 0x95, 0x38, 0x00, 0x01,
	0xc2, 0xcb, 0x8c, 0x39, 0xf0, 0x3d, 0x91, 0xb9, 0x8c,
	0x11, 0x00, 0xba, 0x9d, 0xc5, 0xdb, 0x11, 0xb1, 0x87, 0x57, 0x17, 0xec, 0x44, 0x63, 0xe8, 0xd0, 0x80, 0x03,
	0x83, 0x75, 0x82, 0x87, 0x02, 0xcd, 0x89, 0xdd, 0x78,
	0xb9, 0x00, 0x7b, 0xee, 0xe3, 0x70, 0x1b, 0xdb, 0x1e, 0x5f, 0x34, 0x39, 0x74, 0x25, 0xf6, 0x2a, 0x46, 0x9e,
/*
	0x80, 0x00, 0xb1, 0x84, 0x25, 0x8f, 0xd7, 0xbe, 0x16, 0x0e, 0x97, 0x20, 0x55, 0x2c, 0x8b, 0xcd, 0x34, 0x80,
	0x55, 0xe8, 0x9e
	0x80, 0x00, 0xb1, 0x84, 0x25, 0x8f, 0xd7, 0xbe, 0x16, 0x0e, 0x97, 0x20, 0x55, 0x2c, 0x8b, 0xcd, 0x34, 0x80,
	0x55, 0xe8,
	0x30, 0x00, 0x68, 0x2b, 0x9a, 0x66, 0x75, 0x4c, 0x1b, 0xb1, 0x99, 0x0d, 0xd7, 0xdf, 0x95, 0x38, 0x00, 0x01,
	0xc2, 0xcb,
	0x30, 0x00, 0x68, 0x2b, 0x9a, 0x66, 0x75, 0x4c, 0x1b, 0xb1, 0x99, 0x0d, 0xd7, 0xdf, 0x95, 0x38, 0x00, 0x01,
	0xc2, 0xcb,
	0x30, 0x00, 0x68, 0x2b, 0x9a, 0x66, 0x75, 0x4c, 0x1b, 0xb1, 0x99, 0x0d, 0xd7, 0xdf, 0x95, 0x38, 0x00, 0x01,
	0xc2, 0xcb,
	0x30, 0x00, 0x68, 0x2b, 0x9a, 0x66, 0x75, 0x4c, 0x1b, 0xb1, 0x99, 0x0d, 0xd7, 0xdf, 0x95, 0x38, 0x00, 0x01,
	0xc2, 0xcb,
	0x23, 0x00, 0x3c, 0xa8, 0xdb, 0x51, 0xf9, 0x95, 0xc8, 0x22, 0xe0, 0xbc, 0x65, 0xc0, 0x51, 0x0a, 0x9e, 0x81,
	0x11, 0x07,
	0x23, 0x00, 0x3c, 0xa8, 0xdb, 0x51, 0xf9, 0x95, 0xc8, 0x22, 0xe0, 0xbc, 0x65, 0xc0, 0x51, 0x0a, 0x9e, 0x81,
	0x11, 0x07
*/
};
#endif
AT(.text.howling_dnn_init)WEAK
void howling_dnn_mic_init(u8 sample_rate, u16 samples, u8 channel)
{
#if HOWLING_DNN_RDFT_HW
    howling_dnn_rdft.fft_cft.size            = RDFT_512;
	howling_dnn_rdft.fft_cft.input_type      = 1;
    howling_dnn_rdft.fft_cft.window_en       = 1;
    howling_dnn_rdft.fft_cft.isr_en          = 0;
    howling_dnn_rdft.fft_cft.in_addr         = howling_dnn_rdft.howling_dnn_fft_in;
    howling_dnn_rdft.fft_cft.out_addr        = howling_dnn_rdft.howling_dnn_fft_out;

    howling_dnn_rdft.ifft_cft.size           = RDFT_512;
    howling_dnn_rdft.ifft_cft.output_type    = 1;
    howling_dnn_rdft.ifft_cft.window_en      = 1;
    howling_dnn_rdft.ifft_cft.isr_en         = 0;
    howling_dnn_rdft.ifft_cft.overlap_en     = 1;
    howling_dnn_rdft.ifft_cft.overlap_len    = 0;
    howling_dnn_rdft.ifft_cft.in_addr        = howling_dnn_rdft.howling_dnn_fft_out;
    howling_dnn_rdft.ifft_cft.out_addr       = howling_dnn_rdft.howling_dnn_ifft_out;

    memset(howling_dnn_rdft.howling_dnn_fft_in, 0, sizeof(howling_dnn_rdft.howling_dnn_fft_in));
    memset(howling_dnn_rdft.howling_dnn_fft_out, 0, sizeof(howling_dnn_rdft.howling_dnn_fft_out));
    memset(howling_dnn_rdft.howling_dnn_ifft_out, 0, sizeof(howling_dnn_rdft.howling_dnn_ifft_out));
#endif

#if UARTDUMP_HOWLING_DNN_EN
    memset(dump_head_buf, 0, 14*2);
    memset(UartDump_buf, 0, 2*(14 + FRAME_LEN*2));

    huart_module_init();    //初始化高速串口模块
    dump_buf_init(dump_head_buf,2, huart_putbuf, huart_wait_tx_finish);
#endif

    memset((uint8_t *)&howling_dnn_cb, 0, sizeof(howling_dnn_cb));
    memset((uint8_t *)&howling_dnn_mic_cfg, 0, sizeof(howling_dnn_mic_cfg));

    memset(howling_dnn_pingpangcache_temp, 0, FRAME_LEN*2*2);
    memset(howling_dnn_out_buf, 0, PROCESS_OUT_SAMPLES*2);
	howling_dnn_cb.param_printf           = 1;
	//howling_dnn_cb.nt                     = 3;
	//howling_dnn_cb.noise_db2			   = -15;
	howling_dnn_cb.mask_floor			   = 1000;
	//howling_dnn_cb.sin_post_en			   = 0;
	howling_dnn_cb.smooth_en			   = 1;
	howling_dnn_cb.howling_f_thres		   = 15*32768;//Q15的dB
	howling_dnn_cb.howling_f_thres_hi	   = 5*32768; //Q15的dB
	howling_dnn_cb.howling_exp_papr	       = 16;
	howling_dnn_cb.howling_exp_papr_hi	   = 22;
	howling_dnn_cb.howling_exp_papr2	   = 2;
	howling_dnn_cb.howling_fre_p		   = 6;
	howling_dnn_cb.hiwling_div_thres	   = 15*32768;

    howling_dnn_init(&howling_dnn_cb);

#if 0
    print_r(howling_dnn_pingpangcache_temp,FRAME_LEN*2);
    memcpy(howling_dnn_pingpangcache_temp,test_data,FRAME_LEN*2);
    print_r(howling_dnn_pingpangcache_temp,FRAME_LEN*2);
//    GPIOADE |=BIT(4);
//    GPIOAFEN &= ~(BIT(4));
//    GPIOADIR &= ~(BIT(4));
//    GPIOASET = BIT(4);
//
//    GPIOACLR = BIT(5);

#if 0
    howling_dnn_process_ext_window(howling_dnn_pingpangcache_temp,NULL,0,howling_dnn_rdft.howling_dnn_fft_in);
    fft_hw(&howling_dnn_rdft.fft_cft);
    howling_dnn_fre_process(howling_dnn_rdft.howling_dnn_fft_out, 0, 0);
    ifft_hw(&howling_dnn_rdft.ifft_cft);
    howling_dnn_process_ext_next(howling_dnn_rdft.howling_dnn_ifft_out,NULL,0,howling_dnn_pingpangcache_temp);

#endif
//    howling_dnn_process(howling_dnn_pingpangcache_temp,NULL,0);
//    GPIOASET = BIT(5);
//
    printf("out: \n");
    print_r(howling_dnn_pingpangcache_temp,FRAME_LEN*2);
//    GPIOACLR = BIT(4);
//
    while(1){
        WDT_CLR();
    }
#endif
}

AT(.text.howling_dnn_exit)WEAK
void howling_dnn_mic_exit(void)
{

}

AT(.text.howling_dnn_set.param)WEAK
void howling_dnn_mic_param_set(int8_t howling_dnn_nt)
{

}

AT(.text.howling_dnn_set.mute)WEAK
void howling_dnn_mic_mute_set(uint8_t mute)
{
    howling_dnn_mic_cfg.mute = mute;
}

AT(.text.howling_dnn_get.mute)WEAK
uint8_t howling_dnn_mic_mute_get(void)
{
    return howling_dnn_mic_cfg.mute;
}

#endif
