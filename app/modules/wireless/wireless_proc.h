#ifndef _WIRELESS_PROC_H
#define _WIRELESS_PROC_H


#define WIRELESS_CON_STA_MASK               ((1<<WIRELESS_CON_LINK_NB)-1)

struct wireless_cb_tag {
    uint8_t change_flag;
    uint8_t change_sta;

    uint8_t connected_sta;
    bool alg_en;

#if WIRELESS_MIC_2TNR_EN
    volatile uint8_t device_con_sta;
    volatile uint8_t device_con_role;           //0 : master, 1 : slave
#endif

#if WIRELESS_CON_PAIR_MODE
    uint8_t con_role_switch_flag;
    u32 con_id_switch_ticks;
    uint8_t con_temp;
#endif
};

extern struct wireless_cb_tag wireless_cb;

void wireless_var_init(void);
void wireless_sta_proc(void);
u8 wireless_get_status(void);

void wireless_cmd_reset(u8 index);

#endif // _WIRELESS_PROC_H
