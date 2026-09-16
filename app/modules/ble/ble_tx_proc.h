#ifndef _BLE_TX_PROC_H
#define _BLE_TX_PROC_H

#define BLE_TX_MAX_NB       4
#define BLE_TX_MAX_SIZE     255

typedef struct{
    u8 buf[BLE_TX_MAX_SIZE];
    u16 buf_len;
}ble_tx_str_t;

struct ble_tx_tag {
    ble_tx_str_t ble_tx_str[BLE_TX_MAX_NB];
    u16 con_handle;
    u8 w_idx;
    u8 r_idx;
    u8 total;
};

void *ble_con_buf_alloc(struct ble_tx_tag *ble_tx, u16 len);
void ble_tx_buf_add(struct ble_tx_tag *ble_tx);
void *ble_tx_buf_get(struct ble_tx_tag *ble_tx, u16 *buf_len);
void *ble_tx_buf_peek(struct ble_tx_tag *ble_tx);
void ble_tx_buf_free(struct ble_tx_tag *ble_tx);
#endif
