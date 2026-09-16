#ifndef _BSP_I2S_H_
#define _BSP_I2S_H_

//I2S 基本配置类型
#define I2SCFG_TX        BIT(0)
#define I2SCFG_RX        BIT(1)
#define I2SCFG_SRC       BIT(2)
#define I2SCFG_DMA       BIT(3)
#define I2SCFG_MASTER    BIT(7)   //master or slave sel: 1 master, 0 slave
//I2S 组合类型
#define I2SCFG_DMATX    (I2SCFG_TX | I2SCFG_DMA)
#define I2SCFG_DMARX    (I2SCFG_RX | I2SCFG_DMA)
//I2S MASK BIT
#define I2SCFG_TXMASK   (I2SCFG_TX | I2SCFG_SRC | I2SCFG_DMA)
#define I2SCFG_RXMASK   (I2SCFG_RX | I2SCFG_DMA)
#define I2SCFG_TXRXMASK (I2SCFG_TX | I2SCFG_RX  | I2SCFG_SRC | I2SCFG_DMA)

#if (I2S_SAMPLES_OUT_SET == I2S_16000)
#define I2S_DMA_SAMPLES   40//16k
#else
#define I2S_DMA_SAMPLES   120
#endif
#define I2S_DMABUF_LEN    (I2S_DMA_SAMPLES* 2 * 2 *4)  //tx,rx double_buf, one_sample_4byte
#define IRQ_I2S_VECTOR                  27


#define I2S_IO_G1           0
#define I2S_IO_G2           1
#define I2S_IO_G3           2
#define I2S_IO_G4           3

#define I2S_LEFT_JUSTIFIED  0
#define I2S_NORMAL          1

#define I2S_16BIT           0
#define I2S_32BIT           1

#define I2S_MASTER          0
#define I2S_SLAVE           1

#define I2S_48000           0
#define I2S_16000           6

///公共
typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *params);

//I2S一共有可以配置出8种用法
typedef enum {
   I2S_MASTER_SRCTX         = (I2SCFG_MASTER |I2SCFG_TX |I2SCFG_SRC),
   I2S_MASTER_DMATX         = (I2SCFG_MASTER |I2SCFG_TX |I2SCFG_DMA),
   I2S_MASTER_DMATX_DMARX   = (I2SCFG_MASTER | I2SCFG_TX | I2SCFG_RX | I2SCFG_DMA),
   I2S_MASTER_SRCTX_DMARX   = (I2SCFG_MASTER | I2SCFG_TX | I2SCFG_SRC | I2SCFG_RX | I2SCFG_DMA),
   I2S_MASTER_DMARX         =  (I2SCFG_MASTER | I2SCFG_RX | I2SCFG_DMA),
   I2S_SLAVE_DMARX          = I2SCFG_DMARX,
   I2S_SLAVE_DMATX          = I2SCFG_DMATX,
   I2S_SLAVE_DMATX_DMARX    = (I2SCFG_TX | I2SCFG_RX | I2SCFG_DMA),
}TYPE_I2S_MODE;

typedef struct {
    u16 samples;
    void(*isr_rx_callback)(void *buf, u32 samples, u8 ch_mode, bool i2s_32bit);   //rx_dma收完一个DMA后起中断,可以从buf中取出接收到数据
    void(*isr_tx_callback)(void *buf, u32 samples, u8 ch_mode, bool i2s_32bit);   //tx_dma发送完一个DMA后起中断,要求向buf中填入数据,以备下一次发送
}i2s_dma_cfg_t;

typedef struct {
    u8 mode;
    u8 iomap        : 2;
    u8 bit_mode     : 1;
    u8 data_mode    : 1;
    u8 mclk_sel     : 2;
    u8 mclk_out_en  : 1;
    u8 dma_en       : 1;
    //DMA_CFG
    i2s_dma_cfg_t   dma_cfg;
}i2s_cfg_t;

//对外API接口函数
void i2s_init(i2s_cfg_t *cfg);
void i2s_exit(void);
void i2s_dma_stop(void);
void i2s_dma_start(void);

u32 *i2s_out_get_obuf(u8 buf_switch_flag);
u32 *i2s_in_get_obuf(u8 buf_switch_flag);
#endif
