#include "include.h"

#if ADAPTER_I2S_IN_OUT_EN

#define I2S0_RX_CHANNEL           2         //1:双声道转单声道,  2:保留双声道
#define I2S_CACHE_SIZE            1024
#define MIC_CACHE_SIZE            480

typedef struct {
    audio_callback_t callback;
    i2s_cfg_t i2s_cfg;
} i2s_out_ctl_t;


static uint8_t i2s_rx_cache[I2S_CACHE_SIZE] AT(.buf.i2s);
static uint8_t i2s_tx_cache[MIC_CACHE_SIZE] AT(.buf.i2s);
#if ADAPTER_HARDWARE_SRC1_EN
//SRC只能访问0x40000以下的RAM地址, 数组缓存区直接放bss段
static uint8_t mic_frame[MIC_CACHE_SIZE];
#else
static uint8_t mic_frame[MIC_CACHE_SIZE] AT(.buf.i2s);
#endif

static cbuf_cfg_t rx_cbuf;
static i2s_cfg_t i2s_cfg;
static i2s_out_ctl_t i2s_audio_in_out_cfg;
static uint samples_cnt = 0;

//---------------------------------------------------------------------------------
//I2S AUDIO RX ISR
AT(.com_text.i2s_audio_in)
static void i2s_rx_process(void *buf, u32 samples, u8 ch_mode, bool i2s_32bit)
{
    s32 *ptr32 = (s32 *)buf;
    s16 *ptr16 = (s16 *)buf;
    if(i2s_32bit) {
#if I2S0_RX_CHANNEL == 1
        ch_mode = 1;
        for(uint i = 0; i < samples; i++) {
            ptr16[i] = (s16)(ptr32[2*i]>>16);
        }
#else
        for(uint i = 0; i < samples*ch_mode; i++) {
            ptr16[i] = (s16)(ptr32[i]>>16);
        }
#endif
        cbuf_input_audio((u8 *)buf, samples*ch_mode, &rx_cbuf);
    } else {
#if I2S0_RX_CHANNEL == 1
        ch_mode = 1;
        for(uint i = 0; i < samples; i++) {
            ptr16[i] = ptr16[2*i];
        }
#endif
        cbuf_input_audio((u8 *)buf, samples*ch_mode, &rx_cbuf);
    }
}

//---------------------------------------------------------------------------------
//I2S AUDIO TX ISR
AT(.com_text.i2s_mic_out)
void i2s_tx_process(void *buf, u32 samples, u8 ch_mode, bool iis_32bit)
{
//    GPIOASET = BIT(7);
//    asm("nop");asm("nop");
//    GPIOACLR = BIT(7);
}

//---------------------------------------------------------------------------------
AT(.com_text.i2s_audio_in)
static void i2s_audio_in_sample_get(u8 *ptr, uint samples, u8 ch_mode)
{
    if(ptr != NULL && samples > 0) {
        u32 total_size = cbuf_total_samples_get(&rx_cbuf);
        u32 frame_size = samples*ch_mode;
        if(total_size >= frame_size) {
            cbuf_output_audio(ptr, frame_size, &rx_cbuf);
        } else {
            memset(ptr, 0, frame_size*2);
        }
    }
}

#if I2S_BIT_MODE == I2S_32BIT
AT(.com_text.i2s_mic_out)
void i2s_fram_16_to_32(u8 *obuf, u8 *ibuf, u32 samples, int ch_mode)
{
    s32 *ptr = (s32 *)obuf;
    s16 *ptr16 = (s16 *)ibuf;
    if(ch_mode == 1) {                                      //TX Mono
        for (int i = 0; i< samples; i++) {                  //16->32位扩展
            ptr[2*i] = (s32)(ptr16[i] << 16);
            ptr[2*i+1] = (s32)(ptr16[i] << 16);
        }
    } else {                                                //TX Stero
        for (int i = 0; i< samples; i++) {
            ptr[2*i] = (s32)(ptr16[2*i] << 16);             //16->32位扩展
            ptr[2*i+1] = (s32)(ptr16[2*i+1] << 16);
        }
    }
}
#endif

AT(.com_text.i2s_mic_out)WEAK
void i2s_audio_in_out_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    //一拖二时,MIC数据是分片解码下来的,这里要存够完整的一帧再放到I2S_DMA_BUF里面去传输
    memcpy(i2s_tx_cache + samples_cnt*2, ptr, samples*2);
    samples_cnt += samples;
    if(samples_cnt >= WIRELESS_MIC_SAMPLES_SELECT) {
        samples_cnt = 0;
        #if I2S_BIT_MODE == I2S_32BIT
        u8 *i2s_dma_buf = (u8 *)i2s_out_get_obuf(0);
        i2s_fram_16_to_32(i2s_dma_buf, i2s_tx_cache, WIRELESS_MIC_SAMPLES_SELECT, ch_mode);
        #else
        s16 *i2s_dma_buf = (s16 *)i2s_out_get_obuf(0);
        s16 *mic_pcm = (s16 *)i2s_tx_cache;
        for(uint i = 0; i < WIRELESS_MIC_SAMPLES_SELECT; i++) {
            i2s_dma_buf[2*i] = mic_pcm[i];
            i2s_dma_buf[2*i + 1] = mic_pcm[i];
        }
        #endif
    }

    //将I2S RX AUDIO 推到下一级
    ch_mode = I2S0_RX_CHANNEL;
    ptr = mic_frame;
    i2s_audio_in_sample_get(ptr, samples, ch_mode);

    if(i2s_audio_in_out_cfg.callback){
        i2s_audio_in_out_cfg.callback((u8 *)ptr, samples, ch_mode, NULL);
    }
}

AT(.text.i2s_mic_out)WEAK
void i2s_audio_in_out_output_callback_set(audio_callback_t callback)
{
    i2s_audio_in_out_cfg.callback = callback;
}

AT(.text.i2s_mic_out)WEAK
void i2s_audio_in_out_init(u8 sample_rate, u16 samples, u8 channel)
{

    memset(&i2s_audio_in_out_cfg, 0, sizeof(i2s_audio_in_out_cfg));
    memset(&i2s_cfg, 0x00, sizeof(i2s_cfg));

    memset(i2s_rx_cache, 0x00, sizeof(i2s_rx_cache));
    cbuf_init(&rx_cbuf, (u8 *)(i2s_rx_cache), I2S_CACHE_SIZE);

    i2s_cfg.mode        = I2S_MASTER_DMATX_DMARX;
    i2s_cfg.iomap       = I2S_MAPPING_SEL;
    i2s_cfg.bit_mode    = I2S_BIT_MODE;
    i2s_cfg.data_mode   = I2S_DATA_MODE;
    i2s_cfg.mclk_sel    = I2S_MCLK_SEL;
    i2s_cfg.mclk_out_en = I2S_MCLK_EN;
    if (I2S_DMA_EN) {
        i2s_cfg.dma_cfg.samples = I2S_DMA_SAMPLES;
        i2s_cfg.dma_cfg.isr_rx_callback = i2s_rx_process;
        i2s_cfg.dma_cfg.isr_tx_callback = i2s_tx_process;
    }
    i2s_init(&i2s_cfg);

    //先不要启动I2S,等准备发送时再启动
    //i2s_dma_start();
}

AT(.text.i2s_mic_out)WEAK
void i2s_audio_in_out_exit(void)
{
    printf("i2s_audio_in_out_exit\n");
    i2s_exit();
    samples_cnt = 0;
}
#endif
