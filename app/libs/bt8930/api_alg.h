#ifndef __API_ALG_H
#define __API_ALG_H

typedef struct {
	s32 overdrive;
	u16 denoiseBound;
	u8  smooth_en;
	u8  modelUpdatePars0;
	s32 factor;
	u8  delta_k_up;
	s16 lquantile_sm;
	s32 quan_gap;
	s16 quan_gap_low_len;
	s32 quan_gap_low;
	u8  sin_dnn_en;
	u8  sin_all_en;
	s16 sin_all_len;
	s16 gain_assign;
	u8  prior_opt_idx;
	u8  prior_opt_ada_en;
	u16 low_noise_range;
	u16 spp_fre_p;
	u16 spp_fre_len;
	u16 high_gain_len;
	u8  howling_enh_en;
	s16 howling_thr_low;
	s16 howling_thr_hi;

	u8  ang_cg_mode;
	s32 change_range;
	s16 freq_step;
	s32 warping_coef;
	u8  trad_position;
	s32 fs;
	u8  process_type;
	s32 tanh_threshold;
	u8  gain_thr_en;
	u16 gain_thr;


} gtcrn_48k_qmf_10ms_cb_t;

void gtcrn_48k_qmf_10ms_howling_512_init (gtcrn_48k_qmf_10ms_cb_t *p);
int gtcrn_48k_qmf_10ms_howling_512_process(s16* data);

///------------------------------------------------------------------------------------------
///DNN_L1 算法结构体以及相关API声明
typedef struct {
	s16 nt;
	u8  nt_post;
	s16 exp_range_H;
	s16 exp_range_L;
	u8  model_select;
	u16 min_value;
	u16 nostation_floor;
	u8  wind_thr;
	u8  wind_en;
	u8  noise_ps_rate;
	u8  prior_opt_idx;
	u8  prior_opt_ada_en;
    u8  param_printf;                           //Ê¹ÄÜ²ÎÊý´òÓ¡
    u8  wind_level;
	u16 wind_range;
	u16 low_fre_range;
	u16 low_fre_range0;
	u8  pitch_filter_en;
	u16 mask_floor;
	u8  mask_floor_r;
	u8  music_lev;
	u8  comforN_level;
	u16 gain_expand;
	u8  nn_only;
	u16 nn_only_len;
	//s32 noise_db2;
	//s32 noise_db3;
	u8  smooth_en;
	u16 gain_assign;
	u8  enlarge_v;
    s32 noise_db;
	s32 noise_db2;
	s32 noise_db3;
    s32 noise_db4;
	u8  vad_floor_en;
	u32 prior_sum_v;
	//u16 pitch_filter_range;
} dnn_cb_t;

void dnn_init(dnn_cb_t *dnn_cb);
int dnn_process(s16 *data, s32 *data_32bits, u8 in_24bits_en);
void dnn_process_ext_window(s16 *data, s32 *data_32bits, u8 in_24bits_en,s32* outbuf);
void dnn_fre_process(s32 *f, u8 data_shift, u8 in_24bits_en);
void dnn_process_ext_next(s32 *data, s32 *data_32bits, u8 in_24bits_en,s16* outbuf);

///------------------------------------------------------------------------------------------
///NR_FAR 算法结构体以及相关API声明
void nr_far_process(s16 *data);
void nr_far_init(u16 noise_thr, u16 nr_level);
///------------------------------------------------------------------------------------------
///ROOM_REVERB 算法结构体以及相关API声明
typedef struct
{
	u16 damping_set;        /*高频阻尼，去除高频部分的齿音，擦类的刺耳声*/
	u16 decay_set;          /*衰减率*/
	u16 wet;
	u16 dry;
	u32 samples;
	u8 hpl;                 /*高通滤波器等级0表示关闭,1-10:50hz-500hz,步进50hz*/
}rvb_init_sb;

void room_reverb_process(s16 *sample, u8 idx);
void reverb_buf_init(rvb_init_sb *p, u8 idx);
///------------------------------------------------------------------------------------------
///ECHO 算法结构体以及相关API声明
typedef struct {
	u16 wet;
	u16 dry;
	u16 attenuation;
	s32 delay_length;
	s32 index;
	s32 inter_val0;
	s32 inter_val1;
	s32 inter_val2;
	u8  flag;
	u8  filter_en;
	//s16 delay_rbuf[DELAY_BUF_LENGTH];
} echo_cb_t;

void iir2_filter_init(s32 cutoffFreq_val);
void echo_init_mode(u8 sample_rate, s16 *delay_buf, u16 delay_buf_len, u16 max_delay_len);
void echo_process(s16 *ldata);
void echo_set_param(echo_cb_t *set_param);
void echo_clear_buf(void);

///------------------------------------------------------------------------------------------
///REVERB 算法结构体以及相关API声明
/*plate reverb 初始化结构体*/
typedef struct
{
	u8  hp_level_set;       /*高通滤波等级*/
	u16 predelay_set;       /*预延迟*/
	u16 damping_set;        /*高频阻尼，去除高频部分的齿音，擦类的刺耳声*/
	u16 wet_set;            /*初始化湿度*/
	u16 dry_set;            /*初始化干度*/
	u16 diffusion;     		/*衰减扩散，控制尾部密度*/
	u16 decay_set;			/*衰减率*/
	u8  en;
	u8  modulation;
}prvb_init_sb;

void plate_reverb_process_do(s16 *data, u32 samples);
void plate_buf_init(prvb_init_sb *p);
void plate_reverb_set(prvb_init_sb *buf);
///------------------------------------------------------------------------------------------
///magic 算法结构体以及相关API声明
#define CircleBufo_Len		1536//0.032*Fs

typedef struct {
//Common use
	s32 fre_shift;
	u32 Fs;
	u32 pitch_levels;
	s32 ph1;
	s32 ph2;
	s16 pWtite1;
	s16 pRead1;
	s16 pRead2;
	s16 CircleBuf[CircleBufo_Len];
} pitch_shift2_cb_t;


void pitch_shift2_init(s32 SampleRate, s32 shift, pitch_shift2_cb_t *st);
void pitch_shift2_process(s16 *data, pitch_shift2_cb_t *st);


///------------------------------------------------------------------------------------------
//plc
void plc_soft_init(u8 idx, uint pkt_len);
void plc_soft_process(s16 *int_data, u32 samples, u8 bfi, u8 idx);
void plc_soft_exit(u8 idx);

///------------------------------------------------------------------------------------------
//plc_60   60samples
void plc_soft_60_init(u8 idx, uint pkt_len);
void plc_soft_60_process(s16 *int_data, u32 samples, u8 bfi, u8 idx);
void plc_soft_60_exit(u8 idx);

///------------------------------------------------------------------------------------------
//lc3s
void lc3s_enc_init(u8 sample_rate, u16 samples);
void lc3s_dec_init(u8 sample_rate, u16 samples);
void lc3s_dec(u8 *ibuf, s16 *obuf, uint samples, uint index);
void lc3s_enc(s16 *ibuf, u8 *obuf, uint samples);
uint lc3s_get_frame_size(uint fs_in, u16 samples, u32 bitrate);
void lc3s_dec_exit(uint index);
void lc3s_stereo_enc(s16 *ibuf, u8 *obuf, uint samples);
void lc3s_stereo_dec(u8 *ibuf, s16 *obuf, uint samples);

///------------------------------------------------------------------------------------------
//lc3b
void lc3b_dec(u8 *ibuf, s16 *obuf, uint samples, uint index);
void lc3b_dec_init(u8 sample_rate, u16 samples);
void lc3b_dec_exit(uint index);
void lc3b_enc_init(u8 sample_rate, u16 samples);
void lc3b_enc(void *ibuf, u8 *obuf, uint samples);
void lc3b_enc_exit(void);


u32 exlib_softeq_cb_init(void *cb, u32 cb_len, u32 res_addr, u32 res_len);
void exlib_softeq_coef_update(void *cb, u32 cb_len, u32 *coef, u32 band, bool pregain);  //pregain = 1 , first u32 is pregain,  next is band coef  //pregain=0, all coef is band coef
s32 exlib_softeq_proc(void *cb, s32 input) ;
s16 exlib_softeq_stereo_proc_s16(void *cb, s16 data);
u32 exlib_softeq_band_get(void *cb);

void exlib_drcv1_cb_init(void *cb, u32 cb_len, u32 res_addr, u32 res_len); //drc资源文件更新cb参数
void exlib_drcv1_coef_update(void* cb, u32 *tbl);      //在线调试更新参数
s16 exlib_drc_process_s16(void *cb, s32 data_in);     //饱和到16 bit
s32 exlib_drc_process_s24(void *cb, s32 data_in);     //饱和到24 bit  //undo
s32 exlib_drc_process_gain(void *cb, s32 data_in);
s16 exlib_drc_get_pcm16_s(s32 data, int g);
s32 exlib_drc_get_pcm32_s(s32 data, int g);
s16 exlib_s_clip16(int x);


///------------------------------------------------------------------------------------------
///AINS4 算法结构体以及相关API声明
typedef struct {

	//s32 noise_db2;
	//s32 noise_db3;
	s32 denoiseBound;
	s32 snr_thr;
	u8  overdrive_adapt_en;
	s32 overdrive;
	u8  smooth_en;
	u8  modelUpdatePars0;
	//u8  enlarge_v;
	s16 gainHB_rd;
	u8 delta_k_up;
	//s16 denoiseBound_fix;
	u8 	yuan_en;
	u16 enr_thres;
	s16  low_fre_range;
	s16 prior_opt_idx;
	s16 prior_opt_freh;
	u8  prior_opt_ada_en;
	s16 lquantile_sm;
	s16 hi_gain_len;
	s32 factor;
	s32 quan_gap;
	s16 quan_gap_low_len;
	s32 quan_gap_low;
	s16 speech_update;
	//s16 smooth_v;
	//u8  enr_mean_max_en;
	//s32 enr_nr_thr;
	u8	spp_en;
	s16 ymin_floor;
	s16 ymin_idx;
	s16 gain_ceil;
	s32 gain_ceil_ratio;
	s16 noise_min_floor;
	//s16 smooth_logLrt;
	s16 music_lev;
	u8  noise_min_en;
	s32 ai_vad_hard_thres;
	u16 spp_vad_len;
	s32 qhat_assign;
	s32 qhat_assign_low;
	u8  spp_max_en;
	u8  low_floor_add_l;
	s16 low_mask_floor_add;
	u8  dd_od_ctl_en;
	u8  ai_vad_spp_hard_en;
	s32 spp_thr_noiseprev;
	s16 spp_thr;
	s16 prev_noise_len;
	u8  vad_low_noise_en;
	s16 vad_low_noise_p;
	u8  vad_low_max_en;
	s32 priorModelPars0;
	s16 alp1_dd;

	//u16 pitch_filter_range;
} ains4_cb_t;
void ains4_mic_proc_kick_start(void);
int ains4_process(s16 *data, s32 *data_32bits, u8 in_24bits_en);
void ains4_init(ains4_cb_t *ains4_cb);



///---------------------------------------------------------------------------------------------
///AGC 算法结构体以及相关API声明
typedef struct {
	u32 sampleHz;           //语音采样率
	u8  frameMs;             //一帧语音长度，以毫秒为单位
	u16 frameSh;            //一帧short的个数， 默认一帧10ms
	u16 frameSh_true;
	s16 CompressiondB;
	s16 TargetdBfs;
	u8  limiterEnable;		//是否打开限压器
	u8  frame_2p5ms_en;
	u8	inbits24_en;
	u8 	smooth_en;
	u8  agcMode;			//AGC模式
} agc_cb_t;
void agcInit_do(agc_cb_t* agc_cb);
void AgcProcess_do(s16 *input, s32 pcmLen);
void agcInit(agc_cb_t* agc_cb);
void AgcProcess(s16 *input, s32 pcmLen);

///---------------------------------------------------------------------------------------------
///DNN_L3 算法结构体以及相关API声明
typedef struct {

	u8  model_select;
	u16 min_value;
	u16 nostation_floor;
	//u8  wind_thr;
	//u8  wind_en;
	u8  noise_ps_rate;
	u8  prior_opt_idx;
	u8  prior_opt_ada_en;
    u8  param_printf;                           //使能参数打印

	u16 low_fre_range;

	u8  music_lev;
	u8  music_lev2;

	u8  nn_only;
	u16 nn_only_len;
	u16 gain_assign;
	u8  sin_gain_post_en;
	u16 sin_gain_post_len;
	u16 sin_gain_post_len_f;
	s16 denoiseBound;
	s16 denoiseBound_hi;
	u8  smooth_en;
	u8  ap_test_en;
	u16 hi_gain_len;
	s16 sp_thres;
	//s32 vad_thres;
	s16 prev_noise_len;
	u8  dd_od_ctl_en;
	//s16 low_gain_l;
	u8  hi_gain_mode;
	u8  spp_hi_en;
	u8  spp_max_en;
	s32 spp_hard_thres;
	s16 spp_hard_idx;
	//u16 pitch_filter_range;
} dnn_l3_cb_t;

void dnn_l3_init(dnn_l3_cb_t *dnn_l3_cb);
int dnn_l3_process(s16 *data);

///---------------------------------------------------------------------------------------------
///howling_dnn算法结构体以及相关API声明
void hl_init();
int howling_process(s16* data);
u16 notch_index_ret(void);//eq 参数索引
int howling_process_ext_window(s16* data,s32* outbuf);
int howling_process_ext(s32* fft_in);


typedef struct {
	//s16 nt;
	s16 mask_floor;
    u8  param_printf;                           //使能参数打印
	s32 howling_f_thres;
	s32 howling_f_thres_hi;
	u16 howling_fre_p;
	s32 hiwling_div_thres;
	u32 std_thres;
	s16  howling_exp_papr;
	s16  howling_exp_papr_hi;
	s16  howling_exp_papr2;
	//u8  sin_post_en;
	u8  smooth_en;

	//u16 pitch_filter_range;
} howling_dnn_cb_t;
void howling_dnn_init(howling_dnn_cb_t *dnn_cb);
int howling_dnn_process(s16 *data, s32 *data_32bits, u8 in_24bits_en);
void howling_dnn_fre_process(s32 *f, u8 data_shift, u8 in_24bits_en);
void howling_dnn_process_ext_window(s16 *data, s32 *data_32bits, u8 in_24bits_en,s32* outbuf);
void howling_dnn_process_ext_next(s32 *data, s32 *data_32bits, u8 in_24bits_en,s16* outbuf);

///---------------------------------------------------------------------------------------------
///GTCRN_48K_QMF 算法结构体以及相关API声明
typedef struct {
	s32 overdrive;
	u16 denoiseBound;
	u8  smooth_en;
	u8  modelUpdatePars0;
	s32 factor;
	u8  delta_k_up;
	s16 lquantile_sm;
	s32 quan_gap;
	s16 quan_gap_low_len;
	s32 quan_gap_low;
	u8  sin_dnn_en;
	u8  sin_all_en;
	s16 sin_all_len;
	u8  prior_opt_idx;
	u8  prior_opt_ada_en;
	u16 low_noise_range;
	u16 spp_fre_p;
	u16 spp_fre_len;
	u16 high_gain_len;
    s16 gain_assign_len;
	s16 gain_assign;
	s16 nn_only_len;
    s16 music_lev;
} gtcrn_48k_qmf_cb_t;
void gtcrn_48k_qmf_ns_init (gtcrn_48k_qmf_cb_t *p);
int gtcrn_48k_qmf_ns_process(s16* data);

///---------------------------------------------------------------------------------------------
///FREQ_SHIFT2 算法结构体以及相关API声明
void holwing_ex_init(unsigned short freq, signed short hz, signed short hz_step);   //freq表示音频数据的采样率, //level表示移多少HZ,如15表示正向移15HZ, -15表示负向移15HZ   //内部是以5HZ为单位去移的,level这里最好是5的倍速数
void howlingex_process(short *out, short *in, int samples);
short howlingex_run(short input);

///---------------------------------------------------------------------------------------------
///变声算法结构体以及相关API声明
typedef struct {
	u8    robot_en;
	u32   out_gain;
	u32	  epstep;
	u32   hs_len;
	u32   input_len;
	u32	  output_len;
	u32   quality;
	u32   resamples_outlen;
	//float interp_tmp;
	u32 pitch_rate;
	u32 speed_rate;
	u32 formant_shift;
}mav_init_t;
void voice_change_init(mav_init_t *p, u8 idx);
void voice_change_process(s16 *data, u32 samples, u8 idx) ;

///------------------------------------------------------------------------------------------
///AI大模型降噪算法结构体以及相关API声明
typedef struct {
	s32 overdrive;
	u8  adaptive_floor;
	u16 denoiseBound;
	u16 denoiseBoundLow;
	u16 denoiseBoundHigh;
	u16 denoiseBoundProb;

	u8  smooth_en;
	u8  modelUpdatePars0;
	s32 factor;
	u8  delta_k_up;
	s16 lquantile_sm;
	s32 quan_gap;
	s16 quan_gap_low_len;
	s32 quan_gap_low;
	u8  sin_dnn_en;
	u8  sin_all_en;
	s16 sin_all_len;
	u8  prior_opt_idx;
	u8  prior_opt_ada_en;
	u16 low_noise_range;
	u16 spp_fre_p;
	u16 spp_fre_len;
	u16 high_gain_len;
	s16 gain_assign_len;
	s16 gain_assign;
	s16 nn_only_len;
	s16 music_lev;
	s16 intensity;
	s16 mask_vad_fre_thr;
	u8  spp_max_en;
} gtcrn_L3_cb_t;

void gtcrn_L3_ns_init (gtcrn_L3_cb_t *p);
int gtcrn_L3_ns_process(s16* data);
void npu_exit(void);

///------------------------------------------------------------------------------------------
///YLCRN_L2 AI大模型降噪算法结构体以及相关API声明
typedef struct {
#if 0
	//s32 overdrive;
	float overdrive;
	u8  adaptive_floor;
	//u16 denoiseBound;
	float denoiseBound;
	float denoiseBoundLow;
	float denoiseBoundHigh;
	float denoiseBoundProb;
	float denoiseBoundRatio;
	u8  smooth_en;
	u8  modelUpdatePars0;
	//s32 factor;
	//u8  delta_k_up;
	//s16 lquantile_sm;
	//s32 quan_gap;
	//s16 quan_gap_low_len;
	//s32 quan_gap_low;
	u8  sin_dnn_en;
	u8  sin_all_en;
	s16 sin_all_len;
	u8  prior_opt_idx;
	u8  prior_opt_ada_en;
	u16 low_noise_range;
	//u16 spp_fre_p;
	float spp_fre_p;
	u16 spp_fre_len;
	u16 high_gain_len;
	//s16 music_lev;
	float music_lev;
	s16 prev_noise_len;
	//s16 gain_assign;
	float gain_assign;
	u8  hi_gain_mode;
#else
	s32 overdrive;
	u8  adaptive_floor;
	u16 denoiseBound;
	u16 denoiseBoundLow;
	u16 denoiseBoundHigh;
	u16 denoiseBoundProb;
	u16 denoiseBoundRatio;
	u8  smooth_en;
	u8  modelUpdatePars0;
	//s32 factor;
	//u8  delta_k_up;
	//s16 lquantile_sm;
	//s32 quan_gap;
	//s16 quan_gap_low_len;
	//s32 quan_gap_low;
	u8  sin_dnn_en;
	u8  sin_all_en;
	s16 sin_all_len;
	u8  prior_opt_idx;
	u8  prior_opt_ada_en;
	u16 low_noise_range;
	u16 spp_fre_p;

	u16 spp_fre_len;
	u16 high_gain_len;
	s16 music_lev;

	s16 prev_noise_len;
	s16 gain_assign;
	u8  hi_gain_mode;
#endif

} ylcrn_L2_cb_t;
void ylcrn_L2_ns_init (ylcrn_L2_cb_t *p);
void ylcrn_L2_ns_process(s16* data);

void audio_phase_rot2_init(u8 idx);
void audio_phase_rot2_process(s16* indata, u16 samples, u8 idx);

///---------------------------------------------------------------------------------------------
///dnr fre 算法结构体以及相关API声明
typedef struct {

	//s32 noise_db2;
	//s32 noise_db3;
	s16 denoiseBound;
	s32 overdrive;
	u8  smooth_en;
	s64 enr_thres;
	s16 smooth_v;
	u8  enr_mean_max_en;
	s16 music_lev;
	s32 enr_nr_thr;
	s16 low_fre_range;
	s16 prior_opt_idx;
	s32 in_attack;
	s32 in_release;
	s32 fs;
	s32 noise_init;
	//u16 pitch_filter_range;
} dnr_fre_cb_t;

void dnr_fre_init(dnr_fre_cb_t *dnr_fre_cb);
int dnr_fre_process(s16 *data, s32 *data_32bits, u8 in_24bits_en);

///---------------------------------------------------------------------------------------------
//YLCRN_16K 算法结构体以及相关API声明
typedef struct {
	u8  param_printf;
	s32 overdrive;
	//u8  adaptive_floor;
	u16 denoiseBound;
	u16 denoiseBound_wind;
	u8  wind_en;
	//u16 denoiseBoundLow;
	//u16 denoiseBoundHigh;
	//u16 denoiseBoundProb;
	//u16 denoiseBoundRatio;
	u8  smooth_en;
	u8  modelUpdatePars0;
	//s32 factor;
	//u8  delta_k_up;
	//s16 lquantile_sm;
	//s32 quan_gap;
	//s16 quan_gap_low_len;
	//s32 quan_gap_low;
	u8  sin_dnn_en;
	u8  sin_dnn_wind_en;
	u8  sin_all_en;
	s16 sin_all_len;
	u8  prior_opt_idx;
	u8  prior_opt_ada_en;
	u16 low_noise_range;
	u16 spp_fre_p;
	u16 spp_fre_p_wind;

	u16 spp_fre_len;
	//u16 high_gain_len;
	s16 music_lev;

	s16 prev_noise_len;
	s16 gain_assign;
	s16 gain_assign_wind;
	u8  hi_gain_mode;
	u8 spp_dnn_max_en;
	s16 gain_hard_thres;
	s32 vad_hard_thres;
	u8  vad_en;

	u8 *wind_buf;
	int wind_buf_size;
	void *wn;
} dnn_plus_cb_t;

void dnn_plus_ns_init (dnn_plus_cb_t *cb);
void dnn_plus_ns_process(s16* data);
#endif // __API_CODEC_H

