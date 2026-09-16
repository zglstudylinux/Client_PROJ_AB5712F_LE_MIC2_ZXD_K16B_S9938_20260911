#include "include.h"
#include "os_thread.h"
#include "decoder_prio_trans.h"


AT(.com_text.thread.decoder)
void thread_dec_proc_msg_cb(u32 msg)
{
    switch (msg) {
        case KICK_DEC_PRIO_TRANS:
            decoder_prio_trans_process(0);
            break;
        case KICK_DEC_PRIO_TRANS1:
            decoder_prio_trans_process(1);
            break;

        default:
            break;
    }
}
