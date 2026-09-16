#ifndef _API_EFFECTS_H
#define _API_EFFECTS_H

typedef struct {
    const u8* dbb_param;
    u32 param_len;
    s32* coef_l;
    s32* coef_r;
    u8 dac_band_cnt;
} dbb_param_cb_t;

void codecs_pcm_init(void);
u8 codecs_pcm_is_start(void);

///动态低音音效
int music_dbb_init(dbb_param_cb_t* p);
int music_dbb_update_param(u8 vol_level, u8 bass_level);
int music_dbb_stop(void);
u8 music_dbb_get_bass_level(void);
u8 music_dbb_get_level_cnt(void);

#endif // _API_EFFECTS_H
