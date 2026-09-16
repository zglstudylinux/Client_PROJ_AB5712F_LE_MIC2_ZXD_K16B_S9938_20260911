#include "include.h"

//调试时才打开
//AT(.com_text.debug)
//void debug_dump_info_cache_miss_callback(u32 miss_addr)
//{
//    miss_addr += 0x10000000;
//}

//调试时才打开，否则异常时无法复位
//void sys_error_hook_do(u8 err_no);
//AT(.com_text.err)
//void sys_error_hook(u8 err_no)
//{
//    sys_error_hook_do(err_no);
//}

//正常启动Main函数
int main(void)
{
    sys_cb.rst_reason = sys_rst_init(POWKEY_10S_RESET);
    printf("\nHello BT893X: %x\n", sys_cb.rst_reason);
    printf("SDK : v%04X, LIBS: v%04X\n", SDK_VERSION, LIBS_VERSION);
//    if (sys_cb.rst_reason & BIT(31)) {
//        printf("first power up\n");
//    }
    if(sys_cb.rst_reason & BIT(25)) {
        printf("UART DET reset\n");
    } else if(sys_cb.rst_reason & BIT(19)) {
        if (RTCCON10 & BIT(15)) {
            printf("VUSB4S timeout reset\n");
        } else {
            printf("WKO10S reset\n");
        }
    } else if(sys_cb.rst_reason & BIT(18)) {
        printf("WKUP reset\n");
    } else if(sys_cb.rst_reason & BIT(17)) {
        printf("VUSB reset\n");
    } else if(sys_cb.rst_reason & BIT(16)) {
        printf("WDT reset\n");
    } else if(sys_cb.rst_reason & 0xf00) {
        printf("SW reset\n");
    }

#if ADAPTER_AB_FOT_DEVICE_SUPPORT
	fot_set_base_offset();
#endif


    sys_init();
    func_run();
    return 0;
}

//升级完成
void update_complete(int mode)
{
    sys_cb.rst_reason = sys_rst_init(POWKEY_10S_RESET);
    sys_update_init();
    if (mode == 0) {
        WDT_DIS();
        while (1);
    }
    WDT_RST();
}
