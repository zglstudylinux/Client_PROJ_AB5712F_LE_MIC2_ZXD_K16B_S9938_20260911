#ifndef _BSP_AUX_H
#define _BSP_AUX_H

void bsp_aux_start(void);
void bsp_aux_stop(void);
void aux_channel_mute(u8 ch_sel);
void aux_channel_unmute(u8 ch_sel);
void set_aux_analog_vol(u8 level, u8 auxlr_sel);
#endif
