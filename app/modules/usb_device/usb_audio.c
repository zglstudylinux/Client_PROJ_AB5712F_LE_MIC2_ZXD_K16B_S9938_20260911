#include "include.h"
#include "usb_audio.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

usb_mic_in_cfg_t usb_mic_in_cfg;
u8 src_in[WIRELESS_MIC_SAMPLES_SELECT*2] AT(.buf.src);


AT(.usbdev.com.table)
const u8 desc_usb_device[18] = {
    18,                 // Num bytes of the descriptor
    1,                  // Device Descriptor type
    0x10,               // Revision of USB Spec. LSB (in BCD)
    0x01,               // Revision of USB Spec. MSB (in BCD)
    0,                  // Mass Storage Class
    0,                  // General Mass Storage Sub Class
    0,                  // Do not use Class specific protocol
    64,                 // Max packet size of Endpoint 0

    //jl:e3b4
    0xF8,               // Vendor ID LSB
    0x2E,               // Vendor ID MSB
    0x55,               // Product ID LSB
    0x41,               // Product ID MSB
    0x00,               // Device Revision LSB (in BCD)
    0x01,               // Device Revision MSB (in BCD)

    STR_MANUFACTURER,                  // Index of Manufacture string descriptor
    STR_PRODUCT,                  // Index of Product string descriptor
    STR_SERIAL_NUM,                  // Index of Serial No. string descriptor
    1                   // Num Configurations, Must be 1
//    18,                 // Num bytes of the descriptor
//    1,                  // Device Descriptor type
//    0x01,               // Revision of USB Spec. LSB (in BCD)
//    0x02,               // Revision of USB Spec. MSB (in BCD)
//    0xEF,               // Misecllaneous Class
//    0x02,               // Interface Association Sub Class
//    0x01,               // Do not use Class specific protocol
//    USB_CTRL_SIZE,      // Max packet size of Endpoint 0
//
//    0xAC,               // Vendor ID LSB
//    0x05,               // Vendor ID MSB
//    0x0B,               // Product ID LSB
//    0x11,               // Product ID MSB
//    0x81,               // Device Revision LSB (in BCD)
//    0x26,               // Device Revision MSB (in BCD)
//
//    STR_MANUFACTURER,   // Index of Manufacture string descriptor
//    STR_PRODUCT,        // Index of Product string descriptor
//    STR_SERIAL_NUM,     // Index of Serial No. string descriptor
//    CFG_DESC_MAX,       // Num Configurations
};


#if ADAPTER_USB_MIC_RX_EN && ADAPTER_USB_MIC_STEREO_EN
s16 usb_mic_stereo[WIRELESS_MIC_SAMPLES_SELECT*2] AT(.buf.usb_stereo);
u8 usb_mic_channel = 2;

AT(.com_text.bsp.usb)
void usb_mic_adj_src(void)
{
    s8 new_speed;

    ///通过usb数据方式来调整src，后期需要把src处理函数放到对应模块里统计
    if (!usb_mic_in_cfg.mic_start) {
        usb_mic_in_cfg.speed = 10;
        usb_mic_in_cfg.input_min = -1;
        return;
    }

    u16 usbmic_len = usbmic_len_get()/2;

    u16 input_min = usb_mic_in_cfg.input_min;
    u16 input_cnt = usb_mic_in_cfg.input_cnt;

    input_cnt++;
    if(usbmic_len < input_min) {    //寻找min位置，开始计数
        input_min = usbmic_len;
        input_cnt = 0;
    } else if(input_cnt >= 8) {     //两个周期后，从当前值开始找min
        input_min = usbmic_len;
        input_cnt = 0;
    }

    if(input_cnt == 0) {
        usb_mic_in_cfg.input_min1 = input_min;
    }
    usb_mic_in_cfg.input_min = input_min;
    usb_mic_in_cfg.input_cnt = input_cnt;
    if(input_cnt < 4) {
        return;
    }

    uint input_min1 = usb_mic_in_cfg.input_min1;
    uint usbmic_min = (input_min<input_min1)? input_min : input_min1;

    if (usbmic_min <= 96) {
        new_speed = -1;
    } else if (usbmic_min > (48+192)) {
        new_speed = 1;
    } else {
        new_speed = 0;
    }

    if(new_speed != usb_mic_in_cfg.speed) {
        usb_mic_in_cfg.speed = new_speed;
        src1_adjust_speed(new_speed);
    }

//    static u32 ticks = 0;
//    if(tick_check_expire(ticks, 1000)) {
//        ticks = tick_get();
//        printf("@,%d\n", usbmic_len);
//    } else if(usbmic_min < 48) {
//        printf("#,%d\n", usbmic_len);
//    }

}
#endif

AT(.com_text.bsp.usb)
void ude_set_mic_status_cb(u8 start)
{
   usb_mic_in_cfg.mic_start = start;
}

AT(.com_text.usb_adj)
void adj_usb_src(void)
{
    if (!usb_mic_in_cfg.mic_start ) {
        return;
    }
    u16 usbmic_len = usbmic_len_get();
//    if(usbmic_len > 800 || usbmic_len < 300){
//        my_printf(jiexiu2,usbmic_len);
//    }
    if (usbmic_len < 196) {
//        printf("@,%d\n", usbmic_len);
        src_phase_comp_set(-51);   ///多1/120
    } else if (usbmic_len > 1500) {
//        printf("#,%d\n", usbmic_len);
        src_phase_comp_set(51);     ///少1/120
    }else  {
        src_phase_comp_set(0);
    }
}

AT(.com_text.usb_mic_in)
void usb_mic_in_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
#if ADAPTER_USB_MIC_STEREO_EN
    samples = samples*2;
    usb_mic_adj_src();
#else
    memcpy(src_in,ptr,samples*2);
    samples = src_process(src_in, ptr, samples);
    adj_usb_src();
#endif
    if (usb_mic_in_cfg.mic_start && !usb_mic_in_cfg.mute) {
        usbmic_sdadc_process(ptr, samples, ch_mode);
    }

    if (usb_mic_in_cfg.callback) {
        usb_mic_in_cfg.callback(ptr, samples, ch_mode, params);
    }
}

AT(.com_text.usb_mic_in)
void usb_mic_in_audio_output_callback_set(audio_callback_t callback)
{
    usb_mic_in_cfg.callback = callback;
}

AT(.text.usb_mic_in)
void usb_mic_in_audio_mute_set(u8 mute)
{
    usb_mic_in_cfg.mute = mute;
}

AT(.text.usb_mic_in)
void usb_mic_in_init(u8 sample_rate, u16 samples, u8 channel)
{
    ///还需要lock一下src的代码，先不处理
    memset(&usb_mic_in_cfg, 0, sizeof(usb_mic_in_cfg));
    soft_src_init(2,0);
}




usb_effect_t usb_effect AT(.udev_buf.efc);

AT(.text.usb.audio)
void usb_mic_rm_all(void)
{

}

AT(.text.usb.audio)
void usb_mic_mode_set(u8 mode)
{
    if((mode < 0) || (mode > MIC_MODE_NUM)){
        return;
    }
    usb_mic_rm_all();
    switch(mode){
        case MIC_NORMAL:
            TRACE("MIC_NORMAL\n");
            break;
        case MIC_KTV:
            TRACE("MIC_KTV\n");
            break;
        case MIC_ELEC:
            TRACE("MIC_ELEC\n");
            break;
        case MIC_PITCH:
            TRACE("MIC_PITCH\n");
            break;
        default:
            break;
    }
}

AT(.text.usb.audio)
void usb_mic_mode_switch(u8 direction)
{
    if(direction){
        usb_effect.mic_mode++;
        if(usb_effect.mic_mode >= MIC_MODE_NUM){
            usb_effect.mic_mode = 0;
        }
    }else{
        usb_effect.mic_mode--;
        if((usb_effect.mic_mode < 0) || (usb_effect.mic_mode >= MIC_MODE_NUM)){
            usb_effect.mic_mode = (MIC_MODE_NUM - 1);
        }
    }
    usb_mic_mode_set(usb_effect.mic_mode);
}

AT(.text.usb.audio)
void usb_dac_rm_all(void)
{

}

AT(.text.usb.audio)
void usb_dac_mode_set(u8 mode)
{
    if((mode < 0) || (mode >= DAC_MODE_NUM)){
        return;
    }
    usb_dac_rm_all();
    switch(mode){
        case DAC_NORMAL:
            TRACE("DAC_NORMAL\n");
            break;

        case DAC_3D:
            TRACE("DAC_3D\n");
            break;

        case DAC_BASS:
            TRACE("DAC_BASS\n");
            break;

        case DAC_VBASS:
            TRACE("DAC_VBASS\n");
            break;

        case DAC_CHOURS:
            TRACE("DAC_CHOURS\n");
            break;

        case DAC_PINGPONG:
            TRACE("DAC_PINGPONG\n");
            break;

        case DAC_AUTOWAH:
            TRACE("DAC_AUTOWAH\n");
            break;

        case DAC_VOCAL_REMOVER:
            TRACE("DAC_VOCAL_REMOVER\n");
            break;

    }
}

AT(.text.usb.audio)
void usb_dac_mode_switch(u8 direction)
{
    if(usb_effect.dac_mode < 2){
        usb_effect.dac_mode = 2;
    }
    if(direction){
        usb_effect.dac_mode++;
        if(usb_effect.dac_mode >= DAC_MODE_NUM * 2){
            usb_effect.dac_mode = 2;
        }
    }else{
        usb_effect.dac_mode--;
        if(usb_effect.dac_mode < 2){
            usb_effect.dac_mode = (DAC_MODE_NUM * 2 - 1);
        }
    }
    TRACE("usb_effect.dac_mode: %d\n", usb_effect.dac_mode);
    usb_dac_mode_set(usb_effect.dac_mode % 2 == 0 ? 0 : usb_effect.dac_mode / 2);
}

//USB下行数据接口，data数据传入，len返回底层长度(用于推DAC)
//host out 每1ms样点->48k:192 byte，44.1k:176 byte，22.05k:88 byte，11.025k:44 byte，8k:32 byte
AT(.usbdev.com)
void ude_rx_data(u8 *data, u32 *len)
{
#if 0
    u32 i = 0;
    u32 temp_len = *len;
    for (i = 0; i < temp_len; i++) {
       my_spi_putc(data[i]);
    }
#endif
#if 0
    u32 temp_len = *len;
    dump_putbuf(data[i], temp_len, 0);
#endif
}

//USB上行数据接口，data数据传入，len返回底层长度(用于上行主机)
//host in 每1ms样点->96K:192byte, 48k:96 bytes
AT(.usbdev.com)
void ude_tx_data(u8 *data, int *len)
{
#if 0
    u32 i = 0;
    int temp_len = *len;
    for (i = 0; i < temp_len; i++) {
       my_spi_putc(data[i]);
    }
#endif
#if 0
    int temp_len = *len;
    dump_putbuf(data[i], temp_len, 0);
#endif
}

AT(.text.usb.audio)
void usb_effect_init(void)
{
    usb_effect.mic_mode = MIC_NORMAL;
    usb_effect.dac_mode = DAC_NORMAL;
    usb_mic_mode_set(usb_effect.mic_mode);
    usb_dac_mode_set(2 * usb_effect.dac_mode);
}
