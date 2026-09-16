#ifndef _API_KEY_H
#define _API_KEY_H

extern uint8_t pwr_usage_id;

void key_var_init(void);
u16 key_process(u16 key_val);
bool pwrkey_is_pressed(void);
#endif // _API_KEY_H
