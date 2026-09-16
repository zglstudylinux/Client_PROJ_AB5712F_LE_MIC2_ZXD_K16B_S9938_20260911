#include "include.h"
#include "i2s_audio.h"


#if I2S_AUDIO_OUT_EN
typedef struct {
    audio_callback_t callback;
    i2s_cfg_t i2s_cfg;
} i2s_out_ctl_t;

static i2s_out_ctl_t i2s_out_ctl;

static i2s_audio_buf_t i2s_audio_buf  AT(.buf.i2s.dma);
#if (I2S_SAMPLES_OUT_SET == I2S_16000)
u8 out_buf_16k[80] AT(.buf.i2s.dma);
#endif
static u8 global_ch_mode;

AT(.com_text.i2s_mic_in_and_out)
void i2s_tx_process(void *buf, u32 samples, u8 ch_mode, bool iis_32bit)
{
    ch_mode = global_ch_mode;                   //获取无线麦音频数据声道数
    uint frame_size = samples * ch_mode;

    if(cbuf_total_samples_get(&(i2s_audio_buf.in_cbuf)) >= frame_size) {
        cbuf_output_audio((u8 *)&(i2s_audio_buf.outcache), frame_size, &(i2s_audio_buf.in_cbuf));
    } else {
        memset((u8 *)&(i2s_audio_buf.outcache), 0, samples*2);
    }
    s16 *pcm = (s16 *)&(i2s_audio_buf.outcache);

    if (iis_32bit && (ch_mode == 1)) {
        s32 *dma_buf = (s32 *)buf;
        for (int i = 0; i< samples; i++) {                         //16->32位扩展
            dma_buf[2*i] = (s32)(pcm[i] << 16);
            dma_buf[2*i+1] = (s32)(pcm[i] << 16);
        }
    } else if (iis_32bit && (ch_mode == 2)) {
        s32 *dma_buf = (s32 *)buf;
        for (int i = 0; i< samples; i++) {
            dma_buf[2*i] = (s32)(pcm[2*i] << 16);                    //16->32位扩展
            dma_buf[2*i+1] = (s32)(pcm[2*i+1] << 16);
        }
    } else if (!iis_32bit && (ch_mode == 1)) {
        s16 *dma_buf = (s16 *)buf;
        for (int i = 0; i < samples; i++) {
            dma_buf[2*i] = (s16)(pcm[i]);
            dma_buf[2*i+1] = (s16)(pcm[i]);
        }
    } else if (!iis_32bit && (ch_mode == 2)) {
        s16 *dma_buf = (s16 *)buf;
        for(int i = 0;i < samples; i++) {
            dma_buf[2*i] = (s16)(pcm[2*i]);
            dma_buf[2*i+1] = (s16)(pcm[2*i+1]);
        }
    }
}

AT(.com_text.i2s_audio_out)
void i2s_audio_out_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    uint frame_size = samples * 2;
    global_ch_mode = ch_mode;

#if (I2S_SAMPLES_OUT_SET == I2S_48000)
    cbuf_input_audio((u8 *)ptr, samples*ch_mode, &(i2s_audio_buf.in_cbuf));
#elif (I2S_SAMPLES_OUT_SET == I2S_16000)
    short *in_pcm = (short *)ptr;
	short *out_pcm = (short *)out_buf_16k;

    uint out_cnt = src_frame_resample(0, in_pcm, out_pcm, samples);
    cbuf_input_audio(out_buf_16k, out_cnt * ch_mode, &(i2s_audio_buf.in_cbuf));
#endif
    if(i2s_out_ctl.callback != NULL) {
        i2s_out_ctl.callback(ptr, samples, ch_mode, params);
    }
}

AT(.com_text.i2s_audio_out)
void i2s_audio_output_callback_set(audio_callback_t callback)
{
    i2s_out_ctl.callback = callback;
}

AT(.text.i2s_audio_out)
void i2s_audio_out_init(u8 sample_rate, u16 samples, u8 channel)
{
    i2s_cfg_t *p_cfg = &i2s_out_ctl.i2s_cfg;
    memset(&i2s_out_ctl, 0, sizeof(i2s_out_ctl));


#if (I2S_MODE_SEL == I2S_MASTER)
    p_cfg->mode        = I2S_MASTER_DMATX;
#else
    p_cfg->mode        = I2S_SLAVE_DMATX;
#endif
    p_cfg->iomap       = I2S_MAPPING_SEL;
    p_cfg->bit_mode    = I2S_BIT_MODE;
    p_cfg->data_mode   = I2S_DATA_MODE;
    p_cfg->mclk_sel    = I2S_MCLK_SEL;
    p_cfg->mclk_out_en = I2S_MCLK_EN;
    if (I2S_DMA_EN) {
#if (I2S_SAMPLES_OUT_SET == I2S_16000)
        soft_src_alg_init(0, 48000,16000);
#endif
        cbuf_init(&(i2s_audio_buf.in_cbuf), (u8 *)&(i2s_audio_buf.incache), I2S_INCACHE_SIZE);   //从机模式，用缓存池处理
        p_cfg->dma_cfg.samples = I2S_DMA_SAMPLES;
        p_cfg->dma_cfg.isr_tx_callback = i2s_tx_process;
        p_cfg->dma_cfg.isr_rx_callback = NULL;
    }
    i2s_init(p_cfg);            //I2S初始化
    i2s_dma_start();
}
#endif
