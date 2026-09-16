#ifndef __CONFIG_EXTRA_H__
#define __CONFIG_EXTRA_H__

/*****************************************************************************
 * Module    : SDK版本配置
 *****************************************************************************/
#ifndef SDK_VERSION
    #define SDK_VERSION                 0x0150      //V015.0
#endif

/*****************************************************************************
 * Module    : 系统功能配置
 *****************************************************************************/
#undef  SYS_ADJ_DIGVOL_EN
#define SYS_ADJ_DIGVOL_EN               1

#ifndef SYS_MODE_BREAKPOINT_EN
#define SYS_MODE_BREAKPOINT_EN          0
#endif // SYS_MODE_BREAKPOINT_EN

#ifndef PLUGIN_SYS_INIT_FINISH_CALLBACK
#define PLUGIN_SYS_INIT_FINISH_CALLBACK 0
#endif

#ifndef PLUGIN_FUNC_IDLE_ENTER_CHECK
#define PLUGIN_FUNC_IDLE_ENTER_CHECK    0
#endif


#ifndef FLASH_SPEED_UP_EN
#define FLASH_SPEED_UP_EN               1
#endif

#ifndef MUSIC_AAC_SUPPORT
#define MUSIC_AAC_SUPPORT               0
#endif // MUSIC_AAC_SUPPORT


#if !EQ_DBG_IN_UART
#undef EQ_DBG_IN_UART_VUSB_EN
#define EQ_DBG_IN_UART_VUSB_EN          0
#endif

#define USER_NOIS_INIT()
#define USER_LED_NOISE_ON()
#define USER_LED_NOISE_OFF()

/*****************************************************************************
 * Module    :  充电仓功能选择
 *****************************************************************************/
#if !CHARGE_EN
    #undef CHARGE_BOX_EN
    #define CHARGE_BOX_EN         0
#endif


/*****************************************************************************
 * Module    :  UART功能选择（待整理）
 *****************************************************************************/
#if EQ_DBG_IN_UART || BT_SCO_DUMP_EN/*(强制打开HUART)*/
    #define HUART_EN                    1
#endif

/*****************************************************************************
 * Module    : 音乐功能配置
 *****************************************************************************/
#if !CHARGE_EN
#undef  CHARGE_TRICK_EN
#undef  CHARGE_DC_IN
#undef  CHARGE_DC_NOT_PWRON
#define CHARGE_TRICK_EN             0
#define CHARGE_DC_NOT_PWRON         0
#define CHARGE_DC_IN()              0
#endif

#ifndef PWROFF_PRESS_TIME
#define PWROFF_PRESS_TIME           3               //(n+3)*500
#endif // PWROFF_PRESS_TIME

#ifndef PWRON_PRESS_TIME
#define PWRON_PRESS_TIME            1500           //1.5s
#endif // PWRON_PRESS_TIME

#ifndef DOUBLE_KEY_TIME
#define DOUBLE_KEY_TIME             1
#endif // DOUBLE_KEY_TIME

#ifndef SYS_LIMIT_VOLUME
#define SYS_LIMIT_VOLUME            5
#endif // SYS_LIMIT_VOLUME

#ifndef LINEIN_2_PWRDOWN_TONE_EN
#define LINEIN_2_PWRDOWN_TONE_EN    0   //插入Linein直接关机，是否播放关机提示音。
#endif // LINEIN_2_PWRDOWN_TONE_EN

#ifndef LPWR_WARING_TIMES
#define LPWR_WARING_TIMES           0xff
#endif // LPWR_WARING_TIMES

#if !BUZZER_EN
#undef BUZZER_INIT
#define BUZZER_INIT()
#undef BUZZER_ON
#define BUZZER_ON()
#undef BUZZER_OFF
#define BUZZER_OFF()
#endif

#ifndef LED_LOWBAT_EN
#define LED_LOWBAT_EN              0
#endif // RLED_LOWBAT_EN

#ifndef RLED_LOWBAT_FOLLOW_EN
#define RLED_LOWBAT_FOLLOW_EN       0
#endif // RLED_LOWBAT_FOLLOW_EN

#ifndef SLEEP_DAC_OFF_EN
#define SLEEP_DAC_OFF_EN            1
#endif // SLEEP_DAC_OFF_EN

#ifndef PWRON_FRIST_BAT_EN
#define PWRON_FRIST_BAT_EN          1
#endif // PWRON_FRIST_BAT_EN

#if !LED_DISP_EN
#undef LED_LOWBAT_EN
#undef RLED_LOWBAT_FOLLOW_EN
#undef BLED_FFT_EN
#undef BLED_LOW2ON_EN
#undef BLED_CHARGE_FULL_EN
#undef BT_RECONN_LED_EN
#define LED_LOWBAT_EN              0
#define RLED_LOWBAT_FOLLOW_EN       0
#define BLED_FFT_EN                 0
#define BLED_LOW2ON_EN              0
#define BLED_CHARGE_FULL_EN         0
#define BT_RECONN_LED_EN            0
#endif

#if !LED_DISP_EN
#undef LED_INIT
#define LED_INIT()
#undef LED_SET_ON
#define LED_SET_ON()
#undef LED_SET_OFF
#define LED_SET_OFF()
#endif

#if !LED_PWR_EN
#undef LED_PWR_INIT
#define LED_PWR_INIT()
#undef LED_PWR_SET_ON
#define LED_PWR_SET_ON()
#undef LED_PWR_SET_OFF
#define LED_PWR_SET_OFF()
#endif

#undef GUI_LCD_EN
#define GUI_LCD_EN                      0
#undef  MUSIC_NAVIGATION_EN
#define MUSIC_NAVIGATION_EN             0


#ifndef IR_INPUT_NUM_MAX
#define IR_INPUT_NUM_MAX                999         //最大输入数字9999
#endif // IR_INPUT_NUM_MAX

#ifndef FMRX_THRESHOLD_VAL
#define FMRX_THRESHOLD_VAL              128
#endif // FMRX_THRESHOLD_VAL

#ifndef FMRX_SEEK_DISP_CH_EN
#define FMRX_SEEK_DISP_CH_EN            0
#endif // FMRX_SEEK_DISP_CH_EN


#if ADKEY_MUX_LED_EN
#undef  ADKEY_EN
#undef  ADKEY_MUX_SDCLK_EN
#undef  ADKEY_PU10K_EN

#define ADKEY_EN                      1
#define ADKEY_MUX_SDCLK_EN            0
#define ADKEY_PU10K_EN                  0
#endif // ADKEY_MUX_LED_EN


#ifndef DAC_DRC_EN
#define DAC_DRC_EN                      0
#endif

#if !PWRKEY_EN
#undef PWRKEY_IS_PRESS
#define PWRKEY_IS_PRESS()               0
#endif

#if !TKEY_EN
#undef  TKEY_SOFT_PWR_EN
#undef  TKEY_INEAR_EN
#undef  TKEY_DEBUG_EN
#undef  TKEY_IS_PRESS
#define TKEY_SOFT_PWR_EN           0
#define TKEY_INEAR_EN                 0
#define TKEY_DEBUG_EN              0
#define TKEY_IS_PRESS()                 0
#endif

#if !TKEY_SOFT_PWR_EN
#undef TKEY_LOWPWR_WAKEUP_DIS
#define TKEY_LOWPWR_WAKEUP_DIS     0
#endif


/*****************************************************************************
 * Module    : 录音功能配置
 *****************************************************************************/
#if !FUNC_REC_EN
#undef  FMRX_REC_EN
#undef  AUX_REC_EN
#undef  MIC_REC_EN
#undef  REC_TYPE_SEL
#undef  REC_AUTO_PLAY
#undef  REC_FAST_PLAY
#undef  BT_REC_EN
#undef  BT_HFP_REC_EN
#undef  KARAOK_REC_EN
#undef  REC_STOP_MUTE_1S

#define FMRX_REC_EN                 0
#define AUX_REC_EN                  0
#define MIC_REC_EN                  0
#define REC_AUTO_PLAY               0
#define REC_FAST_PLAY               0
#define BT_REC_EN                   0
#define BT_HFP_REC_EN               0
#define KARAOK_REC_EN               0
#define REC_TYPE_SEL                REC_NO
#define REC_STOP_MUTE_1S            0
#endif //FUNC_REC_EN


#if FMRX_INSIDE_EN
#undef  FMRX_QN8035_EN
#define FMRX_QN8035_EN              0
//#else
//#undef  I2C_SW_EN
//#define I2C_SW_EN                   1
#endif

#if !I2C_SW_EN
#undef  I2C_MUX_SD_EN
#define I2C_MUX_SD_EN               0
#endif

/*****************************************************************************
 * Module    : 蓝牙音乐算法配置
 *****************************************************************************/
#if (!BT_MUSIC_EFFECT_EN) && (BT_MUSIC_EFFECT_DBB_EN || BT_MUSIC_EFFECT_USER_EN)
#error "please set the macro BT_MUSIC_EFFECT_EN!"
#endif

#if !BT_MUSIC_EFFECT_DBB_EN
#undef BT_MUSIC_EFFECT_DBB_BAND_CNT
#define BT_MUSIC_EFFECT_DBB_BAND_CNT    0
#endif

/*****************************************************************************
 * Module    : 通话算法相关配置
 *****************************************************************************/
#define BT_SCO_SMIC_EN                  0
#define BT_SCO_DMIC_EN                  0

#if (BT_SCO_NR_EN || BT_SNDP_SMIC_EN || BT_SCO_SMIC_AI_EN || BT_SCO_NR_USER_SMIC_EN)
#undef BT_SCO_SMIC_EN
#define BT_SCO_SMIC_EN                  1
#endif

#if (BT_SNDP_DMIC_EN || BT_SCO_DMIC_AI_EN || BT_SCO_NR_USER_DMIC_EN || BT_SNDP_FBDM_EN || BT_SNDP_DM_EN)
#undef BT_SCO_DMIC_EN
#define BT_SCO_DMIC_EN                  1
#endif

#if !BT_SCO_SMIC_EN
#undef SMIC_DBG_EN
#define SMIC_DBG_EN                     0
#endif

#if !BT_SCO_DMIC_EN
#undef DMIC_DBG_EN
#define DMIC_DBG_EN                     0
#endif

#if BT_SCO_SMIC_EN && BT_SCO_DMIC_EN
#error "NR warning: please choose only one of the NR algorithms at the same time!\n"
#endif

#if BT_AEC_NLP_BYPASS && BT_AEC_NLP_ONLY
#error "BT_AEC_EN warning: (BT_AEC_NLP_BYPASS==1) and (BT_AEC_NLP_ONLY==1) is not support!\n"
#endif

#if BT_SCO_NR_EN && (BT_SNDP_SMIC_EN || BT_SCO_SMIC_AI_EN)
#error "NR warning: please select only one nr algorithm in BT_SCO_NR_EN, BT_SNDP_SMIC_EN, BT_SCO_SMIC_AI_EN\n"
#endif

#if (BT_SCO_DUMP_EN || BT_AEC_DUMP_EN || BT_SCO_FAR_DUMP_EN)
#error "DUMP warning: please select only one nr algorithm in BT_XXX_DUMP_EN, VUSB_XXX_EN\n"
#endif

#if BT_AEC_DUMP_EN && (BT_SCO_DUMP_EN || BT_SCO_FAR_DUMP_EN || BT_EQ_DUMP_EN)
#error "DUMP warning: please select only one dump in BT_AEC_DUMP_EN, BT_SCO_DUMP_EN, BT_SCO_FAR_DUMP_EN, BT_EQ_DUMP_EN\n"
#endif

#if BT_SCO_DUMP_EN && (BT_SCO_FAR_DUMP_EN || BT_EQ_DUMP_EN)
#error "DUMP warning: please select only one dump in BT_SCO_DUMP_EN, BT_SCO_FAR_DUMP_EN, BT_EQ_DUMP_EN\n"
#endif

#if BT_SCO_FAR_DUMP_EN && (BT_EQ_DUMP_EN)
#error "DUMP warning: please select only one dump in BT_SCO_FAR_DUMP_EN, BT_EQ_DUMP_EN\n"
#endif

/*****************************************************************************
 * Module    : 无线mic功能选择配置
 *****************************************************************************/
#if WIRELESS_SPK_TX_EN //&& WIRELESS_MIC_TX_EN
    #define WIRELESS_CON_FULL_DUPLEX_EN     1       //是否支持全双工
  #if WIRELESS_CON_VERS == 1 || WIRELESS_CON_VERS == 2 || WIRELESS_CON_VERS == 3
    #error "V2/V3/V4 is unsupport full duplex!"     //V2 & V3 & V4 只支持单向传输
  #endif

#elif WIRELESS_MIC_TX_EN
  #if WIRELESS_MIC_FRAME_SIZE == 0
    #error "WIRELESS_MIC_FRAME_SIZE=0 is not allowed!"
  #endif
    #undef ADAPTER_USB_SPK_TX_EN
    #undef ADAPTER_AUX_TX_EN
    #undef WIRELESS_SPK_FRAME_SIZE
    #undef WIRELESS_SPK_DUMP_PER_BER
    #undef WIRELESS_SPK_LOCAL_OUTPUT_EN
    #undef WIRELESS_SPK_ENC_MAX_US
    #undef WIRELESS_SPK_DEC_MAX_US
    #undef WIRELESS_SPK_RETRY_NB
    #define ADAPTER_USB_SPK_TX_EN           0
    #define ADAPTER_AUX_TX_EN               0
    #define WIRELESS_SPK_FRAME_SIZE         0
    #define WIRELESS_SPK_ENC_MAX_US         0
    #define WIRELESS_SPK_DEC_MAX_US         0
    #define WIRELESS_SPK_RETRY_NB           0
    #define WIRELESS_CON_FULL_DUPLEX_EN     0       //是否支持全双工
#else
    #error "At least support WIRELESS_SPK_TX_EN or WIRELESS_MIC_TX_EN!"
#endif

#if WIRELESS_MIC_SAMPLE_RATE_SELECT == SAMPLE_RATE_16K
    #define FRAME_SIZE_MIN                      20      //1.25ms
#elif WIRELESS_MIC_SAMPLE_RATE_SELECT == SAMPLE_RATE_24K
    #define FRAME_SIZE_MIN                      30      //1.25ms
#elif WIRELESS_MIC_SAMPLE_RATE_SELECT == SAMPLE_RATE_32K
    #define FRAME_SIZE_MIN                      40      //1.25ms
#elif WIRELESS_MIC_SAMPLE_RATE_SELECT == SAMPLE_RATE_48K
    #define FRAME_SIZE_MIN                      60      //1.25ms
#else
    #error "WIRELESS_MIC_SAMPLE_RATE_SELECT is not allowed!"
#endif

#if WIRELESS_MIC_SAMPLES_SELECT < FRAME_SIZE_MIN
    #error "WIRELESS_MIC_SAMPLES_SELECT is not allowed!"
#endif
#if (WIRELESS_MIC_SAMPLES_SELECT % FRAME_SIZE_MIN) != 0
    #error "WIRELESS_MIC_SAMPLES_SELECT is not allowed!"
#endif

#define WIRELESS_MIC_DFU_TX_INTERVAL            (WIRELESS_MIC_SAMPLES_SELECT/FRAME_SIZE_MIN)
#ifndef WIRELESS_MIC_TX_INTERVAL
    #define WIRELESS_MIC_TX_INTERVAL            WIRELESS_MIC_DFU_TX_INTERVAL
#endif

#if WIRELESS_CON_VERS < 64

    #define WIRELESS_CON_INTERVAL               60

    #define WIRELESS_CON_COMB_BUF_EN            0       //单包
    #define WIRELESS_CON_CRC_EN                 1

  #if WIRELESS_MIC_TX_INTERVAL == 0
    #error "WIRELESS_MIC_TX_INTERVAL is not allowed!"
  #elif WIRELESS_MIC_TX_INTERVAL == 1                   //1.25ms传不了几byte，组合成2.5ms
    #undef WIRELESS_MIC_TX_INTERVAL
    #define WIRELESS_MIC_TX_INTERVAL            1
    #define WIRELESS_MIC_COMB_NB                WIRELESS_MIC_TX_INTERVAL
    #define WIRELESS_SPK_COMB_NB                WIRELESS_MIC_TX_INTERVAL
  #else
    #define WIRELESS_MIC_COMB_NB                (WIRELESS_MIC_TX_INTERVAL/WIRELESS_MIC_DFU_TX_INTERVAL)
    #define WIRELESS_SPK_COMB_NB                (WIRELESS_MIC_TX_INTERVAL/WIRELESS_MIC_DFU_TX_INTERVAL)
  #endif

  #if (WIRELESS_MIC_SAMPLES_SELECT*WIRELESS_MIC_COMB_NB)%(WIRELESS_MIC_TX_INTERVAL*FRAME_SIZE_MIN) != 0
    #error "WIRELESS_MIC_TX_INTERVAL is not allowed!"
  #endif
  #if WIRELESS_MIC_TX_INTERVAL == 0
    #error "WIRELESS_MIC_TX_INTERVAL is not allowed!"
  #endif
//  #if WIRELESS_MIC_SAMPLES_SELECT%(WIRELESS_MIC_TX_INTERVAL*FRAME_SIZE_MIN) != 0
//    #error "WIRELESS_MIC_TX_INTERVAL is not allowed!"
//  #endif
  #if WIRELESS_CON_INTERVAL%WIRELESS_MIC_TX_INTERVAL != 0
    #error "WIRELESS_CON_INTERVAL is not allowed!"
  #endif

  #if WIRELESS_CON_VERS == 9
    #define WIRELESS_MIC_2TNR_EN                 1
    #define WIRELESS_CON_2TNR_NB                 2           ///两发N收的rx数量, 暂时仅支持配置为2
  #endif
#else
    #error "WIRELESS_CON_VERS is not allowed!"
#endif

#if WIRELESS_MIC_COMB_NB == 0
    #undef WIRELESS_MIC_COMB_NB
    #define WIRELESS_MIC_COMB_NB                1
#endif

#if ADAPTER_LOCAL_MIC_MIX_EN
#undef  SYS_CLK_SEL
#define SYS_CLK_SEL                    SYS_120M
#endif

#ifndef WIRELESS_MIC_BROADCAST_EN
    #define WIRELESS_MIC_BROADCAST_EN            (WIRELESS_CON_VERS == 8)
#endif

/*****************************************************************************
 * Module    : 无线配对码功能配置
 *****************************************************************************/
#if WIRELESS_CON_PAIR_MODE

#undef ADAPTER_SET_TX1_CON_ID              //此处设置的为RX端连接两个TX的配对码，需要在发射端分别设置对应的WIRELESS_CON_CON_ID_SEL配合使用
#undef ADAPTER_SET_TX2_CON_ID
#define ADAPTER_SET_TX1_CON_ID      0x05
#define ADAPTER_SET_TX2_CON_ID      0x00

#else

#undef ADAPTER_SET_TX1_CON_ID
#undef ADAPTER_SET_TX2_CON_ID
#define ADAPTER_SET_TX1_CON_ID      0x00
#define ADAPTER_SET_TX2_CON_ID      0x00


#endif

/*****************************************************************************
 * Module    : ANC相关配置
 *****************************************************************************/
#if ANC_EN
#undef DAC_DNR_EN
#undef SYS_ADJ_DIGVOL_EN
#undef TINY_TRANSPARENCY_EN
#undef DAC_OUT_SPR

#define DAC_DNR_EN                  0       //关掉动态降噪
#define SYS_ADJ_DIGVOL_EN           1       //DAC固定模拟增益调数字增益
#define TINY_TRANSPARENCY_EN        0       //开ANC不能打开小通透
#define DAC_OUT_SPR                 DAC_OUT_48K
#endif

#if TINY_TRANSPARENCY_EN
#undef DAC_DNR_EN
#undef SYS_ADJ_DIGVOL_EN

#define DAC_DNR_EN                  0       //关掉动态降噪
#define SYS_ADJ_DIGVOL_EN           1       //DAC固定模拟增益调数字增益
#endif

#if !ANC_EN
#undef ANC_EQ_RES2_EN
#define ANC_EQ_RES2_EN              0        //如果ANC关闭的情况下，不能开启优先使用资源2
#endif

#if ANC_EQ_RES2_EN
#undef FLASH_RESERVE_SIZE
#define FLASH_RESERVE_SIZE          0x6000
#endif

#if ANC_ALG_EN

#else

#endif // ANC_ALG_EN

/*****************************************************************************
 * Module    : uart0 printf 功能自动配置(自动关闭SD卡，USB)
 *****************************************************************************/
#if (UART0_PRINTF_SEL == PRINTF_NONE)
    //关闭所以打印信息
    #undef printf
    #undef vprintf
    #undef print_r
    #undef print_r16
    #undef print_r32
    #undef printk
    #undef vprintk
    #undef print_kr
    #undef print_kr16
    #undef print_kr32

    #define printf(...)
    #define vprintf(...)
    #define print_r(...)
    #define print_r16(...)
    #define print_r32(...)
    #define printk(...)
    #define vprintk(...)
    #define print_kr(...)
    #define print_kr16(...)
    #define print_kr32(...)
#endif

/*****************************************************************************
 * Module    : I2S 功能配置
 *****************************************************************************/
#if I2S_EN
#if I2S_MODE_SEL
#undef  I2S_DMA_EN
#define I2S_DMA_EN                          1   //slave模式必须打开DMA功能
#endif

#if ADAPTER_I2S_TX_EN
#undef ADAPTER_AUX_TX_EN
#define ADAPTER_AUX_TX_EN                   1   //I2S TX依赖于AUX中断时序进行编码
#endif
#endif // I2S_EN


/*****************************************************************************
 * Module    : SRC 功能配置
 *****************************************************************************/
#if ADAPTER_USB_MIC_STEREO_EN || ADAPTER_USB_SPK_TX_EN || ADAPTER_I2S_TX_EN
#define ADAPTER_HARDWARE_SRC1_EN              1  //硬件SRC进行调速
#else
#define ADAPTER_HARDWARE_SRC1_EN              0
#endif

#if ADAPTER_USB_SPK_TX_EN
#define ADAPTER_SRC_BUF_EN                    1   //使用SRC_BUF对USB音频进行存点
#else
#define ADAPTER_SRC_BUF_EN                    0
#endif

#if WIRELESS_MIC_I2S_INPUT_EN
#define WIRELESS_MIC_HARDWARE_SRC1_EN         1
#else
#define WIRELESS_MIC_HARDWARE_SRC1_EN         0
#endif

#if WIRELESS_MIC_HARDWARE_SRC1_EN && ADAPTER_HARDWARE_SRC1_EN
  #error "WIRELESS_MIC_HARDWARE_SRC1_EN and ADAPTER_HARDWARE_SRC1_EN is not allowed!"
#endif

/*****************************************************************************
 * Module    : 提示音配置
 *****************************************************************************/
#if (!WARNING_TONE_EN)
#undef WARNING_POWER_ON
#undef WARNING_POWER_OFF
#undef WARNING_FUNC_MUSIC
#undef WARNING_FUNC_BT
#undef WARNING_FUNC_CLOCK
#undef WARNING_FUNC_FMRX
#undef WARNING_FUNC_AUX
#undef WARNING_FUNC_USBDEV
#undef WARNING_FUNC_SPEAKER
#undef WARNING_LOW_BATTERY
#undef WARNING_BT_CONNECT
#undef WARNING_BT_DISCONNECT
#undef WARNING_BT_INCALL
#undef WARNING_USB_SD
#undef WARNING_BT_HID_MENU
#undef WARNING_BTHID_CONN
#undef WARNING_MAX_VOLUME
#undef WARNING_BT_PAIR

#define WARNING_POWER_ON               0
#define WARNING_POWER_OFF              0
#define WARNING_FUNC_MUSIC             0
#define WARNING_FUNC_BT                0
#define WARNING_FUNC_CLOCK             0
#define WARNING_FUNC_FMRX              0
#define WARNING_FUNC_AUX               0
#define WARNING_FUNC_USBDEV            0
#define WARNING_FUNC_SPEAKER           0
#define WARNING_LOW_BATTERY            0
#define WARNING_BT_CONNECT             0
#define WARNING_BT_DISCONNECT          0
#define WARNING_BT_INCALL              0
#define WARNING_USB_SD                 0
#define WARNING_BT_HID_MENU            0
#define WARNING_BTHID_CONN             0
#define WARNING_MAX_VOLUME             0
#define WARNING_BT_PAIR                0
#endif


#ifndef UPD_FILENAME
#define UPD_FILENAME                   "fw5000.upd"
#endif


/*****************************************************************************
 * Module    : usb device 功能配置
 *****************************************************************************/
#if (!ADAPTER_USB_MIC_RX_EN)
    #undef  UDE_STORAGE_EN
    #undef  UDE_SPEAKER_EN
    #undef  UDE_HID_EN
    #undef  UDE_MIC_EN
    #undef  UDE_ENUM_TYPE

    #define UDE_STORAGE_EN              0
    #define UDE_IAP_EN                  0
    #define UDE_SPEAKER_EN              0
    #define UDE_HID_EN                  0
    #define UDE_MIC_EN                  0
    #define UDE_ENUM_TYPE               0
#else
    #undef  UDE_STORAGE_EN
    #undef  UDE_IAP_EN
    #undef  UDE_SPEAKER_EN
    #undef  UDE_HID_EN
    #undef  UDE_MIC_EN
    #undef  UDE_ENUM_TYPE

    #define UDE_STORAGE_EN              1
    #define UDE_IAP_EN                  ADAPTER_MFI_EN          //与UDE_STORAGE_EN只能2选1
    #define UDE_SPEAKER_EN              ADAPTER_USB_SPK_TX_EN   //设置设备为speaker#if FUNC_TEST_PRINT//调试打印

    #define UDE_HID_EN                  1
    #define UDE_MIC_EN                  1
    #define UDE_IAP_PROCESS_EN          ADAPTER_MFI_EN    //是否使用底层iAP协议发送
    #define SOFT_I2C_EN                 ADAPTER_MFI_EN    //IAP使用的软件iic协议接口
    #define UDE_ENUM_TYPE               (UDE_STORAGE_EN*0x01 + UDE_SPEAKER_EN*0x02 + UDE_HID_EN*0x04 + UDE_MIC_EN*0x08+ UDE_IAP_EN*0x10)
	#define UDE_ONLY_STORAGE_TYPE       (UDE_STORAGE_EN*0x01 + 0*0x02 + 0*0x04 + 0*0x08)
    #define UDE_ON_STORAGE_TYPE         (0*0x01 + UDE_SPEAKER_EN*0x02 + UDE_HID_EN*0x04 + UDE_MIC_EN*0x08)
    #define UDE_ONLY_SPEAKER_TYPE       (0*0x01 + UDE_SPEAKER_EN*0x02 + 0*0x04 + 0*0x08 + 0*0x10)
#endif

#if !UDE_IAP_EN
    #undef UDE_IAP_PROCESS_EN
    #define UDE_IAP_PROCESS_EN          0
#endif

#define USB_SUPPORT_EN                 (ADAPTER_USB_MIC_RX_EN)

/*****************************************************************************
* Module    : 有冲突或功能上不能同时打开的宏
*****************************************************************************/

#define DAC_EQ_NUM_TOTAL               (DAC_PT_NUM + EQ_APP_NUM + BT_MUSIC_EFFECT_DBB_BAND_CNT)

#if (DAC_EQ_NUM_TOTAL > 20)
#error "error: (DAC_PT_NUM + EQ_APP_NUM + BT_MUSIC_EFFECT_DBB_BAND_CNT) > 20"
#endif

#if (WIRELESS_MIC_ECHO_EN && ADAPTER_ECHO_EN)
#error "WIRELESS_MIC_ECHO_EN && ADAPTER_ECHO_EN is not allowed!"
#endif

#if (WIRELESS_MIC_MAGIC_EN && ADAPTER_MAGIC_EN)
#error "WIRELESS_MIC_MAGIC_EN && ADAPTER_MAGIC_EN is not allowed!"
#endif

#if ADAPTER_AB_FOT_EN
#undef WIRELESS_CON_INTERVAL
#undef ADAPTER_NORMAL_LE_EN
#define WIRELESS_CON_INTERVAL                   10
#define ADAPTER_NORMAL_LE_EN                    1
#endif


#if WIRELESS_MIC_DAC_OUTPUT_EN || WIRELESS_SPK_DAC_OUTPUT_EN
#define DEVICE_DAC_OUTPUT                       1
#else
#define DEVICE_DAC_OUTPUT                       0
#endif
/*****************************************************************************
 * Module    : 算法相关配置
 *****************************************************************************/
#if SOFT_DRC_DUMP_EN && (!MIC_DRC_EN)
    #error "SOFT_DRC_DUMP_EN is not allowed!"
#endif

#define TOOLKIT_AUDIO_DUMP_EN                   (SOFT_DRC_DUMP_EN)


/*****************************************************************************
* Module    : 计算FunctionKey
*****************************************************************************/
//#if BT_SNDP_SMIC_EN
//    #define FUNCKEY_SNDP_NS             0xf8789d28
//#else
//    #define FUNCKEY_SNDP_NS             0
//#endif
#endif // __CONFIG_EXTRA_H__

