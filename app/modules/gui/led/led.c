#include "include.h"
#include "led.h"

#if LED_DISP_EN

#define TRACE_EN                0
#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif // TRACE_EN

typedef struct {
    u32 tick_inst;
    u32 tick_off;
    u8 rled_sta;    //红灯 亮-灭控制
    u8 bled_sta;    //蓝灯 亮-灭控制
    u16 unit;       //50ms单位 (范围：1~255)
    u16 circle;

    bool lowbat;
} led_cb_t;

led_cb_t led_sync AT(.buf.led);
led_cb_t led_cb AT(.buf.led);
led_cb_t led_lb AT(.buf.led);       //low battery


void m_led2_port_init(gpio_t *g, bool ext_res);
void m_port_2led_scan(gpio_t *g);
void led_cfg_port_init(gpio_t *g);
void led_cfg_set_onoff(gpio_t *g, bool onoff);


AT(.com_text.led_disp)
void led_scan(void)
{
    led_cb_t *s;
    u32 tickn = TICKN_GET();

    if (!is_led_scan_enable()) {
        return;
    }

    s = (led_cb.lowbat)? &led_lb : &led_cb;
    if(s->unit == 0) {
		if (RLED_LOWBAT_FOLLOW_EN) {
			if(led_cb.lowbat){
				led_cb.lowbat = false;
			}
		}
        return;
    }

    u32 tickdiff = TICK_SUB(tickn, s->tick_inst);

    if(tickdiff < TICK_MAX_DELAY && s->unit != 0) {
        if(tickdiff % s->unit == 0) {
            u8 bcnt = tickdiff/s->unit;

//            GPIOBSET = BIT(4);
//            my_spi_putc(0x22);
//            my_spi_putc(bcnt);
//            GPIOBCLR = BIT(4);
            if (s->bled_sta & BIT(bcnt)) {
                bled_set_on();
            } else {
                bled_set_off();
            }
            if (s->rled_sta & BIT(bcnt)) {
                rled_set_on();
            } else {
                rled_set_off();
            }

            if(bcnt >= 7) {
                s->tick_inst = TICK_ADD(s->tick_inst, s->circle);
                if(s->circle == 0) {  //只亮一次
                    s->unit = 0;
                }
            }
        }
    }
}

AT(.text.led_disp.set) ALIGNED(256)
void led_set_sta_do(led_cfg_t *sta, led_cb_t *s, bool local)
{
    tws_time_t time;
    memset(&time, 0, sizeof(tws_time_t));
    TICK_TIME_GET(&time, 200);      //200ms后开始显示
    if(local) {
        time.tickoff = 0;
    }

    GLOBAL_INT_DISABLE();
    s->rled_sta = sta->redpat;
    s->bled_sta = sta->bluepat;
    s->unit     = sta->unit * 50;

    if (sta->cycle == 0xff) {
        s->circle = 0;
    } else {
        s->circle = s->unit * 8 + sta->cycle * 50;
    }

    s->tick_inst = TICK_ADD(time.tickn, time.tickoff);
    s->tick_off = time.tickoff;
    GLOBAL_INT_RESTORE();
}

AT(.text.led_disp.set)
bool led_sta_is_change(const led_cfg_t *sta, const led_cb_t *s)
{
    if(sta->cycle != 0xff
       && s->rled_sta == sta->redpat
       && s->bled_sta == sta->bluepat
       && s->unit == sta->unit * 50) {
        return false;
    }

    return true;
}

//获取LED当前设置的状态
AT(.com_text.led_disp)
u8 get_led_sta(u32 type)
{
    if (type) {
        return led_cb.rled_sta;
    } else {
        return led_cb.bled_sta;
    }
}

AT(.com_text.led_disp)
void led_cfg_set_on(gpio_t *g)
{
    led_cfg_set_onoff(g, 1);
}

AT(.com_text.led_disp)
void led_cfg_set_off(gpio_t *g)
{
    led_cfg_set_onoff(g, 0);
}

#if ADKEY_MUX_LED_EN
AT(.text.led_disp)
void led_mux_cfg_port_init(gpio_t *g)
{
    if ((g == NULL) || (g->sfr == NULL)) {
        return;
    }
    g->type = 2;    //ADKEY_MUX
    led_cfg_port_init(g);
}
#endif // ADKEY_MUX_LED_EN

#if LED_PWR_EN
//一个IO口推两个LED灯
void led2_port_init(gpio_t *g)
{
    m_led2_port_init(g, !xcfg_cb.port_2led_resless_en);
}

//每ms扫描一次
AT(.com_text.led_disp)
void port_2led_scan(void)
{
    if ((!sys_cb.port2led_en)) {
        return;
    }
    m_port_2led_scan(&bled_gpio);
}

//1个IO推两个灯，充电状态下不进sleep，否则充电灯异常。
AT(.sleep_text.led)
bool port_2led_is_sleep_en(void)
{
    if ((sys_cb.port2led_en) && (CHARGE_DC_IN())) {
        return false;
    }
    return true;
}
#endif // LED_PWR_EN

AT(.com_text.led_disp)
void bled_set_off(void)
{
    if (!sys_cb.charge_bled_flag) {       //充满状态优先控制
        LED_SET_OFF();
    }
}

AT(.com_text.led_disp)
void bled_set_on(void)
{
   if (!sys_cb.charge_bled_flag) {        //充满状态优先控制
        LED_SET_ON();
    }
}

AT(.com_text.led_disp)
void rled_set_off(void)
{
    if (!sys_cb.charge_sta) {             //充电状态优先控制
        LED_PWR_SET_OFF();
    }
}

AT(.com_text.led_disp)
void rled_set_on(void)
{
   if (!sys_cb.charge_sta) {               //充电状态优先控制
        LED_PWR_SET_ON();
    }
}

AT(.text.led_disp)
void led_init(void)
{
    LED_INIT();
    LED_PWR_INIT();
    memset(&led_sync, 0, sizeof(led_sync));
    memset(&led_cb, 0, sizeof(led_cb));
    memset(&led_lb, 0, sizeof(led_lb));
}

//蓝灯灭
AT(.text.led_disp)
void led_off(void)
{
    led_cb.bled_sta = 0x00;
    bled_set_off();
}

//蓝灯亮
AT(.text.led_disp)
void led_on(void)
{
    led_cb.bled_sta = 0xff;
    bled_set_on();
}

AT(.text.led_disp)
void rled_on(void)
{
    led_cb.rled_sta = 0xff;
    rled_set_on();
}

AT(.text.led_disp)
void rled_off(void)
{
    led_cb.rled_sta = 0x00;
    rled_set_off();
}

//拔出充电红灯灭
AT(.charge_text.det)
void charge_led_off(void)
{
    LED_PWR_SET_OFF();
}

//充电红灯亮
AT(.charge_text.det)
void charge_led_on(void)
{
    led_cb.bled_sta = 0x00;     //充电关蓝灯亮红灯

    LED_SET_OFF();
    LED_PWR_SET_ON();
}

//充满蓝灯常亮
AT(.charge_text.det)
void charge_bled_on(void)
{
    LED_SET_ON();
}

void led_set_sta_lowbat(const led_cfg_t *cfg)
{
    led_cfg_t sta;
    memcpy(&sta, cfg, sizeof(sta));

    if(led_sta_is_change(&sta, &led_lb)) {
        rled_set_off();
        bled_set_off();
        led_set_sta_do(&sta, &led_lb, true);
    }
}

void led_set_sta_normal(const led_cfg_t *cfg)
{
    led_cfg_t sta;
    memcpy(&sta, cfg, sizeof(sta));

    if(led_sta_is_change(&sta, &led_cb)) {
        if(!led_cb.lowbat) {
            rled_set_off();
            bled_set_off();
        }
        led_set_sta_do(&sta, &led_cb, true);

        TRACE("led_normal: circle=%x, uint=%x, tick=%x(%x), (%02x,%02x)\n", led_cb.circle, led_cb.unit, led_cb.tick_inst, 0, led_cb.bled_sta, led_cb.rled_sta);
    }
}

#if (LED_LOWBAT_EN || RLED_LOWBAT_FOLLOW_EN)
void led_lowbat_do(void);
void led_lowbat_follow_do(void);

AT(.text.led_disp.lowbat)
void led_lowbat(void)
{
    if (!led_cb.lowbat) {
        led_lowbat_do();
        led_cb.lowbat = true;
    }
}

AT(.text.led_disp.lowbat) ALIGNED(128)
void led_lowbat_recover(void)
{
    if(led_cb.lowbat){
        GLOBAL_INT_DISABLE();
        rled_set_off();
        bled_set_off();
        led_cb.lowbat = false;
        GLOBAL_INT_RESTORE();
    }
}

AT(.text.led_disp.lowbat)
void led_lowbat_follow_warning(void)
{
    if (!led_cb.lowbat) {
//        led_lowbat_follow();
        led_cb.lowbat = true;
    }
}
#endif


#endif
