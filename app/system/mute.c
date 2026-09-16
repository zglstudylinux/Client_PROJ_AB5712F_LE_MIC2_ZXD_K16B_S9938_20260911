#include "include.h"



AT(.com_text.bsp.sys)
void bsp_loudspeaker_mute(void)
{
    sys_cb.loudspeaker_mute = 1;
}

AT(.com_text.bsp.sys)
void bsp_loudspeaker_unmute(void)
{
    sys_cb.loudspeaker_mute = 0;
}

AT(.text.bsp.sys)
void bsp_sys_mute(void)
{
    if (!sys_cb.mute) {
        sys_cb.mute = 1;
#if (GUI_SELECT != GUI_NO)
        gui_box_flicker_set(5, 0xff, 0xff);
#endif
        dac_fade_out();
        bsp_loudspeaker_mute();
#if DAC_DNR_EN
        dac_dnr_set_sta(0);
#endif
    }
}

AT(.text.bsp.sys)
void bsp_sys_unmute(void)
{
    if (sys_cb.mute) {
        sys_cb.mute = 0;
        bsp_loudspeaker_unmute();
        dac_fade_in();
#if DAC_DNR_EN
        dac_dnr_set_sta(sys_cb.dnr_sta);
#endif
    }
}

AT(.text.bsp.sys)
void bsp_clr_mute_sta(void)
{
    if (sys_cb.mute) {
        sys_cb.mute = 0;
#if DAC_DNR_EN
        dac_dnr_set_sta(sys_cb.dnr_sta);
#endif
    }
}

AT(.com_text.bsp.sys)
bool bsp_get_mute_sta(void)
{
    return sys_cb.mute;
}
