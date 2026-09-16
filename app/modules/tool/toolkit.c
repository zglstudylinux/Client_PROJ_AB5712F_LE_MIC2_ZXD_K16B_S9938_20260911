/*
 * 文件名称: toolkit_effect.c
 * 功能描述: toolkit工具在线调试音效
 ****************************************************************************************
 */

#include "include.h"
#include "toolkit.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif // TRACE_EN

#if EFFECT_DBG_ADJUST_IN_UART
extern u8 eq_rx_buf[EQ_BUFFER_LEN];
static u8 toolkit_tx_ack_buf[14] AT(.buf.toolkit);
static toolkit_bin_info bin_info;
static u8 toolkit_ack = 0; //0： ok 1： fail

AT(.text.toolkit)
void toolkit_ack_fail(void)
{
    //ACK 上位机应答
    toolkit_ack = 1;
}

AT(.text.toolkit)
static u8 toolkit_calc_check_sum(u8 *buf, u16 size)
{
    u32 i, sum = 0;
    for (i = 0; i < size; i++) {
        sum += buf[i];
    }
    return (u8)(-sum);
}

static void toolkit_cmd_process(void)
{
    toolkit_reply_header* reply = (toolkit_reply_header*)eq_rx_buf;
    u32 sumcheck = (u32)little_endian_read_16(eq_rx_buf, 18);
    u16 cmd = little_endian_read_16(eq_rx_buf, 14);
    u8 ack = REPLY_OK;

    if (sumcheck != ALL_MODE_NAME_SUM) {
        ack = REPLY_FAIL;
        TRACE("-->SUMCHECK error %x %x\n", sumcheck, ALL_MODE_NAME_SUM);
    }

    switch (cmd) {
        case TOOL_CMD_GET_BIN:
            reply->size = 14;
            reply->cmd = cmd;
            reply->reply = ack;
            memset(reply->RSVD, 0, 5);
            reply->filelen = effect_res_len_get(EFFECT_IDX_MAX_NB);
            reply->crc16 = (u16)calc_crc(eq_rx_buf, 26, TOOLKIT_SEED);
            bin_info.cnt = 0;
            bin_info.ptr = (u8*) effect_res_addr_get(EFFECT_IDX_MAX_NB);
            bin_info.len = effect_res_len_get(EFFECT_IDX_MAX_NB);
            tx_ack(eq_rx_buf, 28);
            TRACE("TOOL_CMD_GET_BIN %d\n", effect_res_len_get(EFFECT_IDX_MAX_NB));
            break;
        default:
            break;
    }
}

static void toolkit_get_bin(void)
{
    toolkit_get_bin_header* bin_header = (toolkit_get_bin_header*)eq_rx_buf;
    if (bin_info.len == 0) {
        TRACE("TOOL_GET_BIN finish\n");
        return;
    }
    if (bin_info.len > TOOLKIT_GET_BIN_PACKET_LEN) {
        bin_header->size = TOOLKIT_GET_BIN_PACKET_LEN + 3;
        bin_header->cnt = bin_info.cnt;
        memcpy(bin_header->data, bin_info.ptr, TOOLKIT_GET_BIN_PACKET_LEN);
        bin_header->crc16 = (u16)calc_crc(eq_rx_buf, bin_header->size + 12, TOOLKIT_SEED);
        tx_ack(eq_rx_buf, TOOLKIT_GET_BIN_PACKET_LEN + 17);
        bin_info.len -= TOOLKIT_GET_BIN_PACKET_LEN;
        bin_info.ptr += TOOLKIT_GET_BIN_PACKET_LEN;
        bin_info.cnt++;
    } else {
        bin_header->size = bin_info.len + 3;
        bin_header->cnt = bin_info.cnt;
        memcpy(bin_header->data, bin_info.ptr, bin_info.len);
        *(u16*)(bin_header->data + bin_info.len) = (u16)calc_crc(eq_rx_buf, bin_header->size + 12, TOOLKIT_SEED);
        tx_ack(eq_rx_buf, bin_info.len + 17);
        bin_info.len = 0;
    }
#if TRACE_EN
    TRACE("TOOL_GET_BIN : %d %%\n", ((effect_res_len_get(EFFECT_IDX_MAX_NB) - bin_info.len)*100)/(effect_res_len_get(EFFECT_IDX_MAX_NB)));
#endif
}

//在线调试与上位机交互处理相关
AT(.text.toolkit) WEAK
void toolkit_process(void)
{
//    print_r(eq_rx_buf, EQ_BUFFER_LEN);
    u16 size = little_endian_read_16(eq_rx_buf, 12);
    u32 cal_crc = calc_crc(eq_rx_buf, size+XTP_EFFECT_FRAME_HEAD_TAG, 0xffff);
    u32 res_crc = little_endian_read_16(eq_rx_buf, size+XTP_EFFECT_FRAME_HEAD_TAG);

    //ACK 上位机应答
    toolkit_ack = 0;
    memset(toolkit_tx_ack_buf, 0, 14);
    memcpy(toolkit_tx_ack_buf, eq_rx_buf, 12);

    do {
        if (cal_crc != res_crc) {
            printf("-->CRC_ERROR %x %x\n", res_crc, cal_crc);
            toolkit_ack = 1;           //crc校验错误
            break;
        }

        if (0 == memcmp((char *)&eq_rx_buf[0], "CFG_########", 12)) {       //全部发送，检查链路是否一致
            if ((u32)little_endian_read_16(eq_rx_buf, 14) != ALL_MODE_NAME_SUM) {
                toolkit_ack = 1;       //与上位机链路不匹配
            }
            break;
        } else if (0 == memcmp((char*)&eq_rx_buf[0], "CFG#########", 12)) {     //特殊指令
            toolkit_cmd_process();
            return;
        } else if (0 == memcmp((char*)&eq_rx_buf[0], "CFG_BIN#####", 12)) {     //回读BIN文件
            toolkit_get_bin();
            return;
        }

        //遍历所有模块并调用callback更新制定模块
        for(int i = 0; i < CFG_MAX; i++){
            if (0 == memcmp((char *)&eq_rx_buf[0],effect_info[i].effect_cfg_name, 12)) {
//                while(music_effect_is_busy()) {
//                }

                printf("%s Online\n",effect_info[i].effect_cfg_name);
                if(effect_update_callback_tbl[i].effect_update_callback) {
                    effect_update_callback_tbl[i].effect_update_callback(eq_rx_buf,little_endian_read_16(eq_rx_buf, 12)+14,1);
                } else {
                    TRACE("%s NO INIT CALLBACK\n",effect_info[i].effect_cfg_name);
                }
                break;
            }

            if (i == CFG_MAX-1) {
                toolkit_ack = 1;       //名称错误，没有对应模块
            }
        }

    } while(0);


    toolkit_tx_ack_buf[12] = toolkit_ack;
    toolkit_tx_ack_buf[13] = toolkit_calc_check_sum(toolkit_tx_ack_buf, 13);

    tx_ack(toolkit_tx_ack_buf, 14);

    memset(eq_rx_buf, 0, EQ_BUFFER_LEN);
}

AT(.text.effect_adjust_by_tool_init)WEAK
void toolkit_init(void)
{
}

AT(.text.effect_adjust_by_tool_exit)WEAK
void toolkit_exit(void)
{

}
#endif
