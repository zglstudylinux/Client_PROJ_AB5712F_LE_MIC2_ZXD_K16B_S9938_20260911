#include "include.h"
#include "bsp_le_dut.h"

///BQB_RF和FCC TEST通用的接口
#if LE_BQB_RF_EN || LE_FCC_TEST_EN

typedef struct {
    uint8_t buf[RX_ELEMENT_NUM][RX_BUF_SIZE];
    uint8_t len[RX_ELEMENT_NUM];
    uint8_t w_idx;
    uint8_t r_idx;
    uint8_t total_size;
    uint8_t status;
} hci_pkt_cb;

AT(.buf.le_dut.hci_cmd)
static hci_pkt_cb hci_cmd_pkt;
static volatile u16 le_dut_rx_len = 0;
ALIGNED(4)
u8 hci_rx_buffer[128] AT(.buf.le_dut.hci_buf);

//extern u32 __comm_test_vma, __comm_test_lma, __comm_test_size;

void le_dut_huart_init(void)
{
    huart_t huart0;
    memset(&hci_rx_buffer, 0x00, sizeof(hci_rx_buffer));
    memset(&huart0, 0x00, sizeof(huart0));
#if LE_BQB_RF_EN
    huart0.rx_port = HUART_TR_PB3;
    huart0.tx_port = HUART_TR_PB4;
//    huart0.rxbuf_loop = 1;
    huart0.rxisr_en = 1;
    huart0.txisr_en = 1;
    huart0.rxbuf      = hci_rx_buffer;
    huart0.rxbuf_size = 128;

    huart_init(&huart0, 9600);
#else
    huart0.rx_port = xcfg_cb.huart_sel;
    huart0.tx_port = xcfg_cb.huart_sel;
//    huart0.txisr_en = 0;
//    huart0.rxbuf_loop = 1;
    huart0.rxisr_en = 1;
    huart0.txisr_en = 1;
    huart0.rxbuf      = hci_rx_buffer;
    huart0.rxbuf_size = 128;

    huart_init(&huart0, 1500000);
#endif
}

AT(.text.le_dut.hci_proc)
void le_dut_huart_rx_len_set(u16 len)
{
    le_dut_rx_len = len;
}

AT(.text.le_dut.hci_proc)
u16 le_dut_huart_rx_len_get(void)
{
    return le_dut_rx_len;
}

AT(.text.le_dut.hci_proc)
void le_dut_huart_rx_done_cb(void)
{
    le_dut_huart_rx_len_set(huart_get_rxcnt());
    le_hci_cmd_process(hci_rx_buffer, le_dut_huart_rx_len_get());
    msg_enqueue(EVT_HCI_CMD);
}


///初步处理接收到的数据,并保存到pkt_buf,最后通过msg_enqueue的方式去执行命令
AT(.text.le_dut.hci_proc)
void le_hci_cmd_process(u8 *data, u16 len)
{
    if (len >= RX_BUF_SIZE || len < PACKET_MIN){
        hci_cmd_pkt.status   = PACKET_RECV_ERR ;

    } else if((hci_cmd_pkt.total_size >= RX_ELEMENT_NUM)){
        hci_cmd_pkt.status = PACKET_BUF_FULL;

    } else {
        memcpy(hci_cmd_pkt.buf[(hci_cmd_pkt.w_idx)&RX_ELEMENT_NUM_MASK], data, len);
        hci_cmd_pkt.len[(hci_cmd_pkt.w_idx)&RX_ELEMENT_NUM_MASK] = len;
        hci_cmd_pkt.status = PACKET_NO_ERR;
        (hci_cmd_pkt.w_idx)++;
        (hci_cmd_pkt.total_size)++;
    }

    msg_enqueue(EVT_HCI_CMD);
}

void ble_hci_cmd_check(void)
{
    ble_dut_cmd_do();
}

///将pkt_buf地址和tx函数给到蓝牙基带
AT(.text.le_init.dut)
void le_hci_cmd_init(void)
{
//    memcpy(&__comm_test_vma, &__comm_test_lma, (u32)&__comm_test_size);
//    printf("le_hci_cmd_init\n");
    memset(&hci_cmd_pkt, 0x00, sizeof(hci_pkt_cb));
    ble_dut_pkt_buf_set(&hci_cmd_pkt);
    ble_hci_tx_callback_set(huart_tx);

    le_dut_huart_init();
}

AT(.text.le_exit.dut)
void le_hci_cmd_exit(void)
{
}
#endif
