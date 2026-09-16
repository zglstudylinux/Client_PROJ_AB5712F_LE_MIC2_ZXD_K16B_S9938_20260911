#ifndef _PLUGIN_H
#define _PLUGIN_H

#include "multi_lang.h"
#include "port_pwm.h"
#include "port_linein.h"
#include "port_mic.h"
#include "port_led.h"
#include "port_sd.h"
#include "port_tkey.h"
#include "bt_call.h"
#include "effect.h"

#if EQ_MODE_EN
#define MUSIC_EQ_TBL_LEN                        6
#else
#define MUSIC_EQ_TBL_LEN                        1
#endif // EQ_MODE_EN

//effect.bin
extern const u8 eq_effect_idx_tbl[MUSIC_EQ_TBL_LEN];
u32 effect_res_len_get(uint effect_idx);
u8 *effect_res_addr_get(uint effect_idx);
u8 effect_info_offset_get(uint effect_idx);

void plugin_init(void);
void plugin_var_init(void);
void plugin_tmr5ms_isr(void);
void plugin_tmr1ms_isr(void);
void maxvol_tone_play(void);
void minvol_tone_play(void);
void plugin_music_eq(void);
void plugin_playmode_warning(void);

void loudspeaker_mute_init(void);
void loudspeaker_mute(void);
void loudspeaker_unmute(void);
void loudspeaker_disable(void);

void amp_sel_cfg_init(u8 io_num);
void amp_sel_cfg_d(void);
void amp_sel_cfg_ab(void);
void amp_sel_cfg_dis(void);

void sleep_wakeup_config(void);
bool is_sleep_dac_off_enable(void);
extern volatile int pwrkey_detect_flag;
void plugin_vbat_filter(u32 *vbat);
bool plugin_func_idle_enter_check(void);
void plugin_sys_init_finish_callback(void);

void plugin_lowbat_vol_reduce(void);
void plugin_lowbat_vol_recover(void);
void plugin_saradc_init(u16 *adc_ch);
void plugin_saradc_sel_channel(u16 *adc_ch);
void plugin_hfp_karaok_configure(void);
void plugin_karaok_init(void);
void key_knob_process(u16 adc_val, const u8 *knob_level, u8 *key_val);
void magic_voice_switch(void);
#if MP3_SPR_CHANGED_JUMP_NEXT
void plugin_check_mp3_spr(void);
#endif

extern const u16 echo_level_gain_16[16 + 1][2];
extern const u16 echo_level_gain_12[12 + 1][2];
#endif
