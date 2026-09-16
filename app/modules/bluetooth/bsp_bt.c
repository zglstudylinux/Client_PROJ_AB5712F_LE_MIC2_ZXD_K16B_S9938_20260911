#include "include.h"
#include "api.h"


void bt_new_name_init(void);
void ring_tws_disconnect_cb(void);

/********************************************************/
extern void bt_tws_set_operation(uint8_t *cmd);
void dev_vol_set_cb(uint8_t dev_vol, uint8_t media_index, uint8_t setting_type);

//蓝牙连接过程调整主频，加快连接速度
AT(.text.bts.clk) WEAK
void bt_set_sys_clk(uint8_t level)
{
    if(level) {
        sys_clk_req(INDEX_STACK, SYS_120M);
    } else {
        sys_clk_free(INDEX_STACK);
    }
}

void bsp_bt_init(void)
{
    //更新配置工具的设置
    cfg_bt_rf_def_txpwr = xcfg_cb.bt_rf_pwrdec;
    cfg_bt_page_txpwr = xcfg_cb.bt_rf_page_pwrdec;
    cfg_ble_page_txpwr = xcfg_cb.ble_rf_page_pwrdec;
    cfg_ble_page_rssi_thr = xcfg_cb.ble_page_rssi_thr;

    bt_setup();
}

void bsp_bt_close(void)
{
}

#if BT_PWRKEY_5S_DISCOVER_EN
bool bsp_bt_w4_connect(void)
{
    if (xcfg_cb.bt_pwrkey_nsec_discover) {
        while (sys_cb.pwrkey_5s_check) {            //等待检测结束
            WDT_CLR();
            delay_5ms(2);
            bt_thread_check_trigger();
            bsp_res_process();
        }

        //已检测到长按5S，需要直接进入配对状态。播放PAIRING提示音。
        if (sys_cb.pwrkey_5s_flag) {
            return false;
        }
    }
    return true;
}

bool bsp_bt_pwrkey5s_check(void)
{
    bool res = !bsp_bt_w4_connect();
    delay_5ms(2);
    return res;
}

void bsp_bt_pwrkey5s_clr(void)
{
    if (!xcfg_cb.bt_pwrkey_nsec_discover) {
        return;
    }
    sys_cb.pwrkey_5s_flag = 0;
}
#endif // BT_PWRKEY_5S_DISCOVER_EN


void bt_emit_notice(uint evt, void *params)
{
}
