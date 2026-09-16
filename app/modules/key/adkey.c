#include "include.h"

#if ADKEY_MUX_SDCLK_EN
typedef struct {
    uint8_t sdclk_val;
    uint8_t sdclk_convert : 1,                //ADC转换标志
       sdclk_valid   : 1;                //ADC转换数据有效
} adkey_t;

static adkey_t adkey;
#endif

void adkey_init(void)
{
    uint16_t adc_ch = 0;
#if ADKEY_EN
    adc_ch |= BIT(ADKEY_CH);
  #if ADKEY_PU10K_EN
    adcch_io_pu10k_enable(ADKEY_CH);        //开内部10K上拉
  #endif
#endif

#if ADKEY2_EN
    adc_ch |= BIT(ADKEY2_CH);
#endif

#if ADKEY_MUX_SDCLK_EN
    adc_ch |= BIT(SDCLK_AD_CH);
    memset(&adkey, 0x00, sizeof(adkey));
#endif

    saradc_set_channel(adc_ch);
}

#if (ADKEY_EN || ADKEY_MUX_SDCLK_EN)
AT(.com_text.port.key)
static uint8_t adkey_get_key_do(uint8_t key_val)
{
    uint8_t num = 0;

    while (key_val > adkey_table[num].adc_val) {
        num++;
    }

    return adkey_table[num].usage_id;
}
#endif

#if ADKEY_EN
AT(.com_text.port.adkey)
static uint8_t adkey_get_key(void)
{
    return adkey_get_key_do(saradc_get_value8(ADKEY_CH));
}
#endif

#if ADKEY2_EN
AT(.com_text.port.key)
static uint8_t adkey2_get_key(void)
{
    uint8_t num = 0;
    uint8_t *ptr;

    while (saradc_get_value8(ADKEY2_CH) > adkey2_table[num].adc_val) {
        num++;
    }
    return adkey2_table[num].usage_id;
}
#endif

#if ADKEY_MUX_SDCLK_EN
AT(.com_text.key.adkey)
void adkey_mux_convert_done(void)
{
    if (adkey.sdclk_convert) {
        adkey.sdclk_valid = 1;
    } else {
        adkey.sdclk_valid = 0;
    }

    if (sdcard_detect_is_busy()) {
        saradc_clr_channel(BIT(SDCLK_AD_CH));
        adkey.sdclk_convert = 0;
    } else {
        saradc_set_channel(BIT(SDCLK_AD_CH));
        adkey.sdclk_convert = 1;
    }
}

AT(.com_text.port.key)
void adkey_mux_sdclk_w4_convert(void)
{
    if (adkey.sdclk_convert) {
        while (!saradc_is_finish()) {
            WDT_CLR();
        }
    }
}

AT(.com_text.port.adkey)
static uint8_t adkey_mux_get_key(void)
{
    if (!adkey.sdclk_valid) {
        return NO_KEY;
    }
    return adkey_get_key_do(saradc_get_value8(SDCLK_AD_CH));
}
#endif // ADKEY_MUX_SDCLK_EN

AT(.com_text.adkey.get)
uint8_t adkey_get_val(void)
{
    uint8_t key_val = NO_KEY;
#if ADKEY_EN
    if (key_val == NO_KEY) {
        key_val = adkey_get_key();
    }
#endif

#if ADKEY2_EN
    if (key_val == NO_KEY) {
        key_val = adkey2_get_key();
    }
#endif

#if ADKEY_MUX_SDCLK_EN
    //需要放到最后处理,当没进行adc convert需要返回
    if (key_val == NO_KEY) {
        key_val = adkey_mux_get_key();
    }
#endif

    return key_val;
}
