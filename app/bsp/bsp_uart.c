#include "include.h"

#if UART1_EN

typedef struct {
    volatile u8 w_cnt;
    volatile u8 r_cnt;
    u8 resv0;
    u8 resv1;
    u8 *buf;
    u32 ticks;
} uart_cb_t;

static uart_cb_t uart_cb;
static u8 uart_rx_buf[64];
vh_packet_t vh_packet;


#if (CHARGE_BOX_INTF_SEL == INTF_UART1) || (TEST_INTF_SEL == INTF_UART1)
AT(.com_text.bsp.uart)
u8 bsp_uart_packet_parse(vh_packet_t *p, u8 data)
{
    u8 recv_valid  = 0, parse_done = 0;
    u8 cnt = p->cnt;
    if ((cnt == 0) && (data == 0x55)) {
        recv_valid = 1;
    } else if ((cnt == 1) && (data == 0xAA)) {
        p->header = 0xAA55;
        recv_valid = 1;
    } else if ((cnt == 2) && (data == VHOUSE_DISTINGUISH)) {
        p->distinguish = data;
        recv_valid = 1;
    } else if (cnt == 3) {
        p->cmd = data;
        recv_valid = 1;
    } else if (cnt == 4) {
        if (data <= VH_DATA_LEN) {
            p->length = data;
            recv_valid = 1;
        }
    } else if ((cnt > 4) && (cnt <= (p->length + 1 + 4))) {
        recv_valid = 1;
        if (cnt > p->length + 4) {
            p->checksum = data;
            parse_done = 1;                                     //匹配完整充电仓命令包
            recv_valid = 0;
        } else {
            p->buf[cnt - 5] = data;
        }
    }

    if (recv_valid) {
        p->crc = crc8_tbl[p->crc ^ data];
        p->cnt++;
    } else {
        if ((parse_done) && (p->crc != p->checksum)) {
            parse_done = 0;
        }
        p->cnt = 0;
        p->crc = 0;
    }
    return parse_done;
}

#endif


AT(.com_text.uart1.isr)
static void uart1_rx_done_cb(u8 data)
{
#if (((TEST_INTF_SEL == INTF_UART1) && IODM_TEST_EN))
    vh_packet_t *p = &vh_packet;
    u8 parse_done = bsp_uart_packet_parse(p,data);

#if IODM_TEST_EN && (TEST_INTF_SEL == INTF_UART1)
        iodm_packet_recv();
#endif
    }
#endif
#if (CHARGE_EN && (CHARGE_BOX_INTF_SEL == INTF_UART1))
    extern void func_charge_uart_recv_cb(u8 *data, u16 len);
    func_charge_uart_recv_cb(&data, 1);
#endif

#if QTEST_EN && (TEST_INTF_SEL == INTF_UART1)
    if(QTEST_IS_ENABLE()) {
        qtest_packet_uart1_recv(data);
    }
#endif
}

AT(.com_text.uart1.isr)
static void uart1_isr(void)
{
    u8 data;
    if(UART1CON & BIT(9)) {
        data = UART1DATA;
        UART1CPND = BIT(9);
        if(tick_check_expire(uart_cb.ticks,50)){
            uart_cb.w_cnt = uart_cb.r_cnt = 0;
        }
        uart_cb.ticks = tick_get();
        uart_cb.buf[uart_cb.w_cnt & 0x3f] = data;
        uart_cb.w_cnt++;
        uart1_rx_done_cb(data);
    }
}

AT(.com_text.uart)
u8 bsp_uart1_get(u8 *ch)
{
    if (uart_cb.r_cnt != uart_cb.w_cnt) {
        *ch = uart_cb.buf[uart_cb.r_cnt & 0x3f];
        uart_cb.r_cnt++;
        return 1;
    }
    return 0;
}

void bsp_uart1_init(u32 baudrate)
{
    memset(&uart_cb, 0, sizeof(uart_cb));
    memset(&vh_packet, 0, sizeof(vh_packet));
    uart_cb.buf = uart_rx_buf;
    u32 baud = 0;
    if (xcfg_cb.uart1_sel == UART1_TR_PA7) {
        if (UART0_PRINTF_SEL == PRINTF_PA7) {
            FUNCMCON0 = 0x0f << 8;
        }
    }else if(xcfg_cb.uart1_sel == UART1_TR_VUSB){
         if(!sys_cb.vusb_uart_flag){
            if (UART0_PRINTF_SEL == PRINTF_VUSB) {
                FUNCMCON0 = 0x0f << 8;
            }
            PWRCON0 |= BIT(30);                             //Enable VUSB GPIO
#if (CHARGE_BOX_INTF_SEL == INTF_UART1) || (QTEST_EN && (TEST_INTF_SEL == INTF_UART1))
            baudrate = 9600;
#endif
            xcfg_cb.chg_inbox_pwrdwn_en = 0;                  //入仓耳机关机
            sys_cb.vusb_uart_flag = 1;
         }else{
             return;
         }
    }
    baud = ((get_sysclk_nhz() + (baudrate / 2)) / baudrate) - 1;
    CLKGAT0 |= BIT(7);                              //enable uart1 clk
    UART1CON = 0;
    FUNCMCON0 = (0x7 << 28) | ((xcfg_cb.uart1_sel + 1) << 24);          //when RX = 0X7, TX RX共用
    UART1BAUD = (baud << 16) | baud;
    UART1CON = BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(0);   //RX EN, One line, fix baud, 2 Stop bits, Uart EN
    UART1CPND = BIT(8) | BIT(9);
    uart1_register_isr(uart1_isr);
}

#endif

void set_vusb_uart_flag(u8 flag)
{
   sys_cb.vusb_uart_flag = flag;
}

void bsp_vusb_uart_dis(void)
{
    PWRCON0 &= ~BIT(30);                            //disable VUSB GPIO
//    UART1CON = 0;
    CLKGAT0 &= ~BIT(7);                             //disable uart1 clk
    sys_cb.vusb_uart_flag = 0;
}
