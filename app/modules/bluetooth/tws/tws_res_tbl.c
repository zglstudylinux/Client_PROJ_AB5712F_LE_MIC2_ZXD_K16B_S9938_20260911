#include "include.h"


//可自定义修改tws_res的资源类型
AT(.rodata.tws.res.type)
static const u8 tws_res_type_tbl[TWS_RES_MAX] =
{
    [TWS_RES_NUM_0]         = RES_TYPE_MP3,
    [TWS_RES_NUM_1]         = RES_TYPE_MP3,
    [TWS_RES_NUM_2]         = RES_TYPE_MP3,
    [TWS_RES_NUM_3]         = RES_TYPE_MP3,
    [TWS_RES_NUM_4]         = RES_TYPE_MP3,
    [TWS_RES_NUM_5]         = RES_TYPE_MP3,
    [TWS_RES_NUM_6]         = RES_TYPE_MP3,
    [TWS_RES_NUM_7]         = RES_TYPE_MP3,
    [TWS_RES_NUM_8]         = RES_TYPE_MP3,
    [TWS_RES_NUM_9]         = RES_TYPE_MP3,
    [TWS_RES_RING]          = RES_TYPE_MP3,
    [TWS_RES_MAX_VOL]       = WARING_MAXVOL_TYPE,
    [TWS_RES_LOW_BATTERY]   = RES_TYPE_MP3,
    [TWS_RES_CONNECTED]     = RES_TYPE_MP3,
    [TWS_RES_DISCONNECT]    = RES_TYPE_MP3,
    [TWS_RES_PAIRING]       = RES_TYPE_MP3,
    [TWS_RES_LANGUAGE_EN]   = RES_TYPE_MP3,
    [TWS_RES_LANGUAGE_ZH]   = RES_TYPE_MP3,

    [TWS_RES_CALL_HANGUP]   = RES_TYPE_MP3,
    [TWS_RES_CALL_REJECT]   = RES_TYPE_MP3,
    [TWS_RES_REDIALING]     = RES_TYPE_MP3,
    [TWS_RES_MUSIC_MODE]    = RES_TYPE_MP3,
    [TWS_RES_GAME_MODE]     = RES_TYPE_MP3,
    [TWS_RES_NR_DISABLE]    = RES_TYPE_WAV,
    [TWS_RES_ANC]           = RES_TYPE_WAV,
    [TWS_RES_TRANSPARENCY]  = RES_TYPE_WAV,
};

AT(.text.tws.res.type) WEAK
u8 tws_res_get_type(u32 index)
{
    if(index >= TWS_RES_MAX) {
        printf("tws_res: invaild index = %d\n", index);
        return RES_TYPE_INVALID;
    }

    return tws_res_type_tbl[index];
}

AT(.text.tws.res.addr) WEAK
void tws_res_get_addr(u32 index, u32 *addr, u32 *len)
{
    switch(index) {
    case TWS_RES_NUM_0:
        *addr = RES_BUF_NUM_0_MP3;
        *len = RES_LEN_NUM_0_MP3;
        break;

    case TWS_RES_NUM_1:
        *addr = RES_BUF_NUM_1_MP3;
        *len = RES_LEN_NUM_1_MP3;
        break;

    case TWS_RES_NUM_2:
        *addr = RES_BUF_NUM_2_MP3;
        *len = RES_LEN_NUM_2_MP3;
        break;

    case TWS_RES_NUM_3:
        *addr = RES_BUF_NUM_3_MP3;
        *len = RES_LEN_NUM_3_MP3;
        break;

    case TWS_RES_NUM_4:
        *addr = RES_BUF_NUM_4_MP3;
        *len = RES_LEN_NUM_4_MP3;
        break;

    case TWS_RES_NUM_5:
        *addr = RES_BUF_NUM_5_MP3;
        *len = RES_LEN_NUM_5_MP3;
        break;

    case TWS_RES_NUM_6:
        *addr = RES_BUF_NUM_6_MP3;
        *len = RES_LEN_NUM_6_MP3;
        break;

    case TWS_RES_NUM_7:
        *addr = RES_BUF_NUM_7_MP3;
        *len = RES_LEN_NUM_7_MP3;
        break;

    case TWS_RES_NUM_8:
        *addr = RES_BUF_NUM_8_MP3;
        *len = RES_LEN_NUM_8_MP3;
        break;

    case TWS_RES_NUM_9:
        *addr = RES_BUF_NUM_9_MP3;
        *len = RES_LEN_NUM_9_MP3;
        break;

    case TWS_RES_RING:
        *addr = RES_BUF_RING_MP3;
        *len = RES_LEN_RING_MP3;
        break;
    case TWS_RES_CONNECTED:
        *addr = RES_BUF_CONNECTED_MP3;
        *len = RES_LEN_CONNECTED_MP3;
        break;

    case TWS_RES_DISCONNECT:
        *addr = RES_BUF_DISCONNECT_MP3;
        *len = RES_LEN_DISCONNECT_MP3;
        break;

    case TWS_RES_PAIRING:
        *addr = RES_BUF_PAIRING_MP3;
        *len = RES_LEN_PAIRING_MP3;
        break;

    case TWS_RES_LOW_BATTERY:
        *addr = RES_BUF_LOW_BATTERY_MP3;
        *len = RES_LEN_LOW_BATTERY_MP3;
        break;

    case TWS_RES_MAX_VOL:
#if WARING_MAXVOL_TYPE == RES_TYPE_PIANO
//        *addr = (u32)&warning_piano_tbl[PIANO_MAX_VOL];
//        *len = 1;                       //flag
#elif WARING_MAXVOL_TYPE == RES_TYPE_TONE
        *addr = (u32)&warning_tone_tbl[TONE_MAX_VOL];
        *len = 1;                       //flag
#elif WARING_MAXVOL_TYPE == RES_TYPE_WAV
        *addr = RES_BUF_MAX_VOL_WAV;
        *len = RES_LEN_MAX_VOL_WAV;
#elif WARING_MAXVOL_TYPE == RES_TYPE_MP3
        *addr = RES_BUF_MAX_VOL_MP3;
        *len = RES_LEN_MAX_VOL_MP3;
#endif
        break;

#if (LANG_SELECT == LANG_EN_ZH)
    case TWS_RES_LANGUAGE_ZH:
        *addr = RES_BUF_ZH_LANGUAGE_MP3;
        *len = RES_LEN_ZH_LANGUAGE_MP3;
        break;

    case TWS_RES_LANGUAGE_EN:
        *addr = RES_BUF_EN_LANGUAGE_MP3;
        *len = RES_LEN_EN_LANGUAGE_MP3;
        break;
#endif

    case TWS_RES_CALL_HANGUP:
        *addr = RES_BUF_CALL_HANGUP_MP3;
        *len = RES_LEN_CALL_HANGUP_MP3;
        break;

    case TWS_RES_CALL_REJECT:
        *addr = RES_BUF_CALL_REJECT_MP3;
        *len = RES_LEN_CALL_REJECT_MP3;
        break;

    case TWS_RES_REDIALING:
        *addr = RES_BUF_REDIALING_MP3;
        *len = RES_LEN_REDIALING_MP3;
        break;

    case TWS_RES_MUSIC_MODE:
        *addr = RES_BUF_MUSIC_MODE_MP3;
        *len = RES_LEN_MUSIC_MODE_MP3;
        break;

    case TWS_RES_GAME_MODE:
        *addr = RES_BUF_GAME_MODE_MP3;
        *len = RES_LEN_GAME_MODE_MP3;
        break;

//    case TWS_RES_NR_DISABLE:
//        *addr = RES_BUF_NR_DISABLE_WAV;
//        *len = RES_LEN_NR_DISABLE_WAV;
//        break;
//    case TWS_RES_ANC:
//        *addr = RES_BUF_ANC_ON_WAV;
//        *len = RES_LEN_ANC_ON_WAV;
//        break;
//    case TWS_RES_TRANSPARENCY:
//        *addr = RES_BUF_TRANSPARENCY_WAV;
//        *len = RES_LEN_TRANSPARENCY_WAV;
//        break;

    default :
        *addr = 0;
        *len = 0;
        break;
    }
}
