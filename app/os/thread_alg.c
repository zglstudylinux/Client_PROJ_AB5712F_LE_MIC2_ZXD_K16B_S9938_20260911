#include "include.h"
#include "os_thread.h"

AT(.com_text.alc_kick)
void aec_pcm_kick_start(void)
{
    os_send_alg_proc_msg(ENC_MSG_AEC_PCM);
}

AT(.com_text.thread_alg)
void thread_alg_proc_msg_cb(u32 msg)
{
    switch (msg) {
        case KICK_ALG_PRIO_TRANS:
            alg_prio_trans_process();
            break;

        case DNN_L3_PROCESS:
            dnn_L3_mic_proc_cb();
            break;

        case DNN_L1_PROCESS:
            dnn_L1_mic_proc_cb();
            break;

        case AINS4_48K_PROCESS:
            ains4_mic_proc_cb();
            break;

        case AGC_PROCESS:
            agc_mic_proc_cb();
            break;

        case NOTCH_L2_PROCESS:
            howling_notch_L2_mic_proc_cb();
            break;

        case HOWLING_DNN_PROCESS:
            howling_dnn_mic_proc_cb();
            break;

        case KICK_LOCAL_MIC:
#if ADAPTER_LOCAL_MIC_MIX_EN
            local_mic_output();
#endif // ADAPTER_LOCAL_MIC_MIX_EN
            break;

//        case YLCRN_48K_QMF_PROCESS:
//            gtcrn_48k_qmf_mic_proc_cb();
//            break;

        case YLCRN_L3_PROCESS:
            ylcrn_L3_mic_proc_cb();
            break;

        case YLCRN_L2_PROCESS:
            ylcrn_L2_mic_proc_cb();
            break;

        case YLCRN_HOWLING_PROCESS:
            ylcrn_howling_mic_proc_cb();
            break;

        case FREQ_SHIFT2_PROCESS:
            freq_shift2_mic_proc_cb();
            break;

        case VOICE_CHANGE_V2_PROCESS:
            voice_change_v2_mic_proc_cb();
            break;

        case SRC1_ADJ_PROCESS:
            src1_audio_adj_process();
            break;

        case YLCRN_16K_PROCESS:
            ylcrn_16k_mic_proc_cb();
            break;

        case GTCRN_QMF_HOWLING_PROCESS:
            gtcrn_qmf_howling_mic_proc_cb();
            break;
        case ENC_MSG_AEC_PCM:
            aec_pcm_process();
            break;
        default:
            break;
    }
}
