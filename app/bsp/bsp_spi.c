#include "include.h"

//G1 SPI1CLK(PB3), SPI1DI(PE0), SPI1DO(PB4)
//G2 SPI1CLK(PA6), SPI1DI(PA5), SPI1DO(PA7)
//G3 SPI1CLK(PB1), SPI1DI(PB0), SPI1DO(PB2)
//G4 SPI1CLK(PE6), SPI1DI(PE5), SPI1DO(PE7)

#if SPI_HW_EN

u8 spi_rx_buf[SPI_RX_BUF_LEN];

static struct {
    uint16_t rxbuf_size;
    uint8_t *rxbuf;
} spi_cb;

//----------------------------------------------------------------------
//SPI TX/RX 1 BYTE
AT(.com_text.spi)
void spi_byte_tx(u8 data)
{
#if SPI_2WIRE_EN
    SPI1CON &= ~BIT(4);                       //set TX
#endif
    SPI1BUF = data;
    while(!(SPI1CON & BIT(16)));              //Wait pending
}

void spi_byte_rx(u8 *data)
{
#if SPI_2WIRE_EN
    SPI1CON |= BIT(4);                        //set RX
#endif
    SPI1BUF = 0xFF;
    while(!(SPI1CON & BIT(16)));
    *data = SPI1BUF;
}

//----------------------------------------------------------------------
//SPI DMA TX/RX
AT(.com_text.spi)
void spi_dma_tx(u8 *buf, uint len)
{
#if SPI_2WIRE_EN
    SPI1CON &= ~BIT(4);
#endif
    SPI1DMAADR = DMA_ADR(buf);
    SPI1DMACNT = len;
    //while(!(SPI1CON & BIT(16)));
}

void spi_dma_rx(u8 *buf, uint len)
{
#if SPI_2WIRE_EN
    SPI1CON |= BIT(4);
#endif
    SPI1DMAADR = DMA_ADR(buf);
    SPI1DMACNT = len;
    //while(!(SPI1CON & BIT(16)));
}

#if SPI_IRQ_EN
#if SPI_MASTER_EN
AT(.com_text.spi.isr)
static void spi_dma_tx_done_cb(void)
{
    //SPI DMA 已发送完SPI1DMACNT长度的数据
}

#else
AT(.com_text.spi.isr)
static void spi_dma_rx_done_cb(void)
{
    //SPI DMA 已收到spi_cb.rxbuf_size长度的数据
}

static void spi_dma_rx_kick(void)
{
    SPI1CON |= BIT(4);
    SPI1DMAADR = DMA_ADR(spi_cb.rxbuf);
    SPI1DMACNT = spi_cb.rxbuf_size;
}
#endif

AT(.com_text.spi.isr)
static void spi_isr_func(void)
{
    //DMA TX or RX finish
    if(SPI1CON & BIT(16)) {
        SPI1CPND = BIT(16);  //clear pending
    #if SPI_MASTER_EN
        spi_dma_tx_done_cb();
    #else
        spi_dma_rx_done_cb();
        spi_dma_rx_kick();
    #endif
    }
}
#endif // SPI_IRQ_EN

//-----------------------------------------------------
// SPI_3WIRE_DUAL_MODE_EN(2bit数据位宽传输模式)说明:
// 引脚功能:
// CLK
// SDI/SO0 (对应SPI1DO)
// SDI/SO1 (对应SPI1DI)
//
// 传输时序(以MSB方式传输,2数据线交替传输,例如传输1byte的时):
// CLK    : 0   1   2   3
// SDI/SO0: D6  D4  D2  D0
// SDI/SO1: D7  D5  D3  D1
//---------------------------------------------------

void bsp_spi_init(void)
{
    printf("%s\n", __func__);

    CLKGAT0 |= BIT(13);

    spi_cb.rxbuf      = spi_rx_buf;
    spi_cb.rxbuf_size = SPI_RX_BUF_LEN;

#if SPI_MASTER_EN
    SPI_MASTER_CLK_SDO_INIT();
    #if SPI_2WIRE_EN
      //2线模式
      SPI1CON = BIT(SPIEN) | BIT(SPI_BUSMODE_LOWBIT) | BIT(SPIIE);
    #else
      //3线模式
      SPI1CON =  BIT(SPIEN) | BIT(SPIIE);
      #if SPI_3WIRE_DUAL_MODE_EN
        SPI1CON |= BIT(SPI_BUSMODE_HIGNBIT) | BIT(SPIMBEN);
      #endif
      SPI_MASTER_SDI_INIT();
    #endif

    //SPI1BAUD = 24000000/SPI_BAUD_RATE - 1;
    set_spi1_baud(SPI_BAUD_RATE);
#else
    SPI_SALVE_CLK_SDO_INIT();
    SPI1CON =  BIT(SPIEN)  | BIT(SPI_BUSMODE_LOWBIT) | BIT(SPIIE) | BIT(SPI_RXSEL)| BIT(SPISM);

    SPI1DMAADR = DMA_ADR(spi_cb.rxbuf);
    SPI1DMACNT = SPI_RX_BUF_LEN;                  //kick start spi receive
#endif

    FUNCMCON1 = (0x0F<<4);
    FUNCMCON1 |= SPI_MAPPING;

#if SPI_CS_EN
    SPI_CS_IO_INIT() ;
#endif

#if SPI_IRQ_EN
    sys_irq_init(IRQ_SPI_VECTOR, 0, spi_isr_func);
#endif
}


#endif


