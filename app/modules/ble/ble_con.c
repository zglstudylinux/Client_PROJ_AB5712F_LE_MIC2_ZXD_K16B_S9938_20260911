#include "include.h"
#include "ble_con.h"
#include "ble_service.h"

#define BLE_DEBUG_EN                    0

#if BLE_DEBUG_EN
#define BLE_DEBUG(...)                  printf(__VA_ARGS__)
#define BLE_DEBUG_R(...)                print_r(__VA_ARGS__)
#else
#define BLE_DEBUG(...)
#define BLE_DEBUG_R(...)
#endif

#if ADAPTER_NORMAL_LE_EN
AT(.ble_cache.ble_tx)
struct ble_con_tag_t ble_con_tag[BLE_CON_MAX_IDX];
static att_service_handler_t       fota_service;

static struct ble_con_tag_t *ble_con_tag_get_for_handle(uint16_t con_handle)
{
    struct ble_con_tag_t * conn = ble_con_tag;
    for(int i=0; i<BLE_CON_MAX_IDX; i++) {
        if (conn->used && conn->con_handle == con_handle) {
            return conn;
        }
        conn++;
    }
    return NULL;
}

static struct ble_con_tag_t * ble_con_free_connection_get(void){
    struct ble_con_tag_t * conn = ble_con_tag;
    for(int i=0; i<BLE_CON_MAX_IDX; i++) {
        if (conn->used == 0) {
            return conn;
        }
        conn++;
    }
    return NULL;
}

static u8 *ble_con_tx_get_cb(u16 con_handle, u16 *buf_len)
{
    struct ble_con_tag_t *conn = ble_con_tag_get_for_handle(con_handle);

    return ble_tx_buf_get(&conn->ble_tx, buf_len);
}

static void ble_con_tx_cfm_cb(u16 con_handle)
{
    struct ble_con_tag_t *conn = ble_con_tag_get_for_handle(con_handle);

    ble_tx_buf_free(&conn->ble_tx);
}

static bool ble_con_send_buf(u16 con_handle, u16 att_handle, u8 *buf, u16 len)
{
    u8 *ble_tx_buf;
    struct ble_con_tag_t *conn = ble_con_tag_get_for_handle(con_handle);

    if(!conn || !conn->notify_en || len > BLE_TX_MAX_SIZE) {
        BLE_DEBUG("ble_con_send_buf error\n");
        return false;
    }

    ble_tx_buf = ble_con_buf_alloc(&conn->ble_tx, len);
    if(ble_tx_buf == NULL) {
        return false;
    }

    memcpy(ble_tx_buf, buf, len);

    ble_tx_buf_add(&conn->ble_tx);
    att_notify_kick(con_handle, att_handle);
    return true;
}

bool ble_con_send_buf_by_link(uint8_t link, u8 *buf, u16 len)
{
    struct ble_con_tag_t * conn = &ble_con_tag[link];

    return ble_con_send_buf(conn->con_handle, ATT_CHARACTERISTIC_FF14_01_VALUE_HANDLE, buf, len);
}

void ble_update_conn_param(u8 link, u16 interval, u16 latency, u16 timeout)
{
    struct ble_con_tag_t * conn = &ble_con_tag[link];

    gap_update_conn_param(conn->con_handle, interval, latency, timeout);
}

void ble_con_emit_notice(uint evt, void *params)
{
    u8 *packet = params;
    u16 con_handle;
    struct ble_con_tag_t * conn = NULL;

    BLE_DEBUG("ble_con_emit_notice, %d\n", evt);
    switch(evt){
        case BT_NOTICE_BLE_CONNECTED:{
            con_handle = *(u16 *)(&packet[0]);
            BLE_DEBUG("BT_NOTICE_BLE_CONNECTED, con_handle = %d\n", con_handle);
            conn = ble_con_free_connection_get();
            if (conn) {
                conn->con_handle = con_handle;
                conn->used = 1;
                att_tx_cb_register(con_handle, ble_con_tx_get_cb, ble_con_tx_cfm_cb);
                att_max_mtu_register(con_handle, BLE_DEFAULT_MTU);
                app_ble_connect_callback();
            }
        } break;

        case BT_NOTICE_BLE_DISCONNECT:{
            con_handle = *(u16 *)(&packet[0]);
            conn = ble_con_tag_get_for_handle(con_handle);
            if (conn) {
                conn->used = 0;
                gap_adv_en_set(1);
                app_ble_disconnect_callback();
                BLE_DEBUG("BT_NOTICE_BLE_DISCONNECT, con_handle = %d, dis_reason %d\n", conn->con_handle, packet[2]);
            } else {
                BLE_DEBUG("BT_NOTICE_BLE_DISCONNECT, con_handle_error\n");
            }
        } break;

        case BT_NOTICE_BLE_CON_PARAM_UPD: {
            con_handle = *(u16 *)(&packet[0]);
            conn = ble_con_tag_get_for_handle(con_handle);
            if (conn) {
                conn->con_interval = *(u16 *)(&packet[2]);
                conn->con_latency = *(u16 *)(&packet[4]);
                conn->con_sup_timeout = *(u16 *)(&packet[6]);
                BLE_DEBUG("BT_NOTICE_BLE_CON_PARAM_UPD, con_handle = %d, %d, %d, %d\n", conn->con_handle, conn->con_interval, conn->con_latency, conn->con_sup_timeout);
            }
        } break;

        default:
            break;
    }
}

static uint16_t fota_read_callback(uint16_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size)
{
    BLE_DEBUG("fota_read_callback, %d, %d\n", con_handle, attribute_handle);
    if(attribute_handle == ATT_CHARACTERISTIC_FF15_01_VALUE_HANDLE){
        u8 read_data[] = "FOTA";
        if(buffer){
            memcpy(buffer, read_data, sizeof(read_data));
        }
        return sizeof(read_data);
    }

	return 0;
}

static int fota_write_callback(uint16_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    struct ble_con_tag_t *conn = ble_con_tag_get_for_handle(con_handle);

    BLE_DEBUG("fota_write_callback, %d, %d\n", con_handle, attribute_handle);
    BLE_DEBUG_R(buffer, buffer_size);
    if(attribute_handle == ATT_CHARACTERISTIC_FF14_01_CLIENT_CONFIGURATION_HANDLE){
        conn->notify_en = GET_LE16(&buffer[0]);
    }else if(attribute_handle == ATT_CHARACTERISTIC_FF15_01_VALUE_HANDLE){
#if ADAPTER_AB_FOT_EN
        if(fot_app_connect_auth(buffer,buffer_size, FOTA_CON_BLE)){
            fot_recv_proc(buffer, buffer_size);
        }
#endif
    }

	return 0;
}

bool ble_fot_send_packet(u8 *buf, u8 len)
{
    return ble_con_send_buf_by_link(0, buf, len);
}

void ble_con_start_init(void)
{
    memset((u8 *)&ble_con_tag, 0, sizeof(struct ble_con_tag_t));
    att_server_init(ble_get_profile_data());

    // get service handle range
	uint16_t start_handle = ATT_SERVICE_FF12_START_HANDLE;
	uint16_t end_handle   = ATT_SERVICE_FF12_END_HANDLE;

    // register service with ATT Server
	fota_service.start_handle   = start_handle;
	fota_service.end_handle     = end_handle;
	fota_service.read_callback  = &fota_read_callback;
	fota_service.write_callback = &fota_write_callback;
	fota_service.event_handler  = NULL;
	att_server_register_service_handler(&fota_service);
}
#endif
