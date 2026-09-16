#include "include.h"
#include "uart.h"

#if UART0_EN
//-----------------------------------------------------------------------------------------
//UART0 INIT
AT(.com_text.uart)
void uart0_putc(char ch)
{
    while (!(UART0CON & BIT(8)));
    UART0DATA = ch;
}

AT(.com_text.uart.send)
void uart0_send(u8 *data, uint len)
{
    for(uint i = 0; i < len; i++) {
       uart0_putc(data[i]);
    }
}

AT(.com_text.uart0.isr)
void uart0_isr_func(void)
{
    if ( UART0CON & BIT(9)) {  //RX Pending
        UART0CPND = BIT(9);
        u8 data = UART0DATA;
    }
}

//无线麦连接上或断开时会改变系统主频,UART0的波特率也会被修改,需要重新手动调用该接口初始化波特率
AT(.text.uart0.init)
void uart0_baud_rate_init(void)
{
    u32 baud_cfg = (((24000000 + (UART0_BAUD_RATE / 2)) / UART0_BAUD_RATE) - 1);
    UART0BAUD = (baud_cfg << 16) | baud_cfg;
}

AT(.text.uart0.init)
void uart0_init(void)
{
    printf("uart0_init\n");
    UART0_IO_INIT();
    FUNCMCON0 |= ((0xFL << 12) | (0xFL << 8));
    FUNCMCON0 |= (UART0_MAPPING << 12) | (UART0_MAPPING << 8);;

    CLKGAT0 |= BIT(6);
    CLKCON1 = (CLKCON1 & ~(3 << 19)) | (2 << 19);           //uart0_clk select xosc24m_clk

    UART0CON = 0;                                           //uart reset
    UART0CON |= BIT(5);                                     //uart_clk sel uart_inc
    uart0_baud_rate_init();
    UART0CON |= (BIT(7) | BIT(2) | BIT(0));                 // BIT(7):RX_EN, BIT(2):RXIE, BIT(0):UART_EN

    register_isr(IRQ_UART_VECTOR, uart0_isr_func);
    PICPR &= ~BIT(IRQ_UART_VECTOR);
    PICEN |= BIT(IRQ_UART_VECTOR);
}
#endif


//-----------------------------------------------------------------------------------------
//UART1 INIT

//底层有WEAK定义,当主频改变时,会通过该接口重新设置UART1的波特率
//这里uart1_init已经设置了波特率,不期望波特率被修改,故将接口设置为空
void update_uart1baud_in_sysclk_set(u32 baudrate)
{

}

AT(.com_text.uart1)
void uart1_putc(char ch)
{
    while (!(UART1CON & BIT(8)));
    UART1DATA = ch;
}

//UART1配置初始化,将PA7映射到UART1
AT(.text.uart1.init)
void uart1_init(void)
{
      GPIOAFEN |= BIT(7);
      GPIOADIR &= ~BIT(7);
      GPIOADE  |= BIT(7);

      FUNCMCON0 |= 0xFL << 24;
      FUNCMCON0 |= UART1_PA6PA7 << 24;

      CLKGAT0 |= BIT(7);                                      //uart1_clken
      CLKCON1 = (CLKCON1 & ~(3 << 21)) | (2 << 21);           //uart1_clk select xosc24m_clk

      UART1CON = 0;
      UART1CON |= BIT(5);
      u32 uart1_baud = (((24000000 + (1500000 / 2)) / 1500000) - 1);
      UART1BAUD = (uart1_baud << 16) | uart1_baud;
      UART1CON |= (BIT(0));                                  // BIT(7)RX en，BIT(2)RX INT en，BIT(0)uart en

      my_printf_init(uart1_putc);
}


