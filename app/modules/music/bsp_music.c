#include "include.h"

#define FS_CRC_SEED         0xffff

uint calc_crc(void *buf, uint len, uint seed);
bool mp3_res_play_kick(u32 addr, u32 len, bool kick);
bool wav_res_play_kick(u32 addr, u32 len, bool kick);
void wav_res_dec_process(void);
bool wav_res_is_play(void);
bool wav_res_stop(void);
void mp3_res_play_exit(void);

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


void mp3_res_play(u32 addr, u32 len)
{
    if (!wireless_role_is_adapter() && !DEVICE_DAC_OUTPUT) {
        return;
    }
    u16 msg;
    u8 mute_bak;
//    printf("%s: addr: %x, len: %x\n", __func__, addr, len);
    if (len == 0) {
        return;
    }

#if DAC_DNR_EN
    u8 sta = dac_dnr_get_sta();
    dac_dnr_set_sta(0);
#endif

    mute_bak = sys_cb.mute;
    if (sys_cb.mute) {
        bsp_loudspeaker_unmute();
    }
    if(get_music_dec_sta() != MUSIC_STOP) { //避免来电响铃/报号未完成，影响get_music_dec_sta()状态
        music_control(MUSIC_MSG_STOP);
    }
    bsp_change_volume(WARNING_VOLUME);

    mp3_res_play_kick(addr, len, true);

    while (get_music_dec_sta() != MUSIC_STOP) {
        bt_thread_check_trigger();
        sys_run_loop();
        WDT_CLR();
        msg = msg_dequeue();
        if (sys_cb.voice_evt_brk_en) {
            if (((msg == EVT_SD_INSERT) || (msg == EVT_UDISK_INSERT)) && (func_cb.sta != FUNC_MUSIC)) {
                func_message(msg);
                break;
            }
        }
        if (msg != NO_MSG) {
            msg_enqueue(msg);       //还原未处理的消息
        }
    }
    music_control(MUSIC_MSG_STOP);
    bsp_change_volume(bsp_get_vol());
    mp3_res_play_exit();
    sys_cb.mute = mute_bak;
    if (sys_cb.mute) {
        bsp_loudspeaker_mute();
    }
#if DAC_DNR_EN
    dac_dnr_set_sta(sta);
#endif
}

void wav_res_play(u32 addr, u32 len)
{
    if (len == 0) {
        return;
    }


#if DAC_DNR_EN
    u8 sta = dac_dnr_get_sta();
    dac_dnr_set_sta(0);
#endif

#if TINY_TRANSPARENCY_EN
    sys_cb.ttp_sta = sys_cb.ttp_start;
    bsp_ttp_stop();
#endif
    dac1_out_audio_mute_set(1);

    sys_clk_req(INDEX_RES_PLAY, SYS_120M);

    wav_res_play_kick(addr, len, 1);
    while (wav_res_is_play()) {
        bt_thread_check_trigger();
        wav_res_dec_process();
        WDT_CLR();
    }

    wav_res_stop();
    sys_clk_free(INDEX_RES_PLAY);
    dac1_out_audio_mute_set(0);
#if DAC_DNR_EN
    dac_dnr_set_sta(sta);
#endif

#if TINY_TRANSPARENCY_EN
    if (sys_cb.ttp_sta) {
        bsp_ttp_start();
    }
#endif
}

void piano_res_play(u8 type, void *res)
{
    dac1_out_audio_mute_set(1);

    tone_play_kick(type, res, true);
    while (tone_is_playing()) {
        bt_thread_check_trigger();
        piano_play_process();
    }
    tone_play_end();

    dac1_out_audio_mute_set(0);
}

uint8_t bsp_res_play(uint8_t res_idx)
{
//    uint8_t res_type = tws_res_get_type(res_idx);
//
//    u32 cur_time = 0;
//    msc_breakpiont_t brkpt;
//
//    if(func_cb.sta == FUNC_MUSIC){
//        music_get_breakpiont(&brkpt);           //保存当前播放位置
//        cur_time = music_get_cur_time();
//        music_control(MUSIC_MSG_STOP);
//    }
//
//    if (res_type != RES_TYPE_INVALID) {
//        u32 addr, len;
//
//        tws_res_get_addr(res_idx, &addr, &len);
//        if (len != 0) {
//            if(res_type == RES_TYPE_MP3) {
//                bt_audio_bypass();
//                mp3_res_play(addr, len);
//                bt_audio_enable();
//            } else if(res_type == RES_TYPE_WAV) {
//                wav_res_play(addr, len);
//            } else if(res_type == RES_TYPE_PIANO || res_type == RES_TYPE_TONE) {
//                int type = (res_type == RES_TYPE_TONE)? WARNING_TONE : WARNING_PIANO;
//                piano_res_play(type, (void *)addr);
//            }
//        }
//    }
//
//    if(func_cb.sta == FUNC_MUSIC){
//        music_decode_init();
//        music_set_jump(&brkpt);                 //恢复播放位置
//        music_set_cur_time(cur_time);
//    }

    return RES_ERR_INVALID;
}
