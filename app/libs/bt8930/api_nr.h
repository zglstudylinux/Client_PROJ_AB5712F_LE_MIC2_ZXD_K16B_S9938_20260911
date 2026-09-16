#ifndef _API_NR_H
#define _API_NR_H

#define _MAX_GAIN                           (0x800000)
#define SOFT_DIG_P0DB                       (_MAX_GAIN * 1.000000)
#define SOFT_DIG_P1DB                     	(_MAX_GAIN * 1.122018)
#define SOFT_DIG_P2DB                     	(_MAX_GAIN * 1.258925)
#define SOFT_DIG_P3DB                     	(_MAX_GAIN * 1.412538)
#define SOFT_DIG_P4DB                     	(_MAX_GAIN * 1.584893)
#define SOFT_DIG_P5DB                     	(_MAX_GAIN * 1.778279)
#define SOFT_DIG_P6DB                     	(_MAX_GAIN * 1.995262)
#define SOFT_DIG_P7DB                     	(_MAX_GAIN * 2.238721)
#define SOFT_DIG_P8DB                     	(_MAX_GAIN * 2.511886)
#define SOFT_DIG_P9DB                     	(_MAX_GAIN * 2.818383)
#define SOFT_DIG_P10DB                    	(_MAX_GAIN * 3.162278)
#define SOFT_DIG_P11DB                    	(_MAX_GAIN * 3.548134)
#define SOFT_DIG_P12DB                    	(_MAX_GAIN * 3.981072)
#define SOFT_DIG_P13DB                    	(_MAX_GAIN * 4.466836)
#define SOFT_DIG_P14DB                    	(_MAX_GAIN * 5.011872)
#define SOFT_DIG_P15DB                    	(_MAX_GAIN * 5.623413)

#define DUMP_SNDP_TALK                      BIT(0)      //主MIC数据
#define DUMP_SNDP_FF                        BIT(1)      //副MIC数据
#define DUMP_SNDP_NR                        BIT(2)      //降噪后数据
#define DUMP_AEC_INPUT                      BIT(4)      //AEC输入数据
#define DUMP_AEC_FAR                        BIT(5)      //AEC远端数据
#define DUMP_AEC_OUTPUT                     BIT(6)      //AEC输出数据
#define DUMP_FAR_NR_INPUT                   BIT(8)      //远端降噪输入数据
#define DUMP_FAR_NR_OUTPUT                  BIT(9)      //远端降噪输出数据
#define DUMP_EQ_OUTPUT                      BIT(10)     //MIC EQ输出数据

#define DUMP_SNDP_MASK                      0x000f
#define DUMP_AEC_MASK                       0x00f0

#define NR_CFG_TYPE_MASK                    0x1F

//call param
enum NR_TYPE {
    NR_TYPE_NONE            = 0,
    NR_TYPE_AINS,
    NR_TYPE_SNDP_SM,
    NR_TYPE_SNDP_DM,
    NR_TYPE_SNDP_FBDM,
    NR_TYPE_SMIC_AI,                            //自研单MIC AI降噪算法
    NR_TYPE_DMIC_AI,                            //NR_TYPE_DMDNN
    NR_TYPE_USER,
};

enum AEC_TYPE {
    AEC_TYPE_NONE           = 0,
    AEC_TYPE_DEFAULT,                           //默认硬件频域AEC
};

enum AEC_NR_MODE {
    AEC_NR_MODE0            = 0,                //AEC(线性+非线性) + NR
    AEC_NR_MODE1,                               //AEC(线性) + NR + AEC(非线性)
    AEC_NR_MODE2,                               //NR + AEC(线性) + AEC(非线性)
};

typedef struct {
    u8 level;                                   //稳态最大降噪量，范围0~39dB
    u8 dnn_level;                               //dnn最大降噪量，范围0~39dB
    u8 param_printf;
    u16 coef_len;                               //声加配置长度，0：使用自定义参数；非0：使用声加工具配置（从资源读取）
    const u32 *coef;                            //声加配置资源，非0：使用声加工具配置（从资源读取）
} sndp_sm_cb_t;

typedef struct {
    u8 distance;
    //小于保护角的语音被保留，大于抑制角的语音被抑制，中间平滑过渡
    u8 degree;                                  //语音保护角，范围0~90度
    u8 degree1;                                 //语音抑制角 = degree + degree1，范围0~90度

    u8 level;                                   //稳态最大降噪量，范围0~29dB
    u8 dnn_level;                               //dnn最大降噪量，范围0~29dB
    u8 param_printf;
    int bf_upper;                               //固定波束频率上限, 默认128
    const u32 *coef;                            //声加配置资源，非0：使用声加工具配置（从资源读取）
} sndp_dm_cb_t;

typedef struct {
    u8 level;                                   //稳态最大降噪量，范围0~29dB
    u8 dnn_level;                               //dnn最大降噪量，范围0~29dB
    u8 param_printf;

    const u32 *coef;                            //声加配置资源，非0：使用声加工具配置（从资源读取）
} sndp_fbdm_cb_t;

enum NR_CFG_EN {
    NR_CFG_FAR_EN           = BIT(0),           //使能远端降噪
    NR_CFG_NEAR_AINS2_EN    = BIT(1),           //使能声加单麦前AINS2
    NR_CFG_TRUMPET_EN       = BIT(2),           //使能汽车喇叭声降噪
    NR_CFG_SCO_FADE_EN      = BIT(3),           //使能通话前500ms淡入
    NR_CFG_DAC_DNR_EN       = BIT(4),           //使能通话下行————动态降噪
    NR_CFG_DAC_DRC_EN       = BIT(5),           //使能通话下行————DRC
};

typedef struct {
    u32 nt;
    u8  prior_opt_idx;
	u8  ns_ps_rate;
	u8  trumpet_en;
	u8  nt_post;
	u8  exp_range;
} ains_cb_t;

typedef struct {
    u8 distance;                                //双mic间距：mm（步进1mm）
    u8 degree;                                  //语音角度（小于90度）:10+5*N (5度步进，N<17)
    u8 max_degree;                              //最大保护角度（小于90度）:10+5*N (5度步进，N<17)
    u8 level;                                   //降噪强度：2*N（2dB步进，N<30）
    u8 wind_level;
    u8 param_printf;                            //使能参数打印
    u8 rfu[1];
    const int *filter_coef;                     //int coef[512]
    int maxIR;                                  //Q18
    int maxIR2;                                 //Q18
    u16 windnoise_conditioned_eq_x0;            //风噪抑制起始频段(Hz)
    u16 windnoise_conditioned_eq_x1;            //风噪抑制终止频段(Hz)
    int windnoise_conditioned_eq_y1;            //风噪抑制的终止大小(db), 最小为0db
    u8 snr_x0_level;                            //如果在一般信噪比下，降噪不足，就适当调大（0~0.4 步进0.05），默认0.1
    u8 snr_x1_level;                            //如果低信噪比下，语音断断续续，就适当调小（0.5~1 步进0.05），默认0.8
} dmic_cb_t;

//typedef struct {
//	u16 nt;
//	u8  nt_post;
//	s16 exp_range_H;
//	s16 exp_range_L;
//	u8  model_select;
//	u16 min_value;
//	u16 nostation_floor;
//	u8  wind_thr;
//	u8  wind_en;
//	u8  noise_ps_rate;
//	u8  prior_opt_idx;
//	u8  wind_level;
//	u16 wind_range;
//	u16 low_fre_range;
//	u16 low_fre_range0;
//	u8  pitch_filter_en;
//    u8  param_printf;                           //使能参数打印
//	u16 mask_floor;
//	u8  mask_floor_r;
//	u8  music_lev;
//	u8  comforN_level;
//	u16 gain_expand;
//	u8  nn_only;
//	u16 nn_only_len;
//} dnn_cb_t;

typedef struct {
	u8  param_printf;                           //使能参数打印
	u8  mic_cfg;                                //主副麦选择
	//u8  bf_type;                                //beamforming类型
	s16 distance;				                //双麦间距
	u16 degree;					                //拾音角度方向
	s16 nt;						                //普通降噪量
	u8  nt_post;						             //后滤波降噪量
	s16 exp_range;
	//u8  fast_convergence_en;                    //快速收敛功能，对人声有影响，默认不开
	//u16 lowside_corr;                           //下支路相关系数

	//	u8  trumpet_en;                             //喇叭声抑制
	u8  cmp_tbl_dis;
	u8  dm_dnn_en;                              //是否使能双麦AI算法
	u16 min_value;
	u8  model_select;
	u8  noise_ps_rate;
	u8  comp_mic_fre;                           //双麦+AI的参数
	u16 comp_mic_fre_L;                         //传统双麦的参数
	u16 comp_mic_fre_H;                         //传统双麦的参数
	u8  prior_opt_idx;

	u16  low_fre_ns0_range;
	//u8  gamma;
	u8  wind_nr_en;                             //使能风噪
	u8  wind_range;                             //风噪降噪范围
	u8  wind_level;
	u8  wind_sig_choose;
	u8  bf_choose;
	u8  tf_en;
	u16 tf_len;
	u8  tf_norm_en;
	u8  tf_norm_only_en;
	s32 opt_limit;
	s16 exp_range_H;
	s16 exp_range_L;
	u8	music_lev;
	u16 gain_expand;
	u8  nn_only;
	u16 nn_only_len;
	//u16 pitch_filter_range;
} dmns_cb_t;

typedef struct {
    u8 mode;                                    //auto or manual mode
    u8 level;                                   // 降噪量
    u8 min_range1;                              // 噪声谱更新系数
    u8 min_range2;                              // 噪声谱更新系数
    u8 min_range3;                              // 噪声谱更新系数
    u8 min_range4;                              // 噪声谱更新系数
    u8 anksnr_en;                               // 是否使能ank snr
    u16 nera_val;                               // anksnr_en为0时有效，alp1固定值
    u32 block_cnt;
} peri_nr_cfg_t;

typedef struct {
    u8 nr_type;                                 //近端降噪类型
    u8 nr_cfg_en;                               //远端降噪、喇叭声降噪、500ms淡入等降噪配置
    u32 dump_en;
    u8 rfu[2];
    u8 calling_voice_pow;                       //用于呼出电话，响铃之前动态降噪
    u8 calling_voice_cnt;
    u8 calling_voice_temp_cnt;
    void *far_nr;                               //远端降噪算法配置
    int dac_dnr_thr;
} nr_cb_t;

typedef struct {
    u32 type            :4;
    u32 mode            :1;                     //auto or manual mode
    u32 nlp_bypass      :1;                     //aec nlp bypass
    u32 nlp_only        :1;                     //aec nlp only select
    u32 nlp_level       :4;
    u32 nlp_part        :2;
    u32 comforn_level   :4;
    u32 comforn_floor   :10;
    u32 comforn_en      :1;
    u32 xe_add_corr     :16;
    u32 upbin           :8;
    u32 lowbin          :8;
    u32 diverge_th      :4;
    u32 gamma           :16;
    u32 aggrfact        :16;
    u32 depre_mode      :1;
    u32 mic_ch          :1;
    u32 dig_gain        :24;
    u32 echo_th         ;
    u16 far_offset;
} aec_cfg_t;

typedef struct {
    u8 alc_en;
    u8 rfu[1];
    u8 fade_in_step;
    u8 fade_out_step;
    u8 fade_in_delay;
    u8 fade_out_delay;
    s32 far_voice_thr;
} alc_cb_t;

typedef struct {
    aec_cfg_t aec;
    alc_cb_t alc;
    nr_cb_t nr;

    u8 rfu[3];
    u8 mic_eq_en        : 1;
    u8 mic_drc_en       : 1;
    int post_gain;
} call_cfg_t;

void bt_call_init(call_cfg_t *p);
void bt_call_exit(void);
void nr_init(u32 nt);
void bt_dmns_plus_process(s16 *buf);
bool bt_sco_dnn_en(void);

void bt_dnn_init(void *alg_cb);
void bt_sndp_sm_init(void *alg_cb);
void bt_dmns_init(void *alg_cb);
void bt_ains_init(void *alg_cb);
void bt_sndp_fbdm_init(void *alg_cb);
void bt_sndp_dm_init(void *alg_cb);

void near_ains2_init(u32 nt, u8 fre_mid_min, u8 fre_high_min);
void trumpet_denoise_init(u8 level);

bool bt_sco_dac_drc_init(u8 *drc_addr, int drc_len);

bool bt_sco_far_cache_write(u8 *inbuf, u32 wlen);
bool bt_sco_far_cache_read(u8 *outbuf, u32 rlen);
void bt_sco_far_cache_init(void);
#endif
