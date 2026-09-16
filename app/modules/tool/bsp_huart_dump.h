#ifndef __BSP_HUART_DUMP_H
#define __BSP_HUART_DUMP_H

void huart_wait_tx_finish(void);
void huart_putbuf(void *buf, u32 len);
void huart_module_init(void);

#endif
