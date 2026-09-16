#ifndef _API_SDADC_H
#define _API_SDADC_H

#define CHANNEL_L           0x00ff
#define CHANNEL_R           0xff00

#define ADC2DAC_EN          0x01        //ADC-->DAC
#define ADC2SRC_EN          0x02        //ADC-->SRC
#define ADC2IRQ_EN          0x03        //ADC-->IRQ（测试用）
#define ADC2DIR_EN          0x04        //ADC-->DAC（测试用）
#define ADC2SANC_EN         0x05        //ADC-->SANC

typedef void (*pcm_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *params);

typedef struct {
    u16 channel;
    u8 sample_rate;
    u32 anl_gain;
    u32 dig_gain;
    u8 bits_mode;                       //ADC BITS选择；0: 24bits, 1: 16bits, 0xff: 跟随DAC的BIT MODE
    u8 out_ctrl;
    u16 samples;
    pcm_callback_t callback;
} sdadc_cfg_t;

enum {
    SPR_48000,
    SPR_44100,
    SPR_38000,
    SPR_32000,
    SPR_24000,
    SPR_22050,
    SPR_16000,
    SPR_12000,
    SPR_11025,
    SPR_8000,
    SPR_6000,
    SPR_4000,

    SPR_96000,
    SPR_88200,
    SPR_76000,
    SPR_64000,
    SPR_384000,
    SPR_352800,
    SPR_192000,
    SPR_176400,
};

void set_mic_analog_gain(u16 level, u16 channel);         //0~23(共24级), step 3DB (-6db ~ +63db)
void sdadc_set_digital_gain(u16 ch, u16 gain);
void sdadc_set_soft_gain(u16 ch, u32 gain);
void set_aux_analog_vol(u8 level, u8 auxlr_sel);
void set_aux_analog_gain(u32 gain, u16 channel);
void sdadc_dummy(u8 *ptr, u32 samples, int ch_mode);
void sdadc_pcm_2_dac(u8 *ptr, u32 samples, int ch_mode);
void sdadc_pcm_2_dac1(u8 *ptr, u32 samples, int ch_mode);
void sdadc_var_init(void);

int sdadc_init(const sdadc_cfg_t *p_cfg);
int sdadc_start(u16 channel);
int sdadc_exit(u16 channel);
void sdadc_start_kick(u16 channel);

bool sdadc_drc_init(u8 *drc_addr, int drc_len);

//mic
void mic_mute(void);
void mic_unmute(void);

//anc
typedef enum {
	MODE_TWS_FFFB,      //TWS耳机mic2(FF/FB)输入，DACL输出
	MODE_FFFB,          //非TWS mic2、3(FF/FB)输入，DACL/R输出(mic2->dacl，mic3->dacr)

	MODE_TWS_HYBRID = 8,//TWS耳机mic2、3(Hybrid)输入，DACL输出
	MODE_HYBRID,        //非TWS mic0、1、2、3(Hybrid)输入，DACL/R输出(mic01->dacl，mic23->dacr)
} ANC_MODE;

typedef struct {
    u8 drc_en           :1;
    u8 drc_after_eq     :1; //0：adc->drc->eq  1：adc->eq->drc
    u8 filter_en        :1;
    u8 filter_band;
    u8 rfu[2];
    const u32 *filter_coef;
    const u32 *drc_coef;
} anc_drc_t;

typedef struct {
	u32 nos_gain;
	const u32 *nos_param;
	u32 msc_gain;
	const u32 *msc_param;
	anc_drc_t drc;
    u8 mic_ch;
	u8 nos_band;            //ANC降噪eq条数
	u8 msc_band;            //ANC音乐补偿eq条数
    u8 bypass           :1; //ANC EQ Bypass，置1 EQ无效
    u8 msc_dly          :2; //music sample delay
} anc_channel_t;

struct anc_cfg_t {
	ANC_MODE anc_mode;
	u8 spr;
	u8 rfu[2];
	anc_channel_t *ch[4];
	u8 fade_in_step;        //淡入速度，step * 85ms(384k)
	u8 fade_out_step;       //淡出速度，step * 85ms(384k)
    u16 change_en       :1; //change是否使能
    u16 dc_rm           :1;
    u16 fade_en         :1; //淡入淡出使能
    u16 order           :2;
    u16 output_swap     :1; //DACLR互换输出
};

#define ANC_FIX_BIT         23
#define EQ_GAIN_MAX         0x07eca9cd      //24dB
#define EQ_GAIN_MIN         0x00081385      //-24dB
void anc_var_init(void);
void anc_init(void *cfg);
void anc_start(void);
void anc_stop(void);
void anc_exit(void);
bool anc_set_param(void);
void anc_set_eq_param_change(u8 ch);
u8 anc_set_param_is_busy(void);         //ret: BIT(0):lch is busy; BIT(1):rch is busy
void anc_set_tansparency_mode(u8 en);   //使能/关闭通透模式，默认关闭,调用后需要更新参数才起作用
bool anc_is_tansparency_mode(void);     //当前是否处于通透模式
void anc_mic_mute(u8 ch, u8 mute);
void anc_set_mic_gain(u8 ch, u8 anl, u8 gain);
u32 anc_pow10_cal(int index);           //10^(dB/20)*2^23, index = 10*dB, -12.0<dB<12.0
void anc_set_drc_param(void);

void ttp_init(u32 param);
void ttp_exit(void);

void sdadc_pdm_mic_init(u8 mapping);
void sdadc_pdm_mic_exit(u8 mapping);
void lefmic_zero_detect_do(u8 *ptr, u32 samples);

bool sdadcl_set_soft_eq_by_res(u32 addr, u32 len);
bool sdadcr_set_soft_eq_by_res(u32 addr, u32 len);

#endif //_API_SDADC_H
