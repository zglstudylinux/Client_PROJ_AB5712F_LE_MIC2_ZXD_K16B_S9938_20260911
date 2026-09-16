#include "include.h"

#if I2S_AUDIO_IN_EN

#define I2S_ADJ_EN               1
#define I2S_ADJ_INFO_EN          0
#define I2S_RX_CHANNEL_SEL       1      //1:双声道转单声道, 2:保留双声道

static i2s_in_ctl_t i2s_in_ctl;
static i2s_audio_buf_t i2s_audio_buf AT(.i2s_buf);
static u8 i2s_adj_buf[135*4] AT(.i2s_buf);

#if I2S_ADJ_INFO_EN
AT(.com_text.i2s_info)
static const char src1_info[] = "@%d,%d\n";
#endif

//I2S接收中断回调函数,缓存和处理接收数据
//流程:i2s_rx_process ----> src1_audio_adj_process ---> src1_audio_adj_output ---> cbuf

AT(.com_text.i2s_audio_in)
void i2s_rx_process(void *buf, u32 samples, u8 ch_mode, bool i2s_32bit)
{
    s32 *ptr32 = (s32*)buf;
    s16 *ptr16 = (s16*)buf;

    if(i2s_32bit) {                                      //I2S_32BIT
        for (int i = 0; i < samples; i++) {              //32BIT ->16bit
           ptr16[2*i] =  (s16)(ptr32[2*i] >> 16);
           ptr16[2*i+1] =  (s16)(ptr32[2*i+1] >> 16);
        }
    }

#if I2S_RX_CHANNEL_SEL == 1                             //Stereo --> mono
    ch_mode = 1;
    for(int i = 0; i < samples; i++) {
        ptr16[i] = ptr16[2*i];
    }
#endif

    i2s_in_ctl.samples     = samples;
    i2s_in_ctl.ch_mode     = ch_mode;

    //I2S起中断后, 进入低优先级线程调速输入
#if I2S_ADJ_EN
    memcpy(i2s_adj_buf, ptr16, samples*ch_mode*2);
    src_adj_proc_kick_start();
#else
    cbuf_input_audio((u8 *)buf, i2s_in_ctl.samples*i2s_in_ctl.ch_mode, &(i2s_audio_buf.in_cbuf));
#endif
}

#if I2S_ADJ_EN
int i2s_src1_speed_tbl[][2] = {
//  samples,    phase
    {0,          -1},
    {120*1,      -1},
    {120*2,      0},
    {120*3,      1},
    {0xffff,     3},
};

AT(.com_text.i2s_audio_in.adj)
void src1_audio_adj_process(void)
{
    src1_audio_input((u8 *)i2s_adj_buf, i2s_in_ctl.samples, i2s_in_ctl.ch_mode, NULL);
}

AT(.com_text.i2s_audio_in.adj)
void src1_phase_adj_process(void)
{
    s8 new_speed = 0;
    cbuf_cfg_t *cbuf = &(i2s_audio_buf.in_cbuf);

    uint total_samples = cbuf_total_samples_get(cbuf);

    for(uint i=0; i<sizeof(i2s_src1_speed_tbl)/(2*sizeof(int)); i++) {
        if(total_samples <=  i2s_src1_speed_tbl[i][0]) {
            new_speed = i2s_src1_speed_tbl[i][1];
            break;
        }
    }

    if(new_speed != i2s_in_ctl.speed) {
        i2s_in_ctl.speed = new_speed;
        src1_adjust_speed(new_speed);
    }

#if I2S_ADJ_INFO_EN
    static u32 ticks = 0;
    if (tick_check_expire(ticks,1000)) {
        my_printf(src1_info, new_speed, total_samples);
        ticks = tick_get();
    }
#endif
}

AT(.com_text.i2s_audio_in.adj)
void src1_audio_adj_output(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    if(wireless_cb.connected_sta) {
        cbuf_input_audio((u8 *)ptr, samples*ch_mode, &(i2s_audio_buf.in_cbuf));
        src1_phase_adj_process();
    }
}
#else
void src1_audio_adj_process(void)
{

}
#endif

//---------------------------------------------------------------------
AT(.com_text.i2s_audio_in)
void i2s_audio_in_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    cbuf_cfg_t *cbuf = &(i2s_audio_buf.in_cbuf);
    u8 *out_buf = (u8 *)&(i2s_audio_buf.outcache);
    u16 buf_len = cbuf_total_samples_get(cbuf);
    u16 frame_size = samples;


#if (I2S_RX_CHANNEL_SEL == 1)
    if(ch_mode > 1) {
        printf("I2S_RX_CHANNEL_SEL err!!\n");
    }
#elif (I2S_RX_CHANNEL_SEL == 2)
    frame_size = frame_size*2;
#endif

    if(buf_len >= frame_size) {
        cbuf_output_audio(out_buf, frame_size, &(i2s_audio_buf.in_cbuf));
    } else {
        memset(out_buf, 0, frame_size*2);
    }

    if(i2s_in_ctl.callback) {
        i2s_in_ctl.callback(out_buf, samples, ch_mode, params);
    }
}

AT(.com_text.i2s_audio_in)
void i2s_audio_input_callback_set(audio_callback_t callback)
{
    i2s_in_ctl.callback = callback;
}

AT(.text.i2s_audio_in)
void i2s_audio_in_init(u8 sample_rate, u16 samples, u8 channel)
{
    i2s_cfg_t *p_cfg = &i2s_in_ctl.i2s_cfg;
    memset(&i2s_in_ctl, 0, sizeof(i2s_in_ctl));
    cbuf_init(&(i2s_audio_buf.in_cbuf), (u8 *)&(i2s_audio_buf.incache), I2S_INCACHE_SIZE);   //从机模式，用缓存池处理

    p_cfg->mode        = I2S_SLAVE_DMARX;
    p_cfg->iomap       = I2S_MAPPING_SEL;
    p_cfg->bit_mode    = I2S_BIT_MODE;
    p_cfg->data_mode   = I2S_DATA_MODE;
    p_cfg->mclk_sel    = I2S_MCLK_SEL;
    p_cfg->mclk_out_en = I2S_MCLK_EN;
    if (I2S_DMA_EN) {
        p_cfg->dma_cfg.samples = I2S_DMA_SAMPLES;
        p_cfg->dma_cfg.isr_rx_callback = i2s_rx_process;
        p_cfg->dma_cfg.isr_tx_callback = NULL;
    }
    i2s_init(p_cfg);                       //可以改到callback set之后再初始化
    i2s_dma_start();


#if I2S_ADJ_EN
    src1_init(sample_rate, samples, channel);                     //硬件src1调速初始化
    src1_audio_output_callback_set(src1_audio_adj_output);

    if(channel == 2) {
        for(u8 i=0; i< (sizeof(i2s_src1_speed_tbl)/(2*sizeof(int)) - 1); i++) {
            i2s_src1_speed_tbl[i][0] = i2s_src1_speed_tbl[i][0]*2;
        }
    }
#endif
}
#else
AT(.text.i2s_audio_in.adj)
void src1_audio_adj_process(void)
{

}
#endif // I2S_AUDIO_IN_EN
