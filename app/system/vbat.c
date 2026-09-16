#include "include.h"

#if VBAT_DETECT_EN

#define VBAT_CACL_VOLTAGE()     (u32)((VBAT_VALUE() * VBAT2_COEF / adc_cb.vbg) * VBG_VOLTAGE / 10000)

AT(.com_rodata.bb)
const u16 tbl_bat[5] = {2800,3100,3300,3700,4000}; //内部电量检测(2.9V,3.1V,3.4V,3.7V,4.15V)

//AT(.com_rodata.bat)
//const char bat_str[] = "VBAT: %d.%03dV\n";
AT(.com_text.port.vbat)
uint16_t vbat_get_voltage(void)
{
#if DISP_BAT_EN
    static u16 bat_buf = 0;
    static u16 bat_cnt = 0;
    u8 level = 0;
#endif
    static u16 vbat_bak = 0;

    u32 vbat = VBAT_CACL_VOLTAGE();

    //不同方案可能采用不同 vbat 滤波算法, 在方案对应的plugin.c中处理
    plugin_vbat_filter(&vbat);
    //默认的取平均值算法.
    adc_cb.vbat_total = adc_cb.vbat_total - adc_cb.vbat_val + vbat; //均值
    adc_cb.vbat_val = adc_cb.vbat_total>>5;

    if(adc_cb.vbat_val > vbat_bak) {
        vbat = adc_cb.vbat_val - vbat_bak;
    } else {
        vbat = vbat_bak - adc_cb.vbat_val;
    }
    if (vbat >= 30) {   //偏差大于一定值则更新
        vbat_bak = adc_cb.vbat_val;
//        printf(bat_str, adc_cb.vbat_val/1000, adc_cb.vbat_val%1000);
    }
#if DISP_BAT_EN
    if(bat_buf != vbat_bak)
    {
        bat_cnt ++;
        if(bat_cnt > 500 || sys_cb.bat_disp)
        {
            bat_cnt = 0;
            bat_buf = vbat_bak;
            while(bat_buf > tbl_bat[level])
            {
                level ++;
                if(level >= 5)
                    break;
            }
            sys_cb.bat_level = level;
        }
    }
    else
    {
        bat_cnt = 0;
    }
#endif
    return vbat_bak;
}

void vbat_init(void)
{
    saradc_set_channel(BIT(ADCCH_BGOP) | BIT(ADCCH_VBAT));
}

void vbat_voltage_init(void)
{
    adc_cb.vbat_val =  VBAT_CACL_VOLTAGE();
    adc_cb.vbat_total = adc_cb.vbat_val << 5;
    sys_cb.vbat = vbat_get_voltage();
}

int is_lowpower_vbat_warning(void)
{
    if (sys_cb.vbat <= ((u16)LPWR_OFF_VBAT*100+2700)) {
        if (LPWR_OFF_VBAT) {
            if (!sys_cb.lpwr_cnt) {
                sys_cb.lpwr_cnt = 1;
            } else if (sys_cb.lpwr_cnt >= 10) {
                return 1;       //VBAT低电关机
            }
        }
        return 0;               //VBAT低电不关机
    }
#if WARNING_LOW_BATTERY
    else {
        sys_cb.lpwr_cnt = 0;
        if (sys_cb.vbat < ((u16)LPWR_WARNING_VBAT*100 + 2850)) {

    #if LED_LOWBAT_EN
            if (xcfg_cb.rled_lowbat_en) {
                if ((!CHARGE_DC_IN()) && (!RLED_LOWBAT_FOLLOW_EN)) {
                    led_lowbat();
                } else {
                    led_lowbat_recover();
                }
            }
    #endif // LED_LOWBAT_EN
            if (xcfg_cb.lowpower_warning_en) {
                return 2;       //低电压提示音播报
            }
        } else {
    #if LED_LOWBAT_EN
            if (xcfg_cb.rled_lowbat_en) {
                led_lowbat_recover();
            }
    #endif // LED_LOWBAT_EN
        }
        return 0;
    }
#endif // WARNING_LOW_BATTERY
    sys_cb.lpwr_cnt = 0;
    return 0;
}
#endif  //VBAT_DETECT_EN
