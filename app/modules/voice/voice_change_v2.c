#include "include.h"
#include "api_alg.h"
#include "voice_change_v2.h"

/*
 * 文件名称: voice_change_v2_api.c
 * 功能描述: 本文件为软件voice_change_v2处理模块

    AT(.buf.voice_change_v2);
    AT(.rodata.voice_change_v2)
    AT(.text.voice_change_v2_proc)
    AT(.text.voice_change_v2_init)

 ****************************************************************************************
    code + rodata :
    buf           :
    time          : ZOOM_160M: (2.66ms / 5ms)
 */

#if VOICE_CHANGE_V2_EN

#define UARTDUMP_VOICE_CHANGE_V2_EN      1                         //DUMP数据一键配置，需要使用定制上位机
#define VOICE_CHANGE_V2_INFO_PRINT       0
#define FRAME_LEN		                 240                       //算法处理帧长
#define PROCESS_OUT_SAMPLES              120                       //每次存取帧长
s16 voice_change_v2_pingpangcache_temp[FRAME_LEN*2] AT(.buf.voice_change_v2); //乒乓buf缓存
u8 voice_change_v2_out_buf[PROCESS_OUT_SAMPLES*2] AT(.buf.voice_change_v2);   //输出buf中转缓存
WEAK volatile u8 voice_change_v2_flag = 0;                           //乒乓buf 切换标志位
uint8_t voice_change_v2_in_cnt = 0;                                 //写指针
uint8_t voice_change_v2_out_cnt = 0;                                //读指针

static voice_change_v2_mic_cfg_t voice_change_v2_mic_cfg AT(.buf.voice_change_v2);

static mav_init_t mav_init AT(.buf.voice_change_v2);

#if VOICE_CHANGE_V2_INFO_PRINT
AT(.com_text.voice_change_v2)
const char voice_change_v2_info[] = "voice_change_v2 samples = %d, isrcnt = %d (SR_%d)\n";
#endif

#if UARTDUMP_VOICE_CHANGE_V2_EN
static u8 dump_head_buf[14*2]AT(.buf.voice_change_v2);
static u8 UartDump_buf[2][14 + FRAME_LEN*2]AT(.buf.voice_change_v2);
static u8 UartDump_buf_temp[14 + FRAME_LEN*2]AT(.buf.voice_change_v2);
//static u8 huart_buffer_rx[128] AT(.buf.voice_change_v2);
#endif

AT(.text.voice_change_v2_proc)
void voice_change_v2_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{

    if (!voice_change_v2_mic_cfg.mute /*&& wireless_get_status()*/) {
        uint8_t process_num = samples/PROCESS_OUT_SAMPLES;
        s16 *process_pcm_ptr = (s16 *)ptr;
        if (process_num*PROCESS_OUT_SAMPLES != samples) {
            printf("voice_change_v2 input samples err\n");
            return;
        }

        while (process_num) {
            ///OUT取 PING-PONG BUFFER
            memcpy(voice_change_v2_out_buf,&voice_change_v2_pingpangcache_temp[voice_change_v2_out_cnt * PROCESS_OUT_SAMPLES],PROCESS_OUT_SAMPLES*2);
            voice_change_v2_out_cnt++;
            if (voice_change_v2_out_cnt>=FRAME_LEN*2/PROCESS_OUT_SAMPLES) {
                voice_change_v2_out_cnt = 0;
            }
            ///IN存 PING-PONG BUFFER
            memcpy(&voice_change_v2_pingpangcache_temp[voice_change_v2_in_cnt * PROCESS_OUT_SAMPLES], process_pcm_ptr, PROCESS_OUT_SAMPLES * 2);
            voice_change_v2_in_cnt++;
            ///kick 低优先级线程处理
            if (voice_change_v2_in_cnt == FRAME_LEN/PROCESS_OUT_SAMPLES) {
                voice_change_v2_flag = 1;
                ///发射端处理kick 低现场 ，接收端处理攒够点数直接处理
                if (!wireless_role_is_adapter()) {//device
                    voice_change_v2_mic_proc_kick_start();
                } else {//adapter
                    voice_change_v2_mic_proc_cb();
                }
            } else if (voice_change_v2_in_cnt >= FRAME_LEN*2/PROCESS_OUT_SAMPLES) {
                voice_change_v2_in_cnt = 0;
                voice_change_v2_flag = 2;
                ///发射端处理kick 低现场 ，接收端处理攒够点数直接处理
                if (!wireless_role_is_adapter()) {//device
                    voice_change_v2_mic_proc_kick_start();
                } else {//adapter
                    voice_change_v2_mic_proc_cb();
                }
            }
            memcpy(process_pcm_ptr,voice_change_v2_out_buf,PROCESS_OUT_SAMPLES*2);
            /// break while()
            process_num--;

            if (voice_change_v2_mic_cfg.callback) {
                voice_change_v2_mic_cfg.callback((void *)ptr, samples, ch_mode, params);
            }
        }

    }else{

        if (voice_change_v2_mic_cfg.callback) {
            voice_change_v2_mic_cfg.callback((void *)ptr, samples, ch_mode, params);
        }
    }

}

//voice_change_v2算法启动计算 放在低优先级现场处理
AT(.text.voice_change_v2_proc)
void voice_change_v2_mic_proc_cb(void)
{
    s16 *ptr = &voice_change_v2_pingpangcache_temp[0];
    if(voice_change_v2_flag == 2) {
        ptr = &voice_change_v2_pingpangcache_temp[FRAME_LEN];
    }

#if UARTDUMP_VOICE_CHANGE_V2_EN
    s16 *ptr_temp = (s16 *) UartDump_buf_temp;
    dump_dma_wait();
    dump_put2ram(&UartDump_buf[0][0],ptr,FRAME_LEN*2,0); //算法前数据组包

    dump_put2ram(&UartDump_buf[1][0],ptr_temp,FRAME_LEN*2,1);//算法后数据组包
    dump_dma_kick(&UartDump_buf[0][0], (FRAME_LEN*2 + 14) * 2); //最后再一起吐数据，避免前后kick数据打架
#endif

#if VOICE_CHANGE_V2_INFO_PRINT
    static u32 ticks = 0;
    static u32 isr_cnt = 0;
    isr_cnt++;
    if (tick_check_expire(ticks,1000)) {
        my_printf(voice_change_v2_info, FRAME_LEN, isr_cnt, FRAME_LEN*isr_cnt);
        isr_cnt = 0;
        ticks = tick_get();
    }
#endif

    voice_change_process(ptr, FRAME_LEN, 0);

#if UARTDUMP_VOICE_CHANGE_V2_EN
    for(int i = 0; i < FRAME_LEN; i++) {
        ptr_temp[i]  = ptr[i];
    }
#endif // UARTDUMP_VOICE_CHANGE_V2_EN
    voice_change_v2_flag = 0;

}

AT(.text.voice_change_v2_set)
void voice_change_v2_mic_output_callback_set(audio_callback_t callback)
{
    voice_change_v2_mic_cfg.callback = callback;
}
#if 0
AT(.rodata.voice_change_v2)
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
};
#endif
AT(.text.voice_change_v2_init)
void voice_change_v2_mic_init(u8 sample_rate, u16 samples, u8 channel)
{

#if UARTDUMP_VOICE_CHANGE_V2_EN
    memset(dump_head_buf, 0, 14*2);
    memset(UartDump_buf, 0, 2*(14 + FRAME_LEN*2));

    huart_module_init();    //初始化高速串口模块
    dump_buf_init(dump_head_buf,2, huart_putbuf, huart_wait_tx_finish);
#endif

    memset((uint8_t *)&voice_change_v2_mic_cfg, 0, sizeof(voice_change_v2_mic_cfg));

    memset(voice_change_v2_pingpangcache_temp, 0, FRAME_LEN*2*2);
    memset(voice_change_v2_out_buf, 0, PROCESS_OUT_SAMPLES*2);


    memset((uint8_t *)&mav_init, 0, sizeof(mav_init));
    mav_init.pitch_rate    = 1.0f * (1 << 15);//1.6f * (1 << 15);					 //变调 建议范围均在1.0-3.1 Q15
#if 1
	mav_init.formant_shift = 1.2f * (1 << 20);//0.625f * (1 << 20);				     //共振峰修正系数//建议0.3-1.5 Q20
#endif
	//mav_init.pitch_rate = 2.3;//变调            //变速，建议范围均在0.5-3.1
	//mav_init.formant_shift = 0.5;               //共振峰修正系数//建议0.3-1.5
    mav_init.quality = 3;						  //1-8变速质量，越大效果越差，但速度更快
	mav_init.robot_en = 1;
	mav_init.out_gain = 82310;					  //8db
	voice_change_init(&mav_init, 0);              //算法初始化

//    voice_change_v2_mic_cfg.mute = 1;             //上电原声

#if 0
    sys_clk_req(INDEX_KARAOK, SYS_160M);        //先抬高主频
    GPIOBDE |=BIT(4);
    GPIOBFEN &= ~(BIT(4));
    GPIOBDIR &= ~(BIT(4));
    memcpy(voice_change_v2_pingpangcache_temp,test_data,240*2);
    print_r(voice_change_v2_pingpangcache_temp,480);

    GPIOBSET = BIT(4);
    voice_change_process(voice_change_v2_pingpangcache_temp, FRAME_LEN, 0);
    GPIOBCLR = BIT(4);
    print_r(voice_change_v2_pingpangcache_temp,480);

    while(1) {
        WDT_CLR();
    }
#endif
}

AT(.text.voice_change_v2_exit)
void voice_change_v2_mic_exit(void)
{

}

//调用切换要记得先mute一下本模块过音频流，设置完再unmute回来
AT(.text.voice_change_v2_set.param)
void voice_change_v2_mic_param_mode_set(u8 vc_change_mode)
{
    //机器人          pitch_rate = 1.0     formant_shift = 1.2  mav_init.robot_en = 1
    //小黄人          pitch_rate = 2.0     formant_shift = 0.7  mav_init.robot_en = 0 暂时没有泡泡声
    //女神（男变女）  pitch_rate = 1.715   formant_shift = 0.6  mav_init.robot_en = 0
    //男神（男变女）  pitch_rate = 0.8     formant_shift = 0.9  mav_init.robot_en = 0
    //怪兽(魔兽音)    pitch_rate = 0.65    formant_shift = 1.0  mav_init.robot_en = 0
    //娃娃音          pitch_rate = 2.0     formant_shift = 1.0  mav_init.robot_en = 0
    if(vc_change_mode >= VOICE_CHANGE_MAX){
        return;
    }
    switch (vc_change_mode) {

        case VOICE_CHANGE_NONE:
            voice_change_v2_mic_cfg.mute = 1;
            break;
        case VOICE_CHANGE_ROBOT:
            memset((uint8_t *)&mav_init, 0, sizeof(mav_init));
            mav_init.pitch_rate    = 1.0f * (1 << 15);//1.6f * (1 << 15);					 //变调 建议范围均在1.0-3.1 Q15
            mav_init.formant_shift = 1.2f * (1 << 20);//0.625f * (1 << 20);				     //共振峰修正系数//建议0.3-1.5 Q20
            mav_init.quality = 4;						                                     //1-8变速质量，越大效果越差,但速度更快(在本程序中最小为2,不然算力不够)
            mav_init.robot_en = 1;
            mav_init.out_gain = 82310;					 //8db
            voice_change_init(&mav_init, 0);//算法初始化
            voice_change_v2_mic_cfg.mute = 0;
            break;
        case VOICE_CHANGE_MINIONS:
            memset((uint8_t *)&mav_init, 0, sizeof(mav_init));
            mav_init.pitch_rate    = 2.0f * (1 << 15);//1.6f * (1 << 15);					 //变调 建议范围均在1.0-3.1 Q15
            mav_init.formant_shift = 0.7f * (1 << 20);//0.625f * (1 << 20);				     //共振峰修正系数//建议0.3-1.5 Q20
            mav_init.quality = 3;						 //1-8变速质量，越大效果越差，但速度更快
            mav_init.robot_en = 0;
            mav_init.out_gain = 82310;					 //8db
            voice_change_init(&mav_init, 0);//算法初始化
            voice_change_v2_mic_cfg.mute = 0;
            break;
        case VOICE_CHANGE_WOMAN:
            memset((uint8_t *)&mav_init, 0, sizeof(mav_init));
            mav_init.pitch_rate    = 1.715f * (1 << 15);//1.6f * (1 << 15);					 //变调 建议范围均在1.0-3.1 Q15
            mav_init.formant_shift = 0.6f * (1 << 20);//0.625f * (1 << 20);				     //共振峰修正系数//建议0.3-1.5 Q20
            mav_init.quality = 3;						 //1-8变速质量，越大效果越差，但速度更快
            mav_init.robot_en = 0;
            mav_init.out_gain = 82310;					 //8db
            voice_change_init(&mav_init, 0);//算法初始化
            voice_change_v2_mic_cfg.mute = 0;
            break;
        case VOICE_CHANGE_MAN:
            memset((uint8_t *)&mav_init, 0, sizeof(mav_init));
            mav_init.pitch_rate    = 0.8f * (1 << 15);//1.6f * (1 << 15);					 //变调 建议范围均在1.0-3.1 Q15
            mav_init.formant_shift = 0.9f * (1 << 20);//0.625f * (1 << 20);				     //共振峰修正系数//建议0.3-1.5 Q20
            mav_init.quality = 3;						 //1-8变速质量，越大效果越差，但速度更快
            mav_init.robot_en = 0;
            mav_init.out_gain = 82310;					 //8db
            voice_change_init(&mav_init, 0);//算法初始化
            voice_change_v2_mic_cfg.mute = 0;
            break;
        case VOICE_CHANGE_MONSTER:
            memset((uint8_t *)&mav_init, 0, sizeof(mav_init));
            mav_init.pitch_rate    = 0.65f * (1 << 15);//1.6f * (1 << 15);					 //变调 建议范围均在1.0-3.1 Q15
            mav_init.formant_shift = 1.0f * (1 << 20);//0.625f * (1 << 20);				     //共振峰修正系数//建议0.3-1.5 Q20
            mav_init.quality = 3;						 //1-8变速质量，越大效果越差，但速度更快
            mav_init.robot_en = 0;
            mav_init.out_gain = 82310;					 //8db
            voice_change_init(&mav_init, 0);//算法初始化
            voice_change_v2_mic_cfg.mute = 0;
            break;
        case VOICE_CHANGE_BABY:
            memset((uint8_t *)&mav_init, 0, sizeof(mav_init));
            mav_init.pitch_rate    = 2.0f * (1 << 15);//1.6f * (1 << 15);					 //变调 建议范围均在1.0-3.1 Q15
            mav_init.formant_shift = 1.0f * (1 << 20);//0.625f * (1 << 20);				     //共振峰修正系数//建议0.3-1.5 Q20
            mav_init.quality = 3;						 //1-8变速质量，越大效果越差，但速度更快
            mav_init.robot_en = 0;
            mav_init.out_gain = 82310;					 //8db
            voice_change_init(&mav_init, 0);//算法初始化
            voice_change_v2_mic_cfg.mute = 0;
            break;

        default:
            break;
    }
}

AT(.text.voice_change_v2_set.mute)
void voice_change_v2_mic_mute_set(uint8_t mute)
{
    voice_change_v2_mic_cfg.mute = mute;
}

AT(.text.voice_change_v2_get.mute)
uint8_t voice_change_v2_mic_mute_get(void)
{
    return voice_change_v2_mic_cfg.mute;
}
#else
//voice_change_v2算法启动计算 放在低优先级现场处理
AT(.text.voice_change_v2_proc)
void voice_change_v2_mic_proc_cb(void){}
#endif
