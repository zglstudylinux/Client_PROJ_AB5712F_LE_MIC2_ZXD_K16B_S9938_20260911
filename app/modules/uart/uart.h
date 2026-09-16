#ifndef __UART_H
#define __UART_H

#define IRQ_UART_VECTOR    14

typedef void (*isr_t)(void);
isr_t register_isr(int vector, isr_t isr);

//-----------------------------------------------------------------------------------------
#define UART0_PA6PA7       1    //G1 uart0 RX: PA6  TX: PA7
#define UART0_PB1PB2       2    //G2 uart0 RX: PB1  TX: PB2
#define UART0_PB4PB3       3    //G3 uart0 RX: PB4  TX: PB3
#define UART0_PE6PE7       4    //G4 uart0 RX: PE6  TX: PE7

#if (UART0_MAPPING == UART0_PA6PA7)
    #define UART0_IO_INIT() GPIOADE |= (BIT(6) | BIT(7)); GPIOAFEN |= (BIT(6) | BIT(7));\
                            GPIOADIR &= ~BIT(7); GPIOADIR |= BIT(6); GPIOAPU |= BIT(6);
#elif (UART0_MAPPING == UART0_PB1PB2)
    #define UART0_IO_INIT() GPIOBDE |= (BIT(1) | BIT(2)); GPIOBFEN |= (BIT(1) | BIT(2));\
                            GPIOBDIR &= ~BIT(2); GPIOBDIR |= BIT(1); GPIOBPU |= BIT(1);
#elif (UART0_MAPPING == UART0_PB4PB3)
    #define UART0_IO_INIT() GPIOBDE |= (BIT(3) | BIT(4)); GPIOBFEN |= (BIT(3) | BIT(4));\
                            GPIOBDIR &= ~BIT(3); GPIOBDIR |= BIT(4); GPIOBPU |= BIT(4);
#elif (UART0_MAPPING == UART0_PA6PA7)
    #define UART0_IO_INIT() GPIOADE |= (BIT(6) | BIT(7)); GPIOAFEN |= (BIT(6) | BIT(7));\
                            GPIOADIR &= ~BIT(7); GPIOADIR |= BIT(6); GPIOAPU |= BIT(6);
#endif

//-----------------------------------------------------------------------------------------
#define UART1_PA6PA7       1    //G1 uart1 RX: PA6  TX: PA7



void uart0_init(void);
void uart0_baud_rate_init(void);
void uart0_send(u8 *data, uint len);

void uart1_init(void);

#endif
