/* ***************************** */
//#define SEG_B1 //PB0 -> PB4
//#define SEG_C1 //PB1 -> PB4

//#define SEG_A1 //PB1 -> PB0
//#define SEG_B2 //PB0 -> PB1
//#define SEG_C2 //PB4 -> PB0
//#define SEG_D2 //PB4 -> PB1
//#define SEG_E2 //PA7 -> PB1
//#define SEG_F2 //PA7 -> PB0
//#define SEG_G2 //PA7 -> PB4

//#define SEG_A3 //PB2 -> PB1
//#define SEG_B3 //PB1 -> PB2
//#define SEG_C3 //PB2 -> PB0
//#define SEG_D3 //PB0 -> PB2
//#define SEG_E3 //PB2 -> PB4
//#define SEG_F3 //PB4 -> PB2
//#define SEG_G3 //PA7 -> PB2
/* ***************************** */
#ifndef _LEDSEG_5C7S_H
#define _LEDSEG_5C7S_H

#define SET_PB_HIGH(pin) (GPIOB |= pin)   // PBx输出高电平
#define SET_PB_LOW(pin)  (GPIOB &= ~pin)  // PBx输出低电平
#define SET_PA_HIGH(pin) (GPIOA |= pin)   // PA7输出高电平
#define SET_PA_LOW(pin)  (GPIOA &= ~pin)  // PA7输出低电平

// 引脚方向/电平操作宏（根据硬件寄存器定义，假设：DIR=1为输出，0为输入；输出时BIT=1为高，0为低）
#define SET_PB_DIR_OUT(pin) (GPIOBDIR &= ~(pin))  // PBx设为输出
#define SET_PB_DIR_IN(pin)  (GPIOBDIR |= (pin)) // PBx设为输入（高阻）
#define SET_PA_DIR_OUT(pin) (GPIOADIR &= ~(pin))  // PA7设为输出
#define SET_PA_DIR_IN(pin)  (GPIOADIR |= (pin)) // PA7设为输入（高阻）

// 引脚定义（5个GPIO）
#define PB0 BIT(0)
#define PB1 BIT(1)
#define PB2 BIT(2)
#define PB4 BIT(4)
#define PA7 BIT(7)

void ledseg_5c7s_init(void);
void ledseg_5c7s_clr(void);
void scan_single_seg(u8 seg_idx);
void ledseg_5c7s_scan(void);
void ledseg_5c7s_set_seg(u8 seg_idx, u8 state);
void ledseg_5c7s_update_dispbuf_do(u16 new_buf);
void ledseg_5c7s_off(void);

void ledseg_5c7s_test01(void);
void ledseg_5c7s_test02(void);
#endif //_LEDSEG_5C7S_H
