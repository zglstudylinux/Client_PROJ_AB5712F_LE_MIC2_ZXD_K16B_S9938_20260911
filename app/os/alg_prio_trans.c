#include "include.h"
#include "alg_prio_trans.h"


#define INCACHE_SIZE        768
#define OUTCACHE_SIZE       256

static alg_prio_trans_cfg_t alg_prio_trans_cfg AT(.buf.alg_prio_trans);

static uint8_t alg_incache[INCACHE_SIZE] AT(.buf.alg_prio_trans);
static uint8_t alg_outcache[OUTCACHE_SIZE] AT(.buf.alg_prio_trans);

AT(.com_text.alg_prio_trans)
void alg_prio_trans_kick(uint samples, uint ch_mode, audio_callback_t handle)
{
    //alg_prio_trans_cfg.samples     = samples;
    alg_prio_trans_cfg.ch_mode     = ch_mode;
    alg_prio_trans_cfg.handle      = handle;

    ///kick低优先级线程，去获取alg_prio_trans输出模块
    kick_alg_prio_trans();
}

AT(.com_text.alg_prio_trans)
void alg_prio_trans_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    cbuf_cfg_t *cbuf = &(alg_prio_trans_cfg.cbuf);
    cbuf_input_audio(ptr, samples*ch_mode, cbuf);

    if(alg_prio_trans_cfg.callback != NULL) {
        alg_prio_trans_kick(samples, ch_mode, alg_prio_trans_cfg.callback);
    }
}

AT(.com_text.alg_prio_trans)
void alg_prio_trans_process(void)
{
    u8 *cache = alg_outcache;
    cbuf_cfg_t *cbuf = &(alg_prio_trans_cfg.cbuf);
    uint frame_size = alg_prio_trans_cfg.samples*alg_prio_trans_cfg.ch_mode;

    if(alg_prio_trans_cfg.handle) {
        while (cbuf_total_samples_get(cbuf) >= frame_size) {
            cbuf_output_audio((u8 *)cache, frame_size, cbuf);
            alg_prio_trans_cfg.handle(cache, alg_prio_trans_cfg.samples, alg_prio_trans_cfg.ch_mode, NULL);
        }
    }
}

AT(.text.alg_prio_trans)
void alg_prio_trans_audio_output_callback_set(audio_callback_t callback)
{
    alg_prio_trans_cfg.callback = callback;
}

AT(.text.alg_prio_trans)
void alg_prio_trans_audio_mute_set(uint8_t mute)
{
//    alg_prio_trans_cfg.mute = mute;
}

AT(.text.alg_prio_trans)
void alg_prio_trans_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&alg_prio_trans_cfg, 0, sizeof(alg_prio_trans_cfg_t));
    alg_prio_trans_cfg.sample_rate = sample_rate;
    alg_prio_trans_cfg.samples     = samples;

    memset(&alg_incache, 0, INCACHE_SIZE);
    memset(&alg_outcache, 0, OUTCACHE_SIZE);
    cbuf_init(&(alg_prio_trans_cfg.cbuf), alg_incache, INCACHE_SIZE);
}

