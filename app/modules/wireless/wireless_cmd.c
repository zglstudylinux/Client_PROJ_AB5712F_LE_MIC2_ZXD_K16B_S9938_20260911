#include "include.h"
#include "wireless.h"

#define CMD_MAX_NB          4
#define CMD_MAX_SIZE        9

struct cmd_tag {
    u8 buf[CMD_MAX_NB][CMD_MAX_SIZE+1];
    u8 w_idx;
    u8 r_idx;
    u8 total;
};

#if WIRELESS_MIC_2TNR_EN
static struct cmd_tag txcmd[(WIRELESS_CON_2TNR_NB > WIRELESS_CON_LINK_NB) ? WIRELESS_CON_2TNR_NB : WIRELESS_CON_LINK_NB];
#else
static struct cmd_tag txcmd[WIRELESS_CON_LINK_NB];
#endif

void *cmd_buf_alloc(struct cmd_tag *cbuf)
{
    if(cbuf->total < CMD_MAX_NB) {
        u8 w_idx = cbuf->w_idx;
        cbuf->w_idx++;
        if(cbuf->w_idx >= CMD_MAX_NB) {
            cbuf->w_idx = 0;
        }
        return cbuf->buf[w_idx];
    }

    return NULL;
}

void cmd_buf_add(struct cmd_tag *cbuf)
{
    cbuf->total++;
}

void *cmd_buf_get(struct cmd_tag *cbuf)
{
    u8 r_idx = cbuf->r_idx;
    if(cbuf->total > 0) {
        cbuf->r_idx++;
        if(cbuf->r_idx >= CMD_MAX_NB) {
            cbuf->r_idx = 0;
        }
        cbuf->total--;
        return cbuf->buf[r_idx];
    }

    return NULL;
}

void *cmd_buf_peek(struct cmd_tag *cbuf)
{
    u8 r_idx = cbuf->r_idx;
    if(cbuf->total != 0) {
        return cbuf->buf[r_idx];
    }

    return NULL;
}

//--------------------------------------------------------------------------------------
void wireless_cmd_buf_init(void)
{
    memset(&txcmd, 0x00, sizeof(txcmd));
}

void wireless_rcv_cmd(u8 index, u8 *cmd, u8 len)
{
    printf("RX_CMD%d: ", index);
    print_r(cmd, len);
    wireless_rx_cmd(index, cmd, len);
}

bool wireless_send_cmd(u8 index, u8 *cmd, u8 len)
{
    u8 *buf;

    if(len > CMD_MAX_SIZE) {
        return false;
    }

    buf = cmd_buf_alloc(&txcmd[index]);
    if(buf == NULL) {
        return false;
    }

    printf("TX_CMD%d: ", index);
    print_r(cmd, len);

    buf[0] = len;
    memcpy(buf+1, cmd, len);

    cmd_buf_add(&txcmd[index]);

#if WIRELESS_MIC_BROADCAST_EN
    broadcast_source_user_cmd_tx_req(index);
#else
    if (!ble_con_user_cmd_tx_req(index)) {
        cmd_buf_get(&txcmd[index]);
        return false;
    }
#endif
    return true;
}

ALIGNED(64)
void wireless_cmd_reset(u8 index)
{
    struct cmd_tag *cbuf = &txcmd[index];

    GLOBAL_INT_DISABLE();
    cbuf->total = cbuf->r_idx = cbuf->w_idx = 0;
    GLOBAL_INT_RESTORE();
}

//--------------------------------------------------------------------------------------
//以下是库回调函数

//收到一条用户私有命令
void ble_con_user_cmd_rx_cb(u8 index, u8 *pdu)
{
    wireless_rcv_cmd(index, pdu+1, pdu[0]);
}

//获取要发送的用户私有命令
u8 *ble_con_user_cmd_get_tx_cb(u8 index)
{
    return cmd_buf_get(&txcmd[index]);
}

//用户私有命令发送成功
bool ble_con_user_cmd_tx_cfm_cb(u8 index)
{
    //返回值：true=还有数据发送，false=数据已发完
    return cmd_buf_peek(&txcmd[index]);
}
