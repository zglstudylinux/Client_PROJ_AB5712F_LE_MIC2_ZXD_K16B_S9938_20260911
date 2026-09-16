#ifndef __BT_CALL
#define __BT_CALL

extern const int mic_gain_tbl[16];
void bt_sco_aec_init(u8 *sysclk, aec_cfg_t *aec, alc_cb_t *alc);
void bt_sco_nr_init(u8 *sysclk, nr_cb_t *nr);
void bt_sco_nr_exit(void);
void bt_sco_far_nr_init(u8 *sysclk, nr_cb_t *nr);

void bt_sco_sndp_dm_init(u8 *sysclk, nr_cb_t *nr);
void bt_sndp_sm_exit(void);
void bt_sco_dmns_init(u8 *sysclk, nr_cb_t *nr);
void bt_sco_sndp_sm_init(u8 *sysclk, nr_cb_t *nr);
void bt_sco_near_nr_dft_init(u8 *sysclk, nr_cb_t *nr);
void bt_sco_dnn_init(u8 *sysclk, nr_cb_t *nr);
void bt_dnn_exit(void);
void bt_sco_nr_user_init(u8 *sysclk, nr_cb_t *nr);
void bt_sco_sndp_fbdm_init(u8 *sysclk, nr_cb_t *nr);
void bt_sndp_fbdm_exit(void);
void bt_sco_sndp_dm_init(u8 *sysclk, nr_cb_t *nr);
void bt_sndp_dm_exit(void);

void bt_sco_dump_init(u8 *sysclk, call_cfg_t *p);



#endif
