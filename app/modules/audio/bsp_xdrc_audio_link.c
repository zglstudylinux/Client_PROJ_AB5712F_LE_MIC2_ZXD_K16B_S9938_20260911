#include "include.h"

//需要和APP中的一致
#define XDRC_DYEQ_EN       BIT(0)
#define XDRC_PREEQ_EN      BIT(1)
#define XDRC_LPHP_EN       BIT(2)
#define XDRC_DELAYBUF_EN   BIT(3)    //XDRC_LPHP_EN 打开时才有效
#define XDRC_ALLDRC_EN     BIT(4)
#define XDRC_ALL_ALG_EN    (XDRC_DYEQ_EN | XDRC_PREEQ_EN | XDRC_LPHP_EN | XDRC_DELAYBUF_EN | XDRC_ALLDRC_EN)

#define GET_BUF_LE_FLOAT(buf, ofs)        *(float *)((uint8_t *)buf + (ofs))
#define GET_BUF_LE_U32(buf, ofs)          *(uint32_t *)((uint8_t *)buf + (ofs))


//----dump  api-------------------------------------------------------------
//head_buf:   dump模块需要的包头管理buf,长度为 file_total*14
//file_toal:  dump模块同时支持导出的文件个数. 目前PC端软件UartDump暂时只支持最多同时接收6路并保存.
//putbuf:     底层dump函数,这里一般为高速串口DMA发送函数
//wait:       dump等待函数,下一次dump_putbuf时,需要等待上一次putbuf结束
void dump_buf_init(u8 *head_buf, int file_toal,void (*putbuf)(void*buf,u32 len), void (*wait)(void));

//dump_dma_buf:长度需要大于或等于 sizeof(tx_buf) + 14, 其中14是包头的长度. tx_buf中的数据会先打包放到dump_dma_buf中待发送
//tx_buf:      导出的数据存放的buf
//tx_buf_len:  导出数据的长度
//file_idx: 文件序号,0表示UartDump收到数据后保存到文件0, 1表示保存到文件1...
void dump_putbuf(void *dump_dma_buf, void *tx_buf, int tx_buf_len, u8 file_idx);
//为了减少dma等待时间,可以先通过dump_put2ram把数据封包放到ram中,再调用dump_dma_kick一次性导出数据
void dump_put2ram(void *dump_dma_buf, void *tx_buf, int tx_buf_len, u8 file_idx);
void dump_dma_kick(void *dma_buf,u32 dma_len);  //kick一次dma发送
void dump_dma_wait(void);  //等待上一次发送完成
//hart api
void huart_module_init(void);
void huart_wait_tx_finish(void);
void huart_putbuf(void *buf, u32 len);
//----------------------------------------------------------------
//soft_eq apis
u32 exlib_softeq_cb_init(void *cb, u32 cb_len, u32 res_addr, u32 res_len);
void exlib_softeq_coef_update(void *cb, u32 cb_len, u32 *coef, u32 band, bool pregain);  //pregain = 1 , first u32 is pregain,  next is band coef  //pregain=0, all coef is band coef
s32 exlib_softeq_proc(void *cb, s32 input) ;
s16 exlib_softeq_stereo_proc_s16(void *cb, s16 data);
s32 exlib_drc_process_gain(void *cb, s32 data_in);
s16 exlib_drc_get_pcm16_s(s32 data, int g);
s32 exlib_drc_get_pcm32_s(s32 data, int g);
u32 exlib_softeq_band_get(void *cb);
//drc apis
void exlib_drcv1_cb_init(void *cb, u32 cb_len, u32 res_addr, u32 res_len); //drc资源文件更新cb参数
void exlib_drcv1_coef_update(void* cb, u32 *tbl);      //在线调试更新参数
s16 exlib_drc_process_s16(void *cb, s32 data_in);     //饱和到16 bit
s32 exlib_drc_process_s24(void *cb, s32 data_in);     //饱和到24 bit  //undo
s16 exlib_s_clip16(int x);
//-------------------------------------------------------------------------
//pcm delay apis
bool pcmdelay_init(void *cb,void *delay_buf, u16 delay_buf_len, u16 samples_size, u16 delay_samples);
u32 pcmdelay_get_cb_size(void);
u16 pcmdelay_get_delay_byte(void *handle);
s16 pcmdelay_mono_s16(void *handle, s16 pcm);
s32 pcmdelay_mono_s32(void *handle, s32 pcm);
bool pcmdelay_coef_update(void *handle, u16 delay_samples);
bool cartool_res_check(char*res_name, u32 addr, u32 len);
//-------------------------------------------------------------------------
//dy eq apis
void dynamic_eq_process(s32 *samples, u8 idx);
//-------------------------------------------------------------------------
void xdrc_mono_process(u8 *buf, u32 samples);
s16 xdrc_vol_proc(s16 pcm, u16 gain);
volatile u16 sfr_xdrc_pre_vol = 0x7FFF;
volatile u32 xdrc_cfg = 0x00;   //BIT(1) DYEQ   //BIT(0)
//-------------------------------------------------------------------------
//以下cb数组长度需要和api_xdrc.h中一一对应
//max eq band 10
s32 xdrc_preeq_cb[10*7 + 6] AT(.xdrc_ram);  //*coef, *zpara, band, pre_gain, coef(5*BAND), calc_buf((BAND+1)*2)
//lp_cb
s32 xdrc_lp_cb[1*7 + 6] AT(.xdrc_ram);
s32 xdrc_lp_cb2[1*7 + 6] AT(.xdrc_ram);   //分频点过两次
//hp_cb
s32 xdrc_hp_cb[1*7 + 6] AT(.xdrc_ram);
s32 xdrc_hp_cb2[1*7 + 6] AT(.xdrc_ram);   //分频点过两次
//lp_cb
s32 xdrc_lp_exp_cb[1*7 + 6] AT(.xdrc_ram);
//s32 xdrc_lp_exp_cb2[1*7 + 6];
//hp_cb
s32 xdrc_hp_exp_cb[1*7 + 6] AT(.xdrc_ram);
//s32 xdrc_hp_exp_cb2[1*7 + 6];
//drc_cb
u8 xdrc_drclp_cb[14 * 4] AT(.xdrc_ram);
//drc_cb
u8 xdrc_drchp_cb[14 * 4] AT(.xdrc_ram);
//drc_cb
u8 xdrc_drcall_cb[14 * 4] AT(.xdrc_ram);

u8  xdrc_delay_cb[16] AT(.xdrc_ram);
s32 xdrc_delay_buf[256] AT(.xdrc_ram);

//-------------------------------------------------------------------------
AT(.text.xdrc)
void xdrc_ram_clear(void)
{
    memset(xdrc_preeq_cb,0,sizeof(xdrc_preeq_cb));
    memset(xdrc_lp_cb,0,sizeof(xdrc_lp_cb));
    memset(xdrc_lp_cb2,0,sizeof(xdrc_lp_cb2));
    memset(xdrc_hp_cb,0,sizeof(xdrc_hp_cb));
    memset(xdrc_hp_cb2,0,sizeof(xdrc_hp_cb2));
    memset(xdrc_lp_exp_cb,0,sizeof(xdrc_lp_exp_cb));
    memset(xdrc_hp_exp_cb,0,sizeof(xdrc_hp_exp_cb));
    memset(xdrc_drclp_cb,0,sizeof(xdrc_drclp_cb));
    memset(xdrc_drchp_cb,0,sizeof(xdrc_drchp_cb));
    memset(xdrc_drcall_cb,0,sizeof(xdrc_drcall_cb));
    memset(xdrc_delay_cb,0,sizeof(xdrc_delay_cb));
    memset(xdrc_delay_buf,0,sizeof(xdrc_delay_buf));
}
//-------------------------------------------------------------------------
//pre vol
AT(.com_rodata.dac)
const char str_sfr_prevol_set[]  = "\n===>volset callback,pre_vol = 0x%X\n";

AT(.com_text.volset)
bool dac_src_vol_set_callback(uint src_idx, u32 vol)
{
    if (0 == xdrc_cfg) {
        return false;
    }
    if (0 == src_idx) {
        if (vol > 0x7FFF) {
            vol = 0x7FFF;
        }
        sfr_xdrc_pre_vol = vol;
        if((SRC0VOLCON & 0x7FFF) != 0x7FFF) {
            SRC0VOLCON = 0x7FFF | (4<<24);
        }
        printf(str_sfr_prevol_set,sfr_xdrc_pre_vol);
        return true;
    } else {
        return false;
    }
}

AT(.text.xdrc)
void xdrc_pre_vol_set(u16 vol)
{
    sfr_xdrc_pre_vol = vol;
    printf("==>xdrc_pre_vol_set = 0x%X\n", vol);
}

AT(.com_text.xdrc)
s16 xdrc_vol_proc(s16 pcm, u16 gain)
{
    s16 ret =  (pcm * gain)>>15;
    return ret;
}

AT(.text.eq_online_adj)
void xdrc_pre_vol_update(u8 *buf)
{
    u32 vol = GET_BUF_LE_U32(buf,2);
    printf("vol recv = 0x%X\n",vol);
    if (vol > 0x8000) {
        vol = 0x8000;
    }
    xdrc_pre_vol_set((u16)vol);
}

AT(.com_text.exlib_drc_v1)
s16 exlib_s_clip16(int x)
{
	if      (x < -32768) x = -32768;
	else if (x >  32767) x =  32767;
	return  x;
}
//-------------------------------------------------------------------------
//drc delay buf init
AT(.text.xdrc)
void xdrc_delay_samples_init(u16 delay_samples)
{
    pcmdelay_init(xdrc_delay_cb,xdrc_delay_buf,sizeof(xdrc_delay_buf),4,delay_samples);
    printf("pcmdelay_get_delay_byte = %d\n", pcmdelay_get_delay_byte(xdrc_delay_cb));
}

AT(.text.xdrc)
void xdrc_delay_samples_update(u8* buf)
{
    if(buf[1]) {  //en
        xdrc_cfg |= XDRC_DELAYBUF_EN;
    }else {
        xdrc_cfg &= ~XDRC_DELAYBUF_EN;
    }
    u16 delay_samples = GET_BUF_LE_U16(buf,2);
    printf("delaybuf coef, en = %d, delaysamples = %d\n",buf[1], delay_samples);
    pcmdelay_coef_update(xdrc_delay_cb,delay_samples);
}

AT(.text.xdrc)
bool xdrc_delay_init_by_res(char* res_name, u32 addr, u32 len)
{
    u8 *rx_buf = (u8*)addr;
	printf("\n==================>lib xdrc_delay_init_by_res:\n");
    if(!cartool_res_check(res_name,addr,len)) {
        return false;
    }
    u16 size = GET_BUF_LE_U16(rx_buf,12);
    if (size != 6) {
        printf("!!!delay res size error, cur = %d, need=%d\n", size, 6);
        return false;
    }
    xdrc_delay_samples_update(&rx_buf[14]);
    return true;
}


//-------------------------------------------------------------------------
AT(.com_text.xdrc)
void pcm_dual_2_mon(u8 *buf, u32 samples)
{
    s16 *ptr = (s16*)buf;
    for (int i = 0; i < samples; i++) {
        ptr[i] = ptr[2*i]/2 +  ptr[2*i + 1]/2;
    }
}

AT(.com_text.xdrc)
void pcm_mon_2_dual(u8 *buf, u32 samples)
{
    s16 *ptr = (s16*)buf;
    for (int i = samples - 1; i >= 0; i--) {
        ptr[2*i+1] = ptr[i];
        ptr[2*i]   = ptr[i];
    }
}

AT(.com_text.xdrc)
void xdrc_dual_mix2mon_process(u8 *buf, u32 samples)
{
    pcm_dual_2_mon(buf,samples);
    xdrc_mono_process(buf,samples);
    pcm_mon_2_dual(buf,samples);
}


#define XDRC_TIME_IO_TEST      0      //IO 测试XDRC算法耗时  //PB1
#define XDRC_DUMP_EN           0      //打开它后不能在线调音
#define XDRC_INFO_PRINT        0


#if XDRC_INFO_PRINT
AT(.com_text.xdrc_dbg_str)
const char str_bt_xdrc1[] = "btxdrc,samples = %d, xdrc_cfg = 0x%X, vol = 0x%X, delay(%d)= %d\n";
AT(.com_text.xdrc_dbg_str)
const char str_xdrc_cfg_dyeq[] = "dyeq->";
AT(.com_text.xdrc_dbg_str)
const char str_xdrc_cfg_preq[] = "preq->";
AT(.com_text.xdrc_dbg_str)
const char str_xdrc_cfg_lphp[] = "lphp->";
AT(.com_text.xdrc_dbg_str)
const char str_xdrc_cfg_delaybuf[] = "delaybuf->";
AT(.com_text.xdrc_dbg_str)
const char str_xdrc_cfg_alldrc[] = "alldrc->";
AT(.com_text.xdrc_dbg_str)
const char str_xdrc_cfg_n[] = "\n";

AT(.com_text.xdrc_dbg)
void print_drc_cfg_info(void)
{
    u32 cfg = xdrc_cfg;
    if(cfg & (XDRC_DYEQ_EN)) {
        printf(str_xdrc_cfg_dyeq);
    }
    if(cfg & (XDRC_PREEQ_EN)) {
        printf(str_xdrc_cfg_preq);
    }
    if(cfg & (XDRC_LPHP_EN)) {
        printf(str_xdrc_cfg_lphp);
    }
    if(cfg & (XDRC_DELAYBUF_EN)) {
        printf(str_xdrc_cfg_delaybuf);
    }
    if(cfg & (XDRC_ALLDRC_EN)) {
        printf(str_xdrc_cfg_alldrc);
    }
    printf(str_xdrc_cfg_n);

}
#endif

AT(.com_text.xdrc)
void bt_xdrc_processs(u8 *buf, u32 samples, u32 nch, u32 is_24bit, u32 pcm_info)
{
#if XDRC_INFO_PRINT
    static u32 ticks = 0;
    if (tick_check_expire(ticks,1000)) {
        ticks = tick_get();
        printf(str_bt_xdrc1,samples,xdrc_cfg,sfr_xdrc_pre_vol, (bool)(xdrc_cfg&XDRC_DELAYBUF_EN), pcmdelay_get_delay_byte(xdrc_delay_cb)/4);
        print_drc_cfg_info();
    }
#endif
    pcm_dual_2_mon(buf,samples);
#if XDRC_TIME_IO_TEST
    GPIOBSET = BIT(1);
#endif
    xdrc_mono_process(buf,samples);

#if XDRC_TIME_IO_TEST
    GPIOBCLR = BIT(1);
#endif
    pcm_mon_2_dual(buf,samples);

}

//-------------------------------------------------------------------------
//xdrc audio link
#if XDRC_DUMP_EN
static u8 xdrc_dump_head_buf[14*2];   //管理包头需要的buf,uartDump工具目前支持最大同时保存生成6个文件,这里AUX只用到了1路
static u8 xdrc_dump_buf[2][14+512*2 + 14+512] AT(.huart_dump);  //file0: LP+RP 双声道   //file1 MIX
#endif

void xdrc_huart_dump_init(void)
{
#if XDRC_DUMP_EN
    printf("------> %s\n",__func__);
    huart_module_init();    //初始化高速串口模块
    dump_buf_init(xdrc_dump_head_buf,2, huart_putbuf, huart_wait_tx_finish);  //初始化dump模块
#endif

#if XDRC_TIME_IO_TEST
    //PB1 FOR TEST
    GPIOBFEN &= ~BIT(1);   //PB1
    GPIOBDE |= BIT(1);
    GPIOBDIR &= ~BIT(1);
#endif
}


#define DUMP_RES        1    //dump 资源文件测试
#define DUMP_LPHP_MIX   0

AT(.com_text.xdrc)
void xdrc_mono_process(u8 *buf, u32 samples)   //输入为单声数据
{
    s16 pcm16;
    s32 pcm32;
    s32 pcm32_lp;
    s32 pcm32_hp;
    s32 pcm32_lp_exp, drc_gain_lp;
    s32 pcm32_hp_exp, drc_gain_hp;
    s16 *ptr = (s16*)buf;
    u32 cfg_xdrc = xdrc_cfg;

#if XDRC_DUMP_EN && DUMP_LPHP_MIX
    s16 *p16_lphp = (s16*)&xdrc_dump_buf[0][14];
    s16 *p16_mix = (s16*)&xdrc_dump_buf[0][14+512*2 + 14];
#endif

#if XDRC_DUMP_EN && DUMP_RES
    s16 *p16_res = (s16*)&xdrc_dump_buf[0][14];
#endif

    for (int i = 0; i < samples; i++) {
        pcm16 = ptr[i];              //lpcm
 #if XDRC_DUMP_EN && DUMP_RES
        p16_res[2*i] = pcm16;
 #endif
        //PRE VOLUME
        pcm32 = xdrc_vol_proc(pcm16, sfr_xdrc_pre_vol);

        //XDRC_DYEQ_EN
        if (cfg_xdrc & XDRC_DYEQ_EN) {
            dynamic_eq_process(&pcm32, 1);   //DYEQ
        }

         //XDRC_PREEQ_EN
        if (cfg_xdrc & XDRC_PREEQ_EN) {
            pcm32 = exlib_softeq_proc(xdrc_preeq_cb,pcm32);  //6~8 PRE EQ软件EQ
        }

        //XDRC_LPHP_EN
        if (cfg_xdrc & XDRC_LPHP_EN) {
            //LP/HP filter for DrcGain  //计算DRC gain时放宽频带范围
            pcm32_lp_exp = exlib_softeq_proc(xdrc_lp_exp_cb,pcm32);
            //pcm32_lp_exp = exlib_softeq_proc(xdrc_lp_exp_cb2,pcm32_lp_exp);
            pcm32_hp_exp = exlib_softeq_proc(xdrc_hp_exp_cb,pcm32);
            //pcm32_hp_exp = exlib_softeq_proc(xdrc_hp_exp_cb2,pcm32_hp_exp);

            //expand freq to get drc_gain
            drc_gain_lp = exlib_drc_process_gain(xdrc_drclp_cb,pcm32_lp_exp);
            drc_gain_hp = exlib_drc_process_gain(xdrc_drchp_cb,pcm32_hp_exp);

            //drc delay buf
            if (cfg_xdrc & XDRC_DELAYBUF_EN) {
                pcm32 = pcmdelay_mono_s32(xdrc_delay_cb,pcm32);
            }
            //全频pcm32 过 drc_gain
            pcm32_lp = exlib_drc_get_pcm32_s(pcm32,drc_gain_lp);
            pcm32_hp = exlib_drc_get_pcm32_s(pcm32,drc_gain_hp);    //pcm32

        #if 1   //4阶分频DRC
            //LP/HP filter
            pcm32_lp = exlib_softeq_proc(xdrc_lp_cb,pcm32_lp);
            pcm32_lp = exlib_softeq_proc(xdrc_lp_cb2,pcm32_lp);
            pcm32_hp = exlib_softeq_proc(xdrc_hp_cb,pcm32_hp);
            pcm32_hp = exlib_softeq_proc(xdrc_hp_cb2,pcm32_hp);
        #else  //2阶分频DRC
            //LP/HP filter
            pcm32_lp = exlib_softeq_proc(xdrc_lp_cb,pcm32_lp);
            pcm32_hp = exlib_softeq_proc(xdrc_hp_cb,pcm32_hp);
            pcm32_hp = 0 - pcm32_hp;        //hp反向一下
        #endif

    #if XDRC_DUMP_EN  && DUMP_LPHP_MIX
            p16_lphp[2*i] = pcm32_lp;       //导出LP(pcm32_lp)  //pcm32_hp_exp
            p16_lphp[2*i + 1] = pcm32_hp;   //导出HP
    #endif
            pcm32 = pcm32_lp + pcm32_hp;
        }

        //HP/LP MIX
        if (cfg_xdrc & XDRC_ALLDRC_EN) {
            ptr[i] =  exlib_drc_process_s16(xdrc_drcall_cb, pcm32); //总DRC
        } else {
            ptr[i] = exlib_s_clip16(pcm32);
        }
#if XDRC_DUMP_EN && DUMP_LPHP_MIX
        p16_mix[i] =  ptr[i];           //导出MIX
#endif

#if XDRC_DUMP_EN && DUMP_RES
        p16_res[2*i+1] = ptr[i];
#endif
    }

#if XDRC_DUMP_EN && DUMP_LPHP_MIX
    dump_put2ram(&xdrc_dump_buf[0][0],p16_lphp, samples*2*2, 0);      //samples*2 = 512  //lp_hp组合成双声道，导出成文件0
    dump_put2ram(&xdrc_dump_buf[0][14+512*2],p16_mix, 256*2, 1);      //mix后的数据导出成文件1(单声道)
    dump_dma_wait();
    memcpy(&xdrc_dump_buf[1][0],&xdrc_dump_buf[0][0],sizeof(xdrc_dump_buf)/2);
    dump_dma_kick(&xdrc_dump_buf[1][0],14+512*2 + 14+512 );  //0/1一起KICK导出
#endif

#if XDRC_DUMP_EN && DUMP_RES
    dump_put2ram(&xdrc_dump_buf[0][0],p16_res, samples*2*2, 0);      //samples*2 = 512  //res算法前和算法后的数据，导出成文件0
    dump_dma_wait();
    memcpy(&xdrc_dump_buf[1][0],&xdrc_dump_buf[0][0], 14+512*2 );
    dump_dma_kick(&xdrc_dump_buf[1][0],14+512*2);  //0/1一起KICK导出
#endif
}


