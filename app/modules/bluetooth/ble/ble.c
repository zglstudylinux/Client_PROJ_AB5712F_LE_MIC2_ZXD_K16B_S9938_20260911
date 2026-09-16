#include "include.h"

#if LE_EN || BT_ATT_EN

static ble_gatt_characteristic_cb_info_t *characteristic_cb_info[LE_ATT_NUM] AT(.ble_cache.att);
static uint8_t  gatts_profile_table[LE_ATT_NUM * 10] AT(.ble_cache.att);

#if LE_EN
const bool cfg_ble_security_en = LE_PAIR_EN;
bool cfg_ble_sm_sc_en          = LE_SM_SC_EN;
bool cfg_bt_ble_adv            = LE_ADV_POWERON_EN;

//可重定义该函数修改ble地址
#if LE_SM_SC_EN
void ble_get_local_bd_addr(u8 *addr)
{
    bt_get_local_bd_addr(addr);
}
#endif

//可重定义该函数修改ble地址类型
u8 ble_get_local_addr_mode(void)
{
    return LE_ADDRESS_TYPE;
}


void ble_disconnect_callback(void)
{
    printf("--->ble_disconnect_callback\n");
#if GFPS_EN
    gfps_ble_disconnected_callback();
#endif
    app_ble_disconnect_callback();
}

void ble_connect_callback(void)
{
    printf("--->ble_connect_callback\n");
#if GFPS_EN
    gfps_ble_connected_callback();
#endif
    app_ble_connect_callback();
}

//主从切换时获取需要同步给新主机的数据，返回数据长度
uint16_t role_switch_get_user_ble_data(uint8_t *data_ptr)
{
    printf("role_switch_get_user_ble_data\n");

    uint16_t len = app_ble_role_switch_get_data(data_ptr);

    return len;
}
//主从切换时接收到旧主机传递的数据
uint16_t role_switch_set_user_ble_data(uint8_t *data_ptr, uint16_t len)
{

    printf("role_switch_set_user_ble_data data\n");

    app_ble_role_switch_set_data(data_ptr,len);

    return len;
}

uint ble_get_bat_level(void)
{
    return bsp_get_bat_level();
}

#if LE_SM_SC_EN
void ble_get_link_info(void *buf, u16 addr, u16 size)
{
    //printf("bt_read: %04x,%04x, %08lx\n", addr, size, BLE_CM_PAGE(addr));

    cm_read(buf, BLE_CM_PAGE(addr), size);

    //print_r(buf, size);
}

void ble_put_link_info(void *buf, u16 addr, u16 size)
{
    //printf("bt_write: %04x,%04x, %08lx\n", addr, size, BLE_CM_PAGE(addr));
    //print_r(buf, size);

    cm_write(buf, BLE_CM_PAGE(addr), size);
}

void ble_sync_link_info(void)
{
    cm_sync();
}
#endif
#endif // LE_EN

void ble_init_att(void)
{
    ble_gatts_init(gatts_profile_table, sizeof(gatts_profile_table),
                    characteristic_cb_info,
                    LE_ATT_NUM);
    app_ble_init();
}

#endif // LE_EN || BT_ATT_EN
