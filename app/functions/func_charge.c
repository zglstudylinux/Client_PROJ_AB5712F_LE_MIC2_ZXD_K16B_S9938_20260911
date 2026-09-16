#include "include.h"
#include "func.h"


AT(.text.func.charge)
void func_charge_process(void)
{
    WDT_CLR();

#if CHARGE_LOW_POWER_EN
        delay_us(200);
#else
        delay_5ms(1);
#endif

#if CHARGE_EN
        if (charge_charge_on_process()) {
            func_cb.sta = func_cb.last;
        }
#endif
}

AT(.text.func.charge)
static void func_charge_message(u16 msg)
{
    switch (msg) {
        case EVT_CHARGE_DCIN:
            printf("EVT_CHARGE_DCIN\n");
            break;

        case EVT_CHARGE_DCOUT:
            printf("EVT_CHARGE_DCOUT\n");
            break;

        default:
            func_message(msg);
            break;
    }
}

//机盒通讯，uart数据接收中断回调，此处做数据处理
AT(.com_text.func.charge)
void func_charge_uart_recv_cb(u8 *data, u16 len)
{
    if (data == NULL || len == 0) return;

}

AT(.text.func.charge)
static void func_charge_enter(void)
{
    if (!wireless_role_is_adapter() && !DEVICE_DAC_OUTPUT) {
        dac_power_off(1);
    } else {
        dac_power_off(0);
    }

    lock_code_charge();
#if CHARGE_EN
	charge_enter(1);
#endif
}

AT(.text.func.charge)
static void func_charge_exit(void)
{
#if CHARGE_EN
    charge_exit();
#endif
    unlock_code_charge();
    printf("dc out\n");

    bsp_res_set_enable(true);
    func_bt_init();
    en_auto_pwroff();

    if (!bsp_dac_off_for_bt_conn()) {
        dac_restart();
    }

}

AT(.text.func.charge)
void func_charge(void)
{
    printf("%s\n", __func__);

    func_charge_enter();
    while (func_cb.sta == FUNC_CHARGE) {
        func_charge_process();
        func_charge_message(msg_dequeue());
    }

    func_charge_exit();
}
