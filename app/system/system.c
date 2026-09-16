#include "include.h"

enum {
    PWRKEY_IDLE,
    PWRKEY_W4_PRESS_TOUT,
    PWRKEY_W4_RELEASE,
    PWRKEY_END,
};


xcfg_cb_t xcfg_cb;
sys_cb_t sys_cb AT(.buf.bsp.sys_cb);
volatile int micl2gnd_flag;
volatile u32 ticks_50ms;
uint8_t cfg_spiflash_speed_up_en = FLASH_SPEED_UP_EN;        //SPI FLASH提速。部份FLASH不支持提速
uint8_t usb_det_ver_sel     = USB_DET_VER_SEL;

void sd_detect(void);
void tbox_uart_isr(void);
void testbox_init(void);
bool exspiflash_init(void);
void ledseg_6c6s_clr(void);
u8 getcfg_buck_mode_en(void);


#if MUSIC_SDCARD_EN
AT(.com_text.detect)
void sd_detect(void)
{
    if (SD_IS_ONLINE()) {
        if (dev_online_filter(DEV_SDCARD)) {
            my_printf("%s %d\n",__func__,__LINE__);
            sd_insert();
            msg_enqueue(EVT_SD_INSERT);
            my_printf("sd insert\n");
        }
    } else {
        if (dev_offline_filter(DEV_SDCARD)) {
            my_printf("%s %d\n",__func__,__LINE__);
            sd_remove();
            msg_enqueue(EVT_SD_REMOVE);
            my_printf("sd remove\n");
        }
    }
}
#endif


#if PWRKEY_2_HW_PWRON
//软开机模拟硬开关，松开PWRKEY就关机。
AT(.com_text.detect)
void pwrkey_2_hw_pwroff_detect(void)
{
    static int off_cnt = 0;

    if (PWRKEY_IS_PRESS()) {
        off_cnt = 0;
    } else {
        if (off_cnt < 10) {
            off_cnt++;
        } else if (off_cnt == 10) {
            //pwrkey已松开，需要关机
            off_cnt = 20;

            sys_cb.pwroff.hw_pwrdwn = 1;
            sys_cb.poweron_flag = 0;
        }
    }
}
#endif // PWRKEY_2_HW_PWRON

//timer tick interrupt(1ms)
AT(.com_text.timer)
void usr_tmr1ms_isr(void)
{
#if (GUI_SELECT & DISPLAY_LEDSEG)
    gui_scan();                     //7P屏按COM扫描时，1ms间隔
#endif

#if LED_DISP_EN
    port_2led_scan();
#endif // LED_DISP_EN

    plugin_tmr1ms_isr();
#if USER_KEY_KNOB2_EN
    bsp_key_scan();
#endif

    led_scan();
}

//timer tick interrupt(5ms)
AT(.com_text.timer)
void usr_tmr5ms_thread(void)
{
    tmr5ms_cnt++;
    //5ms timer process
    dac_fade_process();
#if !USER_KEY_KNOB2_EN
    bsp_key_scan();
#endif
#if PWRKEY_2_HW_PWRON
    pwrkey_2_hw_pwroff_detect();
#endif

    plugin_tmr5ms_isr();

#if MP3_SPR_CHANGED_JUMP_NEXT
    plugin_check_mp3_spr();
#endif

#if USB_SUPPORT_EN
    usb_detect();
#endif // USB_SUPPORT_EN

    //20ms timer process
    if ((tmr5ms_cnt % 4) == 0) {
#if DAC_DNR_EN
        dac_dnr_detect();
#endif // DAC_DNR_EN

#if CHARGE_EN
        if (xcfg_cb.charge_en) {
            if (CHARGE_DC_IN() || (CHARGE_INBOX() && xcfg_cb.chbox_en)) {
                if (sys_cb.dc_in_filter < CHARGE_DC_IN_FILTER) {
                    sys_cb.dc_in_filter++;
                }
            } else {
                sys_cb.dc_in_filter = 0;
            }
        }
#endif
    }

    //50ms timer process
    if ((tmr5ms_cnt % 10) == 0) {
        ticks_50ms++;
    }

    //100ms timer process
    if ((tmr5ms_cnt % 20) == 0) {
        lowpwr_tout_ticks();
#if UDE_HID_EN
        if (func_cb.sta == FUNC_ADAPTER) {
            ude_tmr_isr();
        }
#endif // UDE_HID_EN
#if ADAPTER_MFI_EN
        if(func_cb.sta == FUNC_ADAPTER) {
            uds_iap_tmr_isr();
        }
#endif
        gui_box_isr();                  //显示控件计数处理

        if (sys_cb.lpwr_cnt > 0) {
            sys_cb.lpwr_cnt++;
        }

        if (sys_cb.key2unmute_cnt) {
            sys_cb.key2unmute_cnt--;
            if (!sys_cb.key2unmute_cnt) {
                msg_enqueue(EVT_KEY_2_UNMUTE);
            }
        }
        dac_fifo_detect();
    }

#if MUSIC_SDCARD_EN
    if (func_cb.sta == FUNC_ADAPTER) {
        sd_detect();
    }
#endif
    //500ms timer process
    if ((tmr5ms_cnt % 100) == 0) {
        sys_cb.cm_times++;
    }

    //1s timer process
    if ((tmr5ms_cnt % 200) == 0) {
        msg_enqueue(MSG_SYS_1S);
        sys_cb.lpwr_warning_cnt++;
    }
    //10s timer process
    if ((tmr5ms_cnt % 6000) == 0) {
        msg_enqueue(EVT_MSG_SYS_30S);
        tmr5ms_cnt = 0;
    }
}

uint bsp_get_bat_level(void)
{
#if VBAT_DETECT_EN
    //计算方法：level = (实测电压 - 关机电压) / ((满电电压 - 关机电压) / 100)
    u16 bat_off = LPWR_OFF_VBAT * 100 + 2700;
    if (bat_off > sys_cb.vbat) {
        return 0;
    }
    uint bat_level = (sys_cb.vbat - bat_off) / ((4200 - bat_off) / 100);
    //printf("bat level: %d %d\n", sys_cb.vbat, bat_level);
    if (bat_level > 100) {
        bat_level = 100;
    }
    return bat_level;
#else
    return 100;
#endif
}

AT(.text.bsp.sys.init)
static void rtc_32k_configure(void)
{
    u32 temp = RTCCON0;

//    //xosc_div768_rtc
//    temp &= ~BIT(6);
//    temp |= BIT(9) | BIT(8);                        //sel xosc_div768_rtc
//    RTCCON0 = temp;
//    RTCCON4 |= BIT(17);                             //xosc24m low power clk enable
//    RTCCON2 = 31249;

    //clk2m_rtc_div32
    temp &= ~BIT(6);
    temp &= ~(BIT(9) | BIT(8));
    temp |= BIT(9);
    temp |= BIT(2) | BIT(0);
    RTCCON0 = temp;
    RTCCON2 = sys_get_rc2m_rtc_clk() / 32 - 1;

}

AT(.text.bsp.sys.init)
bool rtc_init(void)
{
    u32 temp;
    rtc_32k_configure();
    sys_cb.rtc_first_pwron = 0;
    temp = RTCCON0;
    if (temp & BIT(7)) {
        temp &= ~BIT(7);                            //clear first poweron flag
        RTCCON0 = temp;
        sys_cb.rtc_first_pwron = 1;
        printf("rtc 1st pwrup\n");
        return false;
    }

    return true;
}

//UART0打印信息输出GPIO选择，UART0默认G1(PA7)
void uart0_mapping_sel(void)
{
    //等待uart0发送完成
    if(UART0CON & BIT(0)) {
        while (!(UART0CON & BIT(8)));
    }

    GPIOEDE  &= ~BIT(13);
    GPIOEPU  &= ~BIT(13);
    GPIOBPU  &= ~(BIT(2) | BIT(3));
    FUNCMCON0 = (0xf << 12) | (0xf << 8);           //clear uart0 mapping

#if (UART0_PRINTF_SEL == PRINTF_PA7)
    GPIOADE  |= BIT(7);
    GPIOAPU  |= BIT(7);
    GPIOADIR |= BIT(7);
    GPIOAFEN |= BIT(7);
    GPIOADRV |= BIT(7);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PA7;           //RX0 Map To TX0, TX0 Map to G1
#elif (UART0_PRINTF_SEL == PRINTF_PB2)
    GPIOBDE  |= BIT(2);
    GPIOBPU  |= BIT(2);
    GPIOBDIR |= BIT(2);
    GPIOBFEN |= BIT(2);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PB2;           //RX0 Map To TX0, TX0 Map to G2
#elif (UART0_PRINTF_SEL == PRINTF_PB3)
    GPIOBDE  |= BIT(3);
    GPIOBPU  |= BIT(3);
    GPIOBDIR |= BIT(3);
    GPIOBFEN |= BIT(3);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PB3;           //RX0 Map To TX0, TX0 Map to G3
#elif (UART0_PRINTF_SEL == PRINTF_PE7)
    GPIOEDE  |= BIT(7);
    GPIOEPU  |= BIT(7);
    GPIOEDIR |= BIT(7);
    GPIOEFEN |= BIT(7);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PE7;           //RX0 Map To TX0, TX0 Map to G4
#elif (UART0_PRINTF_SEL == PRINTF_PE0)
    GPIOEDE  |= BIT(0);
    GPIOEPU  |= BIT(0);
    GPIOEDIR |= BIT(0);
    GPIOEFEN |= BIT(0);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PE0;           //RX0 Map To TX0, TX0 Map to G5
#elif (UART0_PRINTF_SEL == PRINTF_VUSB)
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_VUSB;          //RX0 Map To TX0, TX0 Map to G8
#endif
}

//开user timer前初始化的内容
AT(.text.bsp.sys.init)
static void sys_var_init(void)
{
    u32 rst_reason = sys_cb.rst_reason;
    memset(&sys_cb, 0, sizeof(sys_cb));
    sys_cb.rst_reason = rst_reason;
    sys_cb.loudspeaker_mute = 1;
    sys_cb.ms_ticks = tick_get();
    sys_cb.pwrkey_5s_check = 1;
    xcfg_cb.vol_max = (xcfg_cb.vol_max) ? 32 : 16;
    sys_cb.hfp2sys_mul = (xcfg_cb.vol_max + 2) / 16;
    if (SYS_INIT_VOLUME > xcfg_cb.vol_max) {
        SYS_INIT_VOLUME = xcfg_cb.vol_max;
    }
    if (WARNING_VOLUME > xcfg_cb.vol_max) {
        WARNING_VOLUME = xcfg_cb.vol_max;
    }

    sys_cb.pwroff.delay_ticks = xcfg_cb.pwroff_press_time * 500 + 1500 - (KEY_LONG_TIMES*5);
    sys_cb.sleep_time = -1L;
    sys_cb.pwroff_time = -1L;
    if (xcfg_cb.sys_sleep_time != 0) {
        sys_cb.sleep_time = (u32)xcfg_cb.sys_sleep_time * 10;   //100ms为单位
    }
    if (xcfg_cb.sys_off_time != 0) {
        sys_cb.pwroff_time = (u32)xcfg_cb.sys_off_time * 10;    //100ms为单位
    }

    sys_cb.sleep_delay = -1L;
    sys_cb.pwroff_delay = -1L;
    sys_cb.sleep_en = 0;
    sys_cb.lpwr_warning_times = LPWR_WARING_TIMES;
    sys_cb.led_scan_en = 1;

    if(xcfg_cb.osci_cap == 0 && xcfg_cb.osco_cap == 0) {        //没有过产测时，使用自定义OSC电容
        xcfg_cb.osci_cap = xcfg_cb.uosci_cap;
        xcfg_cb.osco_cap = xcfg_cb.uosco_cap;
    }
    if(xcfg_cb.ft_rf_param_en == 0 && xcfg_cb.bt_rf_param_en) { //使用自定义参数时，不需要微调
        xcfg_cb.bt_rf_pwrdec = 0;
    }

    saradc_var_init();
    key_var_init();
    plugin_var_init();

    msg_queue_init();
    bsp_res_init();

    dev_init(2);

    sdadc_var_init();

    music_stream_var_init();
    msc_pcm_out_var_init();

    dac_cb_init((DAC_CH_SEL & 0x0f) | (0x200 * DAC_FAST_SETUP_EN) | (0x400 * DAC_VCM_CAPLESS_EN) \
                | (0x800 * DAC_MAXOUT_EN));

    mic_channel_check();

//    wireless_init();
}

AT(.text.bsp.sys.init)
static void sys_io_init(void)
{
    //全部设置成模拟GPIO，防止漏电。使用时，自行配置对应数字GPIO
    GPIOADE = 0;
    GPIOBDE = 0;
    GPIOEDE = 0;
    GPIOFDE = 0;
    GPIOGDE = 0x3F; //MCP FLASH

    uart0_mapping_sel();        //调试UART IO选择或关闭


    GPIOFDIR &= ~BIT(0);  //C10电源控制
    GPIOFDE |= BIT(0);
    GPIOFSET = BIT(0);
}

void xosc_get_cfg_cap(u8 *osci_cap, u8 *osco_cap, u8 *both_cap)
{
    *osci_cap = xcfg_cb.osci_cap;
    *osco_cap = xcfg_cb.osco_cap;
    *both_cap = xcfg_cb.osc_both_cap;
}

#if  0 //port_int_example
AT(.com_text)
const char strisr0[] = ">>[0x%X]_[0x%X]\n";
const char strisr1[] = "portisr->";
AT(.com_text.timer)
void port_isr(void)
{
    printf(strisr0,WKUPEDG,WKUPCPND);
    if (WKUPEDG & (BIT(6) << 16)) {
        WKUPCPND = (BIT(6) << 16);  //CLEAR PENDING
        printf(strisr1);
    }

}

void port_int_example(void)     //sys_set_tmr_enable(1, 1); 前调用 测试OK
{
    GPIOFDE |= BIT(0);  GPIOFDIR |= BIT(0); GPIOFFEN &= ~BIT(0);
    GPIOFPU |= BIT(0);
    sys_irq_init(IRQ_PORT_VECTOR,0, port_isr);
    PORTINTEN |= BIT(21);
    PORTINTEDG |= BIT(21);  //falling edge;
    WKUPEDG |= BIT(6);     //falling edge
    WKUPCON = BIT(6) | BIT(16);  //falling edge wake iput //wakeup int en

    printf("PORTINTEN = 0x%X, PORTINTEDG = 0x%X  WKUPEDG = 0x%X, WKUPCON = 0x%X\n", PORTINTEN, PORTINTEDG, WKUPEDG, WKUPCON);
    WDT_CLR();
    while(1) {
//       printf("WKUPEDG = 0x%X\n", WKUPEDG);
//       printf("GPIOF = 0x%X\n", GPIOF);
//       delay_ms(500);
    }
}
#endif

AT(.text.bsp.power)
bool power_off_check(void)
{
#if CHARGE_EN
    u16 charge_cnt = 0;
#endif

    u8 pwrkey_state = PWRKEY_IDLE;
    u32 pwrkey_ticks;
    uint pwrkey_up_cnt = 0;

    u32 pwron_press_nms = PWRON_PRESS_TIME;         //长按开机时间配置
    if (pwron_press_nms == 0) {
        pwron_press_nms = 15;                       //bootloader 80ms + 15ms, 最小开机时间在100ms左右
    }

    //要等PWRKEY开关释放后再次按下才能重新开机, 否则充电过程中5分钟关机, 低电关机等异常
    if ((PWRKEY_2_HW_PWRON) && (sys_cb.poweron_flag)) {
        pwrkey_state = PWRKEY_W4_RELEASE;
    }

    while (1) {
        WDT_CLR();
        delay_ms(5);
        if (bsp_key_pwr_scan() == pwr_usage_id) {
            pwrkey_up_cnt = 0;
            if (pwrkey_state == PWRKEY_IDLE) {
                pwrkey_state = PWRKEY_W4_PRESS_TOUT;
                pwrkey_ticks = tick_get();

                sys_cb.ms_ticks = pwrkey_ticks;     //记录PWRKEY按键按下的时刻
                sys_cb.pwrkey_5s_check = 1;
            } else if(pwrkey_state == PWRKEY_W4_PRESS_TOUT) {
                if (tick_check_expire(pwrkey_ticks, pwron_press_nms)) {
                    pwrkey_state = PWRKEY_END;
                }
            }
        } else {
            if (pwrkey_up_cnt < 3) {
                pwrkey_up_cnt++;
                if (pwrkey_up_cnt == 3) {
                    pwrkey_up_cnt = 10;
                    pwrkey_state = PWRKEY_IDLE;
                }
            }
        }

#if CHARGE_EN
        if (xcfg_cb.charge_en) {
			charge_cnt++;
			if (charge_cnt > 20) {
                charge_cnt = 0;
                charge_detect(0);
            }
        }
#endif // CHARGE_EN

        if (pwrkey_state == PWRKEY_END) {
            if ((CHARGE_DC_NOT_PWRON) && CHARGE_DC_IN()) {
                continue;
            }
#if LINEIN_2_PWRDOWN_EN
            if (dev_is_online(DEV_LINEIN)) {
                continue;
            }
#endif

            //长按PP/POWER开机
            key_set_ignore(pwr_usage_id);
            sys_cb.poweron_flag = 1;
            sys_cb.pwroff.all_flag = 0;          //清除关机标志

            gui_display(DISP_POWERON);
            led_power_up();
            dac_restart();
            bsp_change_volume(sys_cb.vol);
#if WARNING_POWER_ON
            mp3_res_play(RES_BUF_POWERON_MP3, RES_LEN_POWERON_MP3);
#endif

            if (PWRON_ENTER_BTMODE_EN) {
                func_cb.sta = FUNC_BT;
                if (dev_is_online(DEV_UDISK)) {
                    sys_cb.cur_dev = DEV_UDISK;
                } else {
                    sys_cb.cur_dev = DEV_SDCARD;
                }
            }
            return true;
        } else {
            if (CHARGE_DC_IN()) {
                continue;
            } else {
                return false;
            }
        }
    }
}

AT(.text.bsp.power)
static bool power_on_frist_enable(u32 rtccon9)
{
    if (!PWRON_FRIST_BAT_EN) {
        return false;
    }

    if (rtccon9 & BIT(4)) {                                        //charge inbox wakeup
        return false;
    }
    if (rtccon9 & BIT(2)) {                                        //WKO wakeup不能直接开机
        return false;
    }
    if (rtccon9 & BIT(6)) {                                        //TK Wakeup pending
        return false;
    }

    if ((CHARGE_DC_IN() || (CHARGE_INBOX() && xcfg_cb.chbox_en)) && (CHARGE_DC_NOT_PWRON ) && (xcfg_cb.charge_en)) {   //VUSB充电禁止开机
        return false;
    }
    return true;
}

static bool power_on_check_do_pre(u32 rtccon9)
{
    bool ret = false;
     //第一次上电是否直接开机
    if (power_on_frist_enable(rtccon9)) {
        ret = true;
    }
    if(sys_cb.sw_rst_flag == SW_RST_FLAG){
        ret = true;
    }
    if (sys_cb.rst_reason & BIT(19)) {                  //is wko 10s reset pending
        ret = true;                                     //长按PWRKEY 10S复位后直接开机。
    }

#if !PWRKEY_EN
    if ((!PWRKEY_2_HW_PWRON) && (!sys_cb.tkey_pwrdwn_en)) {
        ret = true;
    }
#endif
    return ret;
}

AT(.text.bsp.power)
static void power_on_check_do(void)
{
    u8 chbox_sta = 1;                               //默认offline

    u32 rtccon9 = RTCCON9;                          //wakeup pending
    printf("power_on_check_do: %08x\n", rtccon9);

    RTCCON9 = 0xffff;                               //Clr pending
    RTCCON10 = BIT(10) | BIT(1) | BIT(0);           //Clr pending
    CRSTPND = 0x1ff0000;                            //clear reset pending
    LVDCON &= ~(0x1f << 8);                         //clear software reset
    RTCCON13 &= ~BIT(24);                           //wko pin0 low level wakeup

    if (power_on_check_do_pre(rtccon9)
#if CHARGE_EN
        || charge_power_on_pre(rtccon9)
#endif
        ) {
        sys_cb.poweron_flag = 1;
        return;
    }

    u8 pwrkey_state = PWRKEY_IDLE;
    u32 pwrkey_ticks;
    uint pwrkey_up_cnt = 0;

    u32 pwron_press_nms = PWRON_PRESS_TIME;             //长按开机时间配置
    if (pwron_press_nms == 0) {
        pwron_press_nms = 15;                           //最小开机时间在100ms左右
    }

    while (1) {
        WDT_CLR();
#if CHARGE_LOW_POWER_EN
        delay_us(350);
#else
        delay_5ms(1);
#endif
        bsp_key_scan_do();
        if (bsp_key_pwr_scan() == pwr_usage_id) {
            pwrkey_up_cnt = 0;
            if (pwrkey_state == PWRKEY_IDLE) {
                pwrkey_state = PWRKEY_W4_PRESS_TOUT;
                pwrkey_ticks = tick_get();

                sys_cb.ms_ticks = pwrkey_ticks;         //记录PWRKEY按键按下的时刻
            } else if(pwrkey_state == PWRKEY_W4_PRESS_TOUT) {
                if (tick_check_expire(pwrkey_ticks, pwron_press_nms)) {
                    pwrkey_state = PWRKEY_END;
                }
            }
        } else {
            if (pwrkey_up_cnt < 3) {
                pwrkey_up_cnt++;
                if (pwrkey_up_cnt == 3) {
                    pwrkey_up_cnt = 10;
                    pwrkey_state = PWRKEY_IDLE;
                }
            }
        }

#if LINEIN_2_PWRDOWN_EN
        linein_detect();
#endif // LINEIN_2_PWRDOWN_EN

#if CHARGE_EN
        if (xcfg_cb.charge_en) {
            chbox_sta = charge_charge_on_process();
            if (chbox_sta == 2) {
                break;                  //充电仓拿起开机
            }
        }
#endif // CHARGE_EN

        if (pwrkey_state == PWRKEY_END) {
#if VBAT_DETECT_EN
            if (sys_cb.vbat <= 2950) {  //电压小于2.95v不开机
                continue;
            }
#endif
            if ((CHARGE_DC_NOT_PWRON) && CHARGE_DC_IN()) {
                continue;
            }
#if LINEIN_2_PWRDOWN_EN
            if (dev_is_online(DEV_LINEIN)) {
                continue;
            }
#endif
            sys_cb.poweron_flag = 1;
            key_set_ignore(pwr_usage_id);   //忽略开机时第1次按键直到松开
            break;
        } else {
            //PWKKEY已松开, 不在充电仓或未充电直接进行关机
            if ((pwrkey_state == PWRKEY_IDLE && pwrkey_up_cnt > 0) && (chbox_sta)) {
                if ((!SOFT_POWER_ON_OFF) || ((!PWRKEY_EN) && (!TKEY_SOFT_PWR_EN))) {
                    break;                          //没有按键软开关机功能，不在充电状态直接开机
                }
#if CHARGE_EN
                charge_exit();
#endif
                unlock_code_charge();
                bsp_saradc_exit();
                sfunc_pwrdown(1);
            }
        }
    }
#if CHARGE_EN
     charge_exit();
#endif
}

AT(.text.bsp.power)
void power_on_check(void)
{
    lock_code_charge();
    power_on_check_do();
#if CHARGE_BOX_EN
    charge_box_reinit();
#endif
    unlock_code_charge();
}

AT(.text.bsp.sys.init)
void sys_init(void)
{
    /// config
    if (!xcfg_init(&xcfg_cb, sizeof(xcfg_cb))) {           //获取配置参数
        printf("xcfg init error\n");
    }

    // io init
    sys_io_init();

    // var init
    sys_var_init();

    // power init
    pmu_init(getcfg_buck_mode_en());

    //audio pll init
    adpll_init(DAC_OUT_SPR);

    // clock init
    sys_clk_set(SYS_CLK_SEL);

    // peripheral init
    rtc_init();
    param_init(sys_cb.rtc_first_pwron);

    //晶振配置
    xosc_init();

    plugin_init();

    if (POWKEY_10S_RESET) {
        WKO_10SRST_EN(0);                                   //10s reset source select  0: wko pin press, 1: touch key press
    } else {
        WKO_10SRST_DIS();
    }

    led_init();
#if DISP_BAT_EN
    sys_cb.bat_disp = 3;
#endif

#if CHARGE_EN
    if (xcfg_cb.charge_en) {
        charge_init();
    }
#endif
    key_init();

    wireless_var_init();

#if BSP_TBOX_TEST_EN
    bsp_vusb_test_process();
#endif

#if BSP_TBOX_TEST_EN
    if (!vusb_test_is_sucess())
#endif
    {
        power_on_check();               //在key_init之后
    }

    gui_init();

    gsensor_init();

    en_auto_pwroff();

    /// enable user timer for display & dac
    sys_set_tmr_enable(1, 1);

    led_disp_sta_set(LED_USER_POWER_ON);
    led_power_up();
    gui_display(DISP_POWERON);



#if ANC_EQ_RES2_EN
    copy_res2flash();
#endif

    bt_init();
   /*
    if (1|| bsp_dac_off_for_bt_conn()) {
        dac_init();

//        func_bt_init();
    } else {
//        func_bt_init();
        dac_init();
    }
    */

    if (wireless_role_is_adapter()) {
        dac_init();
    } else {
#if DEVICE_DAC_OUTPUT
        dac_init();
#else
        adc_init_without_dac();
#endif
    }

    codecs_pcm_init();

#if TINY_TRANSPARENCY_EN
    bsp_ttp_init();
#endif

    bsp_change_volume(sys_cb.vol);

#if WARNING_POWER_ON
    mp3_res_play(RES_BUF_POWERON_MP3, RES_LEN_POWERON_MP3);
    sys_cb.outbox_pwron_flag = 0;
#endif // WARNING_POWER_ON


    if (PWRON_ENTER_BTMODE_EN) {
        func_cb.sta = FUNC_BT;
        if (dev_is_online(DEV_UDISK)) {
            sys_cb.cur_dev = DEV_UDISK;
        } else {
            sys_cb.cur_dev = DEV_SDCARD;
        }
    } else {
        {
            {
                func_cb.sta = FUNC_BT;
            }
        }
    }

#if SYS_MODE_BREAKPOINT_EN
    u8 sta = param_sys_mode_read();
    if (sta != 0 && sta != 0xff) {
        func_cb.sta = sta & 0xf;
        if (func_cb.sta == FUNC_MUSIC) {
            sys_cb.cur_dev = sta >> 4;
        }
    }
#endif // SYS_MODE_BREAKPOINT_EN

#if LINEIN_2_PWRDOWN_EN
    if (dev_is_online(DEV_LINEIN)) {
        sys_cb.pwroff.tone_en = LINEIN_2_PWRDOWN_TONE_EN;
        1
        func_cb.sta = FUNC_PWROFF;
    }
#endif // LINEIN_2_PWRDOWN_EN

#if PLUGIN_SYS_INIT_FINISH_CALLBACK
    plugin_sys_init_finish_callback(); //初始化完成, 各方案可能还有些不同参数需要初始化,预留接口到各方案
#endif

#if HUART_EN
    if(xcfg_cb.huart_en){
        bsp_huart_init();
    }
#endif

#if EQ_DBG_IN_UART
    eq_dbg_init();
#endif

#if SPI_HW_EN
    bsp_spi_init();
#endif

#if MUSIC_SDCARD_EN
    SD_DETECT_INIT();
#endif

#if DISP_BAT_EN
    sys_cb.bat_disp = 0;
#endif
//    GPIOAFEN &= ~BIT(6);
//    GPIOADE |= BIT(6);
//    GPIOADIR &= ~BIT(6);

}


AT(.text.bsp.sys.init)
void sys_update_init(void)
{
    /// config
    if (!xcfg_init(&xcfg_cb, sizeof(xcfg_cb))) {           //获取配置参数
        printf("xcfg init error\n");
    }

    // io init
    sys_io_init();

    // var init
    sys_var_init();
    sys_cb.lang_id = 0;

    // power init
    pmu_init(getcfg_buck_mode_en());

    // peripheral init
    rtc_init();
    param_init(sys_cb.rtc_first_pwron);

    //晶振配置
    xosc_init();

    plugin_init();
    sys_set_tmr_enable(1, 1);

    adpll_init(DAC_OUT_SPR);
    dac_init();
    mp3_res_play(RES_BUF_UPDATE_DONE_MP3, RES_LEN_UPDATE_DONE_MP3);
}
