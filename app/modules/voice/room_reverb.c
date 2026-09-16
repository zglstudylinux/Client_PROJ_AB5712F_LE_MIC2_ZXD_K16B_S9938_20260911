#include "include.h"
#include "room_reverb.h"
#include "api_alg.h"
/*
 * 文件名称: room_reverb.c
 * 功能描述: 本文件为软件ROOM_REVERB 房间混响处理模块
    当前模块按点计算，配置采样率一项固化48k
 ****************************************************************************************
    code : 400 bytes
    buf  : 22K
    time : 待测
 */

#if ROOM_REVERB_EN

static room_reverb_cfg_t room_reverb_cfg AT(.buf.room_reverb);
static rvb_init_sb user_rvb_t AT(.buf.room_reverb);

AT(.text.room_reverb_proc)
void room_reverb_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    s16 *rptr = (s16 *)ptr;

    if(!room_reverb_cfg.mute){
        for(u16 i = 0;i < samples; i++){
            room_reverb_process(&rptr[i],0);
        }
    }

    if (room_reverb_cfg.callback) {
        room_reverb_cfg.callback((u8 *)rptr, samples, ch_mode, params);
    }
}

AT(.text.room_reverb_set.callback)
void room_reverb_audio_output_callback_set(audio_callback_t callback)
{
    room_reverb_cfg.callback = callback;
}

AT(.text.room_reverb_init)
void room_reverb_audio_init(u8 sample_rate, u16 samples, u8 channel)
{
    //管理模块结构体赋值
    memset(&room_reverb_cfg, 0, sizeof(room_reverb_cfg_t));
    room_reverb_cfg.sample_rate = sample_rate;
    room_reverb_cfg.samples     = samples;

    //算法结构体赋值
    memset(&user_rvb_t, 0, sizeof(user_rvb_t));
    u8 idx                 = 0; //0或1，左声道或右声道
    user_rvb_t.damping_set = 10376;//低通滤波器系数，越大截至频率越低，
	user_rvb_t.decay_set   = ROOM_REVERB_LEVEL;//强度
	user_rvb_t.dry         = ROOM_REVERB_DRY;//干度
	user_rvb_t.wet         = ROOM_REVERB_WET;//湿度
	user_rvb_t.samples     = 48000;//采样点
	user_rvb_t.hpl         = 5;/*高通滤波器等级0表示关闭,1-10:50hz-500hz,步进50hz*/
    reverb_buf_init(&user_rvb_t, idx);

}

AT(.text.room_reverb_set.param)
void room_reverb_audio_set_param(u16 room_decay_set, u16 room_dry, u16 room_wet)
{
    //算法结构体赋值
    u8 idx                 = 0; //0或1，左声道或右声道
    user_rvb_t.damping_set = 10376;//低通滤波器系数，越大截至频率越低，
	user_rvb_t.decay_set   = room_decay_set;//强度
	user_rvb_t.dry         = room_dry;//干度
	user_rvb_t.wet         = room_wet;//湿度
	user_rvb_t.samples     = 48000;//采样点
	user_rvb_t.hpl         = 5;/*高通滤波器等级0表示关闭,1-10:50hz-500hz,步进50hz*/
    reverb_buf_init(&user_rvb_t, idx);

}

AT(.text.room_reverb_set.mute)
void room_reverb_audio_mute_set(uint8_t mute)
{
    room_reverb_cfg.mute = mute;
}

AT(.text.room_reverb_exit)
void room_reverb_audio_exit(void)
{

}
#endif
