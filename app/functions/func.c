#include "include.h"

func_cb_t func_cb AT(.buf.func_cb);

AT(.text.func.bt)
void func_bt_init(void)
{
    if (!sys_cb.bt_is_inited) {
        msg_queue_clear();
        wireless_init();
        sys_cb.bt_is_inited = 1;
    }
}

#if VBAT_DETECT_EN

void lowpower_warning_do(void)
{
    if (sys_cb.lpwr_warning_times) {        //低电语音提示次数
        if (RLED_LOWBAT_FOLLOW_EN) {
            led_lowbat_follow_warning();
        }

        sys_cb.lowbat_flag = 1;
//            bsp_res_play(TWS_RES_LOW_BATTERY);
        bsp_piano_warning_play(WARNING_TONE, TONE_LOW_BATTERY);
        plugin_lowbat_vol_reduce();         //低电降低音乐音量

        #if 0
        if (RLED_LOWBAT_FOLLOW_EN) {
            while (get_led_sta(1)) {        //等待红灯闪完
                delay_5ms(2);
            }
            led_lowbat_recover();
        }
        #endif

        if (sys_cb.lpwr_warning_times != 0xff) {
            sys_cb.lpwr_warning_times--;
        }
    }
}

void lowpower_poweroff_do(void)
{
    bsp_piano_warning_play(WARNING_TONE, TONE_LOW_BATTERY);
    sys_cb.pwroff.low_bat_ind = 1;
}

AT(.text.func.msg)
void lowpower_vbat_process(void)
{
    int lpwr_vbat_sta = is_lowpower_vbat_warning();
    if (lpwr_vbat_sta == 1) {
//        bsp_res_play(TWS_RES_LOW_BATTERY);
        lowpower_poweroff_do();
        return;
    }

    //低电提示音播放
    sys_cb.vbat_nor_cnt = 0;
    if (lpwr_vbat_sta == 2) {
        if (sys_cb.lpwr_warning_cnt > xcfg_cb.lpwr_warning_period) {
            sys_cb.lpwr_warning_cnt = 0;
            lowpower_warning_do();
        }
    }
}
#endif // VBAT_DETECT_EN

AT(.text.func.process)
void pwroff_process(void)
{
    struct pwroff_tag *pwroff = &sys_cb.pwroff;

    if(pwroff->state == PWROFF_W4_TIMEOUT) {
        if(tick_check_expire(pwroff->ticks, pwroff->delay_ticks)) {
            pwroff->state = PWROFF_END;
            pwroff->pwr_key_ind = 1;
        }
    }
//    printf("pwroff = %d %d %d \n",pwroff->state,pwroff->all_flag,pwroff->pwr_key_ind);
    if(pwroff->all_flag != 0) {
        if(pwroff->aux_insert_ind) {
            pwroff->tone_en = LINEIN_2_PWRDOWN_TONE_EN;
        } else
        {
            pwroff->tone_en = 1;
        }
        printf("FUNC_PWROFF\n");
        func_cb.sta = FUNC_PWROFF;
    }
}

//AT(.text.func.process)
//static void print_info(void)
//{
//    static u32 ticks = 0;
//    if (tick_check_expire(ticks,1000)) {
//        ticks = tick_get();
//        printf("print_infn: %d %x\n",get_sysclk_nhz(),UART1CON);
//    }
//}

AT(.text.func.process)
void func_process(void)
{
    WDT_CLR();
#if !BSP_TBOX_TEST_EN
    vusb_reset_clr();
#endif // BSP_TBOX_TEST_EN
//    print_info();

#if VBAT_DETECT_EN
    lowpower_vbat_process();
#endif // VBAT_DETECT_EN

    pwroff_process();

#if CHARGE_EN && !CHARGE_5V_POWER_SUPPLY_EN  //使用5V VUSB供电时,保持正常工作,不进入FUNC_CHARGE
    if (xcfg_cb.charge_en) {
        charge_process();
    }
#endif // CHARGE_EN

    if(sys_cb.bt_is_inited) {
        bt_thread_check_trigger();
        bsp_res_process();

        bt_thread_check_trigger();
        bsp_res_process();
    }

	app_process();

    gsensor_process();
#if LED_DISP_EN
    led_disp_proc();
#endif
    sys_run_loop();
}

//func common message process
AT(.text.func.msg)
void func_message(u16 msg)
{
    switch (msg) {
        case MSG_KEY_PP_HOLD:
            if(sys_cb.pwroff.state == PWROFF_IDLE) {
                sys_cb.pwroff.ticks = tick_get();
                sys_cb.pwroff.state = PWROFF_W4_TIMEOUT;
            }
            break;

#if EQ_DBG_IN_UART
        case EVT_ONLINE_SET_EQ:
            bsp_eq_parse_cmd();
            break;

#if EFFECT_DBG_ADJUST_IN_UART
        case EVT_ONLINE_SET_EFFECT:
            printf("EVT_ONLINE_SET_EFFECT\n");
            toolkit_process();
            break;
#endif
#endif
#if TOOLKIT_AUDIO_DUMP_EN
        case EVT_ONLINE_SET_BTMIC_DUMP:
            bsp_btmic_dump_parse_cmd();
            break;
#endif
#if LE_BQB_RF_EN || LE_FCC_TEST_EN
        case EVT_HCI_CMD:
            ble_hci_cmd_check();

            break;
#endif
        default:
            break;
    }

    //调节音量，3秒后写入flash
    if ((sys_cb.cm_vol_change) && (sys_cb.cm_times >= 6)) {
        sys_cb.cm_vol_change = 0;
        param_hfp_vol_write();
        param_sys_vol_write();
        cm_sync();
    }
}

///进入一个功能的总入口
AT(.text.func)
void func_enter(void)
{
    if (sys_cb.cm_vol_change) {
        sys_cb.cm_vol_change = 0;
        param_sys_vol_write();
    }
    param_sync();
    gui_box_clear();
    reset_sleep_delay();
    reset_pwroff_delay();
    func_cb.mp3_res_play = NULL;
    func_cb.set_vol_callback = NULL;
    bsp_clr_mute_sta();
    sys_cb.voice_evt_brk_en = 1;    //播放提示音时，快速响应事件。
    if(func_cb.sta == FUNC_PWROFF) {
        key_set_msg_tbl(NULL);
    } else if(func_cb.sta == FUNC_ADAPTER || func_cb.sta == FUNC_DEVICE){
        key_set_msg_tbl(wireless_mic_key_msg_tbl);
    }
}

AT(.text.func)
void func_exit(void)
{
    u8 func_num;
    u8 funcs_total = get_funcs_total();

    for (func_num = 0; func_num != funcs_total; func_num++) {
        if (func_cb.last == func_sort_table[func_num]) {
            break;
        }
    }
    func_num++;                                     //切换到下一个任务
    if (func_num >= funcs_total) {
        func_num = 0;
    }
    func_cb.sta = func_sort_table[func_num];        //新的任务
#if SYS_MODE_BREAKPOINT_EN
    param_sys_mode_write(func_cb.sta);
#endif // SYS_MODE_BREAKPOINT_EN
}

AT(.text.func)
void func_run(void)
{
    printf("%s\n", __func__);
    if (wireless_role_is_adapter()) {
        func_cb.sta = FUNC_ADAPTER;
    } else {
        func_cb.sta = FUNC_DEVICE;
    }
#if FUNC_LE_DUT_EN
    func_cb.sta  = FUNC_LE_DUT;
#endif
    while (1) {
        func_enter();
        switch (func_cb.sta) {
        case FUNC_DEVICE:
            func_device();
            break;

        case FUNC_ADAPTER:
            func_adapter();
            break;
        case FUNC_CHARGE:
            func_charge();
            break;

        case FUNC_PWROFF:
            func_pwroff(sys_cb.pwroff.tone_en);
            break;

#if FUNC_LE_DUT_EN
        case FUNC_LE_DUT:
            func_le_dut();
            break;
#endif

        default:
            func_exit();
            break;
        }
    }
}
