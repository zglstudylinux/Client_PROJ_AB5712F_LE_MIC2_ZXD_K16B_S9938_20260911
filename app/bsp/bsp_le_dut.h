#ifndef _BSP_BSP_H
#define _BSP_DUT_H

#define PACKET_NO_ERR            0
#define PACKET_BUF_FULL          1
#define PACKET_RECV_ERR          2

#define PACKET_MIN               4

#define RX_ELEMENT_NUM           4
#define RX_ELEMENT_NUM_MASK      (RX_ELEMENT_NUM-1)
#define RX_BUF_SIZE              20


void le_hci_cmd_init(void);
void le_hci_cmd_exit(void);
void le_dut_huart_rx_done_cb(void);
void le_hci_cmd_process(u8 *data, u16 len);
void ble_hci_tx_callback_set(void *handle);
void ble_dut_pkt_buf_set(void *buf);
void ble_hci_cmd_check(void);
void ble_dut_cmd_do(void);
#endif
