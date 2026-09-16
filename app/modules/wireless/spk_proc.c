#include "include.h"
#include "wireless.h"

#if WIRELESS_SPK_TX_EN


#define SPK_DEC_SAMPLE_NBYTE            2
#define SPK_DAC_SAMPLE_NBYTE            2

#define SPK_ENC_BUFFER_SIZE             WIRELESS_SPK_FRAME_SIZE
#define SPK_DEC_BUFFER_SIZE            (WIRELESS_SPK_FRAME_SIZE+1)

#define SPK_DEC_OBUF_SIZE              (WIRELESS_SPK_SAMPLES_SELECT*WIRELESS_SPK_CHANNEL_SELECT*SPK_DEC_SAMPLE_NBYTE)
#define SPK_DAC_OBUF_SIZE              (WIRELESS_SPK_SAMPLES_SELECT*WIRELESS_SPK_CHANNEL_SELECT*SPK_DAC_SAMPLE_NBYTE)
#define SPK_TMP_BUF_SIZ                (WIRELESS_SPK_SAMPLES_SELECT*WIRELESS_SPK_CHANNEL_SELECT*SPK_DAC_SAMPLE_NBYTE)

void bt_get_tick1_time(uint tick_cnt);
void bt_play_sync_tick1(bool from_dac, uint8_t interval);
void wireless_spk_kick(void);

typedef struct {
    u8 frame[SPK_ENC_BUFFER_SIZE];
    audio_callback_t callback;
} spk_enc_t;

typedef struct {
    u8 obuf[SPK_DAC_OBUF_SIZE];
    u8 decout[SPK_DEC_OBUF_SIZE];
    u8 frame[SPK_DEC_BUFFER_SIZE];
    u8 tmp_buf[SPK_TMP_BUF_SIZ];
    u8 fifo_sta;
    u8 last_bfi;
    audio_callback_t callback;

#if WIRELESS_SPK_DUMP_PER_BER
    struct per_info_tag per_info;
#endif
} spk_dec_t;

spk_enc_t spk_enc;
spk_dec_t spk_dec;


//------------------------------------------------------------------------------------------
#if !WIRELESS_MIC_TX_EN
//获取DAC播放偏差
AT(.com_text.spk_dec)
static void spk_dec_dac_get_sync(uint tick_cnt)
{
#if DEVICE_DAC_OUTPUT_EN
    bt_get_dac_fifocnt(tick_cnt);
#endif
}

//DAC播放偏差处理
AT(.com_text.spk_dec)
static void spk_dec_dac_sync_proc(void)
{
#if DEVICE_DAC_OUTPUT_EN
    if(spk_dec.fifo_sta == 0) {
        dac_put_zero(10);
        spk_dec.fifo_sta++;
        return;
    } else {
        bt_play_sync_dac_fifocnt(8, WIRELESS_MIC_TX_INTERVAL/WIRELESS_SPK_COMB_NB*2);
    }
#endif
}

#else // !WIRELESS_MIC_TX_EN
    //打开WIRELESS_MIC_TX_EN时，在mic_proc中已经做了同步处理，spk的调速定义为空
    #define spk_dec_dac_get_sync(n)
    #define spk_dec_dac_sync_proc()
#endif // !WIRELESS_MIC_TX_EN

#if ADAPTER_AUX_TX_EN
    #define spk_enc_adc_get_sync(n)         bt_get_tick1_time(n)                                        //获取ADC采集偏差
    #define spk_enc_adc_sync_proc()         bt_play_sync_tick1(false, WIRELESS_MIC_TX_INTERVAL/WIRELESS_SPK_COMB_NB*2)    //ADC采集偏差处理
#else
    #define spk_enc_adc_get_sync(n)
    #define spk_enc_adc_sync_proc()
#endif


//------------------------------------------------------------------------------------------
AT(.com_text.spk_dec)
void spk_dec_dac_dma_kick(uint tick_cnt)
{
    spk_dec_dac_get_sync(tick_cnt);

    u8 *obuf = spk_dec.obuf;

    //输出到下一级
    if (spk_dec.callback) {
        spk_dec.callback(obuf, WIRELESS_SPK_SAMPLES_SELECT, WIRELESS_SPK_CHANNEL_SELECT, 0);
    }

//    spi_dma_tx(obuf, WIRELESS_SPK_SAMPLES_SELECT*2*2);
}

AT(.com_text.spk_dec)
void spk_dec_kick_cb(u8 idx)
{
    decoder_prio_trans_audio_input(NULL, WIRELESS_SPK_FRAME_SIZE, 1, &idx);
}

AT(.com_text.spk_dec)
void spk_dec_buf_clr(u8 idx)
{
    memset(spk_dec.decout, 0, SPK_DEC_OBUF_SIZE);
}

//format pcm / upsample / mix
AT(.com_text.spk_dec)
static void spk_dec_pcm_out(u8 idx, u8 *obuf)
{
    memcpy(obuf, spk_dec.decout, SPK_DAC_OBUF_SIZE);
}

AT(.com_text.spk_dec)
static void spk_dec_frame_do(u8 idx, u8 *decout)
{
    u8 *frame = spk_dec.frame;
    s16 *tmp_buf = (s16 *)spk_dec.tmp_buf;
    s16 *l_ch = (s16 *)spk_dec.tmp_buf;
    s16 *r_ch = (s16 *)spk_dec.tmp_buf + WIRELESS_SPK_SAMPLES_SELECT;
    s16 *pcm = (s16 *)decout;
    bool bfi = true;
    uint samples = WIRELESS_SPK_SAMPLES_SELECT;

    //只解一次
    bfi = wireless_a2d_get_rx_frame(frame, WIRELESS_SPK_FRAME_SIZE);

    if(!bfi) {
#if (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3S)
        lc3s_stereo_dec(frame, (s16 *)tmp_buf, samples);
#endif
    }

    plc_soft_process(tmp_buf, samples, spk_dec.last_bfi||bfi, 0);
    plc_soft_process(tmp_buf + samples, samples, spk_dec.last_bfi||bfi, 1);

    for(uint i=0; i<samples; i++) {
        pcm[2*i]     = r_ch[i];
        pcm[2*i + 1] = l_ch[i];
    }

    spk_dec.last_bfi = bfi;
}

AT(.com_text.spk_dec)
void spk_dec_audio_input(u8 *ptr, u32 len, int ch_mode, void *params)
{
    u8 idx = 0;

    u8 *obuf = spk_dec.obuf;

    if(wireless_cb.alg_en) {
        spk_dec_frame_do(idx, spk_dec.decout);
        spk_dec_pcm_out(idx, obuf);
    } else {
        memset(obuf, 0x00, SPK_DAC_OBUF_SIZE);
    }

    //DAC调速，避免长时间后播放速度和发射端不匹配
    spk_dec_dac_sync_proc();

    //输出到下一级，放到spk_dec_dac_dma_kick处理了，这里不需要调用
//    if (spk_dec.callback) {
//        spk_dec.callback(obuf, WIRELESS_SPK_SAMPLES_SELECT, WIRELESS_SPK_CHANNEL_SELECT, 0);
//    }
}

AT(.text.spk_dec)
void spk_dec_audio_output_callback_set(audio_callback_t callback)
{
    spk_dec.callback = callback;
}

AT(.text.spk_dec)
void spk_dec_init(u8 sample_rate, u16 samples, u8 channel)
{
    spk_dec.callback = NULL;
}

//------------------------------------------------------------------------------------------
#if ADAPTER_AUX_TX_EN
AT(.com_text.mic_enc)
void spk_enc_adc_dma_kick(uint tick_cnt)
{
    spk_enc_adc_get_sync(tick_cnt);
    wireless_spk_kick();
}
#endif

AT(.com_text.spk_enc)
void spk_enc_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    if(wireless_cb.alg_en) {
#if ADAPTER_AUDIO_FADE_IN_EN
        soft_gain_proc((mic_pcm_t *)ptr, WIRELESS_SPK_SAMPLES_SELECT*WIRELESS_SPK_CHANNEL_SELECT);
#endif
        lc3s_stereo_enc((s16 *)ptr, spk_enc.frame, WIRELESS_SPK_SAMPLES_SELECT);
        wireless_a2d_put_tx_frame(0, spk_enc.frame, WIRELESS_SPK_FRAME_SIZE);
    }

    spk_enc_adc_sync_proc();

//    if (spk_enc.callback) {
//        spk_enc.callback(ptr, samples, ch_mode, NULL);
//    }
}

AT(.text.spk_enc)
void spk_enc_audio_output_callback_set(audio_callback_t callback)
{
    spk_enc.callback = callback;
}

AT(.text.spk_enc)
void spk_enc_init(u8 sample_rate, u16 samples, u8 channel)
{
    spk_enc.callback = NULL;
}

#if ADAPTER_USB_SPK_TX_EN
AT(.com_text.spk_enc)
void spk_enc_kick_cb(u8 idx)
{
    src_buf_kick(WIRELESS_SPK_SAMPLES_SELECT, WIRELESS_SPK_CHANNEL_SELECT, spk_enc_audio_input);
}
#endif

#if WIRELESS_SPK_DUMP_PER_BER
void wireless_spk_per_dump(void)
{
    static u8 delay_1s = 0;
    if(++delay_1s >= 3) {
        wireless_per_dump(&spk_dec.per_info, WIRELESS_SPK_FRAME_SIZE);
        delay_1s = 0;
    }
}
#endif

#endif
