#include "include.h"
#include "huart_audio_out.h"
/*
 * 文件名称: huart_audio_out.c
 * 功能描述: 本文件为HUART DMA 输出数字信号 处理模块
             开启本功能后，在线eq调试会失效
 */
#if ADAPTER_HUART_OUTPUT_EN
#define HUART_AUDIO_BAUD                4000000          //haurt波特率
#define HUART_SAMPLES                   120*2            //16bit = 8bit * 2
#define HUART_TX_PORT                   HUART_TR_PA7     //huart发射引脚
#define HUART_RX_PORT                   HUART_TR_PE6     //huart接收引脚

static huart_tx_str_t huart_tx_str  AT(.data.huart_cb);
static huart_audio_out_cfg_t huart_audio_out_cfg;
static bool huart_audio_out_flag  = false;
u8 audio_tx_buf[HUART_SAMPLES];

AT(.com_text.huart)
bool huart_role_audio_out(void)
{
    return huart_audio_out_flag;
}

//当断开连接时,可用该接口发送命令
AT(.text.huart_proc.input)
void huart_cmd_send(void)
{
    //获取uart_command是否有命令要发
    if (uart_command_tx_value_get()) {
        memset(huart_tx_str.tx_buf, 0, HUART_SAMPLES);
        huart_tx_str.tx_buf_len = HUART_SAMPLES + 1;
        u8 uart_command_buf_len = uart_command_buf_len_get();
        huart_tx_str.tx_type = TX_TYPE_AUDIO_CMD;
        memcpy(huart_tx_str.tx_buf + huart_tx_str.tx_buf_len-1, uart_command_buf_get(), uart_command_buf_len);
        huart_tx_str.tx_buf_len += (uart_command_buf_len);
        huart_tx((u8 *)&huart_tx_str, huart_tx_str.tx_buf_len);
    }
}
AT(.text.huart_proc.input)
void huart_audio_out_input(u8 *ptr, u32 samples, int ch_mode, void *param)
{
    if(!huart_audio_out_cfg.mute){
        huart_tx_str.tx_type = TX_TYPE_AUDIO;
        memcpy(huart_tx_str.tx_buf, ptr, samples * 2);
        huart_tx_str.tx_buf_len = samples * 2 + 1;
#if ADAPTER_HUART_COMMAND_EN
        if (uart_command_tx_value_get()) {              //获取uart_command是否有命令要发
            u8 uart_command_buf_len = uart_command_buf_len_get();
            huart_tx_str.tx_type = TX_TYPE_AUDIO_CMD;
            memcpy(huart_tx_str.tx_buf + huart_tx_str.tx_buf_len - 1, uart_command_buf_get(), uart_command_buf_len);
            huart_tx_str.tx_buf_len += (uart_command_buf_len);
        }
#endif

        huart_tx((u8 *)&huart_tx_str, huart_tx_str.tx_buf_len);
    }

    if (huart_audio_out_cfg.callback) {
        huart_audio_out_cfg.callback(ptr, samples, ch_mode, param);
    }
}

AT(.text.huart_set.callback)
void huart_audio_out_output_callback_set(audio_callback_t callback)
{
    huart_audio_out_cfg.callback = callback;
}

AT(.text.huart_set.mute)
void huart_audio_out_mute_set(uint8_t mute)
{
    huart_audio_out_cfg.mute = mute;
}

AT(.text.huart_init)
void huart_audio_out_init(u8 sample_rate, u16 samples)
{
    memset(&huart_audio_out_cfg,0,sizeof(huart_audio_out_cfg));

#if ADAPTER_HUART_COMMAND_EN
    uart_command_init();
#endif
    if((HUART_TX_PORT == HUART_TR_VUSB)){
         if(!sys_cb.vusb_uart_flag){
            PWRCON0 |= BIT(30);                             //Enable VUSB GPIO
#if (CHARGE_BOX_INTF_SEL == INTF_HUART) || (QTEST_EN /*&& (TEST_INTF_SEL == INTF_HUART)*/)
            baud_rate = 9600;
#endif
            xcfg_cb.chg_inbox_pwrdwn_en=0;                  //入仓耳机关机
            sys_cb.vusb_uart_flag = 1;
         } else {
             return;
         }
    }

    huart_audio_out_flag = true;
    huart_init_do(HUART_TX_PORT, HUART_RX_PORT, HUART_AUDIO_BAUD, NULL, 0, 0, 1);

}

AT(.text.huart_exit)
void huart_audio_out_exit(void)
{

}
#else
AT(.com_text.huart)
bool huart_role_audio_out(void)
{
    return 0;
}
#endif // I2S_TX_AUDIO_EN

