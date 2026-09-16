#include "include.h"


void knob_init(void);
void knob_process(void);
u16 knob_process2(u16 *key_val);

static struct {
    u8 msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX];
    u16 hold_msg;
    u8 ignore_id;
} key_scan;


AT(.com_text.key.table)
const key_shake_tbl_t key_shake_table = {
    .scan_cnt = KEY_SCAN_TIMES,
    .up_cnt   = KEY_UP_TIMES,
    .long_cnt = KEY_LONG_TIMES,
    .hold_cnt = KEY_LONG_HOLD_TIMES,
};

AT(.text.bsp.key.init) ALIGNED(128)
void key_set_msg_tbl(const void *msg_tbl)
{
    u8 msg_buf[KEY_TBL_MAX_NB*KEY_MSG_MAX_IDX];

    if(msg_tbl == NULL) {
        memset(msg_buf, MSG_NO, KEY_TBL_MAX_NB*KEY_MSG_MAX_IDX);
    } else {
        memcpy(msg_buf, msg_tbl, KEY_TBL_MAX_NB*KEY_MSG_MAX_IDX);
    }

    GLOBAL_INT_DISABLE();
    memcpy(key_scan.msg_tbl, msg_buf, KEY_TBL_MAX_NB*KEY_MSG_MAX_IDX);
    GLOBAL_INT_RESTORE();
}

AT(.com_text.bsp.key)
static u16 bsp_key_get_msg(u16 key_evt)
{
    u16 key_id  = key_evt & KEY_ID_MASK;
    u16 evt_type = key_evt & KEY_EVT_MASK;
    if(key_id < KEY_TBL_MAX_NB) {
        u8 msg_idx = key_evt_idx_tbl[evt_type >> KEY_EVT_POS];
        if(msg_idx < KEY_MSG_MAX_IDX) {
            return key_scan.msg_tbl[key_id][msg_idx];
        }
//    } else if(key_id < KEY_SINGLE_MAX_NB) {
//        return key_scan.msg_num_tbl[key_id - KEY_SINGLE_FIRST_IDX];
    }

    return NO_KEY;
}

void key_set_ignore(u8 usage_id)
{
    key_scan.ignore_id = usage_id;
}

AT(.text.bsp.key.init)
void key_init(void)
{
    key_var_init();
    key_set_msg_tbl(NULL);

#if IOKEY_EN
    io_key_init();
#endif

#if ADKEY_EN || ADKEY2_EN
    adkey_init();
#endif

#if KNOB_KEY_EN
    knob_init();
#endif

#if PWRKEY_EN
    pwrkey_init();
#endif

#if VBAT_DETECT_EN
    vbat_init();
#endif

    bsp_saradc_init();

//    bsp_tkey_init();
}

AT(.text.bsp.key)
u8 get_double_key_time(void)
{
    if(DOUBLE_KEY_TIME > 7) {
        return 60;
    } else {
        return (u8)((u8)(DOUBLE_KEY_TIME + 2) * 20 + 1);
    }
}

AT(.com_text.bsp.key)
u16 bsp_key_process(u16 key_val)
{
#if USER_KEY_KNOB2_EN
 	static u8 timer1ms_cnt=0;
    timer1ms_cnt++;
    u16 key_ret = knob_process2(&key_val);
    if (key_ret != 0xffff && key_ret != NO_KEY) {
        return key_ret;
    }
    if(timer1ms_cnt < 5) {
        return NO_KEY;              //貌似有问题
    }
    timer1ms_cnt=0;
#endif

    u16 key_evt = key_process(key_val);
    return key_evt;
}

AT(.com_rodata.bsp.key.str)
const char key_enqueue_str[] = "enqueue: %04x\n";

AT(.com_text.bsp.key)
u8 bsp_key_scan_do(void)
{
    u8 key_val = NO_KEY;

    if (!bsp_saradc_process()) {
        return NO_KEY;
    }

#if KNOB_KEY_EN
    knob_process();
#endif

#if TKEY_EN
    key_val = bsp_tkey_scan();
#endif

#if PWRKEY_EN
    if ((key_val == NO_KEY) && (!PWRKEY_2_HW_PWRON)) {
        key_val = pwrkey_get_val();
    }
#endif

#if IOKEY_EN
    if (key_val == NO_KEY) {
        key_val = get_iokey();
    }
#endif

#if ADKEY_EN || ADKEY2_EN ||ADKEY_MUX_SDCLK_EN
    if (key_val == NO_KEY) {
        key_val = adkey_get_val();
    }
#endif
    return key_val;
}

AT(.com_text.bsp.key)
u8 bsp_key_scan(void)
{
    u8 key_val;
    u16 key_evt = NO_KEY;

    key_val = bsp_key_scan_do();
    key_evt = bsp_key_process(key_val);

    if (key_evt != NO_KEY) {
        u16 key_msg = bsp_key_get_msg(key_evt);
        u16 evt_type = key_evt & KEY_EVT_MASK;
        u8 key_uid = (key_evt & KEY_UID_MASK);

        //过滤掉第1次上电长按PWR键消息
        if(key_scan.ignore_id == key_uid) {
            if(evt_type == KEY_LONG_UP || evt_type == KEY_SHORT_UP) {
                key_scan.ignore_id = MSG_NO;
            }
            return key_val;
        }

        //防止enqueue多个HOLD消息
        if (evt_type == KEY_HOLD) {
            key_scan.hold_msg = key_msg;
            if(msg_queue_check(key_msg) > 0) {
                key_msg = MSG_NO;
            }
        } else {
            if(key_scan.hold_msg != key_msg) {
                if(key_scan.hold_msg == MSG_PWR_HOLD) {
                    msg_enqueue(MSG_PWR_RELEASE);
                }
                key_scan.hold_msg = MSG_NO;
            }
        }

        if(key_msg != MSG_NO) {
            msg_enqueue(key_msg);
//            printf(key_enqueue_str,key_msg);
        }
    }
    return key_val;
}

uint8_t bsp_key_pwr_scan(void)
{
    uint8_t key_val = NO_KEY;

#if TKEY_EN
    key_val = bsp_tkey_scan();
#endif

#if PWRKEY_EN
    if (key_val == NO_KEY) {
        key_val = pwrkey_get_val();
    }
#endif // PWRKEY_EN
    return key_val;
}

