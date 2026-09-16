/*****************************************************************************
 * Copyright (c) 2023 Shenzhen Bluetrum Technology Co.,Ltd. All rights reserved.
 * File      : bsp_effect.h
 * Function  : 音乐音效模块接口
 * History   :
 * Created by yutao on 2023-12-1.
 *****************************************************************************/
#ifndef __BSP_EFFECT_H__
#define __BSP_EFFECT_H__

#define GET_BUF_LE_U16(buf, ofs)         *(uint16_t *)((uint8_t *)buf + (ofs))
#define GET_BUF_LE_U32(buf, ofs)         *(uint32_t *)((uint8_t *)buf + (ofs))

extern volatile u32 xdrc_cfg;

//drc control block
extern s32 xdrc_preeq_cb[10*7 + 6];  //10段pre_eq
//lp_cb
extern s32 xdrc_lp_cb[1*7 + 6];
extern s32 xdrc_lp_cb2[1*7 + 6];
//hp_cb
extern s32 xdrc_hp_cb[1*7 + 6] ;
extern s32 xdrc_hp_cb2[1*7 + 6] ;

//lp_cb
extern s32 xdrc_lp_exp_cb[1*7 + 6];
//s32 xdrc_lp_exp_cb2[1*7 + 6];
//hp_cb
extern s32 xdrc_hp_exp_cb[1*7 + 6];
//s32 xdrc_hp_exp_cb2[1*7 + 6];
//drc_cb
extern u8 xdrc_drclp_cb[14 * 4];
//drc_cb
extern u8 xdrc_drchp_cb[14 * 4];
//drc_cb
extern u8 xdrc_drcall_cb[14 * 4];
//delay
extern u8  xdrc_delay_cb[16];

enum {
    DRCLPHP_DRC_LP = 0,
    DRCLPHP_DRC_HP,
    DRCLPHP_EQ_LP,
    DRCLPHP_EQ_HP,
};

typedef enum {
    MUSIC_EFFECT_DBB            = 0,        //动态低音音效
    MUSIC_EFFECT_SPATIAL_AUDIO,             //空间音效
    MUSIC_EFFECT_VBASS,                     //虚拟低音音效
    MUSIC_EFFECT_ALG_USER       = 8,        //用户自定义音效算法
    MUSIC_EFFECT_MAX            = 16,
} MUSIC_EFFECT_ALG;


void music_effect_init(void);
void music_effect_sco_audio_init_do(void);
void music_effect_sco_audio_exit_do(void);
void music_effect_sfunc_sleep_do(u8 enter);
int music_effect_set_state(MUSIC_EFFECT_ALG alg, u8 state);
bool music_effect_get_state(MUSIC_EFFECT_ALG alg);

///动态低音音效
void music_dbb_audio_start(void);
void music_dbb_audio_stop(void);
void music_dbb_audio_set_vol_do(u8 vol_level);
void music_dbb_audio_set_bass_level(u8 bass_level);
void xdrc_ram_clear(void);
bool cartool_res_check(char*res_name, u32 addr, u32 len);   //数据和在线调试一样的res检测
void dynamic_eq_param_init(void);
bool dyeq_init_by_res(u8 idx, char*res_name, u32 addr, u32 len);
bool xdrc_delay_init_by_res(char* res_name, u32 addr, u32 len);
void xdrc_delay_samples_init(u16 delay_samples);

void bt_xdrc_processs(u8 *buf, u32 samples, u32 nch, u32 is_24bit, u32 pcm_info);
void drclphp_coef_update(u8 *buf);
#endif //__BSP_EFFECT_H__
