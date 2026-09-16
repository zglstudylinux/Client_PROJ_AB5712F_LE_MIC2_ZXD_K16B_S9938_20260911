#ifndef _COMMAND_SYNC_H
#define _COMMAND_SYNC_H

enum{
    HUART_CONNECTED_STA = 0,
    HUART_MIC_MUTE,
};

enum{
    WIRELESS_NO_CONNECTED  = 0x00,
    WIRELESS_CONNECTED0    = 0x01,
    WIRELESS_CONNECTED1    = 0x02,
    WIRELESS_CONNECTED2    = 0x04,
    WIRELESS_CONNECTED3    = 0x08,
    WIRELESS_ALL_CONNECTED = 0x0F,
};

void huart_tx_user_cmd(u8 cmd, u16 data);
void huart_rx_user_cmd(void *cmd);
void huart_cmd_sync(void);
void huart_local_con_update(void);

//外部api
bool huart_role_audio_out(void);
void huart_cmd_send(void);

#endif
