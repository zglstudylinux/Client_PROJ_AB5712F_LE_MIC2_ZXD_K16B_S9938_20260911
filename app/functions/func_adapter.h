#ifndef __FUNC_ADAPTER_H
#define __FUNC_ADAPTER_H


void func_adapter(void);
void func_adapter_init(void);
void func_adapter_set_create_con(void);
void wireless_mic_rec_conn_process(void);

///extern api
void func_adapter_message(u16 msg);

typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *param);

///外部usb audio out api
void usb_audio_out_audio_output_callback_set(audio_callback_t callback);
void usb_audio_out_init(u8 sample_rate, u16 samples, u8 channel);

void i2s_audio_out_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void i2s_audio_output_callback_set(audio_callback_t callback);
void i2s_audio_out_init(u8 sample_rate, u16 samples, u8 channel);

void usbchk_only_device(void);
#endif
