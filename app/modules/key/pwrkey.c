#include "include.h"


AT(.com_text.bsp.key)
void pwrkey10s_counter_clr(void)
{
    if (POWKEY_10S_RESET) {
        RTCCON10 = BIT(10);                 //clear pwrkey10s pending and counter
    }
}

void pwrkey_off(void)
{
    GPIOBDE &= ~BIT(5);
    GPIOBDIR |= BIT(5);
    GPIOBPU &= ~BIT(5);
    GPIOBPD &= ~BIT(5);
    RTCCON13 &= ~(BIT(0) | BIT(4) | BIT(8) | BIT(12));   //wk pin0 disable
}

void pwrkey_init(void)
{
#if PWRKEY_EN
    adcch_io_pu10k_enable(ADCCH_WKO);
    pwr_usage_id = pwrkey_table[0].usage_id;
    RTCCON13 |= BIT(0) | BIT(4) | BIT(12);  //wk pin0 wakeup, input, pullup10k enable

    saradc_set_channel(BIT(ADCCH_WKO));
#else
    pwrkey_off();
#endif
}

AT(.com_text.port.key)
uint8_t pwrkey_get_val(void)
{
#if PWRKEY_EN || PWRKEY_2_HW_PWRON
    uint8_t num = 0;

    while (saradc_get_value8(ADCCH_WKO) > pwrkey_table[num].adc_val) {
        num++;
    }
//    printf("ADCCH_WKO = %d \n",saradc_get_value8(ADCCH_WKO));

    return pwrkey_table[num].usage_id;
#else
    return NO_KEY;
#endif
}

AT(.text.bsp.power)
bool pwrkey_get_status(void)
{
    u32 delay = 20;
    u32 pre_sta = 0;
    u32 sta_cnt = 0;
    while(delay--) {
        u32 tmp_sta = (RTCCON & BIT(19));
        if(pre_sta == tmp_sta) {
            sta_cnt++;
        } else {
            pre_sta = tmp_sta;
            sta_cnt = 0;
        }
        delay_ms(1);
        if(sta_cnt > 4) {
            break;
        }
    }

    return (pre_sta == 0)? true : false;
}
