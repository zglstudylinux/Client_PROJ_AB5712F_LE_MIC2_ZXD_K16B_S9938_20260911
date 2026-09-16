#include "include.h"
#include "huart_audio_in_mix.h"
/*
 * 文件名称: huart_audio_in_mix.c
 * 功能描述: 本文件为HUART DMA 输出数字信号 处理模块
    注意当前芯片为8916A2 / 5632G 只有一组HUART
    开启本功能后，在线eq调试会失效哦~
 ****************************************************************************************
    code：
    buf:
    time :
 */
#if ADAPTER_HUART_INPUT_EN
#define HUART_AUDIO_BAUD                4000000          //haurt波特率
#define HUART_SAMPLES                   120*2            //16bit = 8bit * 2
#define HUART_TX_PORT                   HUART_TR_PA7     //huart发射引脚
#define HUART_RX_PORT                   HUART_TR_PE6     //huart接收引脚
#define HUART_IN_INFO_PRINT             0
#define PROCESS_OUT_SAMPLES             120                       //每次存取帧长


static huart_audio_in_cfg_t huart_audio_in_cfg;
static huart_rx_str_t huart_rx_str AT(.buf.huart_in);
static u8 huart_audio_in_incache[PROCESS_OUT_SAMPLES*2*4] AT(.buf.huart_in);
static u8 huart_audio_in_outcache[PROCESS_OUT_SAMPLES*2] AT(.buf.huart_in);

#if HUART_IN_INFO_PRINT
AT(.com_text.ains4)
const char huart_in_info[] = "HUART_IN samples = %d, isrcnt = %d (SR_%d)\n";
#endif

AT(.text.huart_in_proc.input)
void mic_mix_process_cb(u8 *ptr, u32 samples, int ch_mode, void *param)
{
#if HUART_IN_INFO_PRINT
    static u32 ticks = 0;
    static u32 isr_cnt = 0;
    isr_cnt++;
    if (tick_check_expire(ticks, 1000)) {
        my_printf(huart_in_info, samples, isr_cnt, samples*isr_cnt);
        isr_cnt = 0;
        ticks = tick_get();
    }
#endif

    if(huart_audio_in_cfg.mute) {
        memset(ptr, 0, HUART_RX_MAX_LEN);
        return;
    }

    huart_rx_str_t *huart = (huart_rx_str_t *)ptr;
    u8 *pcm = huart->rx_buf;

#if ADAPTER_HUART_COMMAND_EN
    if (huart->rx_type == TX_TYPE_AUDIO_CMD) {            //解析audio后面附带的命令
        for (u8 i = 0; i < UART_CMMAND_SIZE; i++) {
            uart_command_rx_callback(huart->rx_buf[HUART_SAMPLES + i]);
        }
    }
#endif

    if(!wireless_cb.connected_sta && huart_audio_in_cfg.kick_flag) {
        huart_audio_in_cfg.kick_flag = 0;
    }
    if(huart_audio_in_cfg.kick_flag) {
        cbuf_input_audio(pcm, samples, &huart_audio_in_cfg.huart_audio_in_cbuf);
    } else {
        if (huart_audio_in_cfg.callback) {
            huart_audio_in_cfg.callback(pcm, samples, ch_mode, param);
        }
    }
}

AT(.text.huart_in_proc.input)
void huart_audio_in_input(u8 *ptr, u32 samples, int ch_mode, void *param)
{
    s16 *rptr = (s16 *)ptr;
    uint frame_size = samples;
    uint total_size = cbuf_total_samples_get(&huart_audio_in_cfg.huart_audio_in_cbuf);

    if(!huart_audio_in_cfg.kick_flag) {
        huart_audio_in_cfg.kick_flag = 1;
    }
    if(total_size >= frame_size) {
        cbuf_output_audio((u8 *)huart_audio_in_outcache, samples, &huart_audio_in_cfg.huart_audio_in_cbuf);
        s16 *pcm_mic = (s16 *)huart_audio_in_outcache;
        s16 *pcm_16 = (s16 *)ptr;
        s32 sample = 0;
        for (u8 i = 0; i < samples; i++) {
            sample = pcm_16[i] + pcm_mic[i];
            if (sample > 32767) {
                pcm_16[i] = 32767;
            } else if (sample < -32767){
                pcm_16[i] = -32767;
            } else {
                pcm_16[i] = sample;
            }
        }
    }

    if (huart_audio_in_cfg.callback) {
        huart_audio_in_cfg.callback((u8 *)rptr, samples, ch_mode, param);
    }
}

AT(.text.huart_set.callback)
void huart_audio_in_output_callback_set(audio_callback_t callback)
{
    huart_audio_in_cfg.callback = callback;
}

AT(.text.huart_set.mute)
void huart_audio_in_mute_set(uint8_t mute)
{
    huart_audio_in_cfg.mute = mute;
}

AT(.text.huart_init)
void huart_audio_in_init(u8 sample_rate, u16 samples)
{
#if ADAPTER_HUART_COMMAND_EN
    uart_command_init();
#endif

    memset(&huart_audio_in_cfg, 0, sizeof(huart_audio_in_cfg));
    cbuf_init(&(huart_audio_in_cfg.huart_audio_in_cbuf), huart_audio_in_incache, PROCESS_OUT_SAMPLES*2*4);

    huart_init_do(HUART_TX_PORT, HUART_RX_PORT, HUART_AUDIO_BAUD, (u8 *)huart_rx_str.rx_buf, HUART_RX_MAX_LEN, 1, 1);
}

AT(.text.huart_exit)
void huart_audio_in_exit(void)
{

}
#endif // I2S_TX_AUDIO_EN

