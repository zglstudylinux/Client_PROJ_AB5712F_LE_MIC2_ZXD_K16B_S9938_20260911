#include "config.h"

ENTRY(_start)

//Define the flash max size
__max_flash_size = FLASH_CODE_SIZE;

__base = 0x10000000;

// retention RAM
__cache_stack_vma   = 0x10600;  //for loader
__cache_stack_size  = 512;      //for loader
__stack_vma         = 0x10800;
__stack_ram_size    = 1k;
__comm_vma          = 0x10c00;
#if WIRELESS_MIC_GTCRN_QMF_HOWLING_EN
__comm_ram_size     = 120k;
#elif (WIRELESS_MIC_YLCRN_L3_EN && DEVICE_INTERPHONE_EN)
__comm_ram_size     = 92k;
#else
__comm_ram_size     = 109k;
#endif

__global_comm_ram_size  = 289k; // 从0x10c00 开始能用的ram大小 32*6 = 192k(sram0 ~ sram5) + 100k(NPU_ram) - 3k(rom + loader + stack)
__npu_ram_size      = 100k;     // NPU使用固定地址0x40000 ~ 0x59000, 大小100k

//WS_MIC
__ws_com_ram_vma    = 0x100000; //lc3 com放在aram, 注意这个ram地址不是连续的,实际只用了16k,可以放4k其他东东
__ws_com_ram_size   = 20k;

__adapter_ram_vma   = __comm_vma + __comm_ram_size;
__adapter_ram_size  = __global_comm_ram_size - __comm_ram_size; // 188k
__emit_ram_vma      = __adapter_ram_vma;
#if (WIRELESS_MIC_YLCRN_L3_EN || WIRELESS_MIC_YLCRN_L2_EN || WIRELESS_MIC_GTCRN_QMF_HOWLING_EN || WIRELESS_MIC_YLCRN_16K_EN)
__emit_ram_size     = __global_comm_ram_size - __comm_ram_size - __npu_ram_size;// 188k - 100k = 88k
#else
__emit_ram_size     = __global_comm_ram_size - __comm_ram_size; // 188k
#endif

//addr 0x40000 - 0x59000 为NPU使用的ram
//ains4复用 0x40000 - 0x59000
__ains4_ram_vma      = 0x40000;
__ains4_ram_size     = 100k;

// non retention RAM
__aram_vma          = 0x100000;     //aec_ram
__aram_size         = 20k;
__dram_vma          = 0x108000;     //dec_ram
__dram_size         = 0x4500;       //17.25k
__eram_vma          = 0x107000;     //enc_ram
__eram_size         = 0xa00;        //2.5k
__nram_vma          = 0x40000;      //npu

MEMORY
{
    init                : org = __base,                 len = 512
    flash(rx)           : org = __base + 512,           len = __max_flash_size
    comm(rx)            : org = __comm_vma,             len = __comm_ram_size
    stack               : org = __stack_vma,            len = __stack_ram_size

    aram                : org = __aram_vma,             len = __aram_size
    dram                : org = __dram_vma,             len = __dram_size
    eram                : org = __eram_vma,             len = __eram_size
    nram                : org = __nram_vma,             len = 100k

    //WS_MIC
    com_ws_com          : org = __ws_com_ram_vma,       len = __ws_com_ram_size
    com_adapter         : org = __adapter_ram_vma,      len = __adapter_ram_size
    com_device          : org = __emit_ram_vma,         len = __emit_ram_size
    com_ains4(rx)       : org = __ains4_ram_vma,        len = __ains4_ram_size
}

SECTIONS
{
    .init : {
        *(.reset)
    } > init

    .code_comm : {
        __vertor_vma = .;
        *(.vector)
        *(.com_text*)
        *debug.o(.rodata*)
        *(.com_rodata*)
        *(.data*)
        *(.sdata*)
        *(.plt)
#if LE_BQB_RF_EN || LE_FCC_TEST_EN
        *(.text.le_dut*)
        *(.rodata.le_dut*)
        *(.buf.le_dut*)
#endif

        *(.text.drc_v3_proc*)
        *(.rodata.drc_v3_proc*)
        *(.rodata.sdadc*)
        . = ALIGN(512);
    } > comm AT > flash


    //无线麦
    .code_ws_com : {
       __code_start_lc3com = .;
        *(.text.lc3_com*)
        *(.rodata.lc3)
        *(.rodata.lc3_com*)
        . = ALIGN(4);
        __code_end_lc3com = .;

        *(.text.npu*)
        . = ALIGN(512);
    } > com_ws_com AT > flash

    .data_ws_com (NOLOAD) : {

    } > com_ws_com

    .code_adapter : {
#if ADAPTER_USB_MIC_RX_EN
        //优先分配USB内存地址(此型号usb在地址为0x40000~0x54000不可以访问)
        __code_start_usbdev = .;
        *(.usbdev.com*)
        *(.udev_buf*)
        *(.usb_buf*)
        *(.rodata.iap*)
        . = ALIGN(512);
        __code_end_usbdev = .;

        __code_start_fot = .;
        *(.ude.fot*)
        *(.fot.update*)
        __code_end_fot = .;
#endif

#if !WIRELESS_CON_FULL_DUPLEX_EN
//        __code_start_lc3com = .;
//        *(.text.lc3_com*)
//        *(.rodata.lc3)
//        . = ALIGN(4);
//        __code_end_lc3com = .;

        __code_start_lc3dec = .;
        *(.text.lc3_dec*)
        *(.rodata.lc3_dec*)
        . = ALIGN(4);
        __code_end_lc3dec = .;

        __code_start_plc = .;
        *(.text.plc_proc*)
        *(.rodata.plc*)
        . = ALIGN(4);
        __code_end_plc = .;
#endif

#if (ADAPTER_USB_MIC_RX_EN || (I2S_SAMPLES_OUT_SET == I2S_16000))
        __code_start_src = .;
        *(.text.src_proc*)
        __code_end_src = .;
#endif

#if ADAPTER_FREQ_SHIFT2_EN
        __code_start_freq_shift2 = .;
        *(.text.freq_shift2_proc*)
        *(.rodata.freq_shift2*)
        __code_end_freq_shift2 = .;
#endif

#if ADAPTER_MAGIC_EN
        __code_start_pitch = .;
        *(.text.pitch_shift_proc*)
        *(.rodata.pitch_shift*)
        __code_end_pitch = .;
#endif

#if ADAPTER_ECHO_EN
        __code_start_echo = .;
        *(.text.echo_proc*)
        *(.rodata.echo*)
        __code_end_echo = .;
#endif

#if ADAPTER_ROOM_REVERB_EN
        __code_start_room_reverb = .;
        *(.text.room_reverb_proc*)
        *(.rodata.room_reverb*)
        __code_end_room_reverb = .;
#endif

#if ADAPTER_AGC_EN
        __code_start_agc = .;
        *(.text.agc_proc*)
        *(.rodata.agc*)
        __code_end_agc = .;
#endif

#if ADAPTER_HOWLING_DNN_EN
        __code_start_howling_dnn = .;
        *(.text.howling_dnn_proc*)
        *(.rodata.howling_dnn*)
        __code_end_howling_dnn = .;
#endif

#if ADAPTER_HARDWARE_SRC1_EN
        __code_start_halfsrc = .;
        *(.text.halfsrc_proc*)
        *(.rodata.halfsrc*)
        __code_end_halfsrc = .;

#if ADAPTER_SRC_BUF_EN
        __code_start_src = .;
        *(.text.src_buf*)
        *(.text.src_proc*)
        __code_end_src = .;
#endif
#endif

#if ADAPTER_VOICE_CHANGE_V2_EN
        __code_start_voice_change_v2 = .;
        *(.text.voice_change_v2_proc*)
        *(.rodata.voice_change_v2*)
        __code_end_voice_change_v2 = .;
#endif

        __code_start_soft_eq = .;
        *(.text.soft_eq_proc*)
        *(.rodata.soft_eq*)
        __code_end_soft_eq = .;

        . = ALIGN(512);
    } > com_adapter AT > flash

    .data_adapter (NOLOAD) : {
#if !WIRELESS_CON_FULL_DUPLEX_EN
        __buf_start_lc3dec = .;
        *(.buf.lc3_dec*)
        __buf_end_lc3dec = .;

        __buf_start_plc = .;
        *(.buf.plc_buf*)
        __buf_end_plc = .;
#endif

#if (ADAPTER_USB_MIC_RX_EN || (I2S_SAMPLES_OUT_SET == I2S_16000))
        //软件src(用于USB MIC上行音调速)
        __buf_start_src = .;
        *(.buf.src*)
        __buf_end_src = .;
#endif

#if ADAPTER_FREQ_SHIFT2_EN
        __buf_start_freq_shift2 = .;
        *(.buf.freq_shift2*)
        __buf_end_freq_shift2 = .;
#endif

#if ADAPTER_SRC_BUF_EN
        __buf_start_src_buf = .;
        *(.buf.src_buf*)
        __buf_end_src_buf = .;
#endif
        __buf_start_soft_eq = .;
        *(.buf.soft_eq*)
        __buf_end_soft_eq = .;

        __buf_start_mic_mix = .;
        *(.mic_mix_buf*)
        __buf_end_pitch_mic_mix = .;

        __buf_start_xdrc = .;
        *(.xdrc_ram*)
        __buf_end__xdrc = .;

        __buf_start_prio = .;
        *(.buf.alg_prio_trans*)
        __buf_end__prio = .;

#if ADAPTER_ECHO_EN
        __buf_start_echo = .;
        *(.buf.echo*)
        __buf_end_echo = .;
#endif

#if ADAPTER_MAGIC_EN
        __buf_start_pitch = .;
        *(.buf.pitch_shift*)
        __buf_end_pitch = .;
#endif

#if ADAPTER_ROOM_REVERB_EN
        __buf_start_room_reverb = .;
        *(.buf.room_reverb*)
        __buf_end_room_reverb = .;
#endif

#if ADAPTER_AGC_EN
        __buf_start_agc = .;
        *(.buf.agc*)
        __buf_end_agc = .;
#endif

#if ADAPTER_HOWLING_DNN_EN
        __buf_start_howling_dnn = .;
        *(.buf.howling_dnn*)
        __buf_end_howling_dnn = .;
#endif

#if I2S_EN
        __buf_start_i2s = .;
        *(.buf.i2s*)
        __buf_end_i2s = .;
#endif

#if ADAPTER_VOICE_CHANGE_V2_EN
        __buf_start_voice_change_v2 = .;
        *(.buf.voice_change_v2*)
        __buf_end_voice_change_v2 = .;
#endif
        __buf_start_fot = .;
        *(.fot_data.buf)
        *(.fot_data.com*)
        __buf_end_fot = .;

    } > com_adapter

    .code_device : {
        __code_start_lc3enc = .;
        *(.text.lc3_enc*)
        *(.rodata.lc3_enc*)
        __code_end_lc3enc = .;

///开启双向 需要跑LC3_DEC+PLC, 注意和adapter分区里相关的互斥
#if WIRELESS_CON_FULL_DUPLEX_EN
        __code_start_lc3dec = .;
        *(.text.lc3_dec*)
        *(.rodata.lc3_dec*)
        . = ALIGN(4);
        __code_end_lc3dec = .;

        __code_start_plc = .;
        *(.text.plc_proc*)
        *(.rodata.plc*)
        . = ALIGN(4);
        __code_end_plc = .;
#endif

        *(.text.mic_drc*)
        *(.rodata.mic_drc*)

#if WIRELESS_MIC_HOWLING_DNN_EN
        __code_start_howling_dnn = .;
        *(.text.howling_dnn_proc*)
        *(.rodata.howling_dnn*)
        __code_end_howling_dnn = .;
#endif

#if WIRELESS_MIC_FREQ_SHIFT2_EN
        __code_start_freq_shift2 = .;
        *(.text.freq_shift2_proc*)
        *(.rodata.freq_shift2*)
        . = ALIGN(4);
        __code_end_freq_shift2 = .;
#endif

#if WIRELESS_ALLPASS_FILTER_CHANGE_EN
        __code_start_allpass_filter_change = .;
        *(.text.allpass_filter_change_proc*)
        __code_end_allpass_filter_change = .;
#endif

#if WIRELESS_MIC_ECHO_EN
        __code_start_echo = .;
        *(.text.echo_proc*)
        *(.rodata.echo*)
        . = ALIGN(4);
        __code_end_echo = .;
#endif

#if WIRELESS_MIC_ROOM_REVERB_EN
        __code_start_room_reverb = .;
        *(.text.room_reverb_proc*)
        *(.rodata.room_reverb*)
        __code_end_room_reverb = .;
#endif

#if WIRELESS_MIC_PLAT_REVERB_EN
        __code_start_plat_reverb = .;
        *(.text.plat_reverb_proc*)
        *(.rodata.plat_reverb*)
        __code_end_plat_reverb = .;
#endif

#if WIRELESS_MIC_MAGIC_EN
        __code_start_pitch = .;
        *(.text.pitch_shift2_proc*)
        *(.rodata.pitch_shift2*)
        . = ALIGN(4);
        __code_end_pitch = .;
#endif


#if WIRELESS_MIC_DNN_L1_EN
        __code_start_dnn_L1 = .;
        *(.text.dnn_L1_proc*)
        *(.rodata.dnn_L1*)
        __code_end_dnn_L1 = .;
#endif

#if WIRELESS_MIC_DNN_L3_EN
        __code_start_dnn_L3 = .;
        *(.text.dnn_L3_proc*)
        *(.rodata.dnn_L3*)
        *(.rodata1.dnn_L3*)
        *(.rodata2.dnn_L3*)
        __code_end_dnn_L3 = .;
#endif

#if WIRELESS_MIC_SRC_EN
        __code_start_halfsrc = .;
        *(.text.halfsrc_proc*)
        *(.rodata.halfsrc*)
        __code_end_halfsrc = .;
#endif

#if WIRELESS_MIC_DNR_FRE_EN
        __code_start_dnr_fre = .;
        *(.text.dnr_fre_proc*)
        *(.rodata.dnr_fre*)
        __code_end_dnr_fre = .;
#endif

#if WIRELESS_MIC_NOTCH_L2_EN
        __code_start_howling_notch_L2 = .;
        *(.text.howling_notch_L2_proc*)
        *(.rodata.howling_notch_L2*)
        __code_end_howling_notch_L2 = .;
#endif

#if WIRELESS_MIC_AGC_EN
        __code_start_agc = .;
        *(.text.agc_proc*)
        *(.rodata.agc*)
        __code_end_agc = .;
#endif

#if WIRELESS_VOICE_CHANGE_V2_EN
        __code_start_voice_change_v2 = .;
        *(.text.voice_change_v2_proc*)
        *(.rodata.voice_change_v2*)
        __code_end_voice_change_v2 = .;
#endif

#if WIRELESS_MIC_YLCRN_L3_EN
        __code_start_gtcrn_L3 = .;
        *(.text.gtcrn_L3_proc*)
        *(.rodata.gtcrn_L3*)
        __code_end_gtcrn_L3 = .;
#endif

#if WIRELESS_MIC_YLCRN_L2_EN
        __code_start_ylcrn_L2 = .;
        *(.text.ylcrn_L2_proc*)
        *(.rodata.ylcrn_L2*)
        __code_end_ylcrn_L2 = .;
#endif

#if WIRELESS_MIC_GTCRN_QMF_HOWLING_EN
        __code_start_gtcrn_qmf_howling = .;
        *(.text.gtcrn_qmf_howling_proc*)
        *(.rodata.gtcrn_qmf_howling*)
        __code_end_grcrn_qmf_howling = .;
#endif

#if WIRELESS_MIC_HARDWARE_SRC1_EN
        __code_start_halfsrc = .;
        *(.text.halfsrc_proc*)
        *(.rodata.halfsrc*)
        __code_end_halfsrc = .;
#endif

#if WIRELESS_MIC_PHASE_ROTATION_EN
        __code_start_phase_rotation = .;
        *(.text.phase_rotation_proc*)
        *(.rodata.phase_rotation*)
        __code_end_phase_rotation = .;
#endif

#if WIRELESS_MIC_YLCRN_HOWLING_EN
        __code_start_ylcrn_howling = .;
        *(.text.ylcrn_howling_proc*)
        *(.rodata.ylcrn_howling*)
        __code_end_ylcrn_howling = .;
#endif

#if WIRELESS_MIC_YLCRN_16K_EN
        __code_start_ylcrn_16k = .;
        *(.text.ylcrn_16k_proc*)
        *(.rodata.ylcrn_16k*)
        __code_end_ylcrn_16k = .;

        __code_start_src = .;
        *(.text.src_proc*)
        __code_end_src = .;

#endif

        . = ALIGN(512);
    } > com_device AT > flash

    .data_device (NOLOAD) : {
        __buf_start_lc3enc = .;
        *(.buf.lc3_enc*)
        __buf_end_lc3enc = .;

///开启双向 需要跑LC3_DEC+PLC, 注意和adapter分区里相关的互斥
#if WIRELESS_CON_FULL_DUPLEX_EN
        __buf_start_lc3dec = .;
        *(.buf.lc3_dec*)
        __buf_end_lc3dec = .;

        __buf_start_plc = .;
        *(.buf.plc_buf*)
        __buf_end_plc = .;
#endif

#if WIRELESS_MIC_YLCRN_HOWLING_EN
        __buf_start_ylcrn_howling = .;
        *(.buf.ylcrn_howling*)
        __buf_end_ylcrn_howling = .;
#endif

#if WIRELESS_MIC_PHASE_ROTATION_EN
        __buf_start_phase_rotation = .;
        *(.buf.phase_rotation*)
        __buf_end_phase_rotation = .;
#endif

#if WIRELESS_MIC_YLCRN_L3_EN
        __buf_start_gtcrn_L3 = .;
        *(.buf.gtcrn_L3*)
        __buf_end_gtcrn_L3 = .;
#endif

#if WIRELESS_MIC_YLCRN_L2_EN
        __buf_start_ylcrn_L2 = .;
        *(.buf.ylcrn_L2*)
        __buf_end_ylcrn_L2 = .;
#endif

#if WIRELESS_MIC_GTCRN_QMF_HOWLING_EN
        __buf_start_gtcrn_qmf_howling = .;
        *(.buf.gtcrn_qmf_howling*)
        __buf_end_grcrn_qmf_howling = .;
#endif

#if WIRELESS_VOICE_CHANGE_V2_EN
        __buf_start_voice_change_v2 = .;
        *(.buf.voice_change_v2*)
        __buf_end_voice_change_v2 = .;
#endif

#if WIRELESS_MIC_HOWLING_DNN_EN
        __buf_start_howling_dnn = .;
        *(.buf.howling_dnn*)
        __buf_end_howling_dnn = .;
#endif

#if WIRELESS_MIC_FREQ_SHIFT2_EN
        __buf_start_freq_shift2 = .;
        *(.buf.freq_shift2*)
        __buf_end_freq_shift2 = .;
#endif

#if WIRELESS_ALLPASS_FILTER_CHANGE_EN
         __buf_start_allpass_filter_change = .;
        *(.buf.allpass_filter_change*)
         __buf_end_allpass_filter_change = .;
#endif

#if WIRELESS_MIC_ECHO_EN
        __buf_start_echo = .;
        *(.buf.echo*)
        __buf_end_echo = .;
#endif

#if WIRELESS_MIC_ROOM_REVERB_EN
        __buf_start_room_reverb = .;
        *(.buf.room_reverb*)
        __buf_end_room_reverb = .;
#endif

#if WIRELESS_MIC_PLAT_REVERB_EN
        __buf_start_plat_reverb = .;
        *(.buf.plate_reverb*)
        __buf_end_plat_reverb = .;
#endif


#if WIRELESS_MIC_MAGIC_EN
        __buf_start_pitch = .;
        *(.buf.pitch_shift*)
        __buf_end_pitch = .;
#endif

#if WIRELESS_MIC_DNN_L1_EN
        __buf_start_dnn_L1 = .;
        *(.buf.dnn_L1*)
        __buf_end_dnn_L1 = .;
#endif

#if WIRELESS_MIC_DNN_L3_EN
        __buf_start_dnn_L3 = .;
        *(.buf.dnn_L3*)
        __buf_end_dnn_L3 = .;
#endif

#if WIRELESS_MIC_DNR_FRE_EN
        __buf_start_dnr_fre = .;
        *(.buf.dnr_fre*)
        __buf_end_dnr_fre = .;
#endif

#if WIRELESS_MIC_NOTCH_L2_EN
        __buf_start_howling_notch_L2 = .;
        *(.buf.howling_notch_L2*)
        __buf_end_howling_notch_L2 = .;
#endif

#if WIRELESS_MIC_YLCRN_16K_EN
        __buf_start_ylcrn_16k = .;
        *(.buf.ylcrn_16k*)
        __buf_end_ylcrn_16k = .;

        //软件src(用于USB MIC上行音调速)
        __buf_start_src = .;
        *(.buf.src*)
        __buf_end_src = .;
#endif

#if WIRELESS_MIC_AGC_EN
        __buf_start_agc = .;
        *(.buf.agc*)
        __buf_end_agc = .;
#endif

#if WIRELESS_INTERPHONE_AEC_EN
        __buf_start_aec = .;
        *(.buf.aec*)
        __buf_end_aec = .;
#endif
    } > com_device

    //AINS4
    .code_ains4 : {
        __code_start_ains4 = .;
        *(.text.ains4_proc*)
        *(.rodata.ains4*)
        *(.rodata1.ains4*)
        __code_end_ains4 = .;
        . = ALIGN(512);
    } > com_ains4 AT > flash

    .data_ains4 (NOLOAD) : {
        __buf_start_ains4 = .;
        *(.buf.ains4*)
        __buf_end_ains4 = .;
    } > com_ains4

    .flash : {
        __code_start_stream = .;
        *(.com_stream.text*)
        . = ALIGN(512);
        __code_end_stream = .;

        __code_start_mp3dec = .;
        *(.mp3dec*)
        . = ALIGN(512);
        __code_end_mp3dec = .;

        __code_start_sbcdec = .;
        *(.sbcdec*)
        *(.sbc_com*)
        . = ALIGN(512);
        __code_end_sbcdec = .;

        __code_start_aacdec = .;
        *(.aacdec*)
        . = ALIGN(512);
        __code_end_aacdec = .;

        __code_start_wavdec = .;
        *(.wavdec*)
        . = ALIGN(512);
        __code_end_wavdec = .;

        __code_start_wmadec = .;
        *(.wmadec*)
        . = ALIGN(512);
        __code_end_wmadec = .;

        __code_start_apedec = .;
        *(.apedec*)
        . = ALIGN(512);
        __code_end_apedec = .;

        __code_start_flacdec = .;
        *(.flacdec*)
        . = ALIGN(512);
        __code_end_flacdec = .;

        __code_start_lhdc_dec = .;
        *(.text.lhdc.dec.*)
        *(.rodata.lhdc.dec.*)
        . = ALIGN(512);
        __code_end_lhdc_dec = .;

//        __code_start_usbdev = .;
//        *(.usbdev*)
//        . = ALIGN(512);
//        __code_end_usbdev = .;

        __code_start_wma_rlc44q = .;
        *(.wma_huff_rlc44q_table)
        . = ALIGN(512);
        __code_end_wma_rlc44q = .;

        __code_start_wma_rlc44o = .;
        *(.wma_huff_rlc44o_table)
        . = ALIGN(512);
        __code_end_wma_rlc44o = .;

        __code_start_wma_rlc16 = .;
        *(.wma_huff_rlc16_table)
        . = ALIGN(512);
        __code_end_wma_rlc16 = .;

        __code_start_sleep = .;
        *(.saradc_text*)
        *(.sleep_text*)
        . = ALIGN(512);
        __code_end_sleep = .;

        __code_start_charge = .;
        *(.charge_text*)
        *(.text.charge)
        *(.text.bsp.tkey)
        *(.text.charge_box)
        . = ALIGN(512);
        __code_end_charge = .;

        __code_start_bt_voice = .;
        *(.bt_voice*)
        *(.sco_mic_dnr*)
        . = ALIGN(512);
        __code_end_bt_voice = .;

        __code_start_pwrsave = .;
        *(.text.pwroff*)
        . = ALIGN(512);
        __code_end_pwrsave = .;

        __code_start_bb_test = .;
        *(.bb_test*)
        *(.bb_test_rodata*)
        . = ALIGN(512);
        __code_end_bb_test = .;

         __code_start_piano = .;
        *(.piano_com*)
        . = ALIGN(512);
        __code_end_piano = .;

        __code_start_record = .;
        *(.com_rec*)
        . = ALIGN(512);
        __code_end_record = .;

        __code_start_mav = .;
        *(.text.mav*)
        *(.rodata.mav*)
        *(.text.pitch_shift*)
        *(.rodata.pitch_shift*)
        . = ALIGN(512);
        __code_end_mav = .;

        __code_start_update = .;
        *(.text.update*)
        . = ALIGN(512);
        __code_end_update = .;

        __code_start_huart = .;
        *(.com_huart*)
        . = ALIGN(512);
        __code_end_huart = .;

        __code_start_spdif = .;
        *(.spdif_text*)
        . = ALIGN(512);
        __code_end_spdif = .;

        __code_start_fota = .;
        *(.text.fot.cache*)
        . = ALIGN(512);
        __code_end_fota = .;

        __code_start_spiflash_id = .;
        *(.text.spiflash.*)
        . = ALIGN(512);
        __code_end_spiflash_id = .;

        __code_start_opus = .;
        *(.text.opus.asm)
        *(.text.opus.proc)
        *(.text.opus.ec_tell)
        *(.text.opus.entenc)
        *(.text.opus.mathops)
        *(.rodata.opus*)
        . = ALIGN(512);
        __code_end_opus = .;

        __code_start_bts = .;
        *(.text.stk.tws.txreq*)
        *(.text.stk.l2cap.txreq*)
        . = ALIGN(512);
        *(.text.stk.tws.send*)
        . = ALIGN(512);
        *(.text.stk.tws.recv*)
        . = ALIGN(512);
        *(.text.stk.hci.tx*)
        . = ALIGN(512);
        *(.text.bts.hci.rx*)
        . = ALIGN(512);
        *(.text.stk.bt.acl.tx*)
        . = ALIGN(512);
        __code_end_bts = .;

        *(.irq_init.aligned)
        *(.text.qtest.process)
        *(.text.charge.process)
        *(.text.charge_box.process)
        *(.text.ab_mate.process)
        *(.text.func.bt.process)
        . = ALIGN(512);
        *(.text.ble.chmap*)
        . = ALIGN(512);

        *(.text*)
        *(.rodata*)
        *(.srodata*)
        *(.rela*)
        LONG(0)
        . = ALIGN(512);

    } > flash

    .stack (NOLOAD) : {
        __irq_stack_start = .;
        . = 0x400;
        __irq_stack = .;
    } > stack
    __irq_stack_size = __irq_stack - __irq_stack_start;

    .data_comm (NOLOAD) : {
        . = ALIGN(4);
        __heap_start = .;
        *(.mem_heap)
        __heap_end = .;

        __bss_start = .;
        *(COMMON)
        *(.bss*)
        *(.sbss*)
        *(.buf*)
        *(.qtest_buf)
        *(.dac_obuf*)
        *(.adc_buf*)
        *(.wav_res_buf)
        __bss_end = .;

        *(.mp3_dbg_buf*)

        ///bram
        . = ALIGN(4);
        __btmem_start = .;
        *(.btmem.bthw)
        *(.btmem.bb*)
        *(.btmem.bt*)
        *(.btmem.ble*)
        *(.btmem*)
//        *(.btmem.stack.sdp)
//        *(.btmem.stack.spp)
//        *(.btmem.stack.l2cap)
//        *(.btmem.stack.hid*)

        *(.ble_cache*)
        *(.ble_buf*)
        __btmem_end = .;


    } > comm
#if WIRELESS_INTERPHONE_AEC_EN
    .aram_aec __aram_vma (NOLOAD) : {
        *(.aec_buf)
    } > aram
#else
    .aram_music __aram_vma (NOLOAD) : {
        *(.mp3buf.dec)

        *(.lrc_buf*)
        *(.fnav_buf)
        *(.pff.scan.tempbuf)
        *(.avio_buf)
        *(.diskio_buf)
        *(.pff.buf)
        *(.pff_buf*)
        *(.codecs_buf.*)
        . = 0x4800;
    } > aram
#endif
    .dram_upd __dram_vma (NOLOAD) : {
        . = 0x3800;
        *(.upd_ota*)
        *(.upd_buf*)
        . = 0x4500;
    } > dram

    .dram_mp3 __dram_vma (NOLOAD) : {
        . = 0x1000;
        *(.mp3_decram_buf)
        . = 0x4500;
    } > dram

    .dram_usb __dram_vma (NOLOAD) : {
        *(.usb.device.desc.buf)
        *(.ude.aubuf)
        . = 0x3000;
    } > dram

    .dram_i2s __dram_vma (NOLOAD) : {
        *(.i2s_buf*)
        . = 0x3000;
    } > dram
#if WIRELESS_INTERPHONE_AEC_EN
    .dram_interphone_aec __dram_vma (NOLOAD) : {
        *(.interphone_buf*)
        *(.mic_aec_buf)
    } > dram
#endif
    .dram_eq __dram_vma (NOLOAD) : {
        *(.eq_buf*)
        . = 0x3000;
    } > dram

    .eram_fot __eram_vma (NOLOAD) : {
//        *(.fot_data.buf)
        . = 0xa00;
    } > eram

#if (WIRELESS_MIC_YLCRN_L3_EN || WIRELESS_MIC_YLCRN_L2_EN || WIRELESS_MIC_GTCRN_QMF_HOWLING_EN || WIRELESS_MIC_YLCRN_16K_EN)
    .nram_npu __nram_vma (NOLOAD) : {
        *(.npu_weight)
        *(.npu_ram0)
        *(.npu_ram1)
        . = 0x19000;
    } > nram
#endif

}

//Calc the lma
__bss_size = __bss_end - __bss_start;
__bank_size = SIZEOF(.flash);
__comm_lma = LOADADDR(.code_comm);
__comm_size = SIZEOF(.code_comm);


//无线麦adapter
__code_ws_com_vma       = __ws_com_ram_vma;
__code_ws_com_lma       = LOADADDR(.code_ws_com);
__code_ws_com_size      = SIZEOF(.code_ws_com);
__data_ws_com_size      = SIZEOF(.data_ws_com);
__code_apapter_vma      = __adapter_ram_vma;
__code_apapter_lma      = LOADADDR(.code_adapter);
__code_apapter_size     = SIZEOF(.code_adapter);
__data_adapter_size     = SIZEOF(.data_adapter);
__code_emit_vma         = __emit_ram_vma;
__code_emit_lma         = LOADADDR(.code_device);
__code_emit_size        = SIZEOF(.code_device);
__data_emit_size        = SIZEOF(.data_device);

//AINS4
__code_ains4_vma         = __ains4_ram_vma;
__code_ains4_lma         = LOADADDR(.code_ains4);
__code_ains4_size        = SIZEOF(.code_ains4);
__data_ains4_size        = SIZEOF(.data_ains4);
