#ifndef _API_WIRELESS_MIC_H
#define _API_WIRELESS_MIC_H

typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *param);
typedef void (*module_input_t)(u8 *ptr, u32 samples, int ch_mode, void *param);
typedef void (*module_init_t)(u8 sample_rate, u16 samples, u8 channel);
typedef void (*module_set_callback_t)(module_input_t callback);

//con_type, byte1
enum {
    //type, bit[3:0]
    CON_FLAG_T2R    = 0,     //T2R AUDIO
    CON_FLAG_RSVD0  = 1,
    CON_FLAG_RSVD1  = 2,
    CON_FLAG_NONE   = 3,
    CON_FLAG_RELAY  = 4,

    //flag, bit[7:4]
    CON_FLAG_SLAVE  = BIT(6),
//    CON_FLAG_TWS    = BIT(7),
    CON_FLAG_D2D_CON= BIT(7),
};

typedef enum{
    MIC_TYPE  = 1,
    ENCODER_PRIO_TRANS_TYPE,
    MIC_EQ_DRC_TYPE,
    SOFT_EQ_TYPE,
    MIC_ENC_TYPE,
    DAC0_OUT_TYPE,

    SPK_ENC_TYPE,
    SPK_DEC_TYPE,
    SPK_VOL_TYPE,

    DECODER_PRIO_TRANS_TYPE,
    MIC_DEC_TYPE,
    MIC_MIX_TYPE,
    MIC_DRC,
    SRC_TYPE,
    SRC_BUF_TYPE,

    DAC_OUT_TYPE,
    USB_MIC_IN_TYPE,
    USB_SPK_OUT_TYPE,
    ECHO_TYPE,
    MAGCI_PITCH_SHIFT_TYPE,
    AINS_TYPE,
    DNN_L1_TYPE,
    AINS4_TYPE,
    AGC_MIC_TYPE,
    DNN_L3_TYPE,
    HOWLING_NOTCH_TYPE,
    ROOM_REVERB_TYPE,
    PLAT_REVERB_TYPE,
    HOWLING_AIMASK_TYPE,
    HUART_TX_TYPE,
    HUART_RX_TYPE,
    I2S_OUT_TYPE,
    I2S_IN_TYPE,
    DNN_L1_1024FFT_TYPE,
    GTCRN_48K_TYPE,
    GTCRN_48K_QMF_TYPE,
    FREQ_SHIFT2_TYPE,
    ALLPASS_FILTER_TYPE,
    VOICE_CHANGE_V2_TYPE,
    YLCRN_L3_TYPE,
    YLCRN_L2_TYPE,
    PHASE_ROTATION_TYPE,
	GTCRN_QMF_HOWLING_TYPE,
	YLCRN_HOWLING_TYPE,
	DNR_FRE_TYPE,
	AEC_ALG_TYPE,
	YLCRN_16K_TYPE,
}MODULE_TYPE;

typedef struct {
    u8 module_type;
    u8 module_enable;
    module_init_t  module_init;
    module_input_t module_input;
    module_set_callback_t module_set_callback;
} ws_link_list_t;

void ws_link_list_init(ws_link_list_t *link_list, u8 module_count, u8 sample_rate, u16 samples, u8 channel);


//循环buffer api
typedef struct {
    volatile u16 len;
    u8 *obuf;
    u8 *wptr;
    u8 *rptr;
    u8 *init_buf;
    u8 enable;
    u16 buf_len;
} cbuf_cfg_t;

extern uint16_t cfg_wireless_d2a_dec_us;

void cbuf_init(cbuf_cfg_t *cbuf, u8 *buf, u16 buf_size);
void cbuf_input(u8 *ptr, u16 len, cbuf_cfg_t *emt);
bool cbuf_output(u8 *buf, u16 len, cbuf_cfg_t *emt);
u16  cbuf_total_len_get(cbuf_cfg_t *emt);

void cbuf_input_audio(u8 *ptr, u16 samples, cbuf_cfg_t *emt);
bool cbuf_output_audio(u8 *buf, u16 samples, cbuf_cfg_t *emt);
u16  cbuf_total_samples_get(cbuf_cfg_t *emt);

#endif // _API_WIRELESS_MIC_H
