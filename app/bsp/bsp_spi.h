#ifndef __BSP_SPI_H_
#define __BSP_SPI_H_

#define             SPIEN               0       //SPI Enable bit 0:disable 1:enable
#define             SPISM               1       //Slave mode 0:master 1:slave
#define             SPI_BUSMODE_LOWBIT  2       //data bus width select low bit
#define             SPI_BUSMODE_HIGNBIT 3       //data bus width select high bit
#define             SPI_RXSEL           4       //TX:0 RX:1
#define             SPI_CLKIDS          5       //SPI clock state when idle 0:clock stay at 0 1:clk stay at 1
#define             SPI_SMPS            6       //SPI output edge select bit 0: falling edge 1:rising edge
#define             SPIIE               7       //SPI interrupt enable 0:disable 1:enable
#define             SPILF_EN            8       //SPI LFSR enable bit 0:disable 1:enable
#define             SPIMBEN             9       //SPI multiple bit bus enable bit
#define             SPIIOSS             10      //SPI sample data is at the same clock edge with output data 0:difference 1:same
#define             SPIPND              16      //SPI pending

#define             SPI_RX_BUF_LEN      256

#define SPI_CS_IO_INIT()                SPI_CS_GPIOFEN |= SPI_CS_BIT; SPI_CS_GPIODE |= SPI_CS_BIT; SPI_CS_GPIODIR &= ~SPI_CS_BIT;SPI_CS_GPIOSET = SPI_CS_BIT
#define SPI_CS_OUT_H()                  SPI_CS_GPIOSET = SPI_CS_BIT
#define SPI_CS_OUT_L()                  SPI_CS_GPIOCLR = SPI_CS_BIT

void bsp_spi_init(void);
void spi_byte_tx(u8 data);
void spi_dma_tx(u8 *buf, uint len);
void spi_dma_rx(u8 *buf, uint len);

//外部api
void set_spi1_baud(u32 baud);

#endif  // __BSP_H_
