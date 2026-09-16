#include "include.h"
#include "func.h"

bool power_off_check(void);
void lock_code_pwrsave(void);
void unlock_code_pwrsave(void);

AT(.com_text.sleep)
void lowpwr_tout_ticks(void)
{
    if(sys_cb.sleep_delay != -1L && sys_cb.sleep_delay > 0) {
        sys_cb.sleep_delay--;
    }
    if(sys_cb.pwroff_delay != -1L && sys_cb.pwroff_delay > 0) {
        sys_cb.pwroff_delay--;
    }


}

AT(.com_text.sleep)
bool sys_sleep_check(u32 *sleep_time)
{
    if(*sleep_time > sys_cb.sleep_wakeup_time) {
        *sleep_time = sys_cb.sleep_wakeup_time;
        return true;
    }
    return false;
}

AT(.sleep_text.sleep)
void sys_sleep_cb(u8 lpclk_type)
{
    //注意！！！！！！！！！！！！！！！！！
    //此函数只能调用sleep_text或com_text函数

    //此处关掉影响功耗的模块
    u32 gpiogde = GPIOGDE;
    GPIOGDE = BIT(2) | BIT(4);                  //SPICS, SPICLK

    sys_sleep_proc(lpclk_type);                //enter sleep

    //唤醒后，恢复模块功能
    GPIOGDE = gpiogde;
}

#if USER_INEAR_DETECT_EN
//INEAR状态发生变化，需要唤醒sniff mode
AT(.sleep_text.earin)
bool earin_is_wakeup(void)
{
    if (INEAR_IS_ONLINE()) {
        if (sys_cb.loc_ear_sta) {
            return true;
        }
    } else {
        if (!sys_cb.loc_ear_sta) {
            return true;
        }
    }
    return false;
}
#endif // USER_INEAR_DETECT_EN

//休眠定时器，500ms进一次
AT(.sleep_text.sleep)
uint32_t sleep_timer(void)
{
    uint32_t ret = 0;

    app_sleep_mode_process();
#if CHARGE_EN
    if (xcfg_cb.charge_en) {
        charge_detect(0);
    }
#endif // CHARGE_EN
    if (led_bt_sleep()) {
        sys_cb.sleep_wakeup_time = 50000/312;

#if VBAT_DETECT_EN
        bsp_saradc_init();
        if ((xcfg_cb.lowpower_warning_en) && (sys_cb.vbat < ((u16)LPWR_WARNING_VBAT*100 + 2800))) {
            //低电需要唤醒sniff mode
            ret = 2;
        }
        bsp_saradc_exit();
#endif // VBAT_DETECT_EN

#if CHARGE_EN
        if (!port_2led_is_sleep_en()) {
            ret = 2;
        }
#endif // CHARGE_EN
    } else {
        sys_cb.sleep_wakeup_time = -1L;
    }
    if(sys_cb.pwroff_delay != -1L) {
        if(sys_cb.pwroff_delay > 5) {
            sys_cb.pwroff_delay -= 5;
        } else {
            sys_cb.pwroff_delay = 0;
            return 1;
        }
    }

    if ((PWRKEY_2_HW_PWRON) && (!PWRKEY_IS_PRESS())){
        ret = 1;
    }
    return ret;
}

AT(.text.lowpwr.sleep)
static void sfunc_sleep_dac_off(u8 off)
{

    if (off) {

        if (SLEEP_DAC_OFF_EN) {
            if (!wireless_role_is_adapter() && !DEVICE_DAC_OUTPUT) {
                dac_power_off(1);
            } else {
                dac_power_off(0);                //dac power down
            }
        } else {
            dac_channel_disable();          //only dac channel master disable
        }
    } else {
        if (SLEEP_DAC_OFF_EN) {
            dac_restart();
        } else {
            dac_channel_enable();
        }
    }
}

AT(.text.lowpwr.sleep)
static void sfunc_sleep(void)
{
    uint32_t status, ret, sysclk;
    uint32_t usbcon0, usbcon1;
    u16 pa_de, pb_de, pe_de, pg_de, pf_de;
    u16 adc_ch;

#if VBAT_DETECT_EN
    if (is_lowpower_vbat_warning()) {   //低电不进sniff mode
        return;
    }
#endif // VBAT_DETECT_EN


    printf("%s\n", __func__);
    bt_audio_bypass();
    app_enter_sleep();

    bt_enter_sleep();

    gui_off();
    led_off();
    rled_off();

#if DAC_DNR_EN
    u8 sta = dac_dnr_get_sta();
    dac_dnr_set_sta(0);
#endif

    sfunc_sleep_dac_off(1);
    sys_set_tmr_enable(0, 0);
#if SYS_SLEEP_LEVEL > 2
    sys_set_tmr_tick(0);
#endif

    sys_clk_free_all();                 //释放其它模块sys_clk_req的系统时钟
    sysclk = sys_clk_get();
    sys_clk_set(SYS_24M);

    adc_ch = bsp_saradc_exit();         //close saradc及相关通路模拟
    saradc_set_channel(BIT(ADCCH_VBAT) | BIT(ADCCH_BGOP));

#if CHARGE_EN
    charge_set_stop_time(3600);
    lock_code_charge();
#endif

    usbcon0 = USBCON0;                  //需要先关中断再保存
    usbcon1 = USBCON1;
    USBCON0 = BIT(5);
    USBCON1 = 0;

    {
        DACDIGCON0 &= ~BIT(0);              //disable digital dac
        dac_clk_source_sel(2);              //dac clk select xosc26m_clk
        //close pll0
        PLL0CON0 &= ~(BIT(18) | BIT(6));             //pll0 sdm & analog disable
//        RSTCON0 &= ~BIT(4);                         //pllsdm disable
    }

    //io analog input
    pa_de = GPIOADE;
    pb_de = GPIOBDE;
    pe_de = GPIOEDE;
    pg_de = GPIOGDE;
    pf_de = GPIOFDE;

    GPIOADE = 0;
    GPIOBDE = pb_de & BIT(5);               //保留PB5(wko)的配置
    u16 pe_de_t = 0;
#if CHARGE_EN
    if (CHARGE_DC_IN() && (xcfg_cb.rled_io_sel == IO_PE0)) {
        pe_de_t = BIT(0);                   //PE0做充电指示灯，保持IO数字输出
    }
#endif
    GPIOEDE = pe_de_t;
    GPIOGDE = 0x3F; //MCP FLASH
    GPIOFDE = 0;

    wakeup_disable();
    sleep_wakeup_config();

    sys_cb.sleep_wakeup_time = -1L;
    while(bt_is_sleep()) {
        WDT_CLR();
        vusb_reset_clr();
        bt_thread_check_trigger();
        status = bt_sleep_proc();
#if SYS_SLEEP_LEVEL > 2
        bb_run_loop();
#endif
        if(status == 1) {
            ret = sleep_timer();
            if(ret) {
                sys_cb.pwroff.timeout_ind = (bool)(ret == 1);
                break;
            }
        }

        if (wakeup_get_status()) {
            break;
        }

        if(app_need_wakeup()){
            break;
        }

        if(le_popup_need_wakeup()){
            break;
        }
    }
    GPIOADE = pa_de;
    GPIOBDE = pb_de;
    GPIOEDE = pe_de;
    GPIOGDE = pg_de;
    GPIOFDE = pf_de;

    printf("wakeup: %d\n", wakeup_gpio_get_status(IO_PA7));
    wakeup_disable();

    USBCON0 = usbcon0;
    USBCON1 = usbcon1;

    {
        adpll_init(DAC_OUT_SPR);                //enable adpll
        dac_clk_source_sel(1);                  //dac clk select adda_clk48
    }

    saradc_set_channel(adc_ch);
    bsp_saradc_init();
#if CHARGE_EN
    charge_set_stop_time(18000);
    charge_status(sys_cb.charge_sta);          //update充灯状态
    unlock_code_charge();
#endif // CHARGE_EN
    sys_clk_set(sysclk);
#if SYS_SLEEP_LEVEL > 2
    sys_set_tmr_tick(1);
#endif
    sys_set_tmr_enable(1, 1);

    if (DAC_FAST_SETUP_EN) {
        bsp_loudspeaker_mute();
        delay_5ms(2);
    }

    sfunc_sleep_dac_off(0);

    bsp_change_volume(sys_cb.vol);
#if DAC_DNR_EN
    dac_dnr_set_sta(sta);
#endif

    bt_exit_sleep();
    bt_audio_enable();
    gsensor_wakeup();

    app_exit_sleep();
    printf("sleep_exit\n");
}

AT(.text.lowpwr.sleep)
bool sleep_process(is_sleep_func is_sleep)
{

    if(app_need_wakeup()){
         reset_sleep_delay();
         reset_pwroff_delay();
         return false;
    }


#if CHARGE_EN && (UART0_PRINTF_SEL != PRINTF_VUSB)
    if (xcfg_cb.charge_en && CHARGE_INBOX()) {
        return false;
    }
#endif // CHARGE_EN

    if ((*is_sleep)()) {
        if ((!sys_cb.sleep_en) || (!port_2led_is_sleep_en())) {
            reset_sleep_delay();
            return false;
        }
        ///连着话筒时，不进入休眠模式
        if(wireless_get_status()){
            return false;
        }

        if(sys_cb.sleep_delay == 0) {
            sfunc_sleep();
            reset_sleep_delay();
            reset_pwroff_delay();
            return true;
        }
    } else {
        reset_sleep_delay();
    }
    return false;
}

AT(.text.pwroff.save)
void sfunc_power_save_enter(void)
{
#if CHARGE_EN
    if (xcfg_cb.charge_en) {
        charge_off();
    }
#endif // CHARGE_EN
    pmu_set_mode(0);
    //usb_disable();
    GPIOADE = 0;
    GPIOBDE = BIT(5);                       //pwrkey
    GPIOEDE = 0;
    GPIOFDE = 0;
    GPIOGDE = 0x3F;                         //MCP FLASH

    PICCONCLR = BIT(0);                     //Globle IE disable
    CLKCON0 |= BIT(0);                      //enable RC
    CLKCON0 = (CLKCON0 & ~(0x3 << 2));      //system clk select RC
    PLL0CON0 &= ~BIT(18);                   //pll10 sdm enable disable
    PLL0CON0 &= ~BIT(6);                    //pll0 analog diable
    PLL0CON0 &= ~BIT(12);                   //pll0 ldo disable

    XOSCCON &= ~(0xf<<24);                  //X26 output DIS
    XOSCCON &= ~BIT(10);                    //X26 DIS
}

///充满关机前等待仓休眠5V掉电
AT(.text.pwroff)
u8 sfunc_pwrdown_w4_vusb_offline(void)
{
    u32 timeout = 12000, off_cnt = 0;
    if (xcfg_cb.ch_box_type_sel != 1) {
        while (timeout--) {
            WDT_CLR();
            delay_us(330);                  //5ms
            if (!CHARGE_DC_IN()) {
                off_cnt++;
            } else {
                off_cnt = 0;
            }
            if (off_cnt > 5) {
                return 1;                   //VUSB已掉线，打开VUSB唤醒
            }
        }
    }
    return 0;
}

//硬开关方案，低电时，进入省电状态
AT(.text.pwroff)
void sfunc_lowbat(void)
{
    sfunc_power_save_enter();
    WDT_DIS();
    asm("nop");asm("nop");asm("nop");

    LPMCON |= BIT(0);                   //Sleep mode enable
    asm("nop");asm("nop");asm("nop");
    while(1);
}

//软开关方案，POWER键/低电时，进入关机状态
AT(.text.pwroff)
void sfunc_pwrdown_do(u8 vusb_wakeup_en)
{

    printf("pwrdwn: %d\n", vusb_wakeup_en);
    vusb_reset_clr();
    RTCCON3 &= ~BIT(8);                             //rtc alarm wakeup disable
    RTCCON8 &= ~BIT(15);                            //RI_EN_VUSBDIV = 0
    RTCCON9 = 0xffff;                               //Clr pending
#if TKEY_LOWPWR_WAKEUP_DIS                     //电池无保护板且有内置触摸开关机功能方案，需要打开此宏
    if (sys_cb.vbat < 3050) {
        tkey_wakeup_en = 0;                         //低电关机, 关掉触模唤醒
    }
    if (tkey_wakeup_en) {
        RTCALM = RTCCNT + 300;                      //定时5分钟唤醒检查电池电量
        RTCCON3 |= BIT(8);
    }
#endif
    sfunc_power_save_enter();
    if (!vusb_wakeup_en) {
        RTCCON8 = (RTCCON8 & ~BIT(6)) | BIT(1);     //disable charger function
    }

    WDT_DIS();
    RTCCON11 = (RTCCON11 & ~0x03) | BIT(2);         //WK PIN filter select 8ms
    uint rtccon3 = RTCCON3 & ~BIT(11);
    uint rtccon13 = RTCCON13 & ~(0xf << 12);       //WK pin5~0 wakeup disable
    uint rtccon4 = RTCCON4 & ~(0x7 << 22);
#if CHARGE_EN
    if ((xcfg_cb.charge_en) && (vusb_wakeup_en)) {
        rtccon3 |= BIT(11);                         //VUSB wakeup enable
        RTCCON3 |= BIT(11);                         //VUSB wakeup enable
        RTCCON11 |= BIT(6);                         //VUSB pull out filter VUSB拔出滤波
        delay_us(100);
        RTCCON10 = BIT(3);
    }
#endif

#if BSP_TBOX_TEST_EN
    rtccon3 |= BIT(11);                             //VUSB wakeup enable  //产测模式需要支持5v唤醒
#endif // BSP_TBOX_TEST_EN

    RTCCON3 = rtccon3 & ~(BIT(10) | BIT(14));       //关WK PIN，再打开，以清除Pending
    PWRCON1 &= ~(0x1F<<14);                         //disable Flash Power Gate
    PWRCON1 |= BIT(18);                             //pdown flash power gate
#if TKEY_EN
    if ((RTCCON3 & BIT(12)) || sys_cb.lowbat_flag) {//仓内或低电关机？
        tkey_wakeup_en = 0;
    }

    if (tkey_wakeup_en) {
        RTCCON1 |= BIT(9);                          //tk digital voltage enable
        RTCCON3 |= BIT(14);                         //Touch key long press wakeup
        rtccon4 |= 0x4 << 22;                       //VRTC 0.96V
        TKCON   |= BIT(14);                         //touch key auto enable（低功耗模式，扫描完自动开关）
    } else {
        //保持电源，保证tkey通道能顺利关闭
        RTCCON0 |= BIT(5);                          //tk rst disable
        TKACON0 = 0;
        TKACON1 = 0;
        RTCCON0 &= ~BIT(4);                         //tk interface disable
        RTCCON0 &= ~BIT(0);                         //RC2M_RTC Disable
        RTCCON0 &= ~BIT(5);                         //tk rst enable
        RTCCON1 &= ~BIT(9);                         //tk digital voltage disable
        rtccon4 |= 0x2 << 22;                       //VRTC 0.82V
    }
#else
    RTCCON0 &= ~BIT(0);                             //RC2M_RTC Disable
    rtccon4 |= 0x2 << 22;                           //VRTC 0.82V
#endif
    RTCCON4 = rtccon4;

    RTCCON0 &= ~(BIT(22) | BIT(23));
    rtccon3 = RTCCON3 & ~0x17;                      //Disable VDDCORE VDDIO VDDBUCK, VDDXOEN
    rtccon3 |= BIT(6);                              //Core power down enable, VDDCORE short disable
    rtccon3 &= ~BIT(22);                            //LVCORE_DIS
    RTCCON3 &= ~BIT(22);                            //LVCORE_DIS, lvcore掉电
    rtccon3 |= BIT(7);                              //RI_EN_VDDIO_AON   RTC 2.9V LDO enable
#if PWRKEY_EN
    rtccon3 |= BIT(10);                             //WK pin wake up enable
    rtccon13 |= BIT(0) | BIT(4) | BIT(12) ;         //wk pin0 wakeup, input, pullup10k enable
#endif // PWRKEY_EN
    RTCCON &= ~(3 << 1);                            //rtc选择sys_clk div2, 避免关机时rtc配置来不及生效，导致关机功耗偏大1.5uA
    RTCCON |= BIT(5);                               //PowerDown Reset，如果有Pending，则马上Reset
    RTCCON13 = rtccon13;
    RTCCON3 = rtccon3;
    LPMCON |= BIT(0);                               //sleep mode
    LPMCON |= BIT(1);                               //idle mode
    asm("nop");asm("nop");asm("nop");
    while (1);
}


void sfunc_pwrdown(u8 vusb_wakeup_en)
{
    lock_code_pwrsave();
    sfunc_pwrdown_do(vusb_wakeup_en);
}

AT(.text.lowpwr.pwroff)
void func_pwroff(int pwroff_tone_en)
{
    printf("%s: %d\n", __func__, pwroff_tone_en);

    bsp_res_set_enable(false);
    while(bsp_res_is_playing()) {
        bt_thread_check_trigger();
        bsp_res_process();
    }
    bsp_res_cleanup();

    led_power_down();
#if WARNING_POWER_OFF
    if (SOFT_POWER_ON_OFF) {
        if (pwroff_tone_en == 1) {
            mp3_res_play(RES_BUF_POWEROFF_MP3, RES_LEN_POWEROFF_MP3);
        } else if (pwroff_tone_en == 2) {
        }
    }
#endif // WARNING_POWER_OFF

    gui_off();

    if (SOFT_POWER_ON_OFF) {
        if (!PWRKEY_2_HW_PWRON) {
            u8  dcin_cnt = 0;
            u32 pwrkey_ticks_temp;
            pwrkey_ticks_temp = tick_get();;
            while (PWRKEY_IS_PRESS() || TKEY_IS_PRESS()) {      //等待PWRKWY松开
                if (CHARGE_DC_IN()) {
                    dcin_cnt++;
                    if (dcin_cnt > 3) {
                        sw_reset_kick(SW_RST_DC_IN);        //直接复位进入充电
                    }
                } else {
                    dcin_cnt = 0;
                }
                delay_5ms(2);
                WDT_CLR();
                if(tick_check_expire(pwrkey_ticks_temp, PWROFF_PRESS_TIME)) {
                }
            }
        }
        if (!wireless_role_is_adapter() && !DEVICE_DAC_OUTPUT) {
            dac_power_off(1);
        } else {
            dac_power_off(0);                    //dac power down
        }
        if (CHARGE_DC_IN()) {
            if (power_off_check()) {
                return;
            }
        }
        bsp_saradc_exit();                  //close saradc及相关通路模拟
        if ((PWRKEY_2_HW_PWRON) && (sys_cb.poweron_flag)) {
            RTCCON13 |= BIT(16);            //WK PIN0 High level wakeup
        }

        gsensor_lowpwr();
        sfunc_pwrdown(1);
    } else {

        if (!wireless_role_is_adapter() && !DEVICE_DAC_OUTPUT) {
            dac_power_off(1);
        } else {
            dac_power_off(0);                    //dac power down
        }
        bsp_saradc_exit();                  //close saradc及相关通路模拟

        sfunc_lowbat();                     //低电关机进入Sleep Mode
    }
}
