#include "include.h"
#include "os_thread.h"

void encoder_prio_trans_process(void);
void src_buf_process(void);

#if !ADAPTER_SRC_BUF_EN
AT(.text.src_proc.src_buf.proc)
void src_buf_process(void)
{}
#endif

AT(.com_text.thread.coder)
void thread_enc_proc_msg_cb(u32 msg)
{
    switch (msg) {
        case KICK_ENC_PRIO_TRANS:
            encoder_prio_trans_process();
            break;
        case KICK_ENC_BUF_PROC:
            src_buf_process();
        default:
            break;
    }
}
