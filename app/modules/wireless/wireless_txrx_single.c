/*
 * 单包发射BUF管理，编码一帧当成一个数据包来发射，发射多次实现重传
 *
 *                    -->|------------|<--frame
 *                    -->|TX1TX1------|TX1TX1------|TX1TX1------|TX1TX1------|
 * LINK_NB=1, RETRY=1时, a0,          a1,          a2,          a3,
 * LINK_NB=1, RETRY=2时, a0,a0,       a1,a1,       a2,a2,       a3,a3,
 *
 *                    -->|------------|<--frame
 *                    -->|TX1TX1TX2TX2|TX1TX1TX2TX2|TX1TX1TX2TX2|TX1TX1TX2TX2|
 * LINK_NB=2, RETRY=1时, a0,b0,       a1,b1,       a2,b2,       a3,b3,
 * LINK_NB=2, RETRY=2时, a0,a0,b0,b0, a1,a1,b1,b1, a2,a2,b2,b2, a3,a3,b3,b3,
 ****************************************************************************************
 */

#include "include.h"
#include "wireless_txrx.h"

#if !WIRELESS_CON_COMB_BUF_EN

#define MAX_RETRY_NB            WIRELESS_MIC_RETRY_NB

typedef struct rx_item_tag {
    u8 *buf;
    u8 buf_ofs;
    u8 bfi;                                     //错包标志，bad frame indicate
    u8 chidx;                                   //信道编号
    s8 rssi;                                    //信号强度
    u8 chstatus;
} rx_item_t;

typedef struct rpk_ctl_tag {
    rx_item_t item[2];                          //2个item，按乒乓buffer管理
    u8 rx_tog;                                  //乒乓buffer接收
    u8 rd_tog;                                  //乒乓buffer读取

    u8 comb_nb;                                 //组合个数
    u8 frame_size;                              //RX音频帧大小
    u16 rxbuf_size;                             //RX数据包大小，rxbuf_size = frame_size * comb_nb
} rpk_ctl_t;

typedef struct tpk_ctl_tag {
    u8 *txbuf;                                  //TX数据包的发射buffer
    u8 *buf;                                    //TX数据包的缓存
    u8 frame_size;                              //TX音频帧大小
    u8 txbuf_size;                              //TX数据包大小
    u8 buf_cnt;                                 //缓存数据计数
    u8 first_pkt;
} tpk_ctl_t;


static struct {
#if WIRELESS_MIC_TX_EN
    rpk_ctl_t mic_rx[WIRELESS_CON_LINK_NB];
    u8 mic_rxbuf[WIRELESS_CON_LINK_NB][2*WIRELESS_MIC_RETRY_NB*WIRELESS_MIC_RX_BUFFER_SIZE];
#endif

#if WIRELESS_SPK_TX_EN
    tpk_ctl_t spk_tx;
    u8 spk_txbuf[WIRELESS_SPK_TX_BUFFER_SIZE];
#endif
} adapter_con;

static struct {
#if WIRELESS_MIC_TX_EN
    tpk_ctl_t mic_tx;
    u8 mic_txbuf[WIRELESS_MIC_TX_BUFFER_SIZE];
#endif

#if WIRELESS_SPK_TX_EN
    rpk_ctl_t spk_rx[WIRELESS_CON_LINK_NB];
    u8 spk_rxbuf[WIRELESS_CON_LINK_NB][WIRELESS_SPK_RX_BUFFER_SIZE*WIRELESS_SPK_RETRY_NB];
#endif
} device_con;

void mic_dec_buf_clr(u8 idx);
void mic_dec_kick_cb(u8 idx);
void mic_dec_dac_fifo_get(u8 idx);
void mic_dec_dac_dma_kick(uint tick_cnt);
void mic_enc_adc_dma_kick(uint tick_cnt);
void spk_dec_buf_clr(u8 idx);
void spk_dec_kick_cb(u8 idx);
void spk_dec_dac_dma_kick(uint tick_cnt);
void spk_enc_adc_dma_kick(uint tick_cnt);
void spk_enc_kick_cb(u8 idx);

//------------------------------------------------------------------------------------------
void rpk_init(rpk_ctl_t *rxpkt, u8 *buf, u8 retry_nb, u8 comb_nb, u8 frame_size, u16 rxbuf_size)
{
    rxpkt->comb_nb      = comb_nb;
    rxpkt->frame_size   = frame_size;
    rxpkt->rxbuf_size   = rxbuf_size;
    rxpkt->rx_tog       = 0;
    rxpkt->rd_tog       = 0;

    rxpkt->item[0].buf_ofs = 0;
    rxpkt->item[0].buf  = buf;
    rxpkt->item[0].bfi  = 1;
    buf += rxbuf_size;

    rxpkt->item[1].buf_ofs = 0;
    rxpkt->item[1].buf  = buf;
    rxpkt->item[1].bfi  = 1;
    buf += rxbuf_size;
}

//开始准备RX，会翻转rx_tog接收到新的item中
AT(.com_text.wireless.rx)
void rpk_start(rpk_ctl_t *rxpkt)
{
    u8 rx_tog = !rxpkt->rx_tog;

    rxpkt->rx_tog = rx_tog;

    rxpkt->item[rx_tog].buf_ofs = 0;
    rxpkt->item[rx_tog].chidx   = 37;
    rxpkt->item[rx_tog].bfi     = 1; //初值bfi=1
}

//RX到一个数据包，写入当前rx_tog的item
AT(.com_text.wireless.rx)
void rpk_set_rx(rpk_ctl_t *rxpkt, u8 *ptr, u16 len, bool bfi, u8 chidx, s8 rssi)
{
    u8 rx_tog = rxpkt->rx_tog;
    rx_item_t *item = &rxpkt->item[rx_tog];

    if(len/rxpkt->comb_nb < (rxpkt->frame_size-8)) {
        bfi = 1;
    }
    if(len > rxpkt->rxbuf_size) {
        len = rxpkt->rxbuf_size;
    }

    if(bfi || item->bfi == 0) {    //bfi=1错包，或者item已经收到过正确包时，直接丢弃
        return;
    }

    memcpy(item->buf, ptr, len);

    item->bfi       = 0;
    item->chidx     = chidx;
    item->rssi      = rssi;
    item->chstatus  = 0;
}

//当前item的retry_nb次传输都接收完成，获取rd_tog，准备开始解码
AT(.com_text.wireless.rx)
void rpk_done(rpk_ctl_t *rxpkt)
{
    u8 rx_tog = rxpkt->rx_tog;
    if(rx_tog == rxpkt->rd_tog) {  //RX调度冲突时rx_tog没有翻转，需要手动翻转一下，待完善
        rpk_start(rxpkt);
        rxpkt->rd_tog = !rx_tog;
    } else {
        rxpkt->rd_tog = rx_tog;
    }
}

//从当前rd_tog的item中取出一帧数据
AT(.com_text.wireless.rx)
u8 rpk_get_frame(rpk_ctl_t *rxpkt, u8 *buf, u8 rx_idx)
{
    u8 rd_tog = rxpkt->rd_tog;
    rx_item_t *item = &rxpkt->item[rd_tog];
    u8 *ptr = item->buf + item->buf_ofs;
    u8  bfi = item->bfi;

    if(item->buf_ofs <= rxpkt->rxbuf_size - rxpkt->frame_size) {
        item->buf_ofs += rxpkt->frame_size;
    }
    if(!bfi) {
        memcpy(buf, ptr, rxpkt->frame_size);
    }
    return bfi;
}

//设置当前rx_tog的item的解码状态
AT(.com_text.wireless.rx)
void rpk_set_chstatus(u8 idx, rpk_ctl_t *rxpkt, u8 chstatus, u8 rx_idx)
{
#if !WIRELESS_CON_CRC_EN
    u8 rd_tog = rxpkt->rd_tog;
    rx_item_t *item = &rxpkt->item[rd_tog][rx_idx];

    if(item->chstatus < chstatus) {             //统计每个帧的chstatus，值越大表示错误越多
        item->chstatus = chstatus;
    }
    if(item->buf_ofs >= rxpkt->rxbuf_size) {    //所有帧都取完时，设置信道质量
        wireless_channel_status(idx, item->chidx, item->rssi, item->bfi, item->chstatus);
    }
#endif
}

AT(.com_text.wireless.rx)
void rpk_end(rpk_ctl_t *rxpkt)
{
    rxpkt->item[0].buf_ofs = 0;
    rxpkt->item[0].bfi = 1;
    rxpkt->item[1].buf_ofs = 0;
    rxpkt->item[1].bfi = 1;
}

void tpk_init(tpk_ctl_t *txpkt, u8 *buf, u8 frame_size, u8 txbuf_size)
{
    txpkt->first_pkt    = true;
    txpkt->buf          = buf;
    txpkt->buf_cnt      = 0;
    txpkt->txbuf        = NULL;
    txpkt->frame_size   = frame_size;
    txpkt->txbuf_size   = txbuf_size;
}

AT(.com_text.wireless.tx)
void tpk_set_txbuf(tpk_ctl_t *txpkt, u8 *txbuf)
{
#if WIRELESS_MIC_COMB_NB > 1
    if (txpkt->buf_cnt)
#endif
    {
        if(txpkt->buf_cnt >= txpkt->txbuf_size) {
            //set_txbuf比put_frame来得晚，需要从buf中copy出来
            memcpy(txbuf, txpkt->buf, txpkt->txbuf_size);
            txpkt->buf_cnt = 0;
        } else {
            txpkt->txbuf = txbuf;
        }
    }
}

AT(.com_text.wireless.tx)
void tpk_put_frame(tpk_ctl_t *txpkt, u8 *ptr, u8 size)
{
    GLOBAL_INT_DISABLE();   //关中断memcpy数据太长时有风险
    u8 *txbuf   = txpkt->txbuf;
    u8 *buf     = txpkt->buf;
    u8 buf_cnt  = txpkt->buf_cnt;

    if(txbuf != NULL) {
        //put_frame比set_txbuf来得晚，直接保存到txbuf中
        if(buf_cnt > 0 && buf_cnt < txpkt->txbuf_size) {
            memcpy(txbuf, buf, buf_cnt);
            txbuf += buf_cnt;
        }
        memcpy(txbuf, ptr, size);

        txpkt->buf_cnt = 0;
        txpkt->txbuf = NULL;
    } else {
        //put_frame比set_txbuf来得早，先保存到buf中
        if(buf_cnt < txpkt->txbuf_size) {
            buf += buf_cnt;
            buf_cnt += size;
        } else {
            buf_cnt = size;
        }

        memcpy(buf, ptr, size);
        txpkt->buf_cnt = buf_cnt;
    }
    GLOBAL_INT_RESTORE();
}

AT(.com_text.wireless.tx)
void tpk_end(tpk_ctl_t *txpkt)
{
    txpkt->first_pkt    = true;
    txpkt->buf_cnt      = 0;
    txpkt->txbuf        = NULL;
}

#if WIRELESS_MIC_TX_EN
//------------------------------------------------------------------------------------------
// DEVICE --> ADAPTER
//------------------------------------------------------------------------------------------
//BLE RX回调函数，开始RX
AT(.com_text.wireless.rx)
void wireless_d2a_rxpkt_start_cb(u8 idx)
{
    rpk_start(&adapter_con.mic_rx[idx]);
}

//BLE RX回调函数，RX完一个数据包，一个TX_INTERVAL周期内会RX到RETRY_NB个数据包
AT(.com_text.wireless.rx)
void wireless_d2a_set_rxpkt_cb(u8 idx, u8 *ptr, u16 len, bool bfi, u8 chidx, s8 rssi)
{
    rpk_set_rx(&adapter_con.mic_rx[idx], ptr, len, bfi, chidx, rssi);

#if WIRELESS_CON_CRC_EN
    //有CRC校验或重传时，通过bfi反馈信道质量，用于跳频
    wireless_channel_status(idx, chidx, rssi, bfi, 0);
#endif
}

//从rxbuf获取一个数据帧
AT(.com_text.wireless.rx)
u8 wireless_d2a_get_rx_frame(u8 idx, u8 *buf, uint size)
{
    //从RX数据包里面取出一帧数据，进行解码
    return rpk_get_frame(&adapter_con.mic_rx[idx], buf, 0);
}

AT(.com_text.wireless.rx)
void wireless_d2a_set_ch_status(u8 idx, u8 chstatus)
{
    rpk_set_chstatus(idx, &adapter_con.mic_rx[idx], chstatus, 0);
}

//BLE RX完成，开始触发解码
AT(.com_text.wireless.rx)
void wireless_d2a_set_rxdec_cb(u8 idx, bool rxdone)
{
    if(rxdone) {
        rpk_done(&adapter_con.mic_rx[idx]);
    }

    mic_dec_kick_cb(idx);      //dec流程时间太长，放到线程处理，避免堵住BLE调度
//    mic_dec_prco_cb(idx);
}

#if 0
//RX解码完，启动DAC_DMA
AT(.com_text.wireless.rx)
void wireless_d2a_dac_dma_cb(uint tick_cnt)
{
    mic_dec_dac_dma_kick(tick_cnt);
}
#else
//RX解码前，先获取dac_fifo
AT(.com_text.wireless.rx)
bool wireless_d2a_dac_fifo_cb(u8 idx)
{
    mic_dec_dac_fifo_get(idx);
    return true;
}
#endif

//------------------------------------------------------------------------------------------
//TX前启动ADC采集
AT(.com_text.wireless.tx)
void wireless_d2a_adc_dma_cb(uint tick_cnt)
{
    mic_enc_adc_dma_kick(tick_cnt);
}

//BLE TX回调函数，每个TX周期前回调一次设置当前TX_BUF（实际上用同一个TX_BUF）
AT(.com_text.wireless.tx)
void wireless_d2a_set_txbuf_cb(u8 *ptr, u16 len)
{
    tpk_set_txbuf(&device_con.mic_tx, ptr);
}

//保存一个数据帧到txbuf
AT(.com_text.wireless.tx)
void wireless_d2a_put_tx_frame(u8 *ptr, u16 size)
{
    tpk_put_frame(&device_con.mic_tx, ptr, size);
}

//连接断开时，清除TX/RX状态标记
AT(.com_text.wireless.rx)
void wireless_d2a_end_ind_cb(u8 idx)
{
#if DEVICE_INTERPHONE_EN
    rpk_end(&adapter_con.mic_rx[idx]);
    mic_dec_buf_clr(idx);
    tpk_end(&device_con.mic_tx);
#else
    if(wireless_role_is_adapter()) {
        rpk_end(&adapter_con.mic_rx[idx]);
        mic_dec_buf_clr(idx);
    } else {
//        tpk_end(&device_con.mic_tx);
    }
#endif
}

#endif // WIRELESS_MIC_TX_EN

#if WIRELESS_SPK_TX_EN || DEVICE_INTERPHONE_EN
//------------------------------------------------------------------------------------------
// ADAPTER -> DEVICE
//------------------------------------------------------------------------------------------

//BLE RX回调函数，开始RX
AT(.com_text.ble.rx)
void wireless_a2d_rxpkt_start_cb(u8 idx)
{
#if DEVICE_INTERPHONE_EN
    wireless_d2a_rxpkt_start_cb(idx);
#else
    rpk_start(&device_con.spk_rx[0]);
#endif
}

//BLE RX回调函数，RX完一个数据包，一个TX_INTERVAL周期内会RX到RETRY_NB个数据包
AT(.com_text.ble.rx)
void wireless_a2d_set_rxpkt_cb(u8 idx, u8 *ptr, u16 len, bool bfi, u8 chidx, s8 rssi)
{
#if DEVICE_INTERPHONE_EN
    wireless_d2a_set_rxpkt_cb(idx, ptr, len, bfi, chidx, rssi);
#else
    rpk_set_rx(&device_con.spk_rx[0], ptr, len, bfi, chidx, rssi);
#endif

#if WIRELESS_CON_CRC_EN
    //有CRC校验或重传时，通过bfi反馈信道质量，用于跳频
    wireless_channel_status(idx, chidx, rssi, bfi, 0);
#endif
}

//从rxbuf获取一个数据帧
AT(.com_text.ble.rx)
u8 wireless_a2d_get_rx_frame(u8 *buf, uint size)
{
    //从RX数据包里面取出一帧数据，进行解码
#if DEVICE_INTERPHONE_EN
    return 0;
#else
    return rpk_get_frame(&device_con.spk_rx[0], buf, 0);
#endif
}

//BLE RX完成，开始触发解码
AT(.com_text.spk_dec)
void wireless_a2d_set_rxdec_cb(u8 idx, bool rxdone)
{
#if DEVICE_INTERPHONE_EN
    wireless_d2a_set_rxdec_cb(idx, rxdone);
#else
    if(rxdone) {
        rpk_done(&device_con.spk_rx[0]);
    }

    spk_dec_kick_cb(idx);      //dec流程时间太长，放到线程处理，避免堵住BLE调度
//    spk_dec_prco_cb(idx);
#endif
}

//RX解码完，启动DAC_DMA
AT(.com_text.spk_dec)
void wireless_a2d_dac_dma_cb(uint tick_cnt)
{
#if WIRELESS_SPK_TX_EN
    spk_dec_dac_dma_kick(tick_cnt);
#endif
}

//------------------------------------------------------------------------------------------
//TX前启动ADC采集
AT(.com_text.mic_enc)
void wireless_a2d_adc_dma_cb(uint tick_cnt)
{
#if DEVICE_INTERPHONE_EN
    wireless_d2a_adc_dma_cb(tick_cnt);
#else
#if ADAPTER_AUX_TX_EN
    //ADC的帧长与WIRELESS一致，触发dma后通过callback调用enc
    spk_enc_adc_dma_kick(tick_cnt);
#endif
#endif
}

//TX前启动USB_SPK编码
AT(.com_text.mic_enc)
void wireless_a2d_kick_txenc_cb(u8 idx)
{
#if ADAPTER_USB_SPK_TX_EN
    //USB的帧长与WIRELESS不一致，不能直接通过callback调用enc，需要主动触发
    spk_enc_kick_cb(0);
#endif
}

//BLE TX回调函数，每个TX周期前回调一次设置当前TX_BUF（实际上用同一个TX_BUF）
AT(.com_text.ble.tx)
void wireless_a2d_set_txbuf_cb(u8 idx, u8 *ptr, u16 len)
{
#if DEVICE_INTERPHONE_EN
    wireless_d2a_set_txbuf_cb(ptr, len);
#else
    tpk_set_txbuf(&adapter_con.spk_tx, ptr);
#endif
}

//保存一个数据帧到txbuf
AT(.com_text.ble.tx)
void wireless_a2d_put_tx_frame(u8 idx, u8 *ptr, u16 size)
{
#if DEVICE_INTERPHONE_EN
    tpk_put_frame(&device_con.mic_tx, ptr, size);
#else
    tpk_put_frame(&adapter_con.spk_tx, ptr, size);
#endif
}

//连接断开时，清除TX/RX状态标记
AT(.com_text.ble.rx)
void wireless_a2d_end_ind_cb(u8 idx)
{
#if DEVICE_INTERPHONE_EN
    wireless_d2a_end_ind_cb(idx);
#else
    if(wireless_role_is_adapter()) {
        tpk_end(&adapter_con.spk_tx);
    } else {
        rpk_end(&device_con.spk_rx[0]);
        spk_dec_buf_clr(0);
    }
#endif
}
#endif // WIRELESS_SPK_TX_EN

//------------------------------------------------------------------------------------------
void wireless_con_adapter_init(void)
{
#if WIRELESS_MIC_TX_EN
    for(uint i=0; i<WIRELESS_CON_LINK_NB; i++) {
        rpk_init(&adapter_con.mic_rx[i], adapter_con.mic_rxbuf[i], WIRELESS_MIC_RETRY_NB, WIRELESS_MIC_COMB_NB, WIRELESS_MIC_FRAME_SIZE, WIRELESS_MIC_RX_BUFFER_SIZE);
    }
#endif

#if WIRELESS_SPK_TX_EN && !DEVICE_STEREO_EN
    tpk_init(&adapter_con.spk_tx, adapter_con.spk_txbuf, WIRELESS_SPK_FRAME_SIZE, WIRELESS_SPK_TX_BUFFER_SIZE);
#endif
}

void wireless_con_device_init(void)
{
#if WIRELESS_MIC_TX_EN
    tpk_init(&device_con.mic_tx, device_con.mic_txbuf, WIRELESS_MIC_FRAME_SIZE, WIRELESS_MIC_TX_BUFFER_SIZE);
#endif

#if WIRELESS_SPK_TX_EN && !DEVICE_STEREO_EN
    for(uint i=0; i<WIRELESS_CON_LINK_NB; i++) {
        rpk_init(&device_con.spk_rx[i], device_con.spk_rxbuf[i], WIRELESS_SPK_RETRY_NB, WIRELESS_SPK_COMB_NB, WIRELESS_SPK_FRAME_SIZE, WIRELESS_SPK_RX_BUFFER_SIZE);
    }
#endif
}

#endif // !WIRELESS_CON_COMB_BUF_EN
