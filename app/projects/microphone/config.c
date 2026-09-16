#include "include.h"

AT(.rodata.func.table)
const u8 func_sort_table[] = {

#if FUNC_BT_EN
    FUNC_BT,
#endif

};

AT(.text.func)
u8 get_funcs_total(void)
{
    return sizeof(func_sort_table);
}

u32 getcfg_vddio_sel(void)
{
    return xcfg_cb.vddio_sel;
}

u32 getcfg_vddbt_capless_en(void)
{
    return xcfg_cb.vddbt_capless_en;
}

u32 getcfg_buck_mode_en(void)
{
    uint8_t buck_mode_en = 0;
    if (BUCK_MODE_EN) {
        buck_mode_en |= BIT(0);
        if (xcfg_cb.vddcore_buck_en) {
            buck_mode_en |= BIT(1);
        }
    }
    return buck_mode_en;
}

u32 getcfg_mic_bias_method(u8 mic_ch)
{
    if ((mic_ch == MIC0) || (mic_ch == MIC1) || (mic_ch == MIC2) || (mic_ch == MIC3) || (mic_ch == MIC4)) {
        return xcfg_cb.mic_pwr_sel << 4 | xcfg_cb.mic_bias_method;
    }

    return 0;
}

u32 getcfg_mic_gain(u8 mic_ch)
{
    if ((mic_ch == MIC0) || (mic_ch == MIC1) || (mic_ch == MIC2) || (mic_ch == MIC3) || (mic_ch == MIC4)) {
        return xcfg_cb.mic_anl_gain << 8 | xcfg_cb.bt_mic_dig_gain;
    }

    return 0;
}

