/*
 * 本地MIC混合无线MIC后，一起输出
 *
 ****************************************************************************************
 */
#include "include.h"
#include "mic_mix.h"

#if ADAPTER_LOCAL_MIC_MIX_EN

#define MIC_INCACHE_SIZE            1024
#define DATA_CACHE_LEN              (WIRELESS_MIC_SAMPLES_SELECT*2)

static mic_mix_t mic_mix;
static cbuf_cfg_t mic_mix_in_cbuf;

static u8 mic_incache[MIC_INCACHE_SIZE] AT(.mic_mix_buf.cache);
static u8 mic_outcache[DATA_CACHE_LEN] AT(.mic_mix_buf.cache);


AT(.com_text.bsp.wireless_mic)
void mic_mix_process_cb(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    if(mic_mix.mute) {
        memset(ptr, 0, samples*ch_mode*2);
    }
#if ADAPTER_LOCAL_MIC_EQ_DRC_EN
    //不在adc会调中处理硬件eq/drc要注意传参地址是否合法
    mic_eq_drc_proc((u16 *)ptr,(u16 *)ptr,samples);
#endif
    if(!wireless_cb.connected_sta && mic_mix.kick_flag) {
        mic_mix.kick_flag = 0;
        memset(&mic_incache[0], 0, MIC_INCACHE_SIZE);
    }

    if(mic_mix.kick_flag){
        cbuf_input_audio(ptr, samples, &mic_mix_in_cbuf);
    } else  {
        cbuf_input_audio(ptr, samples, &mic_mix_in_cbuf);
        kick_local_mic();
    }
}

AT(.com_text.bsp.wireless_mic)
void local_mic_output(void)
{

    cbuf_output_audio((u8 *)mic_outcache, WIRELESS_MIC_SAMPLES_SELECT, &mic_mix_in_cbuf);

    mic_mix_audio_input(mic_outcache, WIRELESS_MIC_SAMPLES_SELECT, 0, 0);
}

AT(.text.mic_mix)
void mic_mix_set_mute(u8 mute)
{
    mic_mix.mute = mute;

    if(mic_mix.mute) {
        memset(&mic_incache[0], 0, MIC_INCACHE_SIZE);
    }
}

//无线麦克风连接时走这里输出
AT(.com_text.mic_mix)
void mic_mix_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    uint frame_size = samples;
    uint total_size = cbuf_total_samples_get(&mic_mix_in_cbuf);


    if(samples && wireless_cb.connected_sta){
        if(!mic_mix.kick_flag) {
            mic_mix.kick_flag = 1;
        }

        if(total_size >= frame_size) {
            cbuf_output_audio((u8 *)mic_outcache, samples, &mic_mix_in_cbuf);
            s16 *pcm_mic = (s16 *)mic_outcache;
            s16 *pcm_16 = (s16 *)ptr;
            s32 sample = 0;
            for (u8 i = 0; i < samples; i++) {
                sample = pcm_16[i] + pcm_mic[i];
                if(sample > 32767) {
                    pcm_16[i] = 32767;
                } else if(sample < -32767){
                    pcm_16[i] = -32767;
                } else {
                    pcm_16[i] = sample;
                }
            }
        }
    }
    if (mic_mix.callback) {
        mic_mix.callback(ptr, samples, ch_mode, params);
    }
}

AT(.text.mic_mix)
void mic_mix_output_callback_set(audio_callback_t callback)
{
    mic_mix.callback = callback;
}


AT(.text.mic_mix)
void mic_mix_init(u8 sample_rate, u16 samples, u8 channel)
{
    printf("%s\n", __func__);
    memset(mic_incache,0,MIC_INCACHE_SIZE);
    memset(mic_outcache,0,DATA_CACHE_LEN);
    cbuf_init(&(mic_mix_in_cbuf), mic_incache, MIC_INCACHE_SIZE);
#if ADAPTER_LOCAL_MIC_EQ_DRC_EN
    mic_eq_drc_cfg(0);
    if (mic_eq_set_by_res(RES_BUF_WS_MIC_48K_EQ, RES_LEN_WS_MIC_48K_EQ)) {
    }
    if(mic_drc_set_by_res(RES_BUF_WS_MIC_48K_DRC, RES_LEN_WS_MIC_48K_DRC)) {
    }
#endif

    local_mic_init();
}

#endif
