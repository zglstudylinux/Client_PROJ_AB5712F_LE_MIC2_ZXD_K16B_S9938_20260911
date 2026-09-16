#include "include.h"
#include "ab_mate_tws.h"
#include "ab_mate_ota.h"
#include "ab_mate_timer.h"
#include "ab_mate_profile.h"

#if AB_MATE_APP_EN && BT_TWS_EN

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif

u8 tws_sync_data[128] AT(.app.buf);
u16 tws_sync_data_len;

#if AB_MATE_OTA_EN
u8 tws_sync_ota[512+16] AT(.app.buf);
u16 tws_sync_ota_len;
#endif

uint16_t tws_set_fot_data(uint8_t *data_ptr, uint16_t size)
{
    ab_mate_tws_recv_proc(data_ptr,size);

    return 0;
}


uint16_t tws_get_fot_data(uint8_t *buf)
{
    u16 sync_len = 0;

#if AB_MATE_OTA_EN
    if(tws_sync_ota_len){
        memcpy(buf, tws_sync_ota, tws_sync_ota_len);
        sync_len = tws_sync_ota_len;
        tws_sync_ota_len = 0;
    }else if(tws_sync_data_len){
        memcpy(buf, tws_sync_data, tws_sync_data_len);
        sync_len = tws_sync_data_len;
        tws_sync_data_len = 0;
    }
#else
    if(tws_sync_data_len){
        memcpy(buf, tws_sync_data, tws_sync_data_len);
        sync_len = tws_sync_data_len;
        tws_sync_data_len = 0;
    }
#endif

    return sync_len;
}

void tws_data_sync_do(void)
{
    if(bt_tws_is_connected()){
        bt_tws_sync_fot_data();
    }
}

void ab_mate_tws_cmd_sync_byte(u8 cmd, u8 data)
{
    tws_sync_data[0] = cmd;
    tws_sync_data[1] = data;
    tws_sync_data_len = 2;
    tws_data_sync_do();
}

void ab_mate_tws_cmd_sync_nbyte(u8 cmd, u8* data, u8 len)
{
    u8 offset = 0;

    tws_sync_data[offset++] = cmd;
    memcpy(&tws_sync_data[offset], data, len);
    offset += len;

    tws_sync_data_len = offset;
    tws_data_sync_do();
}

void ab_mate_tws_vbat_sync(void)
{
    if((bt_get_status() < BT_STA_INCOMING) && !ab_mate_ota_is_start()){
        u8 bat_info[2];
        bat_info[0] = ab_mate_app.local_vbat;
        bat_info[1] = ab_mate_app.box_vbat;
        ab_mate_tws_cmd_sync_nbyte(TWS_INFO_VBAT, bat_info, 2);
    }
}

void ab_mate_tws_eq_info_sync(void)
{
#if AB_MATE_EQ_EN
    ab_mate_tws_cmd_sync_nbyte(TWS_INFO_EQ, (u8*)&ab_mate_app.eq_info,sizeof(ab_mate_eq_info_t));
#endif
}

void ab_mate_tws_key_info_sync(void)
{
#if AB_MATE_KEY_EN
    ab_mate_tws_cmd_sync_nbyte(TWS_INFO_KEY, (u8*)&ab_mate_app.remote_key, sizeof(ab_mate_key_info_t));
#endif
}

void ab_mate_tws_device_reset_sync(void)
{
    ab_mate_tws_cmd_sync_byte(TWS_INFO_DEVICE_RST, 0);
}

void ab_mate_tws_bt_name_sync(void)
{
#if AB_MATE_BT_NAME_EN
    u8 bt_name_len = strlen(xcfg_cb.bt_name);
    u8 offset = 0;

    tws_sync_data[offset++] = TWS_INFO_BT_NAME;
    tws_sync_data[offset++] = bt_name_len;
    memcpy(&tws_sync_data[offset],xcfg_cb.bt_name,bt_name_len);
    offset += bt_name_len;

    tws_sync_data_len = offset;
    tws_data_sync_do();
#endif
}

void ab_mate_tws_in_ear_en_sync(void)
{
#if AB_MATE_INERA_EN
    ab_mate_tws_cmd_sync_byte(TWS_INFO_IN_EAR, sys_cb.in_ear_en);
#endif
}

void ab_mate_tws_connect_sta_sync(void)
{
    ab_mate_tws_cmd_sync_byte(TWS_INFO_CONNECT_STA, ab_mate_app.con_sta);
}

void ab_mate_tws_link_clear_sync(void)
{
    ab_mate_tws_cmd_sync_byte(TWS_INFO_BT_LINK_CLEAR, 0);
}

void ab_mate_tws_led_info_sync(void)
{
#if AB_MATE_LED_EN
    ab_mate_tws_cmd_sync_byte(TWS_INFO_LED, sys_cb.led_scan_en);
#endif
}

void ab_mate_tws_anc_cur_level_sync(void)
{
#if AB_MATE_ANC_LEVEL
    ab_mate_tws_cmd_sync_byte(TWS_INFO_ANC_CUR_LEVEL, ab_mate_app.anc_cur_level);
#endif
}

void ab_mate_tws_tp_cur_level_sync(void)
{
#if AB_MATE_ANC_LEVEL
    ab_mate_tws_cmd_sync_byte(TWS_INFO_TP_CUR_LEVEL, ab_mate_app.tp_cur_level);
#endif
}

#if AB_MATE_V3D_AUDIO_EN
void ab_mate_tws_v3d_audio_sync(void)
{
    ab_mate_tws_cmd_sync_byte(TWS_INFO_V3D_AUDIO, ab_mate_app.v3d_audio_en);
}
#endif


void ab_mate_tws_info_all_sync(void)
{
    u8 offset = 0;

#if (CHARGE_BOX_TYPE == CBOX_SSW)
    if(sys_cb.loc_bat & BIT(7)){
        ab_mate_app.local_vbat = sys_cb.loc_bat;
    }else{
        ab_mate_app.local_vbat = bsp_get_bat_level();
    }
#else
    ab_mate_app.local_vbat = bsp_get_bat_level();
#endif

    tws_sync_data[offset++] = TWS_INFO_ALL;

#if AB_MATE_EQ_EN
    memcpy(&tws_sync_data[offset],&ab_mate_app.eq_info,sizeof(ab_mate_eq_info_t));
    offset += sizeof(ab_mate_eq_info_t);
#endif

    tws_sync_data[offset++] = ab_mate_app.local_vbat;

#if AB_MATE_ANC_EN
    tws_sync_data[offset++] = sys_cb.anc_user_mode;
#endif

#if AB_MATE_KEY_EN
    memcpy(&tws_sync_data[offset],&ab_mate_app.local_key,sizeof(ab_mate_key_info_t));
    offset += sizeof(ab_mate_key_info_t);
#endif

#if AB_MATE_LATENCY_EN
    tws_sync_data[offset++] = ab_mate_app.latency_mode;
#endif

#if AB_MATE_INERA_EN
    tws_sync_data[offset++] = sys_cb.in_ear_en;
#endif

#if AB_MATE_BT_NAME_EN
    tws_sync_data[offset++] = strlen(xcfg_cb.bt_name);
    memcpy(&tws_sync_data[offset],xcfg_cb.bt_name,strlen(xcfg_cb.bt_name));
    offset += strlen(xcfg_cb.bt_name);
#endif

    tws_sync_data[offset++] = ab_mate_app.con_sta;

#if AB_MATE_LED_EN
    tws_sync_data[offset++] = sys_cb.led_scan_en;
#endif

#if AB_MATE_V3D_AUDIO_EN
    tws_sync_data[offset++] = ab_mate_app.v3d_audio_en;
#endif

    tws_sync_data_len = offset;
    tws_data_sync_do();

    TRACE("ab_mate_tws_info_all_sync\n");
    TRACE_R(tws_sync_data,offset);
}

void ab_mate_tws_info_all_rsp(void)
{
    u8 offset = 0;

#if (CHARGE_BOX_TYPE == CBOX_SSW)
    if(sys_cb.loc_bat & BIT(7)){
        ab_mate_app.local_vbat = sys_cb.loc_bat;
    }else{
        ab_mate_app.local_vbat = bsp_get_bat_level();
    }
#else
    ab_mate_app.local_vbat = bsp_get_bat_level();
#endif

    tws_sync_data[offset++] = TWS_INFO_ALL_RSP;

    tws_sync_data[offset++] = ab_mate_app.local_vbat;

#if AB_MATE_KEY_EN
    memcpy(&tws_sync_data[offset],&ab_mate_app.local_key,sizeof(ab_mate_key_info_t));
    offset += sizeof(ab_mate_key_info_t);
#endif

    tws_sync_data[offset++] = ab_mate_app.con_sta;

    tws_sync_data_len = offset;
    tws_data_sync_do();

    TRACE("ab_mate_tws_info_all_rsp\n");
    TRACE_R(tws_sync_data,offset);
}

void ab_mate_tws_info_eq_proc(uint8_t *data_ptr, u16 size)
{
    memcpy(&ab_mate_app.eq_info, data_ptr, sizeof(ab_mate_eq_info_t));

    ab_mate_app.do_flag |= FLAG_EQ_SET;

    ab_mate_cm_write(&ab_mate_app.eq_info.mode, AB_MATE_CM_EQ_DATA, 1 + AB_MATE_EQ_BAND_CNT, 2);

#if AB_MATE_EQ_USE_DEVICE
    if(ab_mate_app.eq_info.mode >= AB_MATE_EQ_CUSTOM_INDEX){
        ab_mate_eq_custom_save();
    }
#endif
}

void ab_mate_tws_info_bat_proc(uint8_t *data_ptr, u16 size)
{
    u8 offset = 0;

    ab_mate_app.remote_vbat = data_ptr[offset++];

#if (CHARGE_BOX_TYPE == CBOX_SSW)
    sys_cb.rem_bat = ab_mate_app.remote_vbat;
    sys_cb.rem_house_bat = data_ptr[offset++];
#endif
}

void ab_mate_tws_info_key_proc(uint8_t *data_ptr, u16 size)
{
    memcpy(&ab_mate_app.local_key, data_ptr, sizeof(ab_mate_key_info_t));

    ab_mate_key_set_do();
}

void ab_mate_tws_info_all_proc(uint8_t *data_ptr, u16 size)
{
    u16 flag = 0;
    u16 offset = 0;

#if AB_MATE_EQ_EN
    if(memcmp(&ab_mate_app.eq_info, &data_ptr[offset], sizeof(ab_mate_eq_info_t))){
        memcpy(&ab_mate_app.eq_info, &data_ptr[offset], sizeof(ab_mate_eq_info_t));
        ab_mate_cm_write(&ab_mate_app.eq_info.mode, AB_MATE_CM_EQ_DATA, 1+AB_MATE_EQ_BAND_CNT, 2);
        flag |= BIT(0);
    }
    TRACE("eq_mode:%d\n",ab_mate_app.eq_info.mode);
    TRACE("eq_data:");
    TRACE_R(ab_mate_app.eq_info.gain,10);
    offset += sizeof(ab_mate_eq_info_t);
#endif

    ab_mate_app.remote_vbat = data_ptr[offset++];

#if (CHARGE_BOX_TYPE == CBOX_SSW)
    sys_cb.rem_bat = ab_mate_app.remote_vbat;
#endif
    TRACE("remote_bat:%d\n",ab_mate_app.remote_vbat);

#if AB_MATE_ANC_EN
    if(sys_cb.anc_user_mode != data_ptr[offset]){
        sys_cb.anc_user_mode = data_ptr[offset];
        flag |= BIT(1);
    }
    TRACE("anc_mode:%d\n",sys_cb.anc_user_mode);
    offset++;
#endif

#if AB_MATE_KEY_EN
    if(memcmp(&ab_mate_app.remote_key, &data_ptr[offset], sizeof(ab_mate_key_info_t))){
        memcpy(&ab_mate_app.remote_key, &data_ptr[offset], sizeof(ab_mate_key_info_t));
        ab_mate_cm_write(&ab_mate_app.remote_key, AB_MATE_CM_KEY_REMOTE, sizeof(ab_mate_key_info_t), 2);
    }
    TRACE("key_info:");
    TRACE_R(&ab_mate_app.remote_key,sizeof(ab_mate_key_info_t));
    offset += sizeof(ab_mate_key_info_t);
#endif

#if AB_MATE_LATENCY_EN
    if(ab_mate_app.latency_mode != data_ptr[offset]){
        ab_mate_app.latency_mode = data_ptr[offset];
    #if AB_MATE_LATENCY_USE_CM
        ab_mate_cm_write(&ab_mate_app.latency_mode, AB_MATE_CM_MODE, 1, 2);
    #endif
    }
    TRACE("latency_mode:%d\n",ab_mate_app.latency_mode);
    offset++;
#endif

#if AB_MATE_INERA_EN
    if(sys_cb.in_ear_en != data_ptr[offset]){
        sys_cb.in_ear_en = data_ptr[offset];
        ab_mate_cm_write(&sys_cb.in_ear_en, AB_MATE_CM_IN_EAR, 1, 2);
    }
    TRACE("in_ear_en:%d\n",sys_cb.in_ear_en);
    offset++;
#endif

#if AB_MATE_BT_NAME_EN
    u8 bt_name_len = data_ptr[offset++];
    u8 name_tag = AB_MATE_BT_NAME_TAG;
    u8 bt_name_local_len = strlen(xcfg_cb.bt_name);
    if((bt_name_local_len != bt_name_len) || memcmp(xcfg_cb.bt_name, &data_ptr[offset], bt_name_len)){
        memset(xcfg_cb.bt_name, 0, sizeof(xcfg_cb.bt_name));
        memcpy(xcfg_cb.bt_name, &data_ptr[offset], bt_name_len);
        ab_mate_cm_write(xcfg_cb.bt_name, AB_MATE_CM_BT_NAME, bt_name_len, 2);
        ab_mate_cm_write(&bt_name_len, AB_MATE_CM_BT_NAME_LEN, 1, 2);
        ab_mate_cm_write(&name_tag, AB_MATE_CM_BT_NAME_FLAG, 1, 2);
    }
    TRACE("bt_name:%s\n",xcfg_cb.bt_name);
    offset += bt_name_len;
#endif

    ab_mate_app.rem_con_sta = data_ptr[offset++];
    TRACE("rem_con_sta:%d\n",ab_mate_app.rem_con_sta);

#if AB_MATE_LED_EN
    if(sys_cb.led_scan_en != data_ptr[offset]){
        sys_cb.led_scan_en = data_ptr[offset];
        ab_mate_cm_write(&sys_cb.led_scan_en, AB_MATE_CM_LED_EN, 1, 2);
    }
    TRACE("led_scan_en:%d\n",sys_cb.led_scan_en);
    offset++;
#endif

#if AB_MATE_V3D_AUDIO_EN
    if(ab_mate_app.v3d_audio_en != data_ptr[offset]){
        ab_mate_app.v3d_audio_en = data_ptr[offset];
        ab_mate_app.do_flag |= FLAG_V3D_AUDIO_SET;
    }
    TRACE("v3d_audio_en:%d\n",ab_mate_app.v3d_audio_en);
    offset++;
#endif

    if(ab_mate_app.init_flag){
#if AB_MATE_EQ_EN
        if(flag & BIT(0)){
            if(!sco_is_connected()){
                ab_mate_eq_set_do();
            }
        }
#endif

#if AB_MATE_ANC_EN
        if(flag & BIT(1)){
            bsp_anc_set_mode(sys_cb.anc_user_mode);
        }
#endif
    }

    ab_mate_tws_info_all_rsp();
}

void ab_mate_tws_info_name_proc(uint8_t *data_ptr, u16 size)
{
    u8 bt_name_len = data_ptr[0];

    memset(xcfg_cb.bt_name, 0, sizeof(xcfg_cb.bt_name));
    memcpy(xcfg_cb.bt_name, &data_ptr[1], bt_name_len);

    ab_mate_app.do_flag |= FLAG_BT_NAME_SET;
}

#if AB_MATE_INERA_EN
void ab_mate_tws_info_in_ear_proc(uint8_t *data_ptr, u16 size)
{
    if(sys_cb.in_ear_en != data_ptr[0]){
        sys_cb.in_ear_en = data_ptr[0];
        ab_mate_cm_write(&sys_cb.in_ear_en, AB_MATE_CM_IN_EAR, 1, 2);
    }
}
#endif

void ab_mate_tws_info_all_rsp_proc(uint8_t *data_ptr, u16 size)
{
    u16 offset = 0;

    ab_mate_app.remote_vbat = data_ptr[offset++];
#if (CHARGE_BOX_TYPE == CBOX_SSW)
    sys_cb.rem_bat = ab_mate_app.remote_vbat;
#endif

#if AB_MATE_KEY_EN
    if(memcmp(&ab_mate_app.remote_key, &data_ptr[offset], sizeof(ab_mate_key_info_t))){
        memcpy(&ab_mate_app.remote_key, &data_ptr[offset], sizeof(ab_mate_key_info_t));
        ab_mate_cm_write(&ab_mate_app.remote_key, AB_MATE_CM_KEY_REMOTE, sizeof(ab_mate_key_info_t), 2);
    }
    offset += sizeof(ab_mate_key_info_t);
#endif

    ab_mate_app.rem_con_sta = data_ptr[offset++];
}

void ab_mate_tws_info_connect_sta_proc(uint8_t *data_ptr, u16 size)
{
    ab_mate_app.rem_con_sta = data_ptr[0];

    if(ab_mate_app.rem_con_sta == 0){
#if AB_MATE_OTA_EN
        ab_mate_ota_disconnect_callback();
#endif
    }
}

#if AB_MATE_LED_EN
void ab_mate_tws_info_led_proc(uint8_t *data_ptr, u16 size)
{
    if(sys_cb.led_scan_en != data_ptr[0]){
        sys_cb.led_scan_en = data_ptr[0];
        ab_mate_cm_write(&sys_cb.led_scan_en, AB_MATE_CM_LED_EN, 1, 2);
    }
}
#endif

#if AB_MATE_ANC_EN
void ab_mate_tws_info_anc_cur_level_proc(uint8_t *data_ptr, u16 size)
{
    if(ab_mate_app.anc_cur_level != data_ptr[0]){
        ab_mate_app.anc_cur_level = data_ptr[0];
        ab_mate_anc_level_set(&ab_mate_app.anc_cur_level, 1);

    }
}

void ab_mate_tws_info_tp_cur_level_proc(uint8_t *data_ptr, u16 size)
{
    if(ab_mate_app.tp_cur_level != data_ptr[0]){
        ab_mate_app.tp_cur_level = data_ptr[0];
        ab_mate_tp_level_set(&ab_mate_app.tp_cur_level, 1);
    }
}
#endif

#if AB_MATE_V3D_AUDIO_EN
void ab_mate_tws_info_3d_audio_proc(uint8_t *data_ptr, u16 size)
{
    if(ab_mate_app.v3d_audio_en != data_ptr[0]){
        ab_mate_app.v3d_audio_en = data_ptr[0];
        ab_mate_app.do_flag |= FLAG_V3D_AUDIO_SET;
    }
}
#endif

void ab_mate_tws_recv_proc(uint8_t *data_ptr, u16 size)
{
    u8 info_id = data_ptr[0];
    u8 *p_data = &data_ptr[1];
    u16 data_len = size - 1;

    ab_mate_app.wakeup = 1;

    if(TWS_INFO_OTA != info_id){
        TRACE("tws_recv_proc[%d]:",size);
        TRACE_R(data_ptr,size);
    }

    switch(info_id){
#if AB_MATE_EQ_EN
        case TWS_INFO_EQ:
            TRACE("TWS_INFO_EQ\n");
            ab_mate_tws_info_eq_proc(p_data, data_len);
            break;
#endif

        case TWS_INFO_VBAT:
            TRACE("TWS_INFO_VBAT:%d\n",data_ptr[1]);
            ab_mate_tws_info_bat_proc(p_data, data_len);
            break;

#if AB_MATE_KEY_EN
        case TWS_INFO_KEY:
            TRACE("TWS_INFO_KEY\n");
            ab_mate_tws_info_key_proc(p_data, data_len);
            break;
#endif

        case TWS_INFO_ALL:
            TRACE("TWS_INFO_ALL\n");
            ab_mate_tws_info_all_proc(p_data, data_len);
            break;

#if AB_MATE_OTA_EN
        case TWS_INFO_OTA:
            ab_mate_ota_tws_data_proc(&data_ptr[1],size - 1);
            break;
#endif

#if AB_MATE_BT_NAME_EN
        case TWS_INFO_BT_NAME:
            TRACE("TWS_INFO_BT_NAME\n");
            ab_mate_tws_info_name_proc(p_data, data_len);
            break;
#endif

        case TWS_INFO_DEVICE_RST:
            TRACE("TWS_INFO_DEVICE_RST\n");
            ab_mate_app.do_flag |= FLAG_DEVICE_RESET;
            break;

#if AB_MATE_INERA_EN
        case TWS_INFO_IN_EAR:
            TRACE("TWS_INFO_IN_EAR\n");
            ab_mate_tws_info_in_ear_proc(p_data, data_len);
            break;
#endif

        case TWS_INFO_ALL_RSP:
            TRACE("TWS_INFO_ALL_RSP\n");
            ab_mate_tws_info_all_rsp_proc(p_data, data_len);
            break;

        case TWS_INFO_CONNECT_STA:
            TRACE("TWS_INFO_CONNECT_STA\n");
            ab_mate_tws_info_connect_sta_proc(p_data, data_len);
            break;

        case TWS_INFO_BT_LINK_CLEAR:
            TRACE("TWS_INFO_BT_LINK_CLEAR\n");
            ab_mate_app.do_flag |= FLAG_BT_LINK_INFO_CLEAR;
            break;

#if AB_MATE_LED_EN
        case TWS_INFO_LED:
            TRACE("TWS_INFO_LED\n");
            ab_mate_tws_info_led_proc(p_data, data_len);
            break;
#endif

#if AB_MATE_ANC_EN
        case TWS_INFO_ANC_CUR_LEVEL:
            TRACE("TWS_INFO_ANC_CUR_LEVEL\n");
            ab_mate_tws_info_anc_cur_level_proc(p_data, data_len);
            break;

        case TWS_INFO_TP_CUR_LEVEL:
            TRACE("TWS_INFO_TP_CUR_LEVEL\n");
            ab_mate_tws_info_tp_cur_level_proc(p_data, data_len);
            break;
#endif

#if AB_MATE_V3D_AUDIO_EN
        case TWS_INFO_V3D_AUDIO:
            ab_mate_tws_info_3d_audio_proc(p_data, data_len);
            break;
#endif

        default:
            break;
    }
}


#endif
