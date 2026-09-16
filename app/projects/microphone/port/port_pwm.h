#ifndef __PORT_PWM_H__
#define __PORT_PWM_H__

void pwm_init(void);
void pwm_rgb_write(u8 red, u8 green, u8 blue);
void pwm_rgb_close(void);

void energy_led_init(void);
void energy_led_level_calc(void);
void energy_led_scan(void);

void spi_rgb_init(void);
void spirgb_senddata(u8 num_max, u8 rgb_value[][3], u8 byte_h, u8 byte_l);
void spi_rgb_process(void);
void spi_rgb_off(void);
#endif //__PORT_PWM_H__
