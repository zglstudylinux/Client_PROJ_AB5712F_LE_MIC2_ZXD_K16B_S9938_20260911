#ifndef _BSP_HUART_H
#define _BSP_HUART_H

void bsp_huart_init(void);
u8* huart_get_rxbuf(u16 *len);
extern u8 *huart_rx_buf;
void huart_init_do(u8 tx_port, u8 rx_port, u32 sample_rate, u8 *buf, u16 buf_size,u8 isr_rx_en, u8 isr_tx_en);



#endif
