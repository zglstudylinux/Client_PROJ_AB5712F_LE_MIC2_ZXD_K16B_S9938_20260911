#include "include.h"
#include "ab_link.h"

#if LE_AB_LINK_APP_EN
///////////////////////////////////////////////////////////////////////////
const uint8_t adv_data_const[] = {
    // Flags general discoverable, BR/EDR not supported
    0x02, 0x01, 0x02,
    // Manufacturer Specific Data
    0x03, 0xff, 0x42, 0x06,
    // Complete Local Name
    //0x09, 0x09, 'B', 'l', 'u', 'e', 't', 'r', 'u', 'm',
};

const uint8_t scan_data_const[] = {
    // Name
    0x08, 0x09, 'B', 'L', 'E', '-', 'B', 'O', 'X',
};

u32 ble_get_scan_data(u8 *scan_buf, u32 buf_size)
{
    memset(scan_buf, 0, buf_size);
    u32 data_len = sizeof(scan_data_const);
    memcpy(scan_buf, scan_data_const, data_len);

    //读取BLE配置的蓝牙名称
    int len;
    len = strlen(xcfg_cb.le_name);
    if (len > 0) {
        len = len > 29 ? 29 : len;
        memcpy(&scan_buf[2], xcfg_cb.le_name, len);
        data_len = 2 + len;
        scan_buf[0] = len + 1;
    }
    return data_len;
}

u32 ble_get_adv_data(u8 *adv_buf, u32 buf_size)
{
    memset(adv_buf, 0, buf_size);
    u32 data_len = sizeof(adv_data_const);
    memcpy(adv_buf, adv_data_const, data_len);

//    //读取BLE配置的蓝牙名称
//    int len;
//    len = strlen(xcfg_cb.le_name);
//    if (len > 0) {
//        memcpy(&adv_buf[9], xcfg_cb.le_name, len);
//        adv_buf[7] = len + 1;
//        data_len = 9 + len;
//    }
    return data_len;
}

///////////////////////////////////////////////////////////////////////////
#define MAX_NOTIFY_NUM          8
#define MAX_NOTIFY_LEN          256     //max=256
#define NOTIFY_POOL_SIZE       (MAX_NOTIFY_LEN + sizeof(struct txbuf_tag)) * MAX_NOTIFY_NUM

AT(.ble_cache.att)
uint8_t notify_tx_pool[NOTIFY_POOL_SIZE];

void ble_txpkt_init(void)
{
    txpkt_init(&notify_tx, notify_tx_pool, MAX_NOTIFY_NUM, MAX_NOTIFY_LEN);
    notify_tx.send_kick = ble_send_kick;
}

/***
*
*/
#define BLE_CMD_BUF_LEN     4
#define BLE_CMD_BUF_MASK    (BLE_CMD_BUF_LEN - 1)
#define BLE_RX_BUF_LEN      20

struct ble_cmd_t{
    u8 len;
    u16 handle;
    u8 buf[BLE_RX_BUF_LEN];
};

struct ble_cb_t {
    struct ble_cmd_t cmd[BLE_CMD_BUF_LEN];
    u8 cmd_rptr;
    u8 cmd_wptr;
    bool wakeup;
} ble_cb;


//----------------------------------------------------------------------------
//battery service
//const uint8_t battery_primay_uuid16[2]={0x0f, 0x18};
//static const gatts_uuid_base_st uuid_battery_primay_base = {
//    .type = BLE_GATTS_UUID_TYPE_16BIT,
//    .uuid = battery_primay_uuid16,
//};
//
//const uint8_t battery_uuid16[2]={0x19, 0x2a};
//static const gatts_uuid_base_st uuid_battery_base = {
//    .props = ATT_READ|ATT_NOTIFY,
//    .type = BLE_GATTS_UUID_TYPE_16BIT,
//    .uuid = battery_uuid16,
//};
//static gatts_service_base_st gatts_battery_base;


//----------------------------------------------------------------------------
//test service
const uint8_t test_primay_uuid16[2]={0x10, 0xff};
static const gatts_uuid_base_st uuid_test_primay_base = {
    .type = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid = test_primay_uuid16,
};

const uint8_t test_uuid16[2]={0xf1, 0xff};
static const gatts_uuid_base_st uuid_test_base = {
    .props = ATT_NOTIFY|ATT_WRITE,
    .type = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid = test_uuid16,
};
static gatts_service_base_st gatts_tests_base;

bool ab_link_test_ble_send_packet(u8 *buf, u8 len)
{
    static u32 ble_tick = 0;
    while (!tick_check_expire(ble_tick, 30)) {     //延时30ms再发
        delay_5ms(2);
    }
    ble_tick = tick_get();

    printf("BLE TX [%d]: \n", len);
    print_r(buf, len);

    return (ble_tx_notify(gatts_tests_base.handle, buf, len) == 0);
}

//----------------------------------------------------------------------------
//app service
const uint8_t app_primay_uuid16[2]={0x12, 0xff};
static const gatts_uuid_base_st uuid_app_primay_base = {
    .type = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid = app_primay_uuid16,
};

const uint8_t app_write_uuid16[2]={0x13, 0xff};
static const gatts_uuid_base_st uuid_app_write_base = {
    .props = ATT_WRITE,
    .type = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid = app_write_uuid16,
};

const uint8_t app_notify_uuid16[2]={0x14, 0xff};
static const gatts_uuid_base_st uuid_app_notify_base = {
    .props = ATT_NOTIFY,
    .type = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid = app_notify_uuid16,
};
static gatts_service_base_st gatts_app_notify_base;

static ble_gatt_characteristic_cb_info_t gatts_app_notify_cb_info = {
    .client_config = GATT_CLIENT_CONFIG_NOTIFY,
};

//----------------------------------------------------------------------------
//app
static int gatt_callback_app(uint16_t con_handle, uint16_t handle, uint32_t flag, uint8_t *ptr, uint16_t len)
{
    u8 wptr = ble_cb.cmd_wptr & BLE_CMD_BUF_MASK;
    ble_cb.cmd_wptr++;
    if (len > BLE_RX_BUF_LEN) {
        len = BLE_RX_BUF_LEN;
    }
    memcpy(ble_cb.cmd[wptr].buf, ptr, len);
    ble_cb.cmd[wptr].len = len;
    ble_cb.cmd[wptr].handle = gatts_tests_base.handle;
    ble_cb.wakeup = true;

    return 0;
}

static ble_gatt_characteristic_cb_info_t gatts_test_cb_info = {
    .att_callback_func = gatt_callback_app,
    .client_config = GATT_CLIENT_CONFIG_NOTIFY,
};

static ble_gatt_characteristic_cb_info_t gatts_app_write_cb_info = {
    .att_callback_func = gatt_callback_app,
};

void ab_link_app_process(void)
{
    if (ble_cb.cmd_rptr == ble_cb.cmd_wptr) {
        ble_cb.wakeup = false;
        return;
    }
    u8 rptr = ble_cb.cmd_rptr & BLE_CMD_BUF_MASK;
    ble_cb.cmd_rptr++;
    u8 *ptr = ble_cb.cmd[rptr].buf;
    u8 len = ble_cb.cmd[rptr].len;
    u16 handle = ble_cb.cmd[rptr].handle;

    if (handle == gatts_tests_base.handle) {             //兼容旧版APP
        ab_link_rx_callback(ptr, len);
    }
}



bool ab_link_rx_callback(u8 *ptr, u16 size)
{
    printf("BLE_RX[%d]", size);
    print_r(ptr, size);

    return ab_link_test_ble_send_packet(ptr, size);
}

bool ab_link_need_wakeup(void)
{
    return ble_cb.wakeup;
}

//----------------------------------------------------------------------------
//FOTA
#if LE_AB_FOT_EN
const uint8_t fota_uuid16[2]={0x15, 0xff};
static const gatts_uuid_base_st uuid_fota_base = {
    .props = ATT_READ|ATT_WRITE_WITHOUT_RESPONSE,
    .type  = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid  = fota_uuid16,
};

static int gatt_callback_fota(uint16_t con_handle, uint16_t handle, uint32_t flag, uint8_t *ptr, uint16_t len)
{
    if(fot_app_connect_auth(ptr, len, FOTA_CON_BLE)){
        fot_recv_proc(ptr,len);
    }
    return 0;
}

static ble_gatt_characteristic_cb_info_t gatts_fota_cb_info = {
    .att_callback_func = gatt_callback_fota,
};

bool ble_fot_send_packet(u8 *buf, u8 len)
{
    return ble_tx_notify(gatts_app_notify_base.handle, buf, len);
}
#endif

//----------------------------------------------------------------------------
//
void ble_app_gatts_service_init(void)
{
    int ret = 0;

    //battery service
//    ret = ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
//                                uuid_battery_primay_base.uuid,
//                                uuid_battery_primay_base.type,
//                                NULL);            //PRIMARY
//
//    ret = ble_gatts_characteristic_add(uuid_battery_base.uuid,
//                                       uuid_battery_base.type,
//                                       uuid_battery_base.props,
//                                       &gatts_battery_base.handle,
//                                       NULL);      //characteristic

    //Test Service
    ret = ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                               uuid_test_primay_base.uuid,
                               uuid_test_primay_base.type,
                               NULL);            //PRIMARY

    ret = ble_gatts_characteristic_add(uuid_test_base.uuid,
                                      uuid_test_base.type,
                                      uuid_test_base.props,
                                      &gatts_tests_base.handle,
                                      &gatts_test_cb_info);      //characteristic
//APP Service
    ret = ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                uuid_app_primay_base.uuid,
                                uuid_app_primay_base.type,
                                NULL);            //PRIMARY

    ret = ble_gatts_characteristic_add(uuid_app_write_base.uuid,
                                       uuid_app_write_base.type,
                                       uuid_app_write_base.props,
                                       NULL,
                                       &gatts_app_write_cb_info);      //characteristic

    ret = ble_gatts_characteristic_add(uuid_app_notify_base.uuid,
                                       uuid_app_notify_base.type,
                                       uuid_app_notify_base.props,
                                       &gatts_app_notify_base.handle,
                                       &gatts_app_notify_cb_info);      //characteristic

#if LE_AB_FOT_EN
    ret = ble_gatts_characteristic_add(uuid_fota_base.uuid,
                                       uuid_fota_base.type,
                                       uuid_fota_base.props,
                                       NULL,
                                       &gatts_fota_cb_info);      //characteristic
#endif

    if(ret != BLE_GATTS_SUCCESS){
        printf("gatt err: %d\n", ret);
        return;
    }
}


//----------------------------------------------------------------------------
//
void ab_link_ble_app_init(void)
{
    ble_app_gatts_service_init();
}

#endif // LE_AB_LINK_APP_EN
