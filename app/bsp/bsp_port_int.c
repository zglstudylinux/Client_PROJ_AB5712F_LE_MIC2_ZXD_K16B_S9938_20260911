#include "include.h"

/***************************************************
WAKEUP SOURCE   0   PA15
WAKEUP SOURCE   1   PB1
WAKEUP SOURCE   2   PB2
WAKEUP SOURCE   3   PE0
WAKEUP SOURCE   4   PE7
WAKEUP SOURCE   5   PB5

WAKEUP SOURCE   6   PORT INT SOURCE FALL
WAKEUP SOURCE   7   PORT INT SOURCE RISE
---> PORT INT SOURCE FALL/RISE = {
//src0
PB[15:0], PA[15:0],
//src1
PE[13:0],
};
***************************************************/

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#if 0
//外部中断测试示例
//参数io_num: IO编号, edge: 0下降沿, 1上升沿,  参数pupd  0:不开内部上下拉, 1:开内部上拉, 2:开内部下拉
void gpio_interrupt_config(u8 io_num, u8 edge, u8 pupd_sel)
{
    gpio_t gpio;
    if ((io_num == IO_NONE) || (io_num > IO_MAX_NUM)) {
        return;
    }
    gpio_cfg_init(&gpio, io_num);
    gpio.sfr[GPIOxDE] |= BIT(gpio.num);
    gpio.sfr[GPIOxDIR] |= BIT(gpio.num);
    if (pupd_sel) {
        gpio.sfr[GPIOxPU + pupd_sel - 1] |= BIT(gpio.num);       //开内部上拉或下拉
    }

    if (io_num < IO_PE0) {
        if (edge) {
            PORTINTEDG &= ~BIT(io_num - 5);
        } else {
            PORTINTEDG |= BIT(io_num - 5);
        }
        PORTINTEN |= BIT(io_num - 5);
    } else if (io_num == IO_PE0){
        if (edge) {
            PORTINTEDG &= ~BIT(io_num - 7);
        } else {
            PORTINTEDG |= BIT(io_num - 7);
        }
        PORTINTEN |= BIT(io_num - 7);
    }else {
        if (edge) {
            PORTINTEDG &= ~BIT(io_num - 10);
        } else {
            PORTINTEDG |= BIT(io_num - 10);
        }
        PORTINTEN |= BIT(io_num - 10);
    }

    WKPINMAP = (WKPINMAP & ~(0x03 << 12)) | (0x03 << 12);
    WKPINMAP = (WKPINMAP & ~(0x03 << 14)) | (0x03 << 14);
    WKUPCPND = 0xff << 16;

    if (edge) { //port_src7固定是上升沿
        WKUPEDG &= ~BIT(7);                                         //ports rising edge
        WKUPCON |= BIT(7);                                          //PORT_INT_RISE wakeup enable
        WKUPIE = (WKUPIE & ~(0x03 << 18)) | (0x03 << 18);           //上升沿滤波0: disable, 1: 4个cnt  2:10个cnt,  3:34个cnt
        WKUPIE |= BIT(7);
    } else {     //port_src6 固定是下降沿
        WKUPEDG |= BIT(6);                                          //ports falling edge
        WKUPCON |= BIT(6);                                          //PORT_INT_FALL wakeup enable
        WKUPIE = (WKUPIE & ~(0x03 << 16)) | (0x03 << 16);           //下降沿滤波0: disable, 1: 4个cnt  2:10个cnt,  3:34个cnt
        WKUPIE |= BIT(6);
    }
}

//检测对应IO是否产生边沿
//参数io_num: IO编号, edge: 0下降沿, 1上升沿
AT(.com_text.port)
bool port_edg_check(u8 io_num, u8 edg)
{
    u8 port_intsrc = 0;

    if (io_num < IO_PE0) {
        port_intsrc = io_num - 5;
    } else if (io_num == IO_PE0){
        port_intsrc = io_num - 7;
    }else {
        port_intsrc = io_num - 10;
    }

    if(edg) {
        if(PORTINTRISESRC & BIT(port_intsrc)) {
            return true;
        }
    } else {
        u32 fall_src = ~PORTINTFALLSRC;
        if(fall_src & BIT(port_intsrc)) {
            return true;
        }
    }

    return false;
}

AT(.com_rodata.gpio_int)
static u8 fall_edg[] = "fall P%X\n";
AT(.com_rodata.gpio_int)
static u8 rise_edg[] = "rise P%X\n";

AT(.com_text.port)
void port_isr(void)
{
    if (WKUPEDG & (BIT(6) << 16)) { //PORT INT FALL
        WKUPCPND = (BIT(6) << 16);  //CLEAR PENDING
        if(port_edg_check(IO_PB2, 0)) {
            printf((const char *)fall_edg, 0xB2);
        }

        if(port_edg_check(IO_PF0, 0)) {
            printf((const char *)fall_edg, 0xf0);
        }

    }

    if (WKUPEDG & (BIT(7) << 16)) { //PORT INT RISE
        WKUPCPND = (BIT(7) << 16);  //CLEAR PENDING
        if(port_edg_check(IO_PB2, 1)) {
            printf((const char *)rise_edg, 0xB2);
        }

        if(port_edg_check(IO_PF0, 1)) {
            printf((const char *)rise_edg, 0xF0);
        }
    }
}

void port_int_init(void)
{
    printf("%s\n", __func__);

    gpio_interrupt_config(IO_PB2, 0, 1);  //PB2下降沿触发中断, 配置内部上拉电阻
    gpio_interrupt_config(IO_PF0, 0, 1);  //PF0下降沿触发中断, 配置内部上拉电阻

//    gpio_interrupt_config(IO_PB2, 1, 2);  //PB2上升沿触发中断, 配置内部下拉电阻
//    gpio_interrupt_config(IO_PF0, 1, 2);  //PF0上升沿触发中断, 配置内部下拉电阻

    sys_irq_init(IRQ_PORT_VECTOR, 0, port_isr);
}
#endif
