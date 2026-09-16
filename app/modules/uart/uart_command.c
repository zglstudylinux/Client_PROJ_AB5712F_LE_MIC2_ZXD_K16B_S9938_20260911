#include "include.h"

/*
 * 文件名称: wireless_uart_command.c
 * 功能描述: 串口通信交互协议相关代码，包含收发函数的定义及具体处理细节流程
 */
#if  ADAPTER_HUART_COMMAND_EN
static uart_command_cfg_t uart_command_cfg AT(.buf.uart_cb);
AT(.com_text.ains4)
const char huart_in_info2[] = "rx_state = %d\n";

AT(.com_text.uart_callback)
void uart_command_rx_callback(u8 data)
{
    switch (uart_command_cfg.rx_state) {
        case RX_STATE_WAIT_HEADER:
            if (data == UART_COMMAND_RX_HEADER) {
                uart_command_cfg.wireless_uart_rx.header = data;
                uart_command_cfg.rx_state = RX_START_WAIT_CMD;
            }
            break;
        case RX_START_WAIT_CMD:
            uart_command_cfg.wireless_uart_rx.cmd = data;
            uart_command_cfg.rx_state = RX_START_WAIT_PLAYLOAD_LEN;
            break;
        case RX_START_WAIT_PLAYLOAD_LEN:
            uart_command_cfg.wireless_uart_rx.playload[0] = data;
            uart_command_cfg.rx_state = RX_START_WAIT_PLAYLOAD;
            break;
        case RX_START_WAIT_PLAYLOAD:
            uart_command_cfg.wireless_uart_rx.playload[1] = data;
            uart_command_cfg.rx_state = RX_START_WAIT_CHECK_SUM;
            break;
        case RX_START_WAIT_CHECK_SUM:
            uart_command_cfg.wireless_uart_rx.cmd_check_sum = data;
            ///整包收完毕，等待处理后释放
            uart_command_cfg.rx_state = RX_START_WAIT_FREE;
            msg_enqueue(EVT_HUART_COMMAND_PROC);
            break;

        default:
            break;
    }
}

AT(.text.uart)
static bool uart_command_is_verify(uint8_t *rx_data, u8 rx_len)                         //接收数据 校验函数
{
    bool verify_en = false;
    u8 check_sum = 0;

    if (rx_len) {
        for (u8 i = 0; i < (rx_len - 1); i++) {
            check_sum += rx_data[i];
        }
        if (check_sum == rx_data[rx_len - 1]) {
            verify_en = true;
        }
    }

    return verify_en;
}

AT(.text.uart_rx.proc)
void uart_command_rx_proc(void)            //解析串口指令后处理流程
{
    if (uart_command_is_verify((u8 *)&uart_command_cfg.wireless_uart_rx, sizeof(uart_command_trans_str_t))) {
        printf("wireless_uart_rx_proc\n");
        print_r((u8 *)&uart_command_cfg.wireless_uart_rx, sizeof(uart_command_trans_str_t));
        huart_rx_user_cmd((void *)&uart_command_cfg.wireless_uart_rx);
    }
    if (uart_command_cfg.rx_state == RX_START_WAIT_FREE) {
        uart_command_cfg.rx_state = RX_STATE_WAIT_HEADER;
    }
}

//--------------------------------------------------------------------------------------------------------------------
///tx 2 byte数据
AT(.text.uart_tx.proc)
void uart_command_tx_data(u8 cmd, u8 *data)
{
    memset((u8 *)&uart_command_cfg.wireless_uart_tx, 0, sizeof(uart_command_trans_str_t));
    uart_command_cfg.wireless_uart_tx.header       = UART_COMMAND_TX_HEADER;
    uart_command_cfg.wireless_uart_tx.cmd          = cmd;
    uart_command_cfg.wireless_uart_tx.playload[0]  = data[0];
    uart_command_cfg.wireless_uart_tx.playload[1]  = data[1];
    uart_command_cfg.wireless_uart_tx.cmd_check_sum = (uart_command_cfg.wireless_uart_tx.header + uart_command_cfg.wireless_uart_tx.cmd + uart_command_cfg.wireless_uart_tx.playload[0] + uart_command_cfg.wireless_uart_tx.playload[1]) & 0xff;

//    printf("CMD: %x %x %x\n", cmd, data[0], data[1]);
//    print_r((u8 *)&uart_command_cfg.wireless_uart_tx, sizeof(uart_command_trans_str_t));

    if (uart_command_cfg.tx_value_idx < UART_COMMAND_TBL_NUM) {
        memcpy((u8 *)&uart_command_cfg.uart_command_tx_tbl[uart_command_cfg.tx_w_idx&UART_COMMAND_TBL_MSK], (u8 *)&uart_command_cfg.wireless_uart_tx, sizeof(uart_command_trans_str_t));
        uart_command_cfg.tx_w_idx++;
        uart_command_cfg.tx_value_idx++;
    } else {
        printf("uart_command_tx_data tbl is full\n");
    }
}

AT(.com_text.uart)
bool uart_command_tx_value_get(void)
{
    if (uart_command_cfg.tx_value_idx) {
        return true;
    }
    return false;
}

AT(.com_text.uart)
uint8_t *uart_command_buf_get(void)
{
    uint8_t *uart_command_buf = NULL;

    if (uart_command_cfg.tx_value_idx) {
        uart_command_buf = (u8 *)&uart_command_cfg.uart_command_tx_tbl[uart_command_cfg.tx_r_idx&UART_COMMAND_TBL_MSK];
        uart_command_cfg.tx_r_idx++;
        uart_command_cfg.tx_value_idx--;
    }

    return uart_command_buf;
}

AT(.com_text.uart)
uint8_t uart_command_buf_len_get(void)
{
    return sizeof(uart_command_trans_str_t);
}

AT(.text.uart.init)
void uart_command_init(void)
{

    memset((u8 *)&uart_command_cfg, 0, sizeof(uart_command_cfg));
    uart_command_cfg.rx_state = RX_STATE_WAIT_HEADER;

}

static uint8_t huart_msg_cache[20] AT(.buf.huart_in);

AT(.text.uart_com.huart)
void huart_msg_recv(u8 *ptr, u32 len, int ch_mode, void *param)
{
#if ADAPTER_HUART_MSG_EN
    print_r(ptr, len);
    if(len < HUART_MSG_MAX_SIZE && ptr[0] == 0x55 && ptr[1] == 0xaa) {
        memcpy(huart_msg_cache, ptr, HUART_MSG_MAX_SIZE);
        memset(ptr, 0, HUART_MSG_MAX_SIZE);
        msg_enqueue(EVT_HUART_COMMAND_PROC);
    }
#endif
}

AT(.text.uart_com.uart)
void huart_rx_msg_process(void)
{
//    printf("huart_rx_msg_process\n");

    u8 check = 0;
    u16 msg  = 0;

    for(u8 i = 0; i < 4; i++) {
        check += huart_msg_cache[i];
    }

    if(check != huart_msg_cache[4]) {
        printf("huart msg error\n");
        return;
    } else {
        msg = (huart_msg_cache[3] << 8) | huart_msg_cache[2];
        memset(huart_msg_cache, 0, HUART_MSG_MAX_SIZE);
    }

    switch (msg) {
    case DELETE_LINK_INFO:
        printf("DELETE_LINK_INFO\n");
//        ble_delete_link_info();           //断开连接,并删除配对信息
        break;
    default:
        break;
    }
}


//huart_audio_out端调用
AT(.text.uart)
void huart_msg_send(u16 msg)
{
#if ADAPTER_HUART_MSG_EN
    memset(huart_msg_cache, 0, HUART_MSG_MAX_SIZE);
    huart_msg_cache[0] = 0x55;
    huart_msg_cache[1] = 0xaa;
    huart_msg_cache[2] = msg & 0xFF;
    huart_msg_cache[3] = (msg >> 8) & 0xFF;
    huart_msg_cache[4] = 0;
    for(u8 i = 0; i < 4; i++) {
        huart_msg_cache[4] += huart_msg_cache[i];
    }

    printf("huart msg send:\n");
    print_r(huart_msg_cache, 5);
    huart_tx(huart_msg_cache, 5); //Send 5 byte
#endif
}

#endif

