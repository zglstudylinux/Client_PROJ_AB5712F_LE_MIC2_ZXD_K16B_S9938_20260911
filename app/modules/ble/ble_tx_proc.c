#include "include.h"
#include "ble_tx_proc.h"

#define BLE_DEBUG_EN                    0

#if BLE_DEBUG_EN
#define BLE_DEBUG(...)                  printf(__VA_ARGS__)
#define BLE_DEBUG_R(...)                print_r(__VA_ARGS__)
#else
#define BLE_DEBUG(...)
#define BLE_DEBUG_R(...)
#endif

#if ADAPTER_NORMAL_LE_EN
void *ble_con_buf_alloc(struct ble_tx_tag *ble_tx, u16 len)
{
    if(ble_tx->total < BLE_TX_MAX_NB) {
        u8 w_idx = ble_tx->w_idx;
        ble_tx->w_idx++;
        if(ble_tx->w_idx >= BLE_TX_MAX_NB) {
            ble_tx->w_idx = 0;
        }
        ble_tx->ble_tx_str[w_idx].buf_len = len;
        return ble_tx->ble_tx_str[w_idx].buf;
    }

    return NULL;
}

void ble_tx_buf_add(struct ble_tx_tag *ble_tx)
{
    ble_tx->total++;
}

void *ble_tx_buf_get(struct ble_tx_tag *ble_tx, u16 *buf_len)
{
    u8 r_idx = ble_tx->r_idx;
    if(ble_tx->total > 0) {
        ble_tx->r_idx++;
        if(ble_tx->r_idx >= BLE_TX_MAX_NB) {
            ble_tx->r_idx = 0;
        }
//        ble_tx->total--;
        *buf_len = ble_tx->ble_tx_str[r_idx].buf_len;
        return ble_tx->ble_tx_str[r_idx].buf;
    }

    return NULL;
}

void ble_tx_buf_free(struct ble_tx_tag *ble_tx)
{
    if (ble_tx->total) {
        ble_tx->total--;
    }
}

void *ble_tx_buf_peek(struct ble_tx_tag *ble_tx)
{
    u8 r_idx = ble_tx->r_idx;
    if(ble_tx->total != 0) {
        return ble_tx->ble_tx_str[r_idx].buf;
    }

    return NULL;
}
#endif



