#include "include.h"
#include "encoder_prio_trans.h"

#define ENCODER_BUF_EN      0

#define INCACHE_SIZE        2048
#define OUTCACHE_SIZE       2048

static encoder_prio_trans_cfg_t encoder_prio_trans_cfg AT(.bss.encoder_prio_trans);

#if ENCODER_BUF_EN
static uint8_t encoder_incache[INCACHE_SIZE] AT(.bss.encoder_prio_trans);
static uint8_t encoder_outcache[OUTCACHE_SIZE] AT(.bss.encoder_prio_trans);
#endif

AT(.com_text.encoder_prio_trans)
void encoder_prio_trans_kick(uint samples, uint ch_mode, audio_callback_t handle)
{
    encoder_prio_trans_cfg.samples     = samples;
    encoder_prio_trans_cfg.ch_mode     = ch_mode;
    encoder_prio_trans_cfg.handle      = handle;

    ///kick低优先级线程，去获取encoder_prio_trans输出模块
    kick_encoder_prio_trans();
}

AT(.com_text.encoder_prio_trans)
void encoder_prio_trans_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
#if ENCODER_BUF_EN
    cbuf_cfg_t *cbuf = &(encoder_prio_trans_cfg.cbuf);
    cbuf_input_audio(ptr, samples*ch_mode, cbuf);
#else
    encoder_prio_trans_cfg.ptr = ptr;
#endif

    if(encoder_prio_trans_cfg.callback != NULL) {
        encoder_prio_trans_kick(samples, ch_mode, encoder_prio_trans_cfg.callback);
    }
}

AT(.com_text.encoder_prio_trans)
void encoder_prio_trans_process(void)
{
#if ENCODER_BUF_EN
    u8 *cache = encoder_outcache;
    cbuf_cfg_t *cbuf = &(encoder_prio_trans_cfg.cbuf);
    uint frame_size = encoder_prio_trans_cfg.samples*encoder_prio_trans_cfg.ch_mode;
    uint total_size = cbuf_total_samples_get(cbuf);

    if(encoder_prio_trans_cfg.handle) {
        if (total_size >= frame_size) {
            cbuf_output_audio((u8 *)cache, frame_size, cbuf);
        } else {
            memset(cache, 0x00, frame_size*2);
        }
        encoder_prio_trans_cfg.handle(cache, encoder_prio_trans_cfg.samples, encoder_prio_trans_cfg.ch_mode, NULL);
    }
#else
    u8 *cache = encoder_prio_trans_cfg.ptr;

    if(encoder_prio_trans_cfg.handle) {
        encoder_prio_trans_cfg.handle(cache, encoder_prio_trans_cfg.samples, encoder_prio_trans_cfg.ch_mode, NULL);
    }
#endif
}

AT(.text.encoder_prio_trans)
void encoder_prio_trans_audio_output_callback_set(audio_callback_t callback)
{
    encoder_prio_trans_cfg.callback = callback;
}

AT(.text.encoder_prio_trans)
void encoder_prio_trans_audio_mute_set(uint8_t mute)
{
//    encoder_prio_trans_cfg.mute = mute;
}

AT(.text.encoder_prio_trans)
void encoder_prio_trans_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&encoder_prio_trans_cfg, 0, sizeof(encoder_prio_trans_cfg_t));
    encoder_prio_trans_cfg.sample_rate = sample_rate;
    encoder_prio_trans_cfg.samples     = samples;

#if ENCODER_BUF_EN
    memset(&encoder_incache, 0, INCACHE_SIZE);
    memset(&encoder_outcache, 0, OUTCACHE_SIZE);
    cbuf_init(&(encoder_prio_trans_cfg.cbuf), encoder_incache, INCACHE_SIZE);
#endif
}
