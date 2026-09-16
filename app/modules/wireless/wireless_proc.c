#include "include.h"
#include "wireless.h"


struct wireless_cb_tag wireless_cb;

AT(.com_text.wireless)
u8 wireless_get_status(void)
{
    return wireless_cb.connected_sta;
}

#if WIRELESS_CON_PAIR_MODE
AT(.text.bsp.wireless_cb)
void role_defind(u8 index)
{
    if(wireless_cb.connected_sta == 3) {
        sys_cb.con_role_data[0] = 1;               //主副麦区分完毕标志位
    } else {
        sys_cb.con_role_data[0] = 0;               //主副麦区分完毕标志位
    }
}

AT(.text.bsp.wireless_cb)
u8 role_get(u8 index)
{
    if(sys_cb.con_role_data[0]) {
        if(index) {
            if(sys_cb.con_role_data[1]) {
                return 1;                    //主麦
            } else {
                return 2;                    //副麦
            }
        } else {
            if(!sys_cb.con_role_data[1]) {
                return 1;                    //主麦
            } else {
                return 2;                    //副麦
            }
        }
    }
    return 0;
}
#endif

AT(.text.wireless.cb)
void wireless_emit_notice(uint evt, void *params)
{
//    u32 tmp;
    u8 *packet = params;
    u8 mic_num;
    u8 max_mic_nb = WIRELESS_CON_LINK_NB;
#if WIRELESS_MIC_2TNR_EN
    if (!wireless_role_is_adapter()) {
        max_mic_nb = WIRELESS_CON_2TNR_NB;
    }
#endif

    switch(evt) {
    case BT_NOTICE_WIRELESS_CONNECTED:
        mic_num = packet[0];
        printf("WIRELESS_CONNECTED,%d %d\n", mic_num, packet[8]);

        if(mic_num < max_mic_nb) {
#if LED_DISP_EN
            led_disp_sta_set(LED_USER_CONN);
#endif

#if DEVICE_INTERPHONE_EN
            wireless_cb.connected_sta |= BIT(mic_num);
            wireless_cb.change_sta = 0;
            wireless_cb.change_flag = 1;
            func_device_interphone_set_con_cmp();
            sys_clk_req(INDEX_KARAOK, SYS_120M);        //多设一级台阶，先抬升120M，再抬升至160M
            sys_clk_req(INDEX_KARAOK, SYS_160M);        //先抬高主频
            wireless_interphone_init();
            wireless_cb.alg_en = 1;                     //再使能算法
#else

            if(wireless_cb.connected_sta == 0) {
                sys_clk_req(INDEX_KARAOK, SYS_120M);        //多设一级台阶，先抬升120M，再抬升至160M
                sys_clk_req(INDEX_KARAOK, SYS_160M);        //先抬高主频
                if(wireless_role_is_adapter()) {
                    wireless_adapter_init();
                } else {
                    wireless_device_init();
                }

                wireless_cb.alg_en = 1;
            }
            wireless_cb.connected_sta |= BIT(mic_num);
            wireless_cb.change_sta = 0;
            wireless_cb.change_flag = 1;

#if WIRELESS_CON_PAIR_MODE
            printf("WIRELESS_CON_ROLE, %d\n", sys_cb.con_role);
            if(sys_cb.con_role) {
                sys_cb.con_role_data[1] = mic_num;       //主麦
            } else {
                sys_cb.con_role_data[2] = mic_num;       //副麦
            }
#endif

#endif
#if WIRELESS_MIC_2TNR_EN
        } else if (mic_num == max_mic_nb) {
            ///跑到这里说明时d2d链路，或者侦听发射链路，做特殊处理
            wireless_cb.device_con_sta = 1;
            wireless_cb.device_con_role = packet[8];
            wireless_cb.change_flag = 1;
#endif
        }
#if WIRELESS_CON_PAIR_MODE
            role_defind(mic_num);
#endif
        break;

    case BT_NOTICE_WIRELESS_CONNECT_FAIL:
        mic_num = packet[0];
        printf("WIRELESS_CONNECT_FAIL, %d\n", mic_num);
        if(mic_num < max_mic_nb) {
            wireless_cb.change_sta = 1;
            wireless_cb.change_flag = 1;
#if WIRELESS_MIC_2TNR_EN
        } else if (mic_num == max_mic_nb) {
            wireless_cb.device_con_sta = 0;
            wireless_cb.device_con_role = 0;
            wireless_cb.change_flag = 1;
#endif
        }
        break;

    case BT_NOTICE_WIRELESS_DISCONNECT:
        mic_num = packet[0];
        printf("WIRELESS_DISCONNECT, %d\n", mic_num);
        if(mic_num < max_mic_nb) {
            wireless_cb.connected_sta &= ~BIT(mic_num);
            wireless_cb.change_sta = 2;
            wireless_cb.change_flag = 1;
#if WIRELESS_CON_PAIR_MODE
            wireless_cb.con_temp = 0;
            if(role_get(mic_num) == 2) {                        //副麦断开
                sys_cb.con_role = 1;
            } else if(role_get(mic_num) == 1) {                 //主麦断开
                sys_cb.con_role = 0;
            }
#endif
            wireless_dump_reset(mic_num);
            if(wireless_cb.connected_sta == 0) {
#if LED_DISP_EN
                led_disp_sta_set(LED_USER_CLEAR_PAIR);
#endif
                wireless_cb.alg_en = 0;             //先关闭算法
#if DEVICE_INTERPHONE_EN
                wireless_interphone_exit(mic_num, 0);
#else

                if(wireless_role_is_adapter()) {
                    wireless_adapter_exit(mic_num, 0);
                } else {
                    wireless_device_exit();
                }
#endif
                sys_clk_free(INDEX_KARAOK);             //再还原主频

#if BSP_TBOX_TEST_EN
                if (vusb_test_is_sucess() && cfg_discon_auto_pwroff) {
                        1
                    func_cb.sta = FUNC_PWROFF;
                }
#endif

            } else {
                if(wireless_role_is_adapter()) {
                    wireless_adapter_exit(mic_num, wireless_cb.connected_sta);
                }
            }
#if WIRELESS_MIC_2TNR_EN
        } else if (mic_num == max_mic_nb) {
            wireless_cb.device_con_sta = 0;
            wireless_cb.device_con_role = 0;
            wireless_cb.change_flag = 1;
#endif
        }
        break;

#if WIRELESS_MIC_BROADCAST_EN
    case BT_NOTICE_BROADCAST_SOURCE_TX_EN:
        printf("BT_NOTICE_BROADCAST_SOURCE_TX_EN\n");
        sys_clk_req(INDEX_KARAOK, SYS_160M);
        wireless_device_init();
        wireless_cb.alg_en = 1;
        break;

    case BT_NOTICE_BROADCAST_SOURCE_TX_DIS:
        printf("BT_NOTICE_BROADCAST_SOURCE_TX_DIS\n");
        wireless_cb.change_flag = 1;
        wireless_cb.alg_en = 0;
        wireless_device_exit();
        sys_clk_free(INDEX_KARAOK);
        break;

    case BT_NOTICE_BROADCAST_SINK_SYNC_SETUP:
        printf("BT_NOTICE_BROADCAST_SINK_SYNC_SETUP\n");
        sys_clk_req(INDEX_KARAOK, SYS_160M);
        wireless_adapter_init();
        wireless_cb.alg_en = 1;
        break;

    case BT_NOTICE_BROADCAST_SINK_SYNC_LOST:
        printf("BT_NOTICE_BROADCAST_SINK_SYNC_LOST reason : %d\n", packet[1]);
        {
            u8 reason = packet[1];
            if (reason == 0x08) {
                wireless_cb.change_flag = 1;
            }
            wireless_cb.alg_en = 0;
            wireless_adapter_exit(0, 0);
            sys_clk_free(INDEX_KARAOK);
        }
        break;
#endif

    default:
        break;
    }
#if ADAPTER_HUART_COMMAND_EN
    huart_local_con_update();
#endif
}

AT(.text.func.process.wireless)
void wireless_sta_proc(void)
{
//    u32 conn_flag = 0;
//    if(wireless_cb.connected_sta != sys_cb.disp_sta) {
//        if (xcfg_cb.wireless_adapter_en) {
//            if (wireless_cb.connected_sta > 2) {
//                conn_flag = 3;
//            } else if (wireless_cb.connected_sta < 3 && wireless_cb.connected_sta > 0) {
//                conn_flag = 1;
//            } else {
//                conn_flag = 0;
//            }
//        } else if (xcfg_cb.wireless_device_en) {
//            if (wireless_cb.connected_sta) {
//                conn_flag = 1;
//            } else {
//                conn_flag = 0;
//            }
//        }
//        sys_cb.disp_sta = wireless_cb.connected_sta;
//        switch(conn_flag) {
//        case 0:
//            led_bt_idle();
//            break;
//        case 1:
//            led_bt_connected();
//            break;
//        case 3:
//            led_bt_connected_2mics();
//            break;
//        }
//    }

    wireless_dump_proc();
}

AT(.text.wireless_cmd)
void wireless_rx_user_cmd(u8 index, u8 *ptr, u8 len)
{
//    printf("user_cmd%d: \n", index);
//    print_r(ptr, len);
}

AT(.text.wireless.init)
void wireless_mic_role_init(void)
{
    if (xcfg_cb.wireless_adapter_en) {
        cfg_wireless_role = true;
        return;
    } else if (xcfg_cb.wireless_device_en) {
        cfg_wireless_role = false;
        return;
    } else {
        ///都被去掉默认发射端
        cfg_wireless_role = false;
        return;
    }
}

extern u32 __code_ws_com_vma, __code_ws_com_lma, __code_ws_com_size;
extern u32 __code_apapter_vma, __code_apapter_lma, __code_apapter_size;
extern u32 __code_emit_vma, __code_emit_lma, __code_emit_size;
extern u32 __code_ains4_vma, __code_ains4_lma, __code_ains4_size;

AT(.text.wireless.init)
static void wireless_mic_load_code(void)
{
    memcpy(&__code_ws_com_vma, &__code_ws_com_lma, (u32)&__code_ws_com_size);
    if(xcfg_cb.wireless_adapter_en) {
        memcpy(&__code_apapter_vma, &__code_apapter_lma, (u32)&__code_apapter_size);
        printf("load_code(1): %x, %x, %x\n", &__code_apapter_vma, &__code_apapter_lma, (u32)&__code_apapter_size);
    } else {
        memcpy(&__code_emit_vma, &__code_emit_lma, (u32)&__code_emit_size);
        printf("load_code(0): %x, %x, %x\n", &__code_emit_vma, &__code_emit_lma, (u32)&__code_emit_size);
    }
}

AT(.text.wireless.init)
void load_code_wl_ains4(void)
{
    memcpy(&__code_ains4_vma, &__code_ains4_lma, (u32)&__code_ains4_size);
    printf("load_code(2): %x, %x, %x\n", &__code_ains4_vma, &__code_ains4_lma, (u32)&__code_ains4_size);
}

AT(.text.wireless.init)
void wireless_var_init(void)
{
    wireless_cb.alg_en = 0;
    wireless_mic_role_init();
    wireless_mic_load_code();
    wireless_cmd_init();

    wireless_con_adapter_init();
    wireless_con_device_init();
}
