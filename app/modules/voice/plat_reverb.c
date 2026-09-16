#include "include.h"
#include "plat_reverb.h"
#include "api_alg.h"

#if PLAT_REVERB_EN

static plat_reverb_cfg_t plat_reverb_cfg AT(.buf.plat_reverb);
static prvb_init_sb user_prvb_cb AT(.buf.plat_reverb);

AT(.text.plat_reverb_proc)
void plat_reverb_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    s16 *rptr = (s16 *)ptr;
    if (!plat_reverb_cfg.mute) {
        plate_reverb_process_do(rptr, samples);
    }
    if (plat_reverb_cfg.callback) {
        plat_reverb_cfg.callback((u8 *)rptr, samples, ch_mode, params);
    }
}

AT(.text.plat_reverb_set.callback)
void plat_reverb_audio_output_callback_set(audio_callback_t callback)
{
    plat_reverb_cfg.callback = callback;
}

AT(.text.plat_reverb_init)
void plat_reverb_audio_init(u8 sample_rate, u16 samples)
{
    //管理模块结构体赋值
    memset(&plat_reverb_cfg, 0, sizeof(room_reverb_cfg_t));
    plat_reverb_cfg.sample_rate = sample_rate;
    plat_reverb_cfg.samples     = samples;

    //算法结构体赋值
    memset(&user_prvb_cb, 0, sizeof(user_prvb_cb));
	user_prvb_cb.predelay_set   = 1999;					       //最大值为PREDELAY预延时  值越大回声越大
	user_prvb_cb.wet_set        = 0.5f * (1 << 15);        //湿度  值越大混响效果越明显
	user_prvb_cb.dry_set        = 0.99f * (1 << 15);		   //干度  值越大原声占比越大
	user_prvb_cb.damping_set    = 0.5f * (1 << 15);		   //0.05*2^15阻尼         值越大高频衰减越多，声音越干
	user_prvb_cb.decay_set      = 0.7f * (1 << 15);        //0.5*2^15衰减率       值越小，衰减越快，尾音少（增至0.7效果就很明显）
	user_prvb_cb.diffusion	   = 0.7f * (1 << 15);         //0.5*2^15衰减扩散     控制尾部密度
	user_prvb_cb.hp_level_set   = 1;			               //0-8;0:表示不过高通滤波器，1-7：表示150hz-500hz,步进50hz
	user_prvb_cb.modulation	   = 1;					       //调制开关
	user_prvb_cb.en			   = 1;					       //混响开关
    plate_buf_init(&user_prvb_cb);
    printf("PLATE_REVERB_EFFECT_INIT \n");
}


AT(.text.plat_reverb_set.param)
// pre_dly: 0 ~ 1999 plat_dry: 0 ~ 1000 plat_wet: 0 ~ 1000 damping: 0 ~ 999
// diffusion: 0 ~ 1000 decay: 0 ~ 1000 hp_freq: 0 ~ 8
void plat_reverb_audio_set_param(u16 pre_dly, u16 plat_dry, u16 plat_wet, u16 damping,
                                 u16 diffusion, u16 decay, u8 hp_freq)
{
    if (pre_dly >= 1999) {
        pre_dly = 1999;
    }
    if (plat_dry >= 1000) {
        plat_dry = 1000;
    }
    if (plat_wet >= 1000) {
        plat_wet = 1000;
    }
    if (damping >= 999) {
        damping = 999;
    }
    if (diffusion >= 1000) {
        diffusion = 1000;
    }
    if (decay >= 1000) {
        decay = 1000;
    }
    if (hp_freq >= 8) {
        hp_freq = 8;
    }

    user_prvb_cb.predelay_set = pre_dly;
    user_prvb_cb.wet_set = (plat_wet*0x8000)/1000;
    user_prvb_cb.dry_set = (plat_dry*0x8000)/1000;
    user_prvb_cb.damping_set = (damping*0x8000)/1000;
    user_prvb_cb.decay_set = (decay*0x8000)/1000;
    user_prvb_cb.diffusion = (diffusion*0x8000)/1000;
    user_prvb_cb.hp_level_set = hp_freq;
    user_prvb_cb.modulation = 1;
    user_prvb_cb.en = 1;
    plate_buf_init(&user_prvb_cb);

}

AT(.text.plat_reverb_set.mute)
void plat_reverb_audio_mute_set(uint8_t mute)
{
    plat_reverb_cfg.mute = mute;
}

AT(.text.plat_reverb_exit)
void plat_reverb_audio_exit(void)
{

}
#endif
