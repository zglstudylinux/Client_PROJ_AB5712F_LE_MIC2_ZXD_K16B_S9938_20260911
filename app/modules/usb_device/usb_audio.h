#ifndef _USB_AUDIO_H
#define _USB_AUDIO_H

typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *params);

typedef struct {
    u8  mic_start;
    u8  speed;
    u8  input_cnt;
    u16 input_min;
    u16 input_min1;

    u8 mute;
    u8 sample_rate;
    u16 samples;
    audio_callback_t callback;
} usb_mic_in_cfg_t;

extern usb_mic_in_cfg_t usb_mic_in_cfg;
void usb_mic_in_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void usb_mic_in_audio_output_callback_set(audio_callback_t callback);
void usb_mic_in_audio_mute_set(u8 mute);
void usb_mic_in_init(u8 sample_rate, u16 samples, u8 channel);

void usbmic_sdadc_process(u8 *ptr, u32 samples, int ch_mode);


typedef enum{
    MIC_NORMAL,     //原声
    MIC_KTV,        //Reverb/Echo
    MIC_ELEC,       //电音
    MIC_PITCH,      //魔音
    MIC_MODE_NUM,
}mic_mode_t;

typedef enum{
    DAC_NORMAL,        //原声
    DAC_3D,            //3D音效
    DAC_BASS,          //音乐高低音
    DAC_VBASS,         //虚拟低音
    DAC_CHOURS,        //合唱
    DAC_PINGPONG,      //乒乓效果
    DAC_AUTOWAH,       //哇音
    DAC_VOCAL_REMOVER, //人声消除
    DAC_MODE_NUM,
}dac_mode_t;

typedef struct{
    u8 mic_mode;
    s8 dac_mode;
}usb_effect_t;
#endif
