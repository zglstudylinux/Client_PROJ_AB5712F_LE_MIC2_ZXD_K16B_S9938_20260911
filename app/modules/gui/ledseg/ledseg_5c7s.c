#include "include.h"
#include "ledseg_5c7s.h"

#if (GUI_SELECT == GUI_LEDSEG_5C7S)
// 段定义结构体：存储每个段的高/低引脚及端口
typedef struct {
    u8 high_pin;       // 高电平引脚（如PB1）
    u8 low_pin;        // 低电平引脚（如PB0）
    u8 is_pa;          // 高电平引脚是否为PA7（0=PB，1=PA）
} seg_def_t;

// 16个段的定义列表
seg_def_t seg_list[16] = {
    {PB0, PB4, 0},   // SEG_B1: PB0高 → PB4低
    {PB1, PB4, 0},   // SEG_C1: PB1高 → PB4低
    {PB1, PB0, 0},   // SEG_A1: PB1高 → PB0低
    {PB0, PB1, 0},   // SEG_B2: PB0高 → PB1低
    {PB4, PB0, 0},   // SEG_C2: PB4高 → PB0低
    {PB4, PB1, 0},   // SEG_D2: PB4高 → PB1低
    {PA7, PB1, 1},   // SEG_E2: PA7高 → PB1低（高电平引脚是PA7）
    {PA7, PB0, 1},   // SEG_F2: PA7高 → PB0低
    {PA7, PB4, 1},   // SEG_G2: PA7高 → PB4低
    {PB2, PB1, 0},   // SEG_A3: PB2高 → PB1低
    {PB1, PB2, 0},   // SEG_B3: PB1高 → PB2低
    {PB2, PB0, 0},   // SEG_C3: PB2高 → PB0低
    {PB0, PB2, 0},   // SEG_D3: PB0高 → PB2低
    {PB2, PB4, 0},   // SEG_E3: PB2高 → PB4低
    {PB4, PB2, 0},   // SEG_F3: PB4高 → PB2低
    {PA7, PB2, 1}    // SEG_G3: PA7高 → PB2低
};

//显示缓冲区：16个bit对应16个段（1=点亮，0=熄灭）
u16 seg_buf = 0x0000;  // 例如：seg_buf |= (1<<0) 表示点亮SEG_B1

//数码管控制块
typedef struct {
    u8 scan_cnt;            // 当前扫描的段索引（0-15）
    volatile u8 disp_en;    // 显示使能标志
} ledseg_cb_t;

// 实例化控制块并指定到特殊内存段 (.buf.ledseg.cb)
ledseg_cb_t ledseg_cb AT(.buf.ledseg.cb);

// 数码管硬件初始化
void ledseg_5c7s_init(void)
{
    GPIOBFEN &= ~(PB0 | PB1 | PB2 | PB4);   //通用IO
    GPIOBDE |= (PB0 | PB1 | PB2 | PB4);;     //数字IO
    GPIOBDIR &= ~(PB0 | PB1 | PB2 | PB4);;   //输出模式

    GPIOAFEN &= ~PA7;
    GPIOADE |= PA7;
    GPIOADIR &= ~PA7;

    // 初始化控制块
    ledseg_cb.scan_cnt = 0;
    ledseg_cb.disp_en = 1;  // 使能显示
    ledseg_5c7s_clr();// 初始清空显示
}

// 清除数码管显示
void ledseg_5c7s_clr(void)
{
    seg_buf = 0x0000;
    GPIOBDIR |= (PB0 | PB1 | PB2 | PB4);    //设置为输入
    GPIOADIR |= PA7;
    GPIOB &= ~(PB0 | PB1 | PB2 | PB4);  // 输出低（避免残留电平）
    GPIOA &= ~PA7;
}

// 扫描单个段（内部函数）
void scan_single_seg(u8 seg_idx)
{
    if (seg_idx >= 16) return;

    const seg_def_t *seg = &seg_list[seg_idx];
    u8 high_pin = seg->high_pin;
    u8 low_pin = seg->low_pin;

    // 所有引脚设为输入，避免干扰
    GPIOBDIR |= (PB0 | PB1 | PB2 | PB4);
    GPIOADIR |= PA7;
    GPIOB &= ~(PB0 | PB1 | PB2 | PB4);  // 输出低（避免残留电平）
    GPIOA &= ~PA7;

    // 若缓冲区中该段需要点亮，则配置引脚并输出高低电平
    if (seg_buf & (1 << seg_idx)) {
        // 配置高电平引脚为输出并置高
        if (seg->is_pa) {  // 高电平引脚是PA7
            SET_PA_DIR_OUT(high_pin);
            SET_PA_HIGH(high_pin);
            printf("test\n");
        } else {  // 高电平引脚是PBx
            SET_PB_DIR_OUT(high_pin);
            SET_PB_HIGH(high_pin);
        }

        // 配置低电平引脚为输出并置低（PBx）
        SET_PB_DIR_OUT(low_pin);
        SET_PB_LOW(low_pin);
    }
}

// 数码管扫描函数
AT(.com_text.ledseg)
void ledseg_5c7s_scan(void)
{
    if(!ledseg_cb.disp_en) {
        ledseg_5c7s_clr();
        return;
    }
    // 扫描当前段
    scan_single_seg(ledseg_cb.scan_cnt);
    // 更新扫描索引（循环0-15）
    ledseg_cb.scan_cnt++;
    if(ledseg_cb.scan_cnt >= 16) {
        ledseg_cb.scan_cnt = 0;
    }
    delay_us(100);
}

// 设置单个段的状态（seg_idx：0-15；state：1=亮，0=灭）
void ledseg_5c7s_set_seg(u8 seg_idx, u8 state)
{
    if (seg_idx >= 16) return;

    ledseg_cb.disp_en = 0;  // 禁用扫描，避免冲突
    if (state) {
        seg_buf |= (1 << seg_idx);  // 点亮
    } else {
        seg_buf &= ~(1 << seg_idx); // 熄灭
    }
    ledseg_cb.disp_en = 1;  // 使能扫描
}

// 更新显示缓冲区（带互斥保护）
AT(.text.ledseg)  // 指定函数在.text.ledseg段
void ledseg_5c7s_update_dispbuf_do(u16 new_buf)
{
    ledseg_cb.disp_en = 0;
    seg_buf = new_buf;
    ledseg_cb.disp_en = 1;
}

// 关闭所有显示
void ledseg_5c7s_off(void)
{
    ledseg_cb.disp_en = 0;
    ledseg_5c7s_clr();
    // 强制所有引脚为输入
    GPIOBDIR |= (PB0 | PB1 | PB2 | PB4);
    GPIOADIR |= PA7;
}
#define LEDSEG_TEST 1
#if LEDSEG_TEST
const u16 digit_codes[12] = {
    0x7EFC,  // 0：点亮a、b、c、d、e、f段
    0x0C18,  // 1：点亮b、c段
    0xB76C,  // 2：点亮a、b、g、e、d段
    0x9F3C,  // 3：点亮a、b、g、c、d段
    0xCD98,  // 4：点亮f、g、b、c段
    0xDBB4,  // 5：点亮a、f、g、c、d段
    0xFBF4,  // 6：点亮a、f、g、c、d、e段
    0x0E1C,  // 7：点亮a、b、c段
    0xFFFC,  // 8：点亮所有段(a、b、c、d、e、f、g)
    0xDFBC,  // 9：点亮a、b、c、d、f、g段
    0x0001,  // led -- B1
    0x0002   // led -- c1
};

// 显示数字
void ledseg_5c7s_test01(void) {
    // 初始化数码管
    ledseg_5c7s_init();
    ledseg_cb.disp_en = 1;  // 使能显示

    ledseg_5c7s_update_dispbuf_do(digit_codes[9]);
    while(1) {

        ledseg_5c7s_scan(); //数码管段扫描测试
    }
}

// 依次点亮所有段
void ledseg_5c7s_test02(void) {
    ledseg_5c7s_init();
    ledseg_cb.disp_en = 1;  // 使能显示
    for(uint8_t i = 0; i < 16; i++) {
        ledseg_5c7s_set_seg(i, 1);
        ledseg_5c7s_scan();
        delay_ms(50);
    }

}
#endif
#endif
