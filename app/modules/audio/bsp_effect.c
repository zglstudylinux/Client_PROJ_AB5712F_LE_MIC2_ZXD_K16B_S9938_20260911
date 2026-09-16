/*****************************************************************************
 * Copyright (c) 2023 Shenzhen Bluetrum Technology Co.,Ltd. All rights reserved.
 * File      : bsp_effect.c
 * Function  : 音乐音效模块
 * History   :
 * Created by yutao on 2023-12-1.
 *****************************************************************************/
#include "include.h"



#if BT_MUSIC_EFFECT_EN

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif


void load_code_audio_comm(void);
int music_effect_set_state(MUSIC_EFFECT_ALG alg, u8 state);

extern const u16 dac_dvol_tbl_db[61];
extern const u8 *dac_dvol_table;


typedef struct {
    //音效状态机
    volatile u16 music_effect_state;
    volatile u8 process_flag;
    //主频
    u8 sys_clk;
} music_effect_t;

music_effect_t music_effect;

//状态位
#define DBB_STA_BIT             BIT(MUSIC_EFFECT_DBB)
#define SPATIAL_AUDIO_STA_BIT   BIT(MUSIC_EFFECT_SPATIAL_AUDIO)
#define VBASS_STA_BIT           BIT(MUSIC_EFFECT_VBASS)
#define USER_ALG_STA_BIT        BIT(MUSIC_EFFECT_ALG_USER)
#define DBB_EN(x)               ((x) & (DBB_STA_BIT))
#define SPATIAL_AUDIO_EN(x)     ((x) & (SPATIAL_AUDIO_STA_BIT))
#define VBASS_EN(x)             ((x) & (VBASS_STA_BIT))
#define USER_ALG_EN(x)          ((x) & (USER_ALG_STA_BIT))
//主频设置
#define DBB_SYSCLK_SEL          SYS_24M
#define USER_ALG_SYSCLK_SEL     SYS_48M


#if BT_MUSIC_EFFECT_USER_EN
AT(.audio_text.user)
void alg_user_effect_process(u8 *buf, u32 samples, u32 nch, u32 is_24bit, u32 pcm_info)
{
    //用户自定义音效算法处理示例函数
    //算法中用到的函数可以放在audio_text段或者com_text段，以提高算法效率
    //算法中用到的buffer需要放到music_buff段或者music_exbuff段

}

void alg_user_effect_init(void)
{
    //用户自定义音效算法初始化示例函数
    printf("alg_user_effect_init\n");

}
#endif // BT_MUSIC_EFFECT_USER_EN

///音效算法处理函数
AT(.com_text.codecs.pcm)
void msc_pcm_effect_process(u8 *buf, u32 samples, u32 nch, u32 is_24bit, u32 pcm_info)
{
    music_effect_t* cb = &music_effect;
    u16 state = cb->music_effect_state;

//    if (pcm_info & BIT(0)) {
//        //first frame
//    }

#if 1
    bt_xdrc_processs(buf, samples,nch, is_24bit, pcm_info);
#endif

    if (state) {
        cb->process_flag = 1;
#if BT_MUSIC_EFFECT_USER_EN
        if (USER_ALG_EN(state)) {      //用户自定义的算法处理
            alg_user_effect_process(buf, samples, nch, is_24bit, pcm_info);
        }
#endif // BT_MUSIC_EFFECT_USER_EN
        cb->process_flag = 0;
    }
}

void xdrc_cfg_init(void)
{
   u32 cfg = 0x00;
    if(xcfg_cb.xdrc_dyeq_en) {
        cfg |= XDRC_DYEQ_EN;
    }
    if(xcfg_cb.xdrc_preeq_en) {
        cfg |= XDRC_PREEQ_EN;
    }
    if(xcfg_cb.xdrc_lphp_en) {
        cfg |= XDRC_LPHP_EN;
        if(xcfg_cb.xdrc_delaybuf_en) {
            cfg |= XDRC_DELAYBUF_EN;
        }
    }
    if(xcfg_cb.xdrc_alldrc_en) {
        cfg |= XDRC_ALLDRC_EN;
    }
    xdrc_cfg = cfg;
    printf("\n=================>xdrc_cfg_init = 0x%X\n",xdrc_cfg);
}

#define XDRC_DRCLP_OFFSET    16
#define XDRC_DRCHP_OFFSET    67
#define XDRC_EQLP_OFFSET     118
#define XDRC_EQHP_OFFSET     177
AT(.text.xdrc)
bool xdrc_lphp_init_by_res(char*res_name, u32 addr, u32 len)
{
    u8 idx[4];
    printf("\n============>%s\n",__func__);
    if(!cartool_res_check(res_name,addr,len)) {
        printf("!!!res crc check err\n");
        return false;
    }
    u8 *buf = (u8*)addr;
    u16 size = GET_BUF_LE_U16(buf,12);
    if (size != 224) {
        printf("!!!res size error, cur = %d, need=%d\n", size, 224);
        return false;
    }
    idx[0] = buf[XDRC_DRCLP_OFFSET+2];
    idx[1] = buf[XDRC_DRCHP_OFFSET+2];
    idx[2] = buf[XDRC_EQLP_OFFSET+2];
    idx[3] = buf[XDRC_EQHP_OFFSET+2];
    print_r(idx,4);
    u32 idxval = (idx[3]<<24) | (idx[2]<<16) | (idx[1]<<8) | idx[0];
    if(idxval != (u32)0x03020100) {
        printf("index err:0x%X\n",idxval);
        return false;
    }
    drclphp_coef_update(&buf[XDRC_DRCLP_OFFSET]);
    drclphp_coef_update(&buf[XDRC_DRCHP_OFFSET]);
    drclphp_coef_update(&buf[XDRC_EQLP_OFFSET]);
    drclphp_coef_update(&buf[XDRC_EQHP_OFFSET]);
    return true;
}

///音效部分公共接口
//初始化
void music_effect_init(void)
{
    memset(&music_effect, 0, sizeof(music_effect_t));
    music_effect.sys_clk = SYS_24M;
    load_code_audio_comm();

    printf("music_effect_init\n");
    xdrc_ram_clear();
//    DY EQ
    dynamic_eq_param_init();
    if(!dyeq_init_by_res(1,"DY_EQ",RES_BUF_XDRC_DYEQ_BIN,RES_LEN_XDRC_DYEQ_BIN)) {
        printf_end("!!!DY_EQ RES INIT ERROR");
    }
    //PRE EQ
    exlib_softeq_cb_init(xdrc_preeq_cb, sizeof(xdrc_preeq_cb),RES_BUF_XDRC_PRE_EQ,RES_LEN_XDRC_PRE_EQ);

    if(!xdrc_lphp_init_by_res("DRC_LPHP",RES_BUF_XDRC_DRCLPHP_BIN,RES_LEN_XDRC_DRCLPHP_BIN)) {
        printf_end("!!!DRC_LP_HP RES INIT ERR");
    }

    xdrc_delay_samples_init(10);  //该函数初始化delay模块结构体,不能去掉
    if(!xdrc_delay_init_by_res("DELAY",RES_BUF_XDRC_DELAY_BIN,RES_LEN_XDRC_DELAY_BIN)){
        printf_end("!!!DELAY RES INIT ERR");
    }
    //DRC ALL
    exlib_drcv1_cb_init(xdrc_drcall_cb,sizeof(xdrc_drcall_cb),RES_BUF_XDRC_ALL_DRC,RES_LEN_XDRC_ALL_DRC);
    //POST EQ
    music_set_eq_by_res(RES_BUF_XDRC_POST_EQ, RES_LEN_XDRC_POST_EQ);
    xdrc_cfg_init();

    //现在这里拉高主频
    sys_clk_set(SYS_160M);
}

//进入通话
void music_effect_sco_audio_init_do(void)
{

}

//退出通话
void music_effect_sco_audio_exit_do(void)
{
    load_code_audio_comm();
#if BT_MUSIC_EFFECT_USER_EN
    alg_user_effect_init();
#endif // BT_MUSIC_EFFECT_USER_EN
}

//进出休眠
void music_effect_sfunc_sleep_do(u8 enter)
{
    if (enter == 0) {                    //退出休眠
        sys_clk_req(INDEX_KARAOK, music_effect.sys_clk);
    } else {                             //进入休眠

    }
}

//设置音效状态
int music_effect_set_state(MUSIC_EFFECT_ALG alg, u8 state)
{
    if (alg >= MUSIC_EFFECT_MAX) {
        return -1;
    }

    music_effect_t* cb = &music_effect;
    u8 delay_cnt = 0;
    u8 sys_clk_req_set = SYS_24M;
    u16 new_state = cb->music_effect_state;

    if (state) {
        if (new_state & BIT(alg)) {
            return -2;
        }
        new_state |= BIT(alg);
    } else {
        if ((new_state & BIT(alg)) == 0) {
            return -2;
        }
        new_state &= ~BIT(alg);
    }

    //调整音效的主频设置
    if (new_state) {
        switch (alg) {
#if BT_MUSIC_EFFECT_DBB_EN
        case MUSIC_EFFECT_DBB:
            sys_clk_req_set = DBB_SYSCLK_SEL;
            break;
#endif // BT_MUSIC_EFFECT_DBB_EN

#if BT_MUSIC_EFFECT_USER_EN
        case MUSIC_EFFECT_ALG_USER:
            alg_user_effect_init();
            sys_clk_req_set = USER_ALG_SYSCLK_SEL;
            break;
#endif // BT_MUSIC_EFFECT_USER_EN

        default:
            break;
        }
        if (sys_clk_req_set > cb->sys_clk) {
            sys_clk_req(INDEX_KARAOK, sys_clk_req_set);
            cb->sys_clk = sys_clk_req_set;
        }
    }

    //设置状态
    cb->music_effect_state = new_state;

    //释放音效的主频设置
    if (new_state == 0) {
        while (cb->process_flag) {
            WDT_CLR();
            delay_5ms(1);
            delay_cnt++;
            if (!codecs_pcm_is_start()) {   //音乐播放结束就不等了
                break;
            }
            if (delay_cnt >= 200) {
                printf("music_effect_set_state time out!\n");
                return -3;
            }
        }
        sys_clk_free(INDEX_KARAOK);
        cb->sys_clk = SYS_24M;
    }

    TRACE("music_effect_set_state %x\n", new_state);

    return 0;
}

//获取音效状态
AT(.com_text.effect)
bool music_effect_get_state(MUSIC_EFFECT_ALG alg)
{
    music_effect_t* cb = &music_effect;
    return (bool)((cb->music_effect_state & BIT(alg)) > 0);
}

///动态低音音效
#if BT_MUSIC_EFFECT_DBB_EN

const u8 dbb_coef_param[453] = {
/* 等级总数：11    	*/
/* 滤波器类型：peak  	*/
/* 中心频率：50    	*/
/* Q值：0.75      	*/
/* 增益：-10, -8, -6, -4, -2, 0, 2, 4, 6, 8, 10,   	*/
	0x02, 0x0b, 0xf6, 0xf8, 0xfa, 0xfc, 0xfe, 0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0xcf, 0x37, 0xf5,
	0x07, 0xc5, 0x3e, 0xeb, 0x07, 0x4c, 0x60, 0xe0, 0x0f, 0xb5, 0x9f, 0x1f, 0xf0, 0x6c, 0x89, 0x1f,
	0xf8, 0xbe, 0x88, 0xf7, 0x07, 0xc9, 0x55, 0xec, 0x07, 0x39, 0xc8, 0xe3, 0x0f, 0xc8, 0x37, 0x1c,
	0xf0, 0x7a, 0x21, 0x1c, 0xf8, 0x09, 0xbe, 0xf9, 0x07, 0xda, 0x2a, 0xed, 0x07, 0x90, 0xd2, 0xe6,
	0x0f, 0x71, 0x2d, 0x19, 0xf0, 0x1f, 0x17, 0x19, 0xf8, 0xfc, 0xde, 0xfb, 0x07, 0x98, 0xc0, 0xed,
	0x07, 0x3e, 0x89, 0xe9, 0x0f, 0xc3, 0x76, 0x16, 0xf0, 0x6d, 0x60, 0x16, 0xf8, 0xaa, 0xf2, 0xfd,
	0x07, 0xd7, 0x18, 0xee, 0x07, 0x27, 0xf5, 0xeb, 0x0f, 0xda, 0x0a, 0x14, 0xf0, 0x80, 0xf4, 0x13,
	0xf8, 0x00, 0x00, 0x00, 0x08, 0xa2, 0x34, 0xee, 0x07, 0x46, 0x1e, 0xee, 0x0f, 0xbb, 0xe1, 0x11,
	0xf0, 0x5f, 0xcb, 0x11, 0xf8, 0xdd, 0x0d, 0x02, 0x08, 0x3e, 0x14, 0xee, 0x07, 0xbc, 0x0b, 0xf0,
	0x0f, 0x45, 0xf4, 0x0f, 0xf0, 0xe6, 0xdd, 0x0f, 0xf8, 0x27, 0x23, 0x04, 0x08, 0x28, 0xb7, 0xed,
	0x07, 0xed, 0xc3, 0xf1, 0x0f, 0x14, 0x3c, 0x0e, 0xf0, 0xb2, 0x25, 0x0e, 0xf8, 0xe0, 0x46, 0x06,
	0x08, 0x13, 0x1c, 0xed, 0x07, 0x90, 0x4c, 0xf3, 0x0f, 0x71, 0xb3, 0x0c, 0xf0, 0x0e, 0x9d, 0x0c,
	0xf8, 0x41, 0x80, 0x08, 0x08, 0xe4, 0x40, 0xec, 0x07, 0xbf, 0xaa, 0xf4, 0x0f, 0x42, 0x55, 0x0b,
	0xf0, 0xdd, 0x3e, 0x0b, 0xf8, 0xcc, 0xd6, 0x0a, 0x08, 0xa7, 0x22, 0xeb, 0x07, 0x0c, 0xe3, 0xf5,
	0x0f, 0xf5, 0x1c, 0x0a, 0xf0, 0x8e, 0x06, 0x0a, 0xf8, 0xc3, 0x45, 0xf4, 0x07, 0xd6, 0x6c, 0xe9,
	0x07, 0x37, 0x98, 0xdd, 0x0f, 0xca, 0x67, 0x22, 0xf0, 0x68, 0x4d, 0x22, 0xf8, 0x84, 0xca, 0xf6,
	0x07, 0xeb, 0x9b, 0xea, 0x07, 0x06, 0x4c, 0xe1, 0x0f, 0xfb, 0xb3, 0x1e, 0xf0, 0x92, 0x99, 0x1e,
	0xf8, 0x4f, 0x31, 0xf9, 0x07, 0x57, 0x83, 0xeb, 0x07, 0x38, 0x9a, 0xe4, 0x0f, 0xc9, 0x65, 0x1b,
	0xf0, 0x5b, 0x4b, 0x1b, 0xf8, 0x10, 0x82, 0xfb, 0x07, 0xf0, 0x25, 0xec, 0x07, 0x8d, 0x8d, 0xe7,
	0x0f, 0x74, 0x72, 0x18, 0xf0, 0x01, 0x58, 0x18, 0xf8, 0x74, 0xc4, 0xfd, 0x07, 0xae, 0x85, 0xec,
	0x07, 0xab, 0x2f, 0xea, 0x0f, 0x56, 0xd0, 0x15, 0xf0, 0xdf, 0xb5, 0x15, 0xf8, 0x00, 0x00, 0x00,
	0x08, 0xb0, 0xa3, 0xec, 0x07, 0x35, 0x89, 0xec, 0x0f, 0xcc, 0x76, 0x13, 0xf0, 0x51, 0x5c, 0x13,
	0xf8, 0x2c, 0x3c, 0x02, 0x08, 0x3d, 0x80, 0xec, 0x07, 0xeb, 0xa1, 0xee, 0x0f, 0x16, 0x5e, 0x11,
	0xf0, 0x98, 0x43, 0x11, 0xf8, 0x77, 0x80, 0x04, 0x08, 0xc4, 0x1a, 0xec, 0x07, 0xb9, 0x80, 0xf0,
	0x0f, 0x48, 0x7f, 0x0f, 0xf0, 0xc6, 0x64, 0x0f, 0xf8, 0x81, 0xd4, 0x06, 0x08, 0xd9, 0x71, 0xeb,
	0x07, 0xd5, 0x2b, 0xf2, 0x0f, 0x2c, 0xd4, 0x0d, 0xf0, 0xa8, 0xb9, 0x0d, 0xf8, 0x22, 0x40, 0x09,
	0x08, 0x2e, 0x83, 0xea, 0x07, 0xc9, 0xa8, 0xf3, 0x0f, 0x38, 0x57, 0x0c, 0xf0, 0xb1, 0x3c, 0x0c,
	0xf8, 0x88, 0xcb, 0x0b, 0x08, 0x8d, 0x4b, 0xe9, 0x07, 0x8c, 0xfc, 0xf4, 0x0f, 0x75, 0x03, 0x0b,
	0xf0, 0xec, 0xe8, 0x0a, 0xf8,
};

void music_dbb_eq_index_init(u32* coef_l, u32* coef_r)
{
    dbb_param_cb_t cb;
    cb.dbb_param = dbb_coef_param;
    cb.param_len = sizeof(dbb_coef_param);
    cb.coef_l = (s32*)coef_l;
    cb.coef_r = (s32*)coef_r;
    cb.dac_band_cnt = BT_MUSIC_EFFECT_DBB_BAND_CNT;
    music_dbb_init(&cb);
}

void music_dbb_audio_start(void)
{
    music_effect_t* cb = &music_effect;
    u8 vol_level = dac_dvol_table[sys_cb.vol] + sys_cb.gain_offset;

    if (DBB_EN(cb->music_effect_state)) {
        return;
    }

    if (vol_level > 60) {
        vol_level = 60;
    }
    music_dbb_update_param(vol_level, BT_MUSIC_EFFECT_DBB_DEF_LEVEL);
    music_set_eq_by_num(sys_cb.eq_mode);
    music_effect_set_state(MUSIC_EFFECT_DBB, 1);
}

void music_dbb_audio_stop(void)
{
    music_effect_t* cb = &music_effect;

    if (DBB_EN(cb->music_effect_state) == 0) {
        return;
    }

    music_effect_set_state(MUSIC_EFFECT_DBB, 0);
    music_dbb_stop();
    music_set_eq_by_num(sys_cb.eq_mode);
}

void music_dbb_audio_set_vol_do(u8 vol_level)
{
    u8 bass_level = music_dbb_get_bass_level();
    int res = music_dbb_update_param(vol_level, bass_level);
    music_effect_t* cb = &music_effect;

    if (DBB_EN(cb->music_effect_state) == 0) {
        return;
    }

    //0:不需要change eq，1:先change eq再调音量，2:先调音量再change eq
    if (res == 1) {
        music_set_eq_by_num(sys_cb.eq_mode);
        dac_vol_set(dac_dvol_tbl_db[vol_level]);
    } else if (res == 2) {
        dac_vol_set(dac_dvol_tbl_db[vol_level]);
        music_set_eq_by_num(sys_cb.eq_mode);
    } else {
        dac_vol_set(dac_dvol_tbl_db[vol_level]);
    }
    TRACE("bass_level: %d, vol: -%d dB\n", bass_level, vol_level);
}

void music_dbb_audio_set_bass_level(u8 bass_level)
{
    u8 vol_level = dac_dvol_table[sys_cb.vol] + sys_cb.gain_offset;
    music_effect_t* cb = &music_effect;

    if (DBB_EN(cb->music_effect_state) == 0) {
        return;
    }

    if (vol_level > 60) {
        vol_level = 60;
    }
    music_dbb_update_param(vol_level, bass_level);
    music_set_eq_by_num(sys_cb.eq_mode);
    TRACE("bass_level: %d, vol: -%d dB\n", bass_level, vol_level);
}

#endif // BT_MUSIC_EFFECT_DBB_EN

#endif // BT_MUSIC_EFFECT_EN
