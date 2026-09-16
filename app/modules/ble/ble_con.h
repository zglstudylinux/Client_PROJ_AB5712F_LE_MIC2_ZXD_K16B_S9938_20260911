#ifndef _BLE_CON_H
#define _BLE_CON_H

#include "ble_tx_proc.h"

#define BLE_CON_MAX_IDX     2
#define BLE_DEFAULT_MTU     150

typedef uint8_t *(*att_tx_get_func_t)(u16 con_handle, u16 *buf_len);
typedef void (*att_tx_cfm_func_t)(u16 con_handle);
typedef uint16_t (*att_read_callback_t)(uint16_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size);
typedef int (*att_write_callback_t)(uint16_t con_handle, uint16_t attribute_handle, uint16_t trans_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
typedef void (*att_event_callback_t) (uint8_t event_type, uint8_t *param, uint16_t size);

struct ble_con_tag_t
{
    uint8_t used;
    uint8_t notify_en;
    uint16_t con_handle;
    uint16_t con_interval;
    uint16_t con_latency;
    uint16_t con_sup_timeout;
    struct ble_tx_tag ble_tx;
};

typedef struct att_service_handler {
    void * item;
    uint16_t start_handle;
    uint16_t end_handle;
    att_read_callback_t read_callback;
    att_write_callback_t write_callback;
    att_event_callback_t event_handler;
} att_service_handler_t;

void att_server_register_service_handler(att_service_handler_t * handler);
void att_server_init(uint8_t const * db);
void att_tx_cb_register(uint16_t con_handle, att_tx_get_func_t att_tx_get_cb, att_tx_cfm_func_t att_tx_cfm_cb);
void att_max_mtu_register(uint16_t con_handle, uint16_t max_mtu);
void att_notify_kick(uint16_t con_handle, uint16_t att_handle);

void ble_fota_service_init(void);
void ble_con_start_init(void);
bool ble_con_send_buf_by_link(uint8_t link, u8 *buf, u16 len);
void ble_update_conn_param(u8 link, u16 interval, u16 latency, u16 timeout);
#endif
