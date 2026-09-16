#ifndef __BSP_XDRC_AUDIO_LINK_H
#define __BSP_XDRC_AUDIO_LINK_H

void xdrc_pre_vol_update(u8 *buf);
void xdrc_delay_samples_update(u8* buf);
void huart_tx(const void *buf, uint len);
uint calc_crc(void *buf, uint len, uint seed);
void xdrc_adj_coef_update(void);
void xdrc_adj_parse_cmd_car(void);

#endif
