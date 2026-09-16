#include "include.h"

#define CM_SIZE         0x5000          //参数区至少20k
#define CM_START        (FLASH_SIZE - CM_SIZE)


#if SYS_PARAM_RTCRAM
    #define param_read8(a, b)       rtcram_read(a, (u8 *)&b, 1)
    #define param_write8(a, b)      rtcram_write(a, (u8 *)&b, 1)
    #define param_read16(a, b)      rtcram_read(a, (u8 *)&b, 2)
    #define param_write16(a, b)     rtcram_write(a, (u8 *)&b, 2)
    #define param_read32(a, b)      rtcram_read((a, (u8 *)&b, 4)
    #define param_write32(a, b)     rtcram_write(a, (u8 *)&b, 4)
    #define param_read(a, b, c)     rtcram_read(a, b, c)
    #define param_write(a, b, c)    rtcram_write(a, b, c)
    #define param_sync_do()
#else
    #define param_read8(a, b)       b = cm_read8(PAGE0(a))
    #define param_write8(a, b)      cm_write8(PAGE0(a), b)
    #define param_read16(a, b)      b = cm_read16(PAGE0(a))
    #define param_write16(a, b)     cm_write16(PAGE0(a), b)
    #define param_read32(a, b)      b = cm_read32(PAGE0(a))
    #define param_write32(a, b)     cm_write32(PAGE0(a), b)
    #define param_read(a, b, c)     cm_read(a, PAGE0(b), c)
    #define param_write(a, b, c)    cm_write(a, PAGE0(b), c)
    #define param_sync_do()         cm_sync()
#endif


AT(.text.bsp.param)
void param_init(bool reset)
{
    //LANG ID初值
    if ((LANG_SELECT < LANG_EN_ZH) && xcfg_cb.lang_id >= LANG_EN_ZH) {
        printf("Language Select Error\n");
        xcfg_cb.lang_id = LANG_SELECT;
    }
    cm_init(MAX_CM_PAGE, CM_START, CM_SIZE);
    //printf("CM: %x\n", cm_read8(PAGE0(0)));
    //printf("CM: %x\n", cm_read8(PAGE1(0)));

    sys_cb.rand_seed = sys_get_rand_key();

#if SYS_PARAM_RTCRAM
    //RTC专用
    if(reset) {
        sys_cb.vol = SYS_INIT_VOLUME;
        sys_cb.hfp_vol = SYS_INIT_VOLUME / sys_cb.hfp2sys_mul;
        param_sys_vol_write();

        fmrx_cb.ch_cur = 1;
        fmrx_cb.ch_cnt = 1;
        param_fmrx_chcur_write();
        param_fmrx_chcnt_write();
        memset(fmrx_cb.buf, 0, 26);
        param_fmrx_chbuf_write();

#if (LANG_SELECT == LANG_EN_ZH)
        if (xcfg_cb.lang_id == 2) {
            sys_cb.lang_id = 0;             //出厂默认英文
        } else if (xcfg_cb.lang_id == 3) {
            sys_cb.lang_id = 1;             //出厂默认中文
        } else {
            sys_cb.lang_id = xcfg_cb.lang_id;
        }
        param_lang_id_write();
#endif

        param_sync();
    }
#endif
    param_sys_vol_read();
    if ((SYS_LIMIT_VOLUME == 0) || (sys_cb.vol > VOL_MAX)) {
        sys_cb.vol = SYS_INIT_VOLUME;                   //上电还原到默认音量
    } else {
        if (sys_cb.vol < SYS_LIMIT_VOLUME) {
            sys_cb.vol = SYS_LIMIT_VOLUME;
        }
    }
    sys_cb.a2dp_vol = sys_cb.vol;

    param_hfp_vol_read();
    if (sys_cb.hfp_vol > 15) {
        sys_cb.hfp_vol = 15;
    }

    sys_cb.sw_rst_flag = sw_reset_source_get();

}

AT(.text.bsp.param)
void bsp_param_write(u8 *buf, u32 addr, uint len)
{
    param_write(buf, addr, len);
}

AT(.text.bsp.param)
void bsp_param_read(u8 *buf, u32 addr, uint len)
{
    param_read(buf, addr, len);
}

AT(.text.bsp.param)
void bsp_param_sync(void)
{
    param_sync_do();
}


AT(.text.bsp.param)
void param_sys_vol_write(void)
{
    param_write((u8 *)&sys_cb.vol, PARAM_SYS_VOL, 1);
}

AT(.text.bsp.param)
void param_sys_vol_read(void)
{
    param_read((u8 *)&sys_cb.vol, PARAM_SYS_VOL, 1);
}

AT(.text.bsp.param)
void param_hfp_vol_write(void)
{
    param_write((u8 *)&sys_cb.hfp_vol, PARAM_HSF_VOL, 1);
}

AT(.text.bsp.param)
void param_hfp_vol_read(void)
{
    param_read((u8 *)&sys_cb.hfp_vol, PARAM_HSF_VOL, 1);
}

AT(.text.bsp.param)
void param_lang_id_write(void)
{
    param_write((u8 *)&sys_cb.lang_id, PARAM_LANG_ID, 1);
}

AT(.text.bsp.param)
void param_lang_id_read(void)
{
    param_read((u8 *)&sys_cb.lang_id, PARAM_LANG_ID, 1);
}

AT(.text.bsp.param)
void param_sync(void)
{
    param_sync_do();
}

AT(.text.bsp.param)
u32 param_get_xosc_addr(void)
{
    return PARAM_BT_XOSC_CAP;
}

#if SYS_MODE_BREAKPOINT_EN
AT(.text.bsp.param)
u8 param_sys_mode_read(void)
{
    u8 mode = 0;
    param_read((u8 *)&mode, PARAM_SYS_MODE, 1);
    return mode;
}

AT(.text.bsp.param)
void param_sys_mode_write(u8 mode)
{
    if (mode == FUNC_MUSIC) {
        mode = ((sys_cb.cur_dev & 0x0f) << 4) | mode;   //高4bit用来放USB/SD设备选择
    }
    param_write((u8 *)&mode, PARAM_SYS_MODE, 1);
}
#endif


AT(.text.bsp.param.fota)
void param_fot_addr_write(u8 *param)
{
    param_write((u8 *)param, PARAM_FOT_ADDR, 4);
    param_sync();
}

AT(.text.bsp.param.fota)
void param_fot_addr_read(u8 *param)
{
    param_read(param, PARAM_FOT_ADDR, 4);
}

AT(.text.bsp.param.fota)
void param_fot_remote_ver_write(u8 *param)
{
    param_write((u8 *)param, PARAM_FOT_REMOTE_VER, 2);
    param_sync();
}

AT(.text.bsp.param.fota)
void param_fot_remote_ver_read(u8 *param)
{
    param_read(param, PARAM_FOT_REMOTE_VER, 2);
}

AT(.text.bsp.param.fota)
void param_fot_head_info_write(u8 *param)
{
    param_write((u8 *)param, PARAM_FOT_HEAD_INFO, 8);
    param_sync();
}

AT(.text.bsp.param.fota)
void param_fot_head_info_read(u8 *param)
{
    param_read(param, PARAM_FOT_HEAD_INFO, 8);
}

AT(.text.bsp.param.fota)
void param_fot_hash_write(u8 *param)
{
    param_write((u8 *)param, PARAM_FOT_HASH, 4);
    param_sync();
}

AT(.text.bsp.param.fota)
void param_fot_hash_read(u8 *param)
{
    param_read(param, PARAM_FOT_HASH, 4);
}

AT(.text.bsp.param.fota)
void param_fot_type_write(u8 *param)
{
    param_write((u8 *)param, PARAM_FOT_TYPE, 1);
    param_sync();
}

AT(.text.bsp.param.fota)
void param_fot_type_read(u8 *param)
{
    param_read(param, PARAM_FOT_TYPE, 1);
}
