#ifndef __AB_LINK_H
#define __AB_LINK_H

#include "include.h"

void ab_link_app_process(void);
bool ab_link_rx_callback(u8 *ptr, u16 size);
void ab_link_ble_app_init(void);
bool ab_link_need_wakeup(void);

#endif // __AB_LINK_H
