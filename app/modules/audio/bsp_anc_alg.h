#ifndef __BSP_ANC_ALG_H__
#define __BSP_ANC_ALG_H__


//算法初始化参数
typedef struct {
    u8    type;
    u8    mic_cfg;
    void* alg_param;
    u8    dump_en;
    u32   wind_cnt_total;
    u32   wind_in_cnt_thresold;
    u32   wind_out_cnt_thresold;
    u32   noise_cnt_total;
    u32   noise_in_cnt_thresold;
    u32   noise_out_cnt_thresold;
} anc_alg_param_cb;


void bsp_anc_alg_start(void);
void bsp_anc_alg_stop(void);
void anc_alg_process(void);

#endif // __BSP_ANC_ALG_H__
