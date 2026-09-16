#include "include.h"
#include "wireless.h"

#if WIRELESS_MIC_TX_EN


#define MIC_ENC_BUFFER_SIZE         WIRELESS_MIC_FRAME_SIZE
#define MIC_DEC_BUFFER_SIZE         WIRELESS_MIC_FRAME_SIZE
#if DEVICE_STEREO_EN || WIRELESS_MIC_STEREO
#define MIC_DEC_OBUF_SIZE           (WIRELESS_MIC_SAMPLES_SELECT*WIRELESS_MIC_CHANNEL_SELECT*2*2)
#define MIC_TMP_BUF_SIZ             (WIRELESS_MIC_SAMPLES_SELECT*WIRELESS_MIC_CHANNEL_SELECT*2*2)
#else
#define MIC_DEC_OBUF_SIZE           (WIRELESS_MIC_SAMPLES_SELECT*WIRELESS_MIC_CHANNEL_SELECT*2)
#endif
void dac1_get_fifocnt(u32 tick_cnt);
void dac1_play_sync_fifocnt(u16 high_thr, u16 low_thr);
void wireless_mic_kick(void);
void bt_get_tick1_time(uint tick_cnt);
void bt_play_sync_tick1(bool from_dac, uint8_t interval);
void wireless_spk_kick(void);

typedef struct {
//    audio_callback_t callback;
#if DEVICE_STEREO_EN || WIRELESS_MIC_STEREO
    u8 frame[MIC_ENC_BUFFER_SIZE*2];
#else
    u8 frame[MIC_ENC_BUFFER_SIZE];
#endif
} mic_enc_t;

typedef struct {
    audio_callback_t callback;
    u8 obuf[MIC_DEC_OBUF_SIZE];
    struct {
        u8 buf[MIC_DEC_OBUF_SIZE];
        u16 buf_cnt;
    } pcm[WIRELESS_CON_LINK_NB];

    u8 frame[MIC_DEC_BUFFER_SIZE];
    u8 last_bfi[WIRELESS_CON_LINK_NB];
    u8 frag_samples[2];
#if DEVICE_STEREO_EN || WIRELESS_MIC_STEREO
    u8 decout[MIC_DEC_OBUF_SIZE];
    u8 tmp_buf[MIC_TMP_BUF_SIZ];
#endif
    bool frag0_done_flag;
    u8 sync_idx;
    u8 fifo_sta;
} mic_dec_t;


static mic_enc_t mic_enc AT(.btmem.wireless.dec);
static mic_dec_t mic_dec AT(.btmem.wireless.dec);


//------------------------------------------------------------------------------------------
#define mic_enc_adc_get_sync(n)         bt_get_tick1_time(n)                                        //获取ADC采集偏差
#define mic_enc_adc_sync_proc()         bt_play_sync_tick1(false, WIRELESS_MIC_TX_INTERVAL/WIRELESS_MIC_COMB_NB*2)       //ADC采集偏差处理

//获取DAC播放偏差
AT(.com_text.mic_dec)
static void mic_dec_dac_get_sync(uint tick_cnt)
{
#if ADAPTER_DAC_OUTPUT_EN
    dac1_get_fifocnt(tick_cnt);
#endif
}

//DAC播放偏差处理
AT(.com_text.mic_dec)
static void mic_dec_dac_sync_proc(void)
{
#if ADAPTER_DAC_OUTPUT_EN
    if(mic_dec.fifo_sta == 0) {
        mic_dec.fifo_sta++;
        return;
    } else {
        uint high_thr, low_thr;
        uint samples = cfg_wireless_d2a_dec_us*48/1000;

        high_thr = samples + 12;
        low_thr = samples + 6;

        mic_dec.fifo_sta = 0;
        dac1_play_sync_fifocnt(high_thr, low_thr);
    }
#endif
}

////------------------------------------------------------------------------------------------

AT(.com_text.mic_enc)
void mic_enc_adc_dma_kick(uint tick_cnt)
{
    mic_enc_adc_get_sync(tick_cnt);

    if (wireless_cb.alg_en) {
#if DEVICE_STEREO_EN || WIRELESS_MIC_STEREO
        wireless_spk_kick();
#else
        wireless_mic_kick();
#endif
    }
}
AT(.text.mic_mute)
void mic_enc_mute_set(u8 mute_flag)
{
    sys_cb.mute_flag = mute_flag;
}

AT(.com_text.mic_enc)
void mic_enc_audio_input(u8 *pcm, u32 samples, int ch_mode, void *params)
{
//    GPIOASET = BIT(8);

    samples = WIRELESS_MIC_SAMPLES_SELECT;

//        //发射1k正弦波，测试用
//        void test_data_get_pcm(void *pcm, uint samples);
//        test_data_get_pcm(pcm, samples);

    if(sys_cb.mute_flag) {
        memset(pcm, 0x00, samples*2);
    }



#if (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3S)
#if DEVICE_STEREO_EN || WIRELESS_MIC_STEREO
    lc3s_stereo_enc((s16 *)pcm, mic_enc.frame, samples);
#else
    lc3s_enc((s16 *)pcm, mic_enc.frame, samples);
#endif
#elif (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3B)
    lc3b_enc((s16 *)pcm, mic_enc.frame, samples);
#endif

    wireless_d2a_put_tx_frame(mic_enc.frame, WIRELESS_MIC_FRAME_SIZE);

//    GPIOACLR = BIT(8);

    mic_enc_adc_sync_proc();

//    //输出到下一级
//    if (mic_enc.callback) {
//        mic_enc.callback(ptr, WIRELESS_MIC_SAMPLES_SELECT, ch_mode, params);
//    }
}

AT(.text.mic_enc)
void mic_enc_audio_output_callback_set(audio_callback_t callback)
{
//    mic_enc.callback = callback;
}

AT(.text.mic_enc)
void mic_enc_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&mic_enc, 0x00, sizeof(mic_enc));

//    mic_enc.callback = NULL;
}

void mic_enc_reset(void)
{
}

//------------------------------------------------------------------------------------------
AT(.com_text.mic_enc)
void mic_dec_dac_fifo_get(u8 idx)
{
    mic_dec.sync_idx = idx;
    mic_dec_dac_get_sync(0);
}

AT(.com_text.mic_enc)
void mic_dec_kick_cb(u8 idx)
{
    decoder_prio_trans_audio_input(NULL, WIRELESS_MIC_FRAME_SIZE, 1, &idx);

#if ADAPTER_I2S_IN_OUT_EN
    if (wireless_cb.alg_en) {
        i2s_dma_start();
    }
#endif
}

#if WIRELESS_CON_COMB_BUF_EN
void mic_dec_pcm_out(u8 idx)
{
    s16 *obuf = (s16 *)mic_dec.obuf;
    u8 samples = WIRELESS_MIC_SAMPLES_SELECT/2;

    mic_dec.pcm[idx].buf_cnt = 0;                   //复位pcm[idx].buf_cnt，确保先copy前半段pcm[idx].buf

    s16 *pcm0 = (s16 *)(mic_dec.pcm[0].buf + mic_dec.pcm[0].buf_cnt);
    s16 *pcm1 = (s16 *)(mic_dec.pcm[1].buf + mic_dec.pcm[1].buf_cnt);

#if ADAPTER_MIX_DRC_EN
    mix_drc_audio_input(pcm0, pcm1, obuf, samples);
#else
    for(uint i=0; i<samples; i++) {
        s32 tmp = pcm0[i] + pcm1[i];
        if(tmp > 32767) {
            tmp = 32767;
        } else if(tmp < -32768) {
            tmp = -32768;
        }
        obuf[i] = tmp;
    }
#endif

    mic_dec.pcm[0].buf_cnt += MIC_DEC_OBUF_SIZE/2;
    if(mic_dec.pcm[0].buf_cnt >= MIC_DEC_OBUF_SIZE) {
        mic_dec.pcm[0].buf_cnt = 0;
    }
    mic_dec.pcm[1].buf_cnt += MIC_DEC_OBUF_SIZE/2;
    if(mic_dec.pcm[1].buf_cnt >= MIC_DEC_OBUF_SIZE) {
        mic_dec.pcm[1].buf_cnt = 0;
    }

    //输出到下一级
    if (mic_dec.callback) {
        mic_dec.callback((void *)obuf, samples, 1, NULL);
    }
}
#else // WIRELESS_CON_COMB_BUF_EN

#if (WIRELESS_CON_LINK_NB > 1)
AT(.com_text.mic_dec)
static void mic_dec_pcm_out_samples(s16 *pcm0, s16 *pcm1, uint8_t samples, uint8_t adj_dac_flag)
{
    s16 *obuf = (s16 *)mic_dec.obuf;

#if ADAPTER_MIX_DRC_EN
    mix_drc_audio_input(pcm0, pcm1, obuf, samples);
#else
    for(uint i=0; i<samples; i++) {
        s32 tmp = pcm0[i] + pcm1[i];
        if(tmp > 32767) {
            tmp = 32767;
        } else if(tmp < -32768) {
            tmp = -32768;
        }
        obuf[i] = tmp;
    }
#endif

    //输出到下一级
    if (mic_dec.callback) {
        mic_dec.callback((void *)obuf, samples, 1, NULL);
    }
}

AT(.com_text.mic_dec)
static void mic_dec_pcm_out_frag0(u8 dec_flag)
{
    uint8_t frag0_samples = mic_dec.frag_samples[0];

    s16 *pcm0 = (s16 *)(&mic_dec.pcm[0].buf[0]);
    s16 *pcm1 = (s16 *)(&mic_dec.pcm[1].buf[0]) + (WIRELESS_MIC_SAMPLES_SELECT-frag0_samples);

    mic_dec_pcm_out_samples(pcm0, pcm1, frag0_samples, dec_flag);
}

//format pcm / upsample / mix
AT(.com_text.mic_dec)
static void mic_dec_pcm_out_frag1(u8 dec_flag)
{
    uint8_t frag0_samples = mic_dec.frag_samples[0];
    uint8_t frag1_samples = mic_dec.frag_samples[1];

    s16 *pcm0 = (s16 *)(&mic_dec.pcm[0].buf[0]) + frag0_samples;
    s16 *pcm1 = (s16 *)(&mic_dec.pcm[1].buf[0]);

    mic_dec_pcm_out_samples(pcm0, pcm1, frag1_samples, dec_flag);
}
#endif

AT(.com_text.mic_dec)
static void mic_dec_pcm_out(u8 idx)
{
#if (WIRELESS_CON_LINK_NB == 1)
    //输出到下一级
    if (mic_dec.callback) {
        mic_dec.callback((void *)(&mic_dec.pcm[0].buf[0]), WIRELESS_MIC_SAMPLES_SELECT, 1, NULL);
    }
#else
    u8 con_status;
    u8 dec_flag = 0;

    if(idx == 0) {
        if (mic_dec.frag0_done_flag == 0) {
            //推通道1前半帧+通道2后半帧
            mic_dec_pcm_out_frag0(dec_flag);
            mic_dec.frag0_done_flag = 1;
        }

        //若通道2未连接，把frag1也转换了
        con_status = ble_con_get_status();
        if((con_status & ~BIT(idx)) == 0) {
            mic_dec_pcm_out_frag1(0);
            mic_dec.frag0_done_flag = 0;
        }
    } else {
        //通道2解码完，再转换frag1，输出到OBUF后半段
        if(mic_dec.frag0_done_flag) {
            mic_dec_pcm_out_frag1(dec_flag);
            mic_dec.frag0_done_flag = 0;
        }

        //若通道1未连接，把frag0也转换了
        con_status = ble_con_get_status();
        if((con_status & ~BIT(idx)) == 0) {
            mic_dec_pcm_out_frag0(0);
            mic_dec.frag0_done_flag = 1;
        }
    }
#endif
}
#endif // WIRELESS_CON_COMB_BUF_EN
AT(.com_text.mic_dec)
void mic_dec_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    u8 idx = *(u8 *)params;

    samples = WIRELESS_MIC_SAMPLES_SELECT;
#if DEVICE_STEREO_EN || WIRELESS_MIC_STEREO
    s16 *tmp_buf = (s16 *)mic_dec.tmp_buf;
    s16 *pcm = (s16 *)mic_dec.decout;
#else
    s16 *pcm = (s16 *)&mic_dec.pcm[idx].buf;
#endif

//    GPIOESET = BIT(4);
    if(wireless_cb.alg_en) {
        u8 con_status = ble_con_get_status();
#if WIRELESS_MIC_BROADCAST_EN
        con_status = BIT(idx);
#endif
        if(con_status & BIT(idx)) {
            bool bfi = wireless_d2a_get_rx_frame(idx, mic_dec.frame, WIRELESS_MIC_FRAME_SIZE);

            if(!bfi) {
#if (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3S)
#if DEVICE_STEREO_EN || WIRELESS_MIC_STEREO
                lc3s_stereo_dec(mic_dec.frame, (s16 *)tmp_buf, samples);
#else
                lc3s_dec(mic_dec.frame, pcm, samples, idx);
#endif
#elif (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3B)
				lc3b_dec(mic_dec.frame, pcm, samples, idx);
#endif
            }


#if (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3S)
#if DEVICE_STEREO_EN || WIRELESS_MIC_STEREO
            plc_soft_process(tmp_buf, samples, mic_dec.last_bfi[idx]||bfi, 0);
            plc_soft_process(tmp_buf + samples, samples, mic_dec.last_bfi[idx]||bfi, 1);

            u16 *ptr_16 = (u16 *)tmp_buf;
            for(uint i=0; i<samples; i++) {
                pcm[2*i]     = ptr_16[i];
                pcm[2*i + 1] = ptr_16[i+samples];
            }
#else
            plc_soft_process(pcm, samples, mic_dec.last_bfi[idx]||bfi, idx);
#endif
#elif (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3B)
            plc_soft_60_process(pcm, samples, mic_dec.last_bfi[idx]||bfi, idx);
#endif

            mic_dec.last_bfi[idx] = bfi;

#if WIRELESS_CON_PWR_CTR
            ws_pwr_ctr_rx_process(idx, bfi);
#endif
        } else {
            //断连时跑到mic_dec_reset没那么及时，这里清buf避免出现杂波
            memset(pcm, 0, MIC_DEC_OBUF_SIZE);
        }

#if DEVICE_STEREO_EN || WIRELESS_MIC_STEREO
        if (mic_dec.callback) {
            mic_dec.callback((u8 *)pcm, samples, 2, NULL);
        }
#else
        mic_dec_pcm_out(idx);
#endif
        if(idx == mic_dec.sync_idx) {
            //DAC调速，避免长时间后播放速度和发射端不匹配
            mic_dec_dac_sync_proc();
        }
    } else {
        memset(pcm, 0x00, MIC_DEC_OBUF_SIZE);
    }
//    GPIOECLR = BIT(4);
}

AT(.com_text.mic_dec)
void mic_dec_buf_clr(u8 idx)
{
#if DEVICE_STEREO_EN || WIRELESS_MIC_STEREO
    memset(mic_dec.decout, 0x00, MIC_DEC_OBUF_SIZE);
#else
    memset(mic_dec.pcm[idx].buf, 0x00, MIC_DEC_OBUF_SIZE);
#endif
}

AT(.text.mic_dec)
void mic_dec_audio_output_callback_set(audio_callback_t callback)
{
    mic_dec.callback = callback;
}

static uint16_t mic_dec_us_trans_samples(uint16_t us, uint8_t spr_idx)
{
    uint16_t samples_duration = 0;

    switch(spr_idx) {
        ///对应48k采样率的情况，一个样点对应20.8us
        case SAMPLE_RATE_48K:
            samples_duration = 21;
            break;

        ///对应16k采样率的情况，一个样点对应62.5us
        case SAMPLE_RATE_16K:
            samples_duration = 62;
            break;

        default:
            samples_duration = 21;
            break;
    }

    return (us/samples_duration);
}

AT(.text.mic_dec)
void mic_dec_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&mic_dec, 0x00, sizeof(mic_dec));

#if WIRELESS_CON_LINK_NB > 1 && !WIRELESS_CON_COMB_BUF_EN
    mic_dec.frag_samples[0] = mic_dec_us_trans_samples(ble_single_link_duration_get(), WIRELESS_MIC_SAMPLE_RATE_SELECT)%WIRELESS_MIC_SAMPLES_SELECT;
    mic_dec.frag_samples[1] = WIRELESS_MIC_SAMPLES_SELECT - mic_dec.frag_samples[0];

    printf("frag_samples=%d, %d\n", mic_dec.frag_samples[0], mic_dec.frag_samples[1]);
#endif

//    mic_dec.callback = NULL;
}

void mic_dec_reset(u8 idx)
{
    mic_dec.last_bfi[idx] = 0;
#if DEVICE_STEREO_EN || WIRELESS_MIC_STEREO
    memset(mic_dec.decout, 0x00, MIC_DEC_OBUF_SIZE);
#else
    memset(mic_dec.pcm[idx].buf, 0, MIC_DEC_OBUF_SIZE);
#endif
}


#endif
