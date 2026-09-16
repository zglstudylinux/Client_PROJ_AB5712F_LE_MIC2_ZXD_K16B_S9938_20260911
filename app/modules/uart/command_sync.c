#include"include.h"
#if  ADAPTER_HUART_COMMAND_EN

volatile uint8_t connected_sta = 0;
static bool dis_connected = false;
///设备连接/断开事件通知
///sta低4位用来记录连接状态,设备连上时，对应bit置1，所有设备连上时,sta = 0x0F
AT(.com_text.uart)
void huart_con_notice(u8 sta)
{
    //更新连接状态
    connected_sta = sta;
    if(sta == WIRELESS_NO_CONNECTED) {
        printf("HUART NO_CONNECTED\n");

    } else if(sta & WIRELESS_CONNECTED0) {
        printf("HUART CONNECTED0\n");

    } else if(sta & WIRELESS_CONNECTED1) {
        printf("HUART CONNECTED1\n");

    }else if(sta & WIRELESS_CONNECTED2) {
        printf("HUART CONNECTED2\n");

    }else if(sta & WIRELESS_CONNECTED3) {
        printf("HUART CONNECTED3\n");

    } else if(sta == WIRELESS_ALL_CONNECTED) {
        printf("HUART ALL_CONNECTED\n");
    }
    printf("HUART CONNECTED_STA: 0x%x\n", connected_sta);
}

AT(.text.uart)
void huart_local_con_update(void)
{
    if (!wireless_role_is_adapter() || huart_role_audio_out()) {
        return;
    }

    u8  sta = connected_sta;
    sta &= ~(WIRELESS_CONNECTED0 | WIRELESS_CONNECTED1);
    if(wireless_get_status() == 1){
        sta |= WIRELESS_CONNECTED0;
    } else if(wireless_get_status() == 2){
        sta |= WIRELESS_CONNECTED1;
    } else if(wireless_get_status() == 3){
        sta |= WIRELESS_CONNECTED0 | WIRELESS_CONNECTED1;
    }

    if(sta != connected_sta) {
#if ADAPTER_HUART_OUTPUT_EN
        if(sta == 0) {
            huart_audio_out_mute_set(1);
        } else {
            huart_audio_out_mute_set(0);
        }
#endif // ADAPTER_HUART_OUTPUT_EN
        huart_con_notice(sta);
    }
}

//作用于HUART RX端, HUART RX端收到命令后,进行相应的处理
AT(.text.uart)
void huart_rx_user_cmd(void *cmd_buf)
{
    uart_command_trans_str_t *huart_cmd = (uart_command_trans_str_t *)cmd_buf;
    u8 cmd = huart_cmd->cmd;
    u16 data = (huart_cmd->playload[1] << 8) | huart_cmd->playload[0];
    u8 sta = connected_sta;
    switch(cmd) {
        case HUART_CONNECTED_STA:
            sta &= ~(WIRELESS_CONNECTED2 | WIRELESS_CONNECTED3);
            if(data == 1){
                sta |= WIRELESS_CONNECTED2;
            } else if(data == 2){
                sta |= WIRELESS_CONNECTED3;
            } else if(data == 3){
                sta |= WIRELESS_CONNECTED2 | WIRELESS_CONNECTED3;
            }
            if(sta != connected_sta) {
                huart_con_notice(sta);
            }
            break;
        case HUART_MIC_MUTE:
//            printf("HUART_MIC_MUTE: %d\n", data);
            break;

        default:
            break;
    }
}



//--------------------------------------------------------------------------------------
//作用于HUART TX端, HUART TX端用该接口将一些数据命令同步到HUART RX端
AT(.text.uart)
void huart_tx_user_cmd(u8 cmd, u16 data)
{
    uart_command_tx_data(cmd, (u8 *)&data);

    //无线麦断开时,需要手动使用该接口发送命令数据
    if(!wireless_get_status()){
#if ADAPTER_HUART_OUTPUT_EN && ADAPTER_HUART_COMMAND_EN
        huart_cmd_send();
#endif // ADAPTER_HUART_OUTPUT_EN
        delay_us(850);
    }
}

//1秒同步一次数据,一次最多同步6条命令
AT(.text.uart)
void huart_cmd_sync(void)
{
    if(!huart_role_audio_out()) {
        return;
    }
    //无线麦断开时,暂停同步链接数据避免主RX复位
    if(wireless_get_status()) {
        huart_tx_user_cmd(HUART_CONNECTED_STA, wireless_cb.connected_sta);
        dis_connected = false;
    } else {
        if(!dis_connected) {
            huart_tx_user_cmd(HUART_CONNECTED_STA, wireless_cb.connected_sta);
            dis_connected = true;
        }
    }
}
#else
void huart_local_con_update(void){};
#endif

