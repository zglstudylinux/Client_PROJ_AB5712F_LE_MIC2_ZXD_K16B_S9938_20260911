#include "include.h"
#include "src_buf.h"


#define SRC_INCACHE_SIZE        1440
#define SRC_OUTCACHE_SIZE       512

static src_buf_t src_buf_cb AT(.buf.src_buf.cb);
static uint8_t src_incache[SRC_INCACHE_SIZE] AT(.buf.src_buf.cache);
static uint8_t src_outcache[SRC_OUTCACHE_SIZE] AT(.buf.src_buf.cache);

void src_adjust_speed(int speed);

AT(.text.src_buf)
void src_buf_update_len(uint rlen)
{
    if (src_buf_cb.start) {
        cbuf_cfg_t *cbuf = &(src_buf_cb.cbuf);
        uint total_size = cbuf_total_samples_get(cbuf);

        GLOBAL_INT_DISABLE();
        if(total_size < src_buf_cb.input_min1) {
            src_buf_cb.input_min1 = total_size;
        }
        GLOBAL_INT_RESTORE();
    }
}

AT(.com_rodata.src.speech)
const int src1_speed_tbl[][2] = {
//  samples,    phase
    {0,         -5},
    {48*2,      -3},
    {48*4,      -1},
    {48*6,      0},
    {48*8,      1},
    {48*10,      3},
    {48*12,      5},
    {0xffff,    8},
};

AT(.com_text.i2s_info)
const char src1_info[] = "src = %d %d\n";

AT(.text.src_buf.adj)
void src_buf_adj_src(void)
{
    s8 new_speed = 0;
    cbuf_cfg_t *cbuf = &(src_buf_cb.cbuf);

    uint total_samples = cbuf_total_samples_get(cbuf);

    for(uint i=0; i<sizeof(src1_speed_tbl)/(2*sizeof(int)); i++) {
        if(total_samples <=  src1_speed_tbl[i][0]) {
            new_speed = src1_speed_tbl[i][1];
            break;
        }
    }

    if(new_speed != src_buf_cb.speed) {
        src_buf_cb.speed = new_speed;
        src1_adjust_speed(new_speed);
    }

//    static u32 ticks = 0;
//    if (tick_check_expire(ticks,1000)) {
//        my_printf(src1_info, new_speed, total_samples);
//        ticks = tick_get();
//    }

//    spi_byte_tx(0x21);
//    spi_byte_tx(total_samples>>8);
//    spi_byte_tx(total_samples>>0);
//    spi_byte_tx(new_speed);
}

AT(.text.src_buf.kick)
void src_buf_kick(uint samples, uint ch_mode, audio_callback_t handle)
{
    ///kick低优先级线程，去获取encoder_prio_trans输出模块
    src_buf_cb.samples     = samples;
    src_buf_cb.ch_mode     = ch_mode;
    src_buf_cb.handle      = handle;

    kick_src_buf_proc();
}

AT(.text.src_buf.input)
void src_buf_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    cbuf_cfg_t *cbuf = &(src_buf_cb.cbuf);

    src_buf_adj_src();

    cbuf_input_audio(ptr, samples*ch_mode, cbuf);

    if(src_buf_cb.callback != NULL) {
        src_buf_kick(samples, ch_mode, src_buf_cb.callback);
    }
}

AT(.text.src_buf.proc) WEAK
void src_buf_process(void)
{
    cbuf_cfg_t *cbuf = &(src_buf_cb.cbuf);

    uint frame_size = src_buf_cb.samples*src_buf_cb.ch_mode;
    uint total_size = cbuf_total_samples_get(cbuf);

    if(src_buf_cb.handle) {
        if (total_size >= frame_size) {
            src_buf_update_len(frame_size);
            cbuf_output_audio((u8 *)src_outcache, frame_size, cbuf);
        } else {
            memset(src_outcache, 0x00, frame_size*2);
        }

        src_buf_cb.handle(src_outcache, src_buf_cb.samples, src_buf_cb.ch_mode, NULL);
    }
}

AT(.text.src_buf)
void src_buf_audio_output_callback_set(audio_callback_t callback)
{
    src_buf_cb.callback = callback;
}

AT(.text.src_buf)
void src_buf_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&src_incache, 0, SRC_INCACHE_SIZE);
    memset(&src_outcache, 0, SRC_OUTCACHE_SIZE);

    cbuf_init(&(src_buf_cb.cbuf), src_incache, SRC_INCACHE_SIZE);
}
