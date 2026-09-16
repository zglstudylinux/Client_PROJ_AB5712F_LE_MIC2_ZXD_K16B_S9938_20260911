#ifndef __OS_THREAD_
#define __OS_THREAD_

#include "decoder_prio_trans.h"
#include "encoder_prio_trans.h"
#include "alg_prio_trans.h"

enum {
///usb device message.
    MSG_UDE_EP_RESET = 32,     //usb device ep0 reset message.
    MSG_UDE_CTL_FLOW,          //usb device control flow message.
    MSG_UDE_ISO_IN,            //usb device iso in message(mic)
    MSG_UDE_ISO_OUT,           //usb device iso out message(speaker)
    MSG_UDE_HID_OUT,
    MSG_UDE_IAP_OUT,
    MSG_UDE_IAP_IN,
};

//------------------------------------------------------------------------------------------
//高优先级（要求在一帧时间内完成）
enum {
    KICK_DEC_PRIO_TRANS     = 0,
    KICK_DEC_PRIO_TRANS1,

};

void os_send_dec_proc_msg(u32 msg);
#define kick_decoder_prio_trans(idx)            os_send_dec_proc_msg(KICK_DEC_PRIO_TRANS + (idx))


//------------------------------------------------------------------------------------------
//中优先级（要求在一帧时间内完成）
enum {
    KICK_ENC_PRIO_TRANS     = 0,
    KICK_ENC_BUF_PROC,
};

void os_send_enc_proc_msg(u32 msg);
#define kick_encoder_prio_trans()               os_send_enc_proc_msg(KICK_ENC_PRIO_TRANS)
#define kick_src_buf_proc()                     os_send_enc_proc_msg(KICK_ENC_BUF_PROC)

//------------------------------------------------------------------------------------------
//低优先级
enum {
    KICK_ALG_PRIO_TRANS = 0,
    DNN_L1_PROCESS,
    KICK_LOCAL_MIC,
    AINS4_48K_PROCESS,
    AGC_PROCESS,
    DNN_L3_PROCESS,
    NOTCH_L2_PROCESS,
    HOWLING_DNN_PROCESS,
    YLCRN_48K_QMF_PROCESS,
    FREQ_SHIFT2_PROCESS,
    VOICE_CHANGE_V2_PROCESS,
    YLCRN_L3_PROCESS,
    YLCRN_L2_PROCESS,
    YLCRN_HOWLING_PROCESS,
    SRC1_ADJ_PROCESS,
    GTCRN_QMF_HOWLING_PROCESS,
    ENC_MSG_AEC_PCM,
    YLCRN_16K_PROCESS,
};

void os_alg_sem_pend(uint timeout);             //alg线程等待信号量，timeout时间>=2（单位5ms）
void os_alg_sem_post(void);
void os_send_alg_proc_msg(u32 msg);
void aec_pcm_process(void);
#define kick_alg_prio_trans()                   os_send_alg_proc_msg(KICK_ALG_PRIO_TRANS)
#define ains4_mic_proc_kick_start()             os_send_alg_proc_msg(AINS4_48K_PROCESS)
#define dnn_L1_mic_proc_kick_start()            os_send_alg_proc_msg(DNN_L1_PROCESS)
#define dnn_L3_mic_proc_kick_start()            os_send_alg_proc_msg(DNN_L3_PROCESS)
#define kick_local_mic()                        os_send_alg_proc_msg(KICK_LOCAL_MIC)
#define agc_mic_proc_kick_start()               os_send_alg_proc_msg(AGC_PROCESS)
#define howling_notch_L2_mic_proc_kick_start()  os_send_alg_proc_msg(NOTCH_L2_PROCESS)
#define howling_dnn_mic_proc_kick_start()       os_send_alg_proc_msg(HOWLING_DNN_PROCESS)
#define gtcrn_48k_qmf_mic_proc_kick_start()     os_send_alg_proc_msg(YLCRN_48K_QMF_PROCESS)
#define freq_shift2_mic_proc_kick_start()       os_send_alg_proc_msg(FREQ_SHIFT2_PROCESS)
#define voice_change_v2_mic_proc_kick_start()   os_send_alg_proc_msg(VOICE_CHANGE_V2_PROCESS)
#define ylcrn_L3_mic_proc_kick_start()          os_send_alg_proc_msg(YLCRN_L3_PROCESS)
#define ylcrn_L2_mic_proc_kick_start()          os_send_alg_proc_msg(YLCRN_L2_PROCESS)
#define ylcrn_howling_mic_proc_kick_start()     os_send_alg_proc_msg(YLCRN_HOWLING_PROCESS)
#define src_adj_proc_kick_start()               os_send_alg_proc_msg(SRC1_ADJ_PROCESS)
#define gtcrn_qmf_howling_mic_proc_kick_start() os_send_alg_proc_msg(GTCRN_QMF_HOWLING_PROCESS)
#define ylcrn_16k_mic_proc_kick_start()         os_send_alg_proc_msg(YLCRN_16K_PROCESS)

#endif // __THREAD_ALG_
