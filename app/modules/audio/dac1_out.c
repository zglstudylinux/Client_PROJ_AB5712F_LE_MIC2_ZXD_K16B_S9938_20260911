#include "include.h"
#include "dac1_out.h"

static dac_out_cfg_t dac1_out_cfg;


AT(.com_text.dac1_out)
void dac1_out_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    s16 *rptr = (s16 *)ptr;
//    s32 *rptr = (s32 *)ptr;
    uint16_t i;

    if (dac1_out_cfg.mute) {
        goto dac1_out_callback;
    }

    if (ch_mode == 2) {
        for(i = 0; i < samples; i++) {
            dac1_put_sample_16bit(rptr[2*i], rptr[2*i + 1]);
        }
    } else {
        for(i = 0; i < samples; i++) {
            dac1_put_sample_16bit(rptr[i], rptr[i]);
        }
    }

dac1_out_callback:
    if (dac1_out_cfg.callback) {
        dac1_out_cfg.callback(ptr, samples, ch_mode, params);
    }
}

//usb下行音频推dac
AT(.com_text.dac1_out)
void usb_speaker_dac1_out_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    s32 *rptr = (s32 *)ptr;
    uint16_t i;

    if (dac1_out_cfg.mute) {
        goto dac1_out_callback;
    }

    for(i = 0; i < samples; i+=1) {
        usb_speaker_dac1_put_sample_32bit(rptr[i]);
    }

dac1_out_callback:
    if (dac1_out_cfg.callback) {
        dac1_out_cfg.callback(ptr, samples, ch_mode, params);
    }
}


AT(.text.dac1_out)
void dac1_out_audio_output_callback_set(audio_callback_t callback)
{
    dac1_out_cfg.callback = callback;
}

AT(.text.dac1_out)
void dac1_out_audio_mute_set(uint8_t mute)
{
    if(mute == 0) {
        dac1_aubuf_init();
        dac1_spr_set(dac1_out_cfg.sample_rate);
        dac1_vol_set(0x7fff);
        dac1_fade_in();
    }
    dac1_out_cfg.mute = mute;
}

AT(.text.dac1_out)
void dac1_out_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&dac1_out_cfg, 0, sizeof(dac1_out_cfg));
//    dac1_out_param_cfg_t *dac1_out_param_temp_ptr = dac1_out_param_get_by_sam_rate(sample_rate);
//    dac1_out_cfg.low_thr = dac1_out_param_temp_ptr->low_thr;
//    dac1_out_cfg.high_thr = dac1_out_param_temp_ptr->high_thr;
    dac1_out_cfg.sample_rate = sample_rate;

    dac1_aubuf_init();
    dac1_spr_set(sample_rate);
    dac1_out_audio_mute_set(0);
    dac1_phase_set(0);
    dac1_vol_set(0x7fff);
    dac1_fade_in();
}

//--------------------------------------------------------------------------------------------------
static dac_out_cfg_t dac0_out_cfg;

AT(.com_text.dac0_out)
void dac0_out_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    s16 *rptr = (s16 *)ptr;
//    s32 *rptr = (s32 *)ptr;
    uint16_t i;

    if (dac0_out_cfg.mute) {
        goto dac0_out_callback;
    }
#if WIRELESS_MIC_DAC_OUTPUT_EN                        //对讲机功能输出暂时支持单声道
    for(i = 0; i < samples; i++) {
        dac_put_sample_16bit(rptr[i], rptr[i]);
    }
#else
    if (ch_mode == 2 || (ch_mode&BIT(0))) {    //pcm is stereo?
        for(i = 0; i < samples; i++) {
            dac_put_sample_16bit(rptr[2*i], rptr[2*i + 1]);
        }
    } else {
        for(i = 0; i < samples; i++) {
            dac_put_sample_16bit(rptr[i], rptr[i]);
        }
    }
#endif
dac0_out_callback:
    if (dac0_out_cfg.callback) {
        dac0_out_cfg.callback(ptr, samples, ch_mode, params);
    }
}

AT(.text.dac0_out)
void dac0_out_audio_output_callback_set(audio_callback_t callback)
{
    dac0_out_cfg.callback = callback;
}

AT(.text.dac0_out)
void dac0_out_audio_mute_set(uint8_t mute)
{
    if(mute == 0) {
        dac_aubuf_init();
        dac_spr_set(dac0_out_cfg.sample_rate);
        dac_vol_set(0x7fff);
        dac_fade_in();
    }
    dac0_out_cfg.mute = mute;
}

AT(.text.dac0_out)
void dac0_out_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&dac0_out_cfg, 0, sizeof(dac0_out_cfg));
//    dac1_out_param_cfg_t *dac1_out_param_temp_ptr = dac1_out_param_get_by_sam_rate(sample_rate);
//    dac0_out_cfg.low_thr = dac1_out_param_temp_ptr->low_thr;
//    dac0_out_cfg.high_thr = dac1_out_param_temp_ptr->high_thr;
    dac0_out_cfg.sample_rate = sample_rate;

    dac_aubuf_init();
    dac_spr_set(sample_rate);
//    dac_out_audio_mute_set(0);

    dac_vol_set(0x7fff);
    dac_fade_in();
}
