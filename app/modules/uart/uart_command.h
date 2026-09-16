#ifndef _UART_COMMAND_H
#define _UART_COMMAND_H

#define UART_COMMAND_TX_HEADER      0x54
#define UART_COMMAND_RX_HEADER      0x54

#define UART_COMMAND_TBL_NUM        6
#define UART_COMMAND_TBL_MSK        (UART_COMMAND_TBL_NUM - 1)
#define UART_COMMAND_MAX_LEN        2

#define UART_CMMAND_SIZE            (3 + UART_COMMAND_MAX_LEN) //header + cmd + playload + cmd_check_sum

enum{
    RX_STATE_WAIT_HEADER    = 0,
    RX_START_WAIT_CMD,
    RX_START_WAIT_PLAYLOAD_LEN,
    RX_START_WAIT_PLAYLOAD,
    RX_START_WAIT_CHECK_SUM,

    RX_START_WAIT_FREE,
};

typedef struct {
    uint8_t header;
    uint8_t cmd;
    uint8_t playload[UART_COMMAND_MAX_LEN];
    uint8_t cmd_check_sum;
} uart_command_trans_str_t;

typedef struct {
    uart_command_trans_str_t wireless_uart_tx;
    uart_command_trans_str_t wireless_uart_rx;
    uint8_t rx_state;
    uint8_t rx_header_sync_flag;
    uart_command_trans_str_t uart_command_tx_tbl[UART_COMMAND_TBL_NUM];
    uint8_t tx_w_idx;
    uint8_t tx_r_idx;
    uint8_t tx_value_idx;
} uart_command_cfg_t;

void uart_command_rx_callback(u8 data);
void uart_command_rx_proc(void);
void uart_command_init(void);
void uart_command_tx_data(u8 cmd, u8 *data);
bool uart_command_tx_value_get(void);
uint8_t *uart_command_buf_get(void);
uint8_t uart_command_buf_len_get(void);

//---------------------------------------------------------------------------
//adapter主==>adapter副
#define HUART_MSG_MAX_SIZE  20

typedef enum {
    DELETE_LINK_INFO = 0x0A,
    OTHER,

} huart_msg;

void huart_msg_recv(u8 *ptr, u32 len, int ch_mode, void *param);
void huart_rx_msg_process(void);
void huart_msg_send(u16 msg);

#endif
