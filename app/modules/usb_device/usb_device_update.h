#ifndef _USB_DEVICE_UPDATE_H
#define _USB_DEVICE_UPDATE_H

#define little_endian_read_16(buf, ofs)         *(uint16_t *)((uint8_t *)buf + (ofs))
#define little_endian_read_32(buf, ofs)         *(uint32_t *)((uint8_t *)buf + (ofs))

bool usb_fot_device_wakeup(void);

void usb_dev_update_rx_pkt(uint8_t *ptr, uint8_t len);
void usb_dev_update_pkt_save(u8 *ptr, uint8_t len);
void usb_dev_update_pkt_req(void);
void usb_dev_update_sta_rsp(uint8_t update_role, uint8_t sta);
void usb_dev_update_process(void);
void usb_dev_update_reset(void);
void usb_dev_update_init(u8 *param, uint8_t len);

//外部api
void usb_device_hid_send_user(u8 report_id, u8 *data, int len, int type);

void fot_init(void);
void fot_tws_update_data_write(u8 *data, u32 addr, u32 len);
void fot_tws_update_file_hash_set(u32 file_hash);
void fot_tws_sync_update_done(void);
void fot_tws_rsp_clear(void);
uint8_t fot_tws_rsp_get();
uint8_t fot_tws_remote_err_get();

void tws_fot_fill_in_data(uint8_t *data_ptr, uint16_t size);
void bt_tws_sync_fot_data(void);

void ble_audio_ctr_cb(uint8_t enable);
u8 wireless_get_status(void);
#endif
