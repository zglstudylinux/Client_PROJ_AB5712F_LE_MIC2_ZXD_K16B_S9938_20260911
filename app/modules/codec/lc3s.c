#include "include.h"

#if DEVICE_STEREO_EN || WIRELESS_MIC_STEREO
uint8_t cfg_lc3s_frame_len          = WIRELESS_MIC_FRAME_SIZE/2;
#else
uint8_t cfg_lc3s_frame_len          = WIRELESS_MIC_FRAME_SIZE;
#endif

#if WIRELESS_MIC_FRAME_SIZE == 30
uint32_t cfg_lc3s_bitrate           = 96000;
#else
uint32_t cfg_lc3s_bitrate           = 80000;
#endif


///compression ratio formula = (sampling rate * bit depth) / bitrate
uint8_t cfg_lc3b_frame_len          = WIRELESS_MIC_FRAME_SIZE;
#if WIRELESS_MIC_FRAME_SIZE == 15
uint32_t cfg_lc3b_bitrate           = 96000;
#elif WIRELESS_MIC_FRAME_SIZE == 20
uint32_t cfg_lc3b_bitrate           = 128000;
#endif

//-----------------------------------------------------------------------
#if WIRELESS_CON_FULL_DUPLEX_EN || WIRELESS_MIC_STEREO
#define ENCODER_NUM     2   //用于立体声编码
#else
#define ENCODER_NUM     1
#endif

#if (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3S)
typedef struct
{
    uint8_t  buf[888];
    uint32_t size;
    signed int  nSamples_in;
} lc3s_encoder_t;

static lc3s_encoder_t encoder[ENCODER_NUM] AT(.buf.lc3_enc);

AT(.text.lc3_init.enc)
lc3s_encoder_t *lc3_encoder_malloc(uint8_t index)
{
    if(index >= ENCODER_NUM) {
        return NULL;
    }
    return &encoder[index];
}

AT(.text.lc3_init.enc)
bool lc3s_enc_is_stereo(void)
{
#if DEVICE_STEREO_EN || WIRELESS_MIC_STEREO
    return true;
#else
    if(wireless_role_is_adapter()) {
        return true;
    }

    return false;
#endif
}

#if WIRELESS_CON_FULL_DUPLEX_EN || WIRELESS_MIC_STEREO
static s16 lc3s_tmp_buf[120] AT(.buf.lc3_enc);

AT(.text.lc3_enc)
s16 *lc3s_tmp_buf_malloc(void)
{
    return (s16 *)lc3s_tmp_buf;
}
#else
void lc3s_stereo_enc(s16 *ibuf, u8 *obuf, uint samples){}
void lc3s_stereo_dec(u8 *ibuf, s16 *obuf, uint samples){}
#endif
#endif
