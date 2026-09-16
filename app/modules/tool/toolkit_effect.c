#include "include.h"
#include "toolkit_effect.h"

/*
 * 文件名称: effect_adjust_by_res.c
 * 功能描述: 使用上位机修改音效相关处理
 ****************************************************************************************
    code : 待测
    buf  : 待测
    time : 待测
设计流程: 上位机工具会根据链路一键生成 effect_info[]
          根据收到的数据比对 effect_info[] 中的EFFECT_MODE_IDX，匹配则进入callback中进行相应的处理，否则返回

 */

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#define TRACE_R32(...)            print_r32(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#define TRACE_R32(...)
#endif // TRACE_EN


#if EFFECT_DBG_ADJUST_EN

void local_mic_eq_effect_update_callback(u8 *buf, u32 len, u8 params)
{

#if WIRELESS_MIC_EQ_DRC_EN
    tool_eq_coef_t *tool_eq_coef = (tool_eq_coef_t *)(buf + 14);
    if(params == 1) {//update
        if (wireless_role_is_adapter()) {
            toolkit_ack_fail();
            return;
        }
        mic_eq_set_by_param(tool_eq_coef->max_bandnum, (const u32 *)&tool_eq_coef->gain);
    }
#else
    toolkit_ack_fail();
#endif
}

void local_mic_drc_effect_update_callback(u8 *buf, u32 len, u8 params)
{
//    tool_drc_coef_t *tool_drc_coef = (tool_drc_coef_t *)(buf + 14);
#if MIC_DRC_EN
    //软件drc
    if (params == 1) {//update
        if (wireless_role_is_adapter()) {
            toolkit_ack_fail();
            return;
        }
        drc_mic_drc_set_param((u32) buf, len);
    }
#elif WIRELESS_MIC_EQ_DRC_EN
    //硬件drc
    if (params == 1) {//update
        if (wireless_role_is_adapter()) {
            toolkit_ack_fail();
            return;
        }
        mic_drc_set_by_param_toolkit(buf);
    }
#else
    toolkit_ack_fail();
#endif
}

void mic_echo_effect_update_callback(u8 *buf, u32 len, u8 params)
{
#if ECHO_EN
    tool_echo_pitch_coef_t *tool_echo_pitch_coef = (tool_echo_pitch_coef_t *)(buf + 14);//去掉包头
    if (params == 1){//update
        TRACE("channel %d\n", tool_echo_pitch_coef->channel);
        TRACE("enable %d\n", tool_echo_pitch_coef->enable);
        TRACE("lp_filter_en %d\n", tool_echo_pitch_coef->lp_filter_en);
        TRACE("attention %d\n", tool_echo_pitch_coef->attention);
        TRACE("delay %d\n", tool_echo_pitch_coef->delay);
        TRACE("cutoff_freq %d\n", tool_echo_pitch_coef->cutoff_freq);
        TRACE("dry %d\n", tool_echo_pitch_coef->dry);
        TRACE("wet %d\n", tool_echo_pitch_coef->wet);
        echo_audio_param_set(tool_echo_pitch_coef->attention, tool_echo_pitch_coef->delay, tool_echo_pitch_coef->cutoff_freq, tool_echo_pitch_coef->lp_filter_en, tool_echo_pitch_coef->dry, tool_echo_pitch_coef->wet);
    }
#endif
}

void mic_magic_effect_update_callback(u8 *buf, u32 len, u8 params)
{
#if MAGIC_EN
    tool_magic_pitch_coef_t *tool_magic_pitch_coef = (tool_magic_pitch_coef_t *)(buf + 14);//去掉包头
    if (params == 1) {//update
        TRACE("channel %d\n", tool_magic_pitch_coef->channel);
        TRACE("enable %d\n", tool_magic_pitch_coef->enable);
        TRACE("shfit_hz %d\n", tool_magic_pitch_coef->shfit_hz);

        magic_audio_mute_set(!tool_magic_pitch_coef->enable);
        magic_audio_param_set(tool_magic_pitch_coef->shfit_hz);
    }
#endif
}

void mix_mic_eq_effect_update_callback(u8 *buf, u32 len, u8 params)
{
#if 0
//    tool_eq_coef_t *tool_eq_coef = (tool_eq_coef_t *)(buf + 14);
    if (params == 1 && wireless_get_status()) {//update

    }
#else
    toolkit_ack_fail();
#endif //
}

void mix_mic_drc_effect_update_callback(u8 *buf, u32 len, u8 params)
{
#if ADAPTER_MIX_DRC_EN
//    tool_drc_coef_t *tool_drc_coef = (tool_drc_coef_t *)(buf + 14);

    if (params == 1 && wireless_get_status()) {//update
        mix_drc_set_param((u32) buf, len);
    }
#else
    toolkit_ack_fail();
#endif
}

void usb_mic_eq_effect_update_callback(u8 *buf, u32 len, u8 params)
{
#if 0
    tool_eq_coef_t *tool_eq_coef = (tool_eq_coef_t *)(buf + 14);
    if (params == 1) {//update
        usb_mic_pacc_set_eq_by_param(tool_eq_coef->max_bandnum, (const u32 *)&tool_eq_coef->gain);
    }
#else
    toolkit_ack_fail();
#endif
}

void usb_mic_drc_effect_update_callback(u8 *buf, u32 len, u8 params)
{
#if 0
    tool_drc_coef_t *tool_drc_coef = (tool_drc_coef_t *)(buf + 14);
    if (params == 1) {//update
        usb_mic_pacc_set_drc_by_param(tool_drc_coef->coeff_cnt, (const u32 *)&tool_drc_coef->in_attack_coeff);
    }
#else
    toolkit_ack_fail();
#endif
}

void usb_mic1_eq_effect_update_callback(u8 *buf, u32 len, u8 params)
{
#if 0
    tool_eq_coef_t *tool_eq_coef = (tool_eq_coef_t *)(buf + 14);
    if (params == 1) {//update
        usb_mic1_stereo_pacc_set_eq_by_param(tool_eq_coef->max_bandnum, (const u32 *)&tool_eq_coef->gain);
    }
#else
    toolkit_ack_fail();
#endif
}

void usb_mic1_drc_effect_update_callback(u8 *buf, u32 len, u8 params)
{
#if 0
    tool_drc_coef_t *tool_drc_coef = (tool_drc_coef_t *)(buf + 14);
    if (params == 1) {//update
        usb_mic1_stereo_pacc_set_drc_by_param(tool_drc_coef->coeff_cnt, (const u32 *)&tool_drc_coef->in_attack_coeff);
    }
#else
    toolkit_ack_fail();
#endif
}

void usb_mic0_eq_effect_update_callback(u8 *buf, u32 len, u8 params)
{
#if 0
    tool_eq_coef_t *tool_eq_coef = (tool_eq_coef_t *)(buf + 14);
    if (params == 1) {//update
        usb_mic0_stereo_pacc_set_eq_by_param(tool_eq_coef->max_bandnum, (const u32 *)&tool_eq_coef->gain);
    }
#else
    toolkit_ack_fail();
#endif
}

void usb_mic0_drc_effect_update_callback(u8 *buf, u32 len, u8 params)
{
#if 0
    tool_drc_coef_t *tool_drc_coef = (tool_drc_coef_t *)(buf + 14);
    if (params == 1) {//update
        usb_mic0_stereo_pacc_set_drc_by_param(tool_drc_coef->coeff_cnt, (const u32 *)&tool_drc_coef->in_attack_coeff);
    }
#else
    toolkit_ack_fail();
#endif
}

#endif
