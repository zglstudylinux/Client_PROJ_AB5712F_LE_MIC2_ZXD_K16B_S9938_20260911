#include "include.h"
#include "api.h"

#if BT_SPP_EN
#define SPP_TX_BUF_INX      4
#define SPP_TX_BUF_LEN      256     //max=512
#define SPP_POOL_SIZE       (SPP_TX_BUF_LEN + sizeof(struct txbuf_tag)) * SPP_TX_BUF_INX


AT(.ble_buf.stack.spp)
uint8_t spp_tx_pool[SPP_POOL_SIZE];

void spp_txpkt_init(void)
{
    txpkt_init(&spp_tx_ch0, spp_tx_pool, SPP_TX_BUF_INX, SPP_TX_BUF_LEN);
    spp_tx_ch0.send_kick = spp_send_kick;
}

u16 get_spp_mtu_size(void)
{
    return SPP_TX_BUF_LEN;
}

void spp_rx_callback(uint8_t *bd_addr, uint8_t ch, uint8_t *packet, uint16_t size)
{
    if (ch == SPP_SERVICE_CH1) {
#if GFPS_EN
        gfps_spp_recv_callback(packet, size);
#endif
    }
}

void spp_connect_callback(uint8_t *bd_addr, uint8_t ch)
{
    printf("--->spp_connect_callback ch:%d\n",ch);

    if (ch == SPP_SERVICE_CH1) {
#if GFPS_EN
        gfps_spp_connected_callback();
#endif
    }

}

void spp_disconnect_callback(uint8_t *bd_addr, uint8_t ch)
{
    printf("--->spp_disconnect_callback ch:%d\n",ch);
}

#endif

