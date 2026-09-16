#include "include.h"
#include "func.h"


#if FUNC_AUX_EN
AT(.text.func.aux.msg)
void func_aux_message(u16 msg)
{
    switch (msg) {
        case MSG_MUSIC_PLAY_PAUSE:
            func_aux_pause_play();
            break;

        case EVT_LINEIN_REMOVE:
            func_cb.sta = FUNC_BT;
            break;

        default:
            func_message(msg);
            break;
    }
}
#endif // FUNC_AUX_EN
