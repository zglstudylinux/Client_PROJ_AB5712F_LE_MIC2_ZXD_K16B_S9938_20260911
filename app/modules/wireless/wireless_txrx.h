#ifndef __WIRELESS_TXRX_H
#define __WIRELESS_TXRX_H


void wireless_con_adapter_init(void);
void wireless_con_device_init(void);
void wireless_d2a_set_ch_status(u8 idx, u8 chstatus);
u8 wireless_d2a_get_rx_frame(u8 idx, u8 *buf, uint size);
void wireless_d2a_set_ch_status1(u8 idx, u8 chstatus);
void wireless_d2a_put_tx_frame(u8 *ptr, u16 size);
void wireless_a2d_put_tx_frame(u8 idx, u8 *ptr, u16 size);
u8 wireless_a2d_get_rx_frame(u8 *buf, uint size);
void wireless_a2d_set_ch_status(u8 idx, u8 chstatus);


#endif // __WIRELESS_TXRX_H

