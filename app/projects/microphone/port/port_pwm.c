#include "include.h"


#if ENERGY_LED_EN    //能量灯软件PWM输出.

#define  ENERGY_LED_NUM 4         //灯的个数
#define  ENERGY_PWM_MAX_DUTY  8   //一个灯分8个亮度.

#define  ENERGY_LED_INIT()     {GPIOBDE |= 0x0F; GPIOBDIR &= ~0x0F;}
#define  ENERGY_LED_0_ON()     GPIOBSET = BIT(0)
#define  ENERGY_LED_0_OFF()    GPIOBCLR = BIT(0)
#define  ENERGY_LED_1_ON()     GPIOBSET = BIT(1)
#define  ENERGY_LED_1_OFF()    GPIOBCLR = BIT(2)
#define  ENERGY_LED_2_ON()     GPIOBSET = BIT(2)
#define  ENERGY_LED_2_OFF()    GPIOBCLR = BIT(2)
#define  ENERGY_LED_3_ON()     GPIOASET = BIT(3)
#define  ENERGY_LED_3_OFF()    GPIOACLR = BIT(3)

u8 pwm_duty_buf[ENERGY_LED_NUM];

AT(.com_text.rgb.tbl)   //非线性量化表.
u8 const energy_qtz_tbl[4 * 8] = {
     1,14,18,22,26,30,35,40,
     41,43,46,48,51,53,56,60,
     61,63,66,68,71,73,76,80,
     81,83,85,88,91,93,96,100,
};

void energy_led_init(void)
{
    ENERGY_LED_INIT();
}

AT(.com_text.rgb)
void energy_led_level_calc(void)    //约5ms调用一次.
{
    u8 level,i;
    u16 energy;
    static u8 disp_level,last_level;
    static u8 time_5ms_cnt = 0;

    time_5ms_cnt++;
    if(time_5ms_cnt >= 4){   //20ms计算一次能量
        time_5ms_cnt  = 0;

        energy = dac_pcm_pow_calc();
        //printf("[%X_%d]",rgb_led_en,energy);
        if (FUNC_MUSIC == func_cb.sta) {   //各模式能量效果可能不一样.
            energy = energy/450;
        } else if (FUNC_FMRX == func_cb.sta) {
            energy = energy/350;
        } else{
            energy = energy/300;
        }
        //非线性量化表
        for (i = 0, last_level = 0; i < 4*8; i++) {
           if (energy < energy_qtz_tbl[i]) {
               break;
           }
           last_level++;
        }
    }
    //能量相同, 不用更新
    if(disp_level == last_level){
        return;
    }

    //能量减少时,慢慢下降
    if (disp_level > last_level) {
        disp_level--;
    } else {
        disp_level = last_level;
    }

    //能量转为占空比,存放到pwm_duty_buf中.
    level = disp_level;
    for (i=0; i<ENERGY_LED_NUM; i++){
        if (level >= ENERGY_PWM_MAX_DUTY) {
            pwm_duty_buf[i] = 0;
            level -= ENERGY_PWM_MAX_DUTY;
        } else {
            pwm_duty_buf[i] = (1<<level) - 1;
            pwm_duty_buf[i] = ~pwm_duty_buf[i];
            level = 0;
        }
    }
}

AT(.com_text.rgb)
void energy_led_scan(void)  //建议1ms扫描一次.
{
    static u8 scan_bit = 0;
    if (pwm_duty_buf[0] & BIT(scan_bit)){
        ENERGY_LED_0_OFF();
    } else {
        ENERGY_LED_0_ON();
    }

    if (pwm_duty_buf[1] & BIT(scan_bit)) {
        ENERGY_LED_1_OFF();
    } else {
        ENERGY_LED_1_ON();
    }

    if (pwm_duty_buf[2] & BIT(scan_bit)) {
        ENERGY_LED_2_OFF();
    } else {
        ENERGY_LED_2_ON();
    }

    if (pwm_duty_buf[3] & BIT(scan_bit)) {
        ENERGY_LED_3_OFF();
    } else {
        ENERGY_LED_3_ON();
    }

    if (++scan_bit >= ENERGY_PWM_MAX_DUTY){ //ENERGY_PWM_MAX_DUTY= 8
        scan_bit = 0;
    }
}
#endif // ENERGY_LED_EN


#if SPI_RGB_CTR_EN
//RGB:WS2812驱动
//波特率2M, 1个bit的时间是0.5us, 1个字节就是0.5*8 = 4us, 假设1个字节作为1个原码周期:
//0x80作为0码, 前1个bit为高电平0.5us, 后7个bit为低电平3.5us, 周期4us
//0xE0作为1码, 前3个bit为高电平1.5us, 后5个bit为低电平2.5us, 周期4us

#define RGB_NUM             12                  //RGB数量
#define RGB_DATA_LEN        24                  //1个RGB由24个原码组成(1个原码占1个字节)

typedef enum{                                   //RGB控制编号, 每个RGB可独立控制
   RGB_LED1 = 0,
   RGB_LED2,
   RGB_LED3,
   RGB_LED4,
   RGB_LED5,
   RGB_LED6,
   RGB_LED7,
   RGB_LED8,
   RGB_LED9,
   RGB_LED10,
   RGB_LED11,
   RGB_LED12,
   RGB_LED_NUM_MAX,
}rgb_led_index;

u8 rgb_val[RGB_NUM][RGB_DATA_LEN] AT(.buf.led); //控制12个RGB的数据BUF

//把颜色转成原码数据
AT(.com_text.led_disp)
void rgb_calc(u8 led_index, u8 red, u8  green, u8 blue)
{
    u32 rgb_led = (green << 16) | (red << 8) | blue;
    u8 *ptr = (u8 *)rgb_val[led_index];

    //按照RGB高位先发原则, 填充数据
    for(s8 i = RGB_DATA_LEN - 1; i >= 0; i--) {
        if(rgb_led&0x1) {
            ptr[i] = 0xE0;   //1码
        } else {
            ptr[i] = 0x80;   //0码
        }
        rgb_led = rgb_led >> 1;
    }
}

//调用此函数更新RGB数据
AT(.com_text.rgb)
void rgb_update(void)
{
    SPI1DMAADR = DMA_ADR(rgb_val);
    SPI1DMACNT = sizeof(rgb_val);

    //是否需要阻塞等待SPI传输完成, 传输周期4us*12*24 = 1152 us, 假如控制周期大于传输周期, 则不需要阻塞
//    while(!(SPI1CON & BIT(16))){  //wait send finish
//        WDT_CLR();
//    }
}

//硬件SPI, PA7作为SPIDO输出
void rgb_spi1_init(void)
{
    printf("%s\n", __func__);

    CLKGAT0 |= BIT(13);

    GPIOAFEN |=  BIT(7);
    GPIOADE  |= BIT(7);
    GPIOADIR &= ~BIT(7);

    SPI1CON = BIT(SPIEN) | BIT(SPI_BUSMODE_LOWBIT);
    set_spi1_baud(2000000);      //通过该接口设置波特率, 在更新主频时, 波特率会重新校准

    FUNCMCON1 = (0x0F<<4);      //Clear these bit
    FUNCMCON1 |= SPI1MAP_G2;
}

void spi_rgb_test(void)
{
    //初始化SPI
    rgb_spi1_init();

    //设置每个RGB的颜色
    rgb_calc(RGB_LED1, 255, 0, 0);
    rgb_calc(RGB_LED2, 255, 0, 0);
    rgb_calc(RGB_LED3, 255, 0, 0);
    rgb_calc(RGB_LED4, 255, 0, 0);
    rgb_calc(RGB_LED5, 255, 0, 0);
    rgb_calc(RGB_LED6, 255, 0, 0);
    rgb_calc(RGB_LED7, 255, 0, 0);
    rgb_calc(RGB_LED8, 255, 0, 0);
    rgb_calc(RGB_LED9, 255, 0, 0);
    rgb_calc(RGB_LED10, 255, 0, 0);
    rgb_calc(RGB_LED11, 255, 0, 0);
    rgb_calc(RGB_LED12, 255, 128, 64);

    //刷新数据
    rgb_update();
}
#endif