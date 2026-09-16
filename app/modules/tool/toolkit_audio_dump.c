#include "include.h"
#include "toolkit_audio_dump.h"
#include "api.h"

#if TOOLKIT_AUDIO_DUMP_EN

#define DUMP_FRAME_LEN		             480                      //dump数据最大支持帧长
#define DUMP_BAUD		                 8000000                  //dump数据波特率配置, 受限于逻辑分析仪
static u8 toolkit_btmic_dump_buf_temp[2 * DUMP_FRAME_LEN*4] AT(.bss); //24bit 双通道
static btmic_dump_cfg_t btmic_dump_cfg;

static void bsp_toolkit_btmic_audio_dump_huart_init(void)
{
    huart_t huart0;
    memset(&huart0, 0x00, sizeof(huart0));
    huart0.rx_port = HUART_TR_PB3;        //HUART串口选择: PA15: 0, PB2: 1, PB3: 2, PB4: 3, PB15: 4, PE10: 5, PE13: 6, VUSB: 7
    huart0.tx_port = HUART_TR_PB3;
    huart0.rxisr_en = 1;
    huart0.txisr_en = 1;
    huart0.rxbuf   = eq_rx_buf;
    huart0.rxbuf_size = EQ_BUFFER_LEN;
    huart_init(&huart0, DUMP_BAUD);
}

static void bsp_toolkit_btmic_audio_dump_type_init(void)
{
    u16 data_type = 0;
    data_type |= BIT(UNI_DUMP_MIC_EFFECT_DATA_MIC);
    data_type |= BIT(UNI_DUMP_MIC_EFFECT_DATA_MIC_ALG);
//    toolkit_btmic_audio_dump_init(data_type, UNI_DUMP_PCM_BITS_24, UNI_DUMP_PCM_SPR_48000, 0);
    toolkit_btmic_audio_dump_init(data_type, UNI_DUMP_PCM_BITS_16, UNI_DUMP_PCM_SPR_48000, 0);
}

void audio_dump_init(void)
{
    printf("%s\n", __func__);
    btmic_dump_cfg.mute = 1;
    memset(toolkit_btmic_dump_buf_temp, 0, sizeof(toolkit_btmic_dump_buf_temp));

    bsp_toolkit_btmic_audio_dump_huart_init();
    bsp_toolkit_btmic_audio_dump_type_init();
}

//params 0：填充左声道 1:填充右声道 ，需要成双成对使用
AT(.com_huart.text)
void audio_dump_input(void *ptr, u32 samples, u32 params, u8 is_24bits)
{
    s32 *rptr_24bit = (s32 *)ptr;
    u8  *rptr_24bit_dump = &toolkit_btmic_dump_buf_temp[0];

    s16 *rptr_16bit = (s16 *)ptr;
    s16 *rptr_16bit_dump = (s16 *)&toolkit_btmic_dump_buf_temp[0];

    if (btmic_dump_cfg.mute) {
        return;
    }

    if (is_24bits) {//24bit
        if (params == 0) {
            //加个huart判断，避免发送的数据被意外改写
//            huart_wait_txdone();
            //原始数据放左通道
            for (u16 i=0; i<samples; i++) {
                rptr_24bit_dump[i*6 + 0] = (rptr_24bit[i] >>0)&0xff;
                rptr_24bit_dump[i*6 + 1] = (rptr_24bit[i] >>8)&0xff;
                rptr_24bit_dump[i*6 + 2] = (rptr_24bit[i] >>16)&0xff;
            }
//            huart_tx(&toolkit_btmic_dump_buf_temp[0], samples*6);
//            //kick硬件huart数据dump， 实际上是当前数据的左声道+上一帧数据的右声道
            toolkit_btmic_audio_dump_process(&toolkit_btmic_dump_buf_temp[0], samples);
        } else if (params == 1) {
            //加个huart判断，避免发送的数据被意外改写
            huart_wait_txdone();
            //算法后数据放右通道
            for (u16 i=0; i<samples; i++) {
                rptr_24bit_dump[i*6 + 3] = (rptr_24bit[i] >>0)&0xff;
                rptr_24bit_dump[i*6 + 4] = (rptr_24bit[i] >>8)&0xff;
                rptr_24bit_dump[i*6 + 5] = (rptr_24bit[i] >>16)&0xff;
            }
//            huart_tx(&toolkit_btmic_dump_buf_temp[0], samples*6);
//            toolkit_btmic_audio_dump_process(&toolkit_btmic_dump_buf_temp[0], samples);
        }
    } else {//16bit
        if (params == 0) {
            //原始数据放左通道
            for(u16 i=0; i<samples; i++) {
                rptr_16bit_dump[i*2 + 0] = rptr_16bit[i];
            }
            //kick硬件huart数据dump， 实际上是当前数据的左声道+上一帧数据的右声道
            toolkit_btmic_audio_dump_process(&toolkit_btmic_dump_buf_temp[0], samples);
        } else if (params == 1) {
            //加个huart判断，避免发送的数据被意外改写
            huart_wait_txdone();
            //算法后数据放右通道
            for (u16 i=0; i<samples; i++) {
                rptr_16bit_dump[i*2 + 1] = rptr_16bit[i];
            }
        }
    }

}

void bsp_btmic_dump_parse_cmd(void)
{
    u32 res_crc, cal_crc;
    print_r(eq_rx_buf, eq_rx_buf[5]);
    u16 size = eq_rx_buf[5];
    u8 tool_ack = 0;
    u8 cmd_id = eq_rx_buf[7];
    printf("%s 0x%x\n", __func__, cmd_id);
    res_crc = little_endian_read_16(eq_rx_buf, size-2);
    cal_crc = calc_crc(eq_rx_buf, size - 2, 0xffff);
    do {
        if(cal_crc != res_crc) {
            printf("CRC_ERROR\n", cal_crc, res_crc);
            tool_ack = 1;           //crc校验错误
            break;
        }
    } while(0);

    //[0-4]头部不动
    eq_rx_buf[5]  = 12;        //len = 12
//    eq_rx_buf[6]  = 0x1;       //协议ID
//    eq_rx_buf[7]  = 0x1;       //指令ID
//    eq_rx_buf[8]  = 0;         //指令ID

    eq_rx_buf[9]  = tool_ack;  //ACK 0:成功  1：错误

    cal_crc = calc_crc(eq_rx_buf, 12 - 2, 0xffff);
    eq_rx_buf[10] = (u8)cal_crc;
    eq_rx_buf[11] = (u8)(cal_crc >> 8);

    huart_wait_txdone();
    tx_ack(eq_rx_buf, 12);

    if (!tool_ack) {
        if (cmd_id == BTMIC_DUMP_START_CMD) {
            btmic_dump_cfg.mute = 0;
        } else if (cmd_id == BTMIC_DUMP_STOP_CMD) {
            btmic_dump_cfg.mute = 1;
            bsp_toolkit_btmic_audio_dump_type_init();
        }
    }
}

#endif

