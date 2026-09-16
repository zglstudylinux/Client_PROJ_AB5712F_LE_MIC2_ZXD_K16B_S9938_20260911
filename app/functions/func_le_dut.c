#include "include.h"
#include "func.h"
#include "func_le_dut.h"

#if FUNC_LE_DUT_EN


AT(.text.func.le_dut)
static void func_le_dut_process(void)
{
    func_process();
}

AT(.text.func.le_dut)
static void func_le_dut_enter(void)
{
    msg_queue_clear();

    le_hci_cmd_init();

    func_bt_init();
}

AT(.text.func.le_dut)
static void func_le_dut_exit(void)
{
    bt_off();
    func_cb.last = FUNC_LE_DUT;
}

AT(.text.func.le_dut)
void func_le_dut(void)
{
    printf("%s\n", __func__);
    func_le_dut_enter();

    while (func_cb.sta == FUNC_LE_DUT) {
        func_le_dut_process();
        func_message(msg_dequeue());
    }

    func_le_dut_exit();
}


#endif
