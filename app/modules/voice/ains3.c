#include "include.h"
#include "api_alg.h"
#include "ains3.h"
/*
 * 文件名称: ains3_api.c
 * 功能描述: 本文件为软件AINS3处理模块
    AT(.buf.ains3);
    AT(.rodata.ains3)
    AT(.text.ains3_proc)
    AT(.text.ains3_init)

 ****************************************************************************************
    code + rodata :
    buf           :
    time :
 */

#if AINS3_EN

#define UARTDUMP_AINS3_EN       1                         //DUMP数据一键配置，需要使用定制上位机
#define AINS3_INFO_PRINT        1
#define AINS3_RDFT_HW           1                         //是否使用硬件FFT处理,软件fft底层处理若是关闭则无法编译通过
#define FRAME_LEN		        240                       //算法处理帧长
#define PROCESS_OUT_SAMPLES     120                       //每次存取帧长
s16 ains3_pingpangcache_temp[FRAME_LEN*2] AT(.buf.ains3); //乒乓buf缓存
u8 ains3_out_buf[PROCESS_OUT_SAMPLES*2] AT(.buf.ains3);   //输出buf中转缓存
WEAK volatile u8 ains_flag = 0;                           //乒乓buf 切换标志位
uint8_t ains3_in_cnt = 0;                                 //写指针
uint8_t ains3_out_cnt = 0;                                //读指针

static ains3_cb_t ains3_cb AT(.buf.ains3);
static ains3_mic_cfg_t ains3_mic_cfg AT(.buf.ains3);

#if AINS3_RDFT_HW
typedef struct{
    fft_cfg_t fft_cft;
    ifft_cfg_t ifft_cft;
    s32 ains3_fft_in[512];
    s32 ains3_fft_out[512];
    s32 ains3_ifft_out[512];
}ains3_rdft_t;
ains3_rdft_t ains3_rdft AT(.buf.ains3);
#endif

//AINS3 48k 降噪参数定义
int8_t ains3_nt                  = AINS3_PAR_NT; // -20 ~ 40
uint8_t ains3_prior_opt_idx      = AINS3_PAR_OPT;// 0 ~ 19

#if AINS3_INFO_PRINT
AT(.com_text.ains3)
const char ains3_info[] = "AINS3 samples = %d, isrcnt = %d (SR_%d)\n";
#endif

#if UARTDUMP_AINS3_EN
static u8 dump_head_buf[14*2]AT(.buf.ains3);
static u8 UartDump_buf[2][14 + FRAME_LEN*2]AT(.buf.ains3);
static u8 UartDump_buf_temp[14 + FRAME_LEN*2]AT(.buf.ains3);
#endif

AT(.text.ains3_proc)WEAK
void ains3_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    if (!ains3_mic_cfg.mute && wireless_get_status()) {
        uint8_t process_num = samples/PROCESS_OUT_SAMPLES;
        s16 *process_pcm_ptr = (s16 *)ptr;
        if (process_num*PROCESS_OUT_SAMPLES != samples) {
            printf("ains3 input samples err\n");
            return;
        }

        while (process_num) {
            ///OUT取 PING-PONG BUFFER
            memcpy(ains3_out_buf,&ains3_pingpangcache_temp[ains3_out_cnt * PROCESS_OUT_SAMPLES],PROCESS_OUT_SAMPLES*2);
            ains3_out_cnt++;
            if(ains3_out_cnt>=FRAME_LEN*2/PROCESS_OUT_SAMPLES){
                ains3_out_cnt = 0;
            }
            ///IN存 PING-PONG BUFFER
            memcpy(&ains3_pingpangcache_temp[ains3_in_cnt * PROCESS_OUT_SAMPLES], process_pcm_ptr, PROCESS_OUT_SAMPLES * 2);
            ains3_in_cnt++;
            ///kick 低优先级线程处理
            if(ains3_in_cnt == FRAME_LEN/PROCESS_OUT_SAMPLES){
                ains_flag = 1;
                ///发射端处理kick 低现场 ，接收端处理攒够点数直接处理
                if (!wireless_role_is_adapter()) {//device
                    ains3_mic_proc_kick_start();
                }else{//adapter
                    ains3_mic_proc_cb();
                }
            }else if(ains3_in_cnt >= FRAME_LEN*2/PROCESS_OUT_SAMPLES){
                ains3_in_cnt = 0;
                ains_flag = 2;
                ///发射端处理kick 低现场 ，接收端处理攒够点数直接处理
                if (!wireless_role_is_adapter()) {//device
                    ains3_mic_proc_kick_start();
                }else{//adapter
                    ains3_mic_proc_cb();
                }
            }
            memcpy(process_pcm_ptr,ains3_out_buf,PROCESS_OUT_SAMPLES*2);
            /// break while()
            process_num--;
#if AINS3_INFO_PRINT
                static u32 ticks = 0;
                static u32 isr_cnt = 0;
                isr_cnt++;
                if (tick_check_expire(ticks,1000)) {
                    my_printf(ains3_info, samples, isr_cnt, samples*isr_cnt);
                    isr_cnt = 0;
                    ticks = tick_get();
                }
#endif
            if (ains3_mic_cfg.callback) {
                ains3_mic_cfg.callback((void *)ptr, samples, ch_mode, params);
            }
        }

    }else{

        if (ains3_mic_cfg.callback) {
            ains3_mic_cfg.callback((void *)ptr, samples, ch_mode, params);
        }
    }

}

//ains3算法启动计算 放在低优先级现场处理
AT(.text.ains3_proc)WEAK
void ains3_mic_proc_cb(void)
{
    s16 *ptr = &ains3_pingpangcache_temp[0];

    if(ains_flag == 2){
        ptr = &ains3_pingpangcache_temp[FRAME_LEN];
    }
#if UARTDUMP_AINS3_EN
    s16 *ptr_temp = (s16 *) UartDump_buf_temp;
    dump_dma_wait();
    dump_put2ram(&UartDump_buf[0][0],ptr,FRAME_LEN*2,0); //算法前数据组包

    dump_put2ram(&UartDump_buf[1][0],ptr_temp,FRAME_LEN*2,1);//算法后数据组包
    dump_dma_kick(&UartDump_buf[0][0], (FRAME_LEN*2 + 14) * 2); //最后再一起吐数据，避免前后kick数据打架
#endif

#if !AINS3_RDFT_HW
//    ains3_process(ptr);
#endif

#if AINS3_RDFT_HW
    u32 i = 0;
    ains3_process_ext_window(ptr,ains3_rdft.ains3_fft_in);
    fft_hw(&ains3_rdft.fft_cft);
    nr_process_do((s32 *)ains3_rdft.ains3_fft_out);
    ifft_hw(&ains3_rdft.ifft_cft);
    ains3_process_ext_output(ptr);
    for(i = 0; i < 240; i++) {
        ptr[i] = ains3_rdft.ains3_ifft_out[i];
    }

#if UARTDUMP_AINS3_EN
    for(int i = 0; i < FRAME_LEN; i++) {
        ptr_temp[i]  = ptr[i];
    }
#endif // UARTDUMP_DNN_EN
#endif

    ains_flag = 0;

}

AT(.text.ains3_set)WEAK
void ains3_mic_output_callback_set(audio_callback_t callback)
{
    ains3_mic_cfg.callback = callback;
}

AT(.text.ains3_init)WEAK
void ains3_mic_init(u8 sample_rate, u16 samples, u8 channel)
{

#if AINS3_RDFT_HW
    ains3_rdft.fft_cft.size            = RDFT_512;
	ains3_rdft.fft_cft.input_type      = 1;
    ains3_rdft.fft_cft.window_en       = 1;
    ains3_rdft.fft_cft.isr_en          = 0;
    ains3_rdft.fft_cft.in_addr         = ains3_rdft.ains3_fft_in;
    ains3_rdft.fft_cft.out_addr        = ains3_rdft.ains3_fft_out;

    ains3_rdft.ifft_cft.size           = RDFT_512;
    ains3_rdft.ifft_cft.output_type    = 1;
    ains3_rdft.ifft_cft.window_en      = 1;
    ains3_rdft.ifft_cft.isr_en         = 0;
    ains3_rdft.ifft_cft.overlap_en     = 1;
    ains3_rdft.ifft_cft.overlap_len    = 0;
    ains3_rdft.ifft_cft.in_addr        = ains3_rdft.ains3_fft_out;
    ains3_rdft.ifft_cft.out_addr       = ains3_rdft.ains3_ifft_out;

    memset(ains3_rdft.ains3_fft_in, 0, sizeof(ains3_rdft.ains3_fft_in));
    memset(ains3_rdft.ains3_fft_out, 0, sizeof(ains3_rdft.ains3_fft_out));
    memset(ains3_rdft.ains3_ifft_out, 0, sizeof(ains3_rdft.ains3_ifft_out));
#endif

#if UARTDUMP_AINS3_EN
    memset(dump_head_buf, 0, 14*2);
    memset(UartDump_buf, 0, 2*(14 + FRAME_LEN*2));

    huart_module_init();    //初始化高速串口模块
    dump_buf_init(dump_head_buf,2, huart_putbuf, huart_wait_tx_finish);
#endif
    memset((uint8_t *)&ains3_cb, 0, sizeof(ains3_cb));
    memset((uint8_t *)&ains3_mic_cfg, 0, sizeof(ains3_mic_cfg));

    memset(ains3_pingpangcache_temp, 0, FRAME_LEN*2*2);
    memset(ains3_out_buf, 0, PROCESS_OUT_SAMPLES*2);

    if(ains3_nt >= 40){
        ains3_nt = 40;
    }else if(ains3_nt <= -20){
        ains3_nt = -20;
    }

    if(ains3_prior_opt_idx >=19){
        ains3_prior_opt_idx = 19;
    }

    ains3_cb.nt                = ains3_nt;
	ains3_cb.prior_opt_idx     = ains3_prior_opt_idx;//if 256fft  need choose 3
    ///以下参数用户无须修改
    ains3_cb.prior_opt_ada_en  = 0;
	ains3_cb.music_lev         = 36;
	ains3_cb.music_lev_hi      = 61;
	ains3_cb.music_lev_hi_range= 36;
	ains3_cb.ns_ps_rate        = 1;
	ains3_cb.low_fre_lev       = 7;
	ains3_cb.low_fre_range     = 256;//if 1024fft ：low_fre_range=512	; if 512fft ：low_fre_range=256	; if 256fft ：low_fre_range=128
	ains3_cb.ns_range_h        = 512;//if 1024fft ：ns_range_h=512	; if 512fft ：ns_range_h=256	; if 256fft ：low_fre_range=128
	ains3_cb.ns_range_l        = 0;//if 1024fft ：ns_range_l=1	; if 512fft ：ns_range_l=1	; if 256fft ：ns_range_l=1

	ains3_cb.noise_db          = -360;//单位db，例：小于-30db为噪声。 当nr_mode为1或2时有效
	ains3_cb.noise_db2         = -210;
	ains3_cb.noise_db3         = -260;
	ains3_cb.smooth_en         = 1;

    ains3_init(&ains3_cb);

}

AT(.text.ains3_exit)WEAK
void ains3_mic_exit(void)
{

}

AT(.text.ains3_set.param)WEAK
void ains3_mic_param_set(int8_t ains3_nt,u8 ains3_idx)
{
    if(ains3_nt >= 40){
        ains3_nt = 40;
    }else if(ains3_nt <= -20){
        ains3_nt = -20;
    }
    if(ains3_idx >=19){
        ains3_idx = 19;
    }

    ains3_cb.nt                = ains3_nt;
	ains3_cb.prior_opt_idx     = ains3_prior_opt_idx;//if 256fft  need choose 3
    ///以下参数用户无须修改
    ains3_cb.prior_opt_ada_en  = 0;
	ains3_cb.music_lev         = 36;
	ains3_cb.music_lev_hi      = 61;
	ains3_cb.music_lev_hi_range= 36;
	ains3_cb.ns_ps_rate        = 1;
	ains3_cb.low_fre_lev       = 7;
	ains3_cb.low_fre_range     = 256;//if 1024fft ：low_fre_range=512	; if 512fft ：low_fre_range=256	; if 256fft ：low_fre_range=128
	ains3_cb.ns_range_h        = 512;//if 1024fft ：ns_range_h=512	; if 512fft ：ns_range_h=256	; if 256fft ：low_fre_range=128
	ains3_cb.ns_range_l        = 0;//if 1024fft ：ns_range_l=1	; if 512fft ：ns_range_l=1	; if 256fft ：ns_range_l=1

	ains3_cb.noise_db          = -360;//单位db，例：小于-30db为噪声。 当nr_mode为1或2时有效
	ains3_cb.noise_db2         = -210;
	ains3_cb.noise_db3         = -260;
	ains3_cb.smooth_en         = 1;

    ains3_init(&ains3_cb);
}

AT(.text.ains3_set.mute)WEAK
void ains3_mic_mute_set(uint8_t mute)
{
    ains3_mic_cfg.mute = mute;
}

AT(.text.ains3_get.mute)WEAK
uint8_t ains3_mic_mute_get(void)
{
    return ains3_mic_cfg.mute;
}

#endif
