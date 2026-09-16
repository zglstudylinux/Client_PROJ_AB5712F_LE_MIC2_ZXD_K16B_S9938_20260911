#ifndef __API_EQ_DRC_H
#define __API_EQ_DRC_H


//DRC初始化相关结构体
typedef struct {
    int threshold;
    int gain;
    int slope;
} drc_v3_coeff_t;

typedef struct {
    //预置值
    int in_attack;
    int in_release_his;
    int out_attack;
    int out_release;
    int in_tav;
    drc_v3_coeff_t coeffs[4];

    //中间值
    int in_level;
    int in_level_r;
    int in_level_l;
    int out_gain;
} drc_v3_cb_t;

//EQ
struct eq_coef_tbl_t {
    const int * coef_0;            //tbl_alpha
    const int * coef_1;            //tbl_cos_w0
};

void eq_var_init(void);
void bass_treble_coef_cal(void *eq_coef, int gain, int mode);                //gain:-12dB~12dB, mode:0(bass), 1(treble)
void eq_coef_cal(void *eq_coef, int gain);


//mix drc
bool drc_v3_init(const void *bin, int bin_size, drc_v3_cb_t *drc_cb); //软件drc初始化（通过资源文件）
s32 drc_v3_calc(s32 sample, drc_v3_cb_t *drc_cb, u8 is_24bit_en);     //软件drc pcm数据处理（每次1个样点）
bool drc_v3_init_toolkit(const void *bin, int bin_size, drc_v3_cb_t *drc_cb); //软件drc初始化（通过toolkit资源文件）

//mic eq/drc
void mic_eq_drc_cfg(bool is_24bits);                                //配置eq_drc模式
void mic_eq_drc_proc(u16 *input, u16 *output, u16 samples);         //eq_drc pcm数据处理（需要先设置eq参数才能启用）
void mic_eq_drc_set_gain(u32 gain);                                 //设置eq_drc前置增益
void mic_eq_drc_off(void);                                          //关闭eq_drc
void mic_eq_set_by_param(u8 band_cnt, const u32 *eq_param);         //单独设置eq参数，设置后才使能eq（通过EQ工具）
bool mic_eq_set_by_res(u32 addr, u32 len);                          //单独设置eq参数，设置后才使能eq（通过资源文件）
bool mic_eq_set_is_done(void);                                      //判断上一次设置EQ是否完成，1：已完成，0=处理中
void mic_eq_off(void);                                              //单独关闭eq功能
void mic_drc_set_by_param(u8 band_cnt, const u32 *drc_param);       //单独设置drc参数，设置后才使能drc（通过EQ工具）
bool mic_drc_set_by_res(u32 addr, u32 len);                         //单独设置drc参数，设置后才使能drc（通过资源文件）
void mic_drc_off(void);                                             //单独关闭drc功能

//dac eq/drc
bool music_set_eq_by_res(u32 addr, u32 len);
void music_set_eq_by_num(u8 num);
void music_eq_off(void);
void music_set_eq(u8 band_cnt, const u32 *eq_param);
void music_set_eq_gain(u32 gain);
bool music_set_eq_is_done(void);    //判断上一次设置EQ是否完成，1：已完成
bool music_set_drc_by_res(u32 addr, u32 len);
void music_set_drc(u8 band_cnt, const u32 *drc_param);
void music_drc_on(void);            //使能music drc
void music_drc_off(void);           //关闭music drc
bool music_eq_divband_init(u8 ch, u8 div_band, u32 *param);
void music_eq_divband_exit(void);



#endif // __API_EQ_DRC_H
