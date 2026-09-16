#ifndef _BLE_ADV_H
#define _BLE_ADV_H

//ADV_TYPE
enum {
    ADV_TYPE_IND = 0,
    ADV_TYPE_DIRECT_HI_IND,
    ADV_TYPE_SCAN_IND,
    ADV_TYPE_NONCONN_IND,
    ADV_TYPE_DIRECT_LO_IND,
};

void ble_adv_start_init(void);
#endif
