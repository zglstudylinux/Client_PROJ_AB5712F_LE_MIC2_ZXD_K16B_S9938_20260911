#ifndef __TOOLKIT_DUMP_H
#define __TOOLKIT_DUMP_H

//应用场景定义
typedef enum {
    UNI_DUMP_APPLICATION_SCO = 0x0,
    UNI_DUMP_APPLICATION_ANC,
    UNI_DUMP_APPLICATION_MUSIC_EFFECT,
    UNI_DUMP_APPLICATION_MIC_EFFECT,
} uni_dump_application_t;

//通话算法场景数据类型定义
typedef enum {
    UNI_DUMP_SCO_DATA_TALK_MIC = 0x0,
    UNI_DUMP_SCO_DATA_FF_MIC,
    UNI_DUMP_SCO_DATA_FB_MIC,
    UNI_DUMP_SCO_DATA_ALG_DATA,
    UNI_DUMP_SCO_DATA_FAR_MIC,
    UNI_DUMP_SCO_DATA_FAR_ALG,
    UNI_DUMP_SCO_DATA_AEC_ALG,
} uni_dump_sco_data_type_bit_t;

//ANC场景数据类型定义
typedef enum {
    UNI_DUMP_ANC_DATA_FF_0_MIC = 0x0,
    UNI_DUMP_ANC_DATA_FB_0_MIC,
    UNI_DUMP_ANC_DATA_FF_1_MIC,
    UNI_DUMP_ANC_DATA_FB_1_MIC,
    UNI_DUMP_ANC_DATA_HYBRID_ANC_0_OUT,
    UNI_DUMP_ANC_DATA_HYBRID_ANC_1_OUT,
    UNI_DUMP_ANC_DATA_SPK_L,
    UNI_DUMP_ANC_DATA_SPK_R,
    UNI_DUMP_ANC_DATA_MUSIC_L,
    UNI_DUMP_ANC_DATA_MUSIC_R,
    UNI_DUMP_ANC_DATA_TALK_MIC,
} uni_dump_anc_data_type_bit_t;

//音乐音效场景数据类型定义
typedef enum {
    UNI_DUMP_MUSIC_EFFECT_DATA_MUSIC = 0x0,
    UNI_DUMP_MUSIC_EFFECT_DATA_MUSIC_ALG,
} uni_dump_music_effect_data_type_bit_t;

//MIC音效场景数据类型定义
typedef enum {
    UNI_DUMP_MIC_EFFECT_DATA_MIC = 0x0,
    UNI_DUMP_MIC_EFFECT_DATA_MIC_ALG,
} uni_dump_mic_effect_data_type_bit_t;

//音频数据位宽
typedef enum {
    UNI_DUMP_PCM_BITS_16 = 0x0,
    UNI_DUMP_PCM_BITS_24,
} uni_dump_pcm_bits_t;

//音频数据采样率
typedef enum {
    UNI_DUMP_PCM_SPR_4000 = 0x0,
    UNI_DUMP_PCM_SPR_8000,
    UNI_DUMP_PCM_SPR_11025,
    UNI_DUMP_PCM_SPR_12000,
    UNI_DUMP_PCM_SPR_16000,
    UNI_DUMP_PCM_SPR_22050,
    UNI_DUMP_PCM_SPR_24000,
    UNI_DUMP_PCM_SPR_32000,
    UNI_DUMP_PCM_SPR_36000,
    UNI_DUMP_PCM_SPR_44100,
    UNI_DUMP_PCM_SPR_48000,
    UNI_DUMP_PCM_SPR_88200,
    UNI_DUMP_PCM_SPR_96000,
    UNI_DUMP_PCM_SPR_176400,
    UNI_DUMP_PCM_SPR_192000,
    UNI_DUMP_PCM_SPR_352800,
    UNI_DUMP_PCM_SPR_384000,
} uni_dump_pcm_spr_t;

//帧头部
typedef struct __attribute__((packed)) {
    int8_t      fixed_header[4];
    uint8_t     version;
    uint16_t    length;
    uint8_t     frame_cnt;
    uint8_t     application;
    uint32_t    data_types;
    uint8_t     pcm_nch : 4;
    uint8_t     pcm_bits : 4;
    uint8_t     pcm_spr;
    uint8_t     resv;
} uni_dump_header_t;

typedef struct {
    u8 mute;
} btmic_dump_cfg_t;

//指令
enum {
    BTMIC_DUMP_CMD = 0x0,
    BTMIC_DUMP_START_CMD,
    BTMIC_DUMP_STOP_CMD,
};

void audio_dump_init(void);
void audio_dump_input(void *ptr, u32 samples, u32 params, u8 is_24bits);
void tx_ack(uint8_t *packet, uint16_t len);
uint calc_crc(void *buf, uint len, uint seed);
void bsp_btmic_dump_parse_cmd(void);
int toolkit_btmic_audio_dump_init(u16 data_type, u8 pcm_bits, u8 pcm_spr, u8 path_type);
void toolkit_btmic_audio_dump_process(void* buf, u32 samples);
#endif // __AUDIO_DUMP_H
