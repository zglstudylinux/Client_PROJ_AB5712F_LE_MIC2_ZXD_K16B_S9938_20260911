#include "include.h"
#include "api_alg.h"
#include "soft_eq.h"

/*
 * 文件名称: soft_eq_api.c
 * 功能描述: 本文件为软件soft_eq处理模块
    AT(.buf.soft_eq);
    AT(.rodata.soft_eq)
    AT(.text.soft_eq_proc)
    AT(.text.soft_eq_init)

 ****************************************************************************************
    code + rodata : 5K
    buf           : 16K
    time          : 2.5ms
 */

#if SOFT_EQ_EN

#define UARTDUMP_SOFT_EQ_EN          0                         //DUMP数据一键配置，需要使用定制上位机
#define HOWLING_SOFT_EQ_RDFT_HW      0                         //是否使用硬件FFT处理,软件fft底层处理若是关闭则无法编译通过
#define FRAME_LEN		             120                       //算法处理帧长
#define PROCESS_OUT_SAMPLES          120                       //每次存取帧长
s16 soft_eq_pingpangcache_temp[FRAME_LEN*2] AT(.buf.soft_eq); //乒乓buf缓存
u8 soft_eq_out_buf[PROCESS_OUT_SAMPLES*2] AT(.buf.soft_eq);   //输出buf中转缓存
WEAK volatile u8 soft_eq_flag = 0;                           //乒乓buf 切换标志位
uint8_t soft_eq_in_cnt = 0;                                 //写指针
uint8_t soft_eq_out_cnt = 0;                                //读指针

static soft_eq_mic_cfg_t soft_eq_mic_cfg AT(.buf.soft_eq);

soft_eq_t soft_eq_var AT(.buf.soft_eq);
AT(.text.soft_eq_drc.init)
static bool res_crc_check(u8 *ptr, eq_param_t *eq)
{
    u32 res_crc, cal_crc, band;
    if (ptr[0] == 'E' && ptr[1] == 'Q') {                               //V1: Q23
        band = ptr[3];
        res_crc = ptr[band*27+11] << 8 | ptr[band*27+10];
        cal_crc = calc_crc(ptr, band*27+10, 0xffff);
        eq->version = 0;
        eq->param = (u32 *)&ptr[4];
        eq->sample = ptr[2] & 0x07;
    } else if (ptr[0] == 'E' && ptr[1] == 'q' && ptr[2] == 0x01 && ptr[3] == 0x06 ) {      //V2: Q27 + SOFT全采样
        band = ptr[7];
        //res_crc = ptr[band*27+15] << 8 | ptr[band*27+14];
        //cal_crc = calc_crc(ptr, band*27+14, 0xffff);
        res_crc = cal_crc = 1; //校验接口调用异常暂时不做校验
        eq->version = ptr[2];
        eq->param = (u32 *)&ptr[8];
        eq->sample = ptr[3];
        my_printf("Please check the res name. \nThis function only is used for soft_all_32k.\r\n");
    } else if (ptr[0] == 'E' && ptr[1] == 'q' && ptr[2] == 0x01) {      //V2: Q27
        band = ptr[7];
        res_crc = ptr[band*27+15] << 8 | ptr[band*27+14];
        cal_crc = calc_crc(ptr, band*27+14, 0xffff);
        eq->version = ptr[2];
        eq->param = (u32 *)&ptr[8];
        eq->sample = ptr[3];
    } else {
        return false;
    }
    eq->band_cnt = band;
    if (res_crc == cal_crc) {
        return true;
    } else {
        return false;
    }
}

AT(.text.soft_eq_drc.init)
bool soft_eq_set_by_res(u32 addr, u32 len)
{
    do {
        if (addr == 0 || (len) == 0) {
            break;
        }
        u8 *ptr = (u8 *)(addr);
        eq_param_t eq_param;
        memset(&eq_param, 0, sizeof(eq_param_t));
        //验证CRC
        if (!res_crc_check(ptr, &eq_param)) {
            break;
        }
//        my_printf("eq_param.band_cnt:%x\n", eq_param.band_cnt);
        if (eq_param.band_cnt == 0) {
            break;
        }
//        my_printf("eq_param.version:%x\n", eq_param.version);
        if (eq_param.version != EQ_VERSION) {
            printf("eq version error:%x\n", eq_param.version);
            break;
        }
        soft_eq_init((const u32 *)eq_param.param, eq_param.band_cnt, &soft_eq_var);
        return true;

    } while (0);
    return false;
}

#if HOWLING_SOFT_EQ_RDFT_HW
typedef struct{
    fft_cfg_t fft_cft;
    ifft_cfg_t ifft_cft;
    s32 soft_eq_fft_in[512];
    s32 soft_eq_fft_out[512];
    s32 soft_eq_ifft_out[512];
}soft_eq_rdft_t;
static soft_eq_rdft_t soft_eq_rdft AT(.data.soft_eq);
#endif

#if UARTDUMP_SOFT_EQ_EN
static u8 dump_head_buf[14*2]AT(.buf.soft_eq);

static u8 UartDump_buf[2][14 + FRAME_LEN*2]AT(.buf.soft_eq);
static u8 UartDump_buf_temp[14 + FRAME_LEN*2]AT(.buf.soft_eq);
#endif



AT(.text.soft_eq_proc)WEAK
void soft_eq_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
//    if (0) {
//        uint8_t process_num = samples/PROCESS_OUT_SAMPLES;
//        s16 *process_pcm_ptr = (s16 *)ptr;
//        if (process_num*PROCESS_OUT_SAMPLES != samples) {
//            printf("soft_eq input samples err\n");
//            return;
//        }
//
//        while (process_num) {
//            ///OUT取 PING-PONG BUFFER
//            memcpy(soft_eq_out_buf,&soft_eq_pingpangcache_temp[soft_eq_out_cnt * PROCESS_OUT_SAMPLES],PROCESS_OUT_SAMPLES*2);
//            soft_eq_out_cnt++;
//            if(soft_eq_out_cnt>=FRAME_LEN*2/PROCESS_OUT_SAMPLES){
//                soft_eq_out_cnt = 0;
//            }
//            ///IN存 PING-PONG BUFFER
//            memcpy(&soft_eq_pingpangcache_temp[soft_eq_in_cnt * PROCESS_OUT_SAMPLES], process_pcm_ptr, PROCESS_OUT_SAMPLES * 2);
//            soft_eq_in_cnt++;
//            ///kick 低优先级线程处理
//            if(soft_eq_in_cnt == FRAME_LEN/PROCESS_OUT_SAMPLES){
//                soft_eq_flag = 1;
//                ///发射端处理kick 低现场 ，接收端处理攒够点数直接处理
//                if (!wireless_role_is_adapter()) {//device
////                    soft_eq_mic_proc_kick_start();
//                }else{//adapter
//                    soft_eq_mic_proc_cb();
//                }
//            }else if(soft_eq_in_cnt >= FRAME_LEN*2/PROCESS_OUT_SAMPLES){
//                soft_eq_in_cnt = 0;
//                soft_eq_flag = 2;
//                ///发射端处理kick 低现场 ，接收端处理攒够点数直接处理
//                if (!wireless_role_is_adapter()) {//device
////                    soft_eq_mic_proc_kick_start();
//                }else{//adapter
//                    soft_eq_mic_proc_cb();
//                }
//            }
//            memcpy(process_pcm_ptr,soft_eq_out_buf,PROCESS_OUT_SAMPLES*2);
//            /// break while()
//            process_num--;
//
//            if (soft_eq_mic_cfg.callback) {
//                soft_eq_mic_cfg.callback((void *)ptr, samples, ch_mode, params);
//            }
//        }
//
//    }else{

        if (soft_eq_mic_cfg.callback) {
            soft_eq_mic_cfg.callback((void *)ptr, samples, ch_mode, params);
        }
//    }

}

//soft_eq算法启动计算 放在低优先级现场处理
AT(.text.soft_eq_proc)WEAK
void soft_eq_mic_proc_cb(void)
{
    s16 *ptr = &soft_eq_pingpangcache_temp[0];
    if(soft_eq_flag == 2){
        ptr = &soft_eq_pingpangcache_temp[FRAME_LEN];
    }
#if UARTDUMP_SOFT_EQ_EN
    s16 *ptr_temp = (s16 *) UartDump_buf_temp;
    dump_dma_wait();
    dump_put2ram(&UartDump_buf[0][0],ptr,FRAME_LEN*2,0); //算法前数据组包

    dump_put2ram(&UartDump_buf[1][0],ptr_temp,FRAME_LEN*2,1);//算法后数据组包
    dump_dma_kick(&UartDump_buf[0][0], (FRAME_LEN*2 + 14) * 2); //最后再一起吐数据，避免前后kick数据打架
#endif

    for (int i=0; i<FRAME_LEN; i++) {
        ptr[i] = soft_eq_biquad(ptr[i], &soft_eq_var);
    }


#if HOWLING_SOFT_EQ_RDFT_HW
//    dnn_process_ext_window(ptr,NULL,0,soft_eq_rdft.soft_eq_fft_in);
//    fft_hw(&soft_eq_rdft.fft_cft);
//    dnn_fre_process(soft_eq_rdft.soft_eq_fft_out, 0, 0);
//    ifft_hw(&soft_eq_rdft.ifft_cft);
//    dnn_process_ext_next(soft_eq_rdft.soft_eq_ifft_out,NULL,0,ptr);
#if UARTDUMP_SOFT_EQ_EN
    for(int i = 0; i < FRAME_LEN; i++) {
        ptr_temp[i]  = ptr[i];
    }
#endif // UARTDUMP_SOFT_EQ_EN
#endif

    soft_eq_flag = 0;

}

AT(.text.soft_eq_set)WEAK
void soft_eq_mic_output_callback_set(audio_callback_t callback)
{
    soft_eq_mic_cfg.callback = callback;
}

s32 soft_eq_biquad_s32(s32 input, soft_eq_t *iir_filter_sb);
AT(.text.soft_eq_proc)WEAK
s32 soft_eq_mic_audio_input2(s32 input)
{
    return soft_eq_biquad_s32(input, &soft_eq_var);
}

AT(.text.soft_eq_init)WEAK
void soft_eq_mic_init(u8 sample_rate, u16 samples, u8 channel)
{
#if HOWLING_SOFT_EQ_RDFT_HW
    soft_eq_rdft.fft_cft.size            = RDFT_512;
	soft_eq_rdft.fft_cft.input_type      = 1;
    soft_eq_rdft.fft_cft.window_en       = 1;
    soft_eq_rdft.fft_cft.isr_en          = 0;
    soft_eq_rdft.fft_cft.in_addr         = soft_eq_rdft.soft_eq_fft_in;
    soft_eq_rdft.fft_cft.out_addr        = soft_eq_rdft.soft_eq_fft_out;

    soft_eq_rdft.ifft_cft.size           = RDFT_512;
    soft_eq_rdft.ifft_cft.output_type    = 1;
    soft_eq_rdft.ifft_cft.window_en      = 1;
    soft_eq_rdft.ifft_cft.isr_en         = 0;
    soft_eq_rdft.ifft_cft.overlap_en     = 1;
    soft_eq_rdft.ifft_cft.overlap_len    = 0;
    soft_eq_rdft.ifft_cft.in_addr        = soft_eq_rdft.soft_eq_fft_out;
    soft_eq_rdft.ifft_cft.out_addr       = soft_eq_rdft.soft_eq_ifft_out;

    memset(soft_eq_rdft.soft_eq_fft_in, 0, sizeof(soft_eq_rdft.soft_eq_fft_in));
    memset(soft_eq_rdft.soft_eq_fft_out, 0, sizeof(soft_eq_rdft.soft_eq_fft_out));
    memset(soft_eq_rdft.soft_eq_ifft_out, 0, sizeof(soft_eq_rdft.soft_eq_ifft_out));
#endif

#if UARTDUMP_SOFT_EQ_EN
    memset(dump_head_buf, 0, 14*2);
    memset(UartDump_buf, 0, 2*(14 + FRAME_LEN*2));

    huart_module_init();    //初始化高速串口模块
    dump_buf_init(dump_head_buf,2, huart_putbuf, huart_wait_tx_finish);
#endif

//    memset((uint8_t *)&soft_eq_cb, 0, sizeof(soft_eq_cb));
    memset((uint8_t *)&soft_eq_mic_cfg, 0, sizeof(soft_eq_mic_cfg));

    memset(soft_eq_pingpangcache_temp, 0, FRAME_LEN*2*2);
    memset(soft_eq_out_buf, 0, PROCESS_OUT_SAMPLES*2);

    u32 eq_len;
    u32 eq_addr;

    eq_len   = RES_LEN_WS_SOFT_EQ_EQ;;
    eq_addr  = RES_BUF_WS_SOFT_EQ_EQ;

    soft_eq_set_by_res(eq_addr, eq_len);
}

AT(.text.soft_eq_exit)WEAK
void soft_eq_mic_exit(void)
{

}

AT(.text.soft_eq_set.param)WEAK
void soft_eq_mic_param_set(int8_t soft_eq_nt)
{

}

AT(.text.soft_eq_set.mute)WEAK
void soft_eq_mic_mute_set(uint8_t mute)
{
    soft_eq_mic_cfg.mute = mute;
}

AT(.text.soft_eq_get.mute)WEAK
uint8_t soft_eq_mic_mute_get(void)
{
    return soft_eq_mic_cfg.mute;
}

#endif
