#include "include.h"


#if ANC_ALG_EN

#define TRACE_EN                    0

#if TRACE_EN
#define TRACE(...)                  printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif // TRACE_EN


void anc_alg_init(anc_alg_param_cb* p);


anc_alg_param_cb ada_param_cb AT(.anc_data.ada_param_cb);


void bsp_anc_alg_start(void)
{
    if (sys_cb.anc_alg_en == 1) {
        return;
    }

    //调整主频
    sys_clk_req(INDEX_ANC, SYS_48M);

    //参数初始化
//    ada_param_cb.alg_param = (void*)&anc_asm;
    ada_param_cb.dump_en = ANC_ALG_DUMP_EN;

    //算法初始化
    anc_alg_init(&ada_param_cb);

    //SDADC初始化
    audio_path_init(AUDIO_PATH_ANC_ALG);
    audio_path_start(AUDIO_PATH_ANC_ALG);

    sys_cb.anc_alg_en = 1;

    TRACE("ANC alg: start\n");
}

void bsp_anc_alg_stop(void)
{
    if (sys_cb.anc_alg_en == 0) {
        return;
    }

    sys_cb.anc_alg_en = 0;

    //SDADC关闭
    audio_path_exit(AUDIO_PATH_ANC_ALG);

    //调整主频
    sys_clk_free(INDEX_ANC);

    TRACE("ANC alg: stop\n");
}

AT(.com_text.anc.alg_process)
void anc_alg_process(void)
{
    if (sys_cb.anc_alg_en == 1) {

    }
}

void anc_alg_audio_path_cfg_set(sdadc_cfg_t* cfg)
{
//    //增益设置
//    cfg->anl_gain = ((xcfg_cb.micl_anl_gain) | (xcfg_cb.micr_anl_gain<<8) | (xcfg_cb.micm_anl_gain<<16));
//    if (xcfg_cb.anc_alg_dgain_en) {
//        cfg->dig_gain = ((xcfg_cb.anc_micl_dig_gain) | (xcfg_cb.anc_micr_dig_gain<<8) | (xcfg_cb.anc_micm_dig_gain<<16));
//    } else {
//        cfg->dig_gain = ((xcfg_cb.bt_micl_dig_gain) | (xcfg_cb.bt_micr_dig_gain<<8) | (xcfg_cb.bt_micm_dig_gain<<16));
//    }
}

#endif // ANC_ALG_EN
