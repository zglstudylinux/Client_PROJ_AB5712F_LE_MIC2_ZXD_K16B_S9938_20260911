#ifndef _USB_DEVICE_USER_H
#define _USB_DEVICE_USER_H

#include "usb_device_storage.h"

void do_host_none_user(uds_t *uds);
uint do_host_in_user(uds_t *uds);
void do_host_out_user(uds_t *uds);
u8 get_xlink_cur_state(void);
void do_host_out_fot_update(uds_t *uds);
uint do_host_in_fot_update(uds_t *uds);
void usb_fot_device_process(void);

void fot_write_start(uds_t *uds);
uint do_host_in_fot_update(uds_t *uds);
void do_host_out_fot_update(uds_t *uds);
void do_host_none_fot_update(uds_t *uds);
#endif // _USB_DEVICE_USER_H
