#include "include.h"
#include "ble_adv.h"

#if ADAPTER_NORMAL_LE_EN
///////////////////////////////////////////////////////////////////////////
const uint8_t adv_data_const[] = {
    // Flags general discoverable, BR/EDR not supported
    0x02, 0x01, 0x06,
    // Manufacturer Specific Data
    0x03, 0xff, 0x42, 0x06,
    // Incomplete List of 16-bit Service Class UUIDs
    0x03, 0x02, 0x12, 0x18,
    // Appearance: Mouse
    0x03, 0x19, 0xc2, 0x03,
};

const uint8_t scan_rsp_data_const[] = {
    // Name
    0x08, 0x09, 'B', 'L', 'E', '-', 'B', 'O', 'X',
};

static u32 ble_get_scan_rsp_data(u8 *scan_buf, u32 buf_size)
{
    memset(scan_buf, 0, buf_size);
    u32 data_len = sizeof(scan_rsp_data_const);
    memcpy(scan_buf, scan_rsp_data_const, data_len);

    //读取BLE配置的蓝牙名称
    int len;
    len = strlen(xcfg_cb.ws_name);
    if (len > 0) {
        memcpy(&scan_buf[2], xcfg_cb.ws_name, len);
        data_len = 2 + len;
        scan_buf[0] = len + 1;
    }
    return data_len;
}

static u32 ble_get_adv_data(u8 *adv_buf, u32 buf_size)
{
    memset(adv_buf, 0, buf_size);
    u32 data_len = sizeof(adv_data_const);
    memcpy(adv_buf, adv_data_const, data_len);

    return data_len;
}

static void ble_adv_param_init(void)
{
    struct adv_param_t adv_info;

    memset(&adv_info,0,sizeof(adv_info));
    adv_info.adv_int_min = adv_info.adv_int_max = 0x30;
    adv_info.adv_type = ADV_TYPE_IND;
    adv_info.channel_map = 0x07;

    gap_adv_param_set(&adv_info);
}

static void ble_adv_data_init(void)
{
    struct adv_data_t adv_data;

    adv_data.data_len = ble_get_adv_data(adv_data.data, sizeof(adv_data.data));

    gap_adv_data_set(&adv_data);
}

static void ble_scan_rsp_data_init(void)
{
    struct adv_data_t scan_rsp_data;

    scan_rsp_data.data_len = ble_get_scan_rsp_data(scan_rsp_data.data, sizeof(scan_rsp_data.data));

    gap_scan_rsp_data_set(&scan_rsp_data);
}

static void ble_adv_en_init(void)
{
    gap_adv_en_set(1);
}

void ble_adv_start_init(void)
{
    ble_adv_param_init();
    ble_adv_data_init();
    ble_scan_rsp_data_init();
    ble_adv_en_init();
}
#endif
