/*****************************************************************************
 * Module    : Config
 * File      : config.h
 * Function  : SDK配置文件
 *****************************************************************************/

#ifndef USER_CONFIG_CLIP_MIC_H
#define USER_CONFIG_CLIP_MIC_H
#include "config_define.h"


/*****************************************************************************
 * Module    : Function选择相关配置
 *****************************************************************************/
#define FUNC_BT_EN                      1   //是否打开蓝牙功能
#define FUNC_IDLE_EN                    0   //是否打开IDLE功能
#define FUNC_LE_DUT_EN                  0   //是否打开DUT测试模式

/*****************************************************************************
 * Module    : 系统功能选择配置
 *****************************************************************************/
#define SYS_CLK_SEL                     SYS_24M                 //选择系统时钟
#define BUCK_MODE_EN                    xcfg_cb.buck_mode_en    //是否BUCK MODE
#define POWKEY_10S_RESET                xcfg_cb.powkey_10s_reset
#define SOFT_POWER_ON_OFF               1                       //是否使用软开关机功能
#define PWRKEY_2_HW_PWRON               0                       //用PWRKEY模拟硬开关
#define GUI_SELECT                      GUI_NO                  //GUI Display Select
#define UART0_PRINTF_SEL                PRINTF_VUSB             //选择UART打印信息输出IO，或关闭打印信息输出
#define SYS_PARAM_RTCRAM                0                       //是否系统参数保存到RTCRAM
#define PWRON_ENTER_BTMODE_EN           0                       //是否上电默认进蓝牙模式
#define SLEEP_DAC_OFF_EN                (is_sleep_dac_off_enable()) //sfunc_sleep是否关闭DAC， 复用MICL检测方案不能关DAC。
#define SYS_INIT_VOLUME                 xcfg_cb.sys_init_vol        //系统默认音量
#define SYS_LIMIT_VOLUME                xcfg_cb.sys_recover_initvol //开机最小音量，避免开机时音量太小误认为没开机
#define BSP_TBOX_TEST_EN                0                       //是否通过PB3做串口，用于dut测试指令

/*****************************************************************************
 * Module    : SPIFLASH配置
 *****************************************************************************/
#define FLASH_SIZE                      FSIZE_1M              //LQFP48芯片内置1MB，其它封装芯片内置512KB(实际导出prd文件要小于492K)
#define FLASH_CODE_SIZE                 450K                    //程序使用空间大小，code_size <= flash_size - [params(20k) + res2(4k) + res.bin]
#define FLASH_RESERVE_SIZE              0x5000                  //程序保留区空间大小，打开ANC_EQ_RES2_EN需要保留24K
#define FLASH_ERASE_4K                  1                       //是否支持4K擦除
#define FLASH_DUAL_READ                 1                       //是否支持2线模式
#define FLASH_QUAD_READ                 0                       //是否支持4线模式
#define FLASH_SPEED_UP_EN               1                       //SPI FLASH提速。

/*****************************************************************************
 * Module    : 蓝牙功能配置
 *****************************************************************************/
#define LE_FCC_TEST_EN                  0   //BLE FCC测试使能,双线串口通信,RX-PB3,TX-PB4,波特率1500000(需要同时打开FUNC_LE_DUT_EN)
#define LE_BQB_RF_EN                    0   //BLE DUT测试模式,双线串口通信,RX-PB3,TX-PB4,波特率9600(需要同时打开FUNC_LE_DUT_EN)

/*****************************************************************************
 * Module    : 无线功能选择配置
 *****************************************************************************/
//公共配置
#define WIRELESS_CON_CODEC_SEL                  WS_CODEC_LC3S   //编解码选择
#define WIRELESS_CON_FREQ_BAND                  0               //无线mic工作频段 0:2402M~2480M  1:2200M~2278M  2:2320M~2398M  3:2500M~2578M  4: 2482M~2560M
#define WIRELESS_CON_LINK_NB                    1               //支持连几路无线麦（目前只支持1/2路）
#define WIRELESS_CON_VERS                       6               //传输机制版本(2=V3一般用于k歌话筒, 6=V7一般用于领夹麦, 8=V9用于一拖多)
#define WIRELESS_CON_CRC_EN                     1               //链路是否支持CRC校验
#define WIRELESS_CON_BONDING_EN                 0               //组队绑定
#define WIRELESS_CON_RSSI_THR                   70              //设置连接范围RSSI(-dbm)，该值越小，组队范围越小(产线建议设成70左右，以防距离太远也能配)
#define WIRELESS_CON_PWR_CTR                    0               //是否支持自动调节pwr，近距离降低，远距离增加
#define WIRELESS_SPK_TX_EN                      0               //适配器是否支持SPK发射（下行音频）
#define WIRELESS_MIC_TX_EN                      1               //设备端是否支持MIC发射（上行音频）
#define WIRELESS_DUMP_EN                        0               //打印无线PER、RSSI等信息（调试用）
#define WIRELESS_CON_CON_ID_SEL                 0x00            //无线mic连接id，需要发射端跟接收端id一致才可连接，可以调用ble_set_con_id来动态修改id，实现动态连接
#define WIRELESS_CON_PAIR_MODE                  0               //是否使能配对码功能(一拖二两个发射端连接id不同,需要配合WIRELESS_CON_CON_ID_SEL使用)


//无线MIC配置（上行音频，DEVICE --> ADAPTER）
#define WIRELESS_MIC_SAMPLE_RATE_SELECT     SAMPLE_RATE_48K //无线mic采样率选择
#define WIRELESS_MIC_SAMPLES_SELECT         120             //无线mic每帧采样样点数选择  16k：20, ADPCM压缩后是samples*2/4+3
#define WIRELESS_MIC_CHANNEL_SELECT         2               //无线mic声道选择，1为单声道，2为双声道（目前只支持单声道）
#define WIRELESS_MIC_FRAME_SIZE             50              //每帧压缩后大小（FRAME_SIZE*LINK_NB*RETRY_NB暂时不能超过72byte）
#define WIRELESS_MIC_RETRY_NB               2               //重传次数（暂时支持1/2）
#define WIRELESS_MIC_ENC_MAX_US             1200             //编码运算时长（单位us）
#define WIRELESS_MIC_DEC_MAX_US             1900            //解码运算时长（单位us，开PLC时约900us）
#define WIRELESS_MIC_TX_INTERVAL            2               //传输周期=n*1.25ms（默认值是2，改为4时传输次数可增加到5次）


//设备端私有配置
#define WIRELESS_MIC_STEREO                     1               //发射端立体声功能使能宏，开启后传输立体声音频
#define WIRELESS_MIC_DNN_L1_EN                  0               //发射端DNN_L1算法使能 (发射端处理，默认开启后主频会抬至160M)
#define WIRELESS_MIC_DNN_L3_EN                  0               //发射端DNN_L3算法使能 (发射端处理，默认开启后主频会抬至160M)
#define WIRELESS_MIC_ECHO_EN                    0               //发射端是否开启 ECHO音效
#define WIRELESS_MIC_ECHO_DELAY                 WIRELESS_MIC_ECHO_EN*120//ECHO运算时间
#define WIRELESS_MIC_MAGIC_EN                   0               //发射端是否开启 MAGIC音效
#define WIRELESS_MIC_MAGIC_DELAY                WIRELESS_MIC_MAGIC_EN*220//MAGIC运算时间
#define WIRELESS_MIC_DAC_OUTPUT_EN              0               //发射端是否使能DAC输出
#define MIC_DRC_EN                              0               //发射器是否开启数字增益控制
#define WIRELESS_MIC_SOFT_DRC_DELAY             MIC_DRC_EN*300  //硬件SRC运算时间
#define WIRELESS_MIC_SRC_EN                     0               //硬件SRC重采样模块 32K -> 48k 80samples ->120samples
#define WIRELESS_MIC_SRC_DELAY                  WIRELESS_MIC_SRC_EN*361//硬件SRC运算时间
#define WIRELESS_MIC_AINS4_EN                   0               //AINS4算法使能  (发射端处理，默认开启后主频会抬至160M)
#define WIRELESS_MIC_AGC_EN                     0               //发射端AGC算法使能(720样点对齐)
#define WIRELESS_MIC_AGC_DELAY                  WIRELESS_MIC_AGC_EN*0//AGC 运算时间
#define WIRELESS_MIC_EQ_DRC_EN                   0
#define WIRELESS_MIC_EQ_DRC_DELAY               WIRELESS_MIC_EQ_DRC_EN*323//EQ 运算时间
#define WIRELESS_MIC_ROOM_REVERB_EN             0               //房间混响使能
#define WIRELESS_MIC_ROOM_REVERB_DELAY          WIRELESS_MIC_ROOM_REVERB_EN*355//房间混响运算时间
#define WIRELESS_ALLPASS_FILTER_CHANGE_EN       0               //发射端是否开启 随机相位(防啸叫)
#define WIRELESS_ALLPASS_FILTER_CHANGE_DELAY    WIRELESS_ALLPASS_FILTER_CHANGE_EN*200//随机相位运算时间
#define WIRELESS_MIC_FREQ_SHIFT2_EN             0
#define WIRELESS_FREQ_SHIFT2_DELAY              WIRELESS_MIC_FREQ_SHIFT2_EN*950//随机相位运算时间
#define WIRELESS_MIC_NOTCH_L2_EN                0               //NOTCH_L2算法使能,需要配合硬件eq（WIRELESS_MIC_EQ_DRC_EN） 以及 eqchange使用否则效果和延时不佳
#define WIRELESS_MIC_HOWLING_DNN_EN             0               //HOWLING_DNN使能
#define WIRELESS_MIC_I2S_INPUT_EN               0               //设备端I2S输出功能
#define WIRELESS_VOICE_CHANGE_V2_EN             0               //变音
#define WIRELESS_MIC_PLAT_REVERB_EN             0
#define WIRELESS_MIC_PLAT_REVERB_DELAY          WIRELESS_MIC_PLAT_REVERB_EN * 940
#define WIRELESS_MIC_PHASE_ROTATION_EN          0               //设备端是否开启 相位旋转
#define WIRELESS_MIC_YLCRN_L3_EN                0               //设备端是否开启 YLCRN_L3 AI DNN大模型自研降噪算法
#define WIRELESS_MIC_YLCRN_L2_EN                0               //设备端是否开启 YLCRN_L2 AI DNN大模型自研降噪算法(10ms帧)
#define WIRELESS_MIC_YLCRN_HOWLING_EN           0               //设备端是否开启 YLCRN_HOWLING
#define WIRELESS_MIC_GTCRN_QMF_HOWLING_EN       0               //设备端是否开启 GTCRN_QMF_HOWLING 防啸叫算法（10ms帧）
#define WIRELESS_MIC_DNR_FRE_EN                 0
#define WIRELESS_MIC_DNR_FRE_DELAY              WIRELESS_MIC_DNR_FRE_EN * 720   //发射端DNR_FRE运算时间
#define WIRELESS_MIC_YLCRN_16K_EN               0
#define WIRELESS_MIC_YLCRN_16K_DELAY            WIRELESS_MIC_YLCRN_16K_EN * 240

//适配器端私有配置
#define ADAPTER_AGC_EN                          0               //AGC算法使能
#define ADAPTER_ECHO_EN                         0               //混响使能
#define ADAPTER_MAGIC_EN                        0               //魔音使能
#define ADAPTER_FREQ_SHIFT2_EN                  0               //移频使能
#define ADAPTER_HOWLING_DNN_EN                  0               //HOWLING_DNN使能
#define ADAPTER_DAC_OUTPUT_EN                   1               //适配器dac输出
#define ADAPTER_MIX_DRC_EN                      0               //混音DRC功能(用于一拖二混音,还有noisegate)
#define ADAPTER_MIX_DRC_DELAY                   ADAPTER_MIX_DRC_EN*300
#define ADAPTER_MIC_EQ_DRC_EN                   0               //接收端是否开启EQ/DRC功能(可以处理无线MIC音频和本地MIC混合后的音频)
#define ADAPTER_SOFT_EQ_EN                      0               //soft_eq
#define ADAPTER_SOFT_EQ_DELAY                   ADAPTER_SOFT_EQ_EN*300
#define ADAPTER_LOCAL_MIC_MIX_EN                0               //接收端本地麦使能，支持EQ/DRC 是否连接无线麦都可以工作
#define ADAPTER_LOCAL_MIC_EQ_DRC_EN             0               //接收端本地麦是否使能支持EQ/DRC(仅对本地MIC进行EQ/DRC处理,不能和ADAPTER_MIC_DRC_EN同时开)
#define APAPTER_LOCAL_AUX_EN                    0               //接收端本地输入是否改为AUX输入，默认MIC输入
#define ADAPTER_MFI_EN                          0               //是否支持MFI认证功能(根据实际硬件,修改I2C通信引脚)
#define ADAPTER_USB_MIC_RX_EN                   0               //接收端是否打开usb mic#if FUNC_TEST_PRINT//调试打印
#define ADAPTER_USB_UPDATE_EN                   0               //适配器是否支持usb升级功能(若编译后代码超过492k，需要把FLASH_SIZE改成2M才能支持FOTA)
#define ADAPTER_USB_MIC_STEREO_EN               0               //接收端usb mic是否支持真立体声(USB默认已经是假的立体声,若使用I2S之类的输入可以使用USB真立体声)
#define ADAPTER_I2S_OUTPUT_EN                   0               //接收端是否打开I2S输出
#define ADAPTER_I2S_IN_OUT_EN                   0               //接收端是否支持I2S双向传输(主机模式)
#define ADAPTER_ROOM_REVERB_EN                  0               //房间混响使能
#define ADAPTER_NORMAL_LE_EN                    0               //适配器是否支持多开一条普通BLE连接，可用于支持FOTA或简单透传
#define ADAPTER_AB_FOT_EN                       0               //适配器是否支持FOTA升级，若编译后代码超过492k，需要把FLASH_SIZE改成2M才能支持FOTA
#define ADAPTER_AB_FOT_DEVICE_SUPPORT           0 				//适配器FOTA支持同步升级发射端
#define AB_FOT_TYPE             				AB_FOT_TYPE_NORMAL //适配器FOTA类型选择
#define ADAPTER_SOFT_GAIN_AFTER_MIX_EN          0               //适配器mix后软件增益
#define ADAPTER_VOICE_CHANGE_V2_EN              0               //变音(暂不支持)
#define ADAPTER_USB_STORAGE_AUDIO_CLK         SYS_160M          //USB audio和Storage共存功能同时开启主频抬上去，避免挂在SD卡失败
#define ADAPTER_USB_STORAGE_AUDIO               0               //USB audio和Storage共存功能
/*****************************************************************************
 * Module    : SD0配置
 *****************************************************************************/
#define MUSIC_SDCARD_EN                    0
#define SD0_MAPPING                     SD0MAP_G2   //选择SD0 mapping
#define SD_DETECT_INIT()                sdcard_detect_init()
#define SD_IS_ONLINE()                  sdcard_is_online()
#define IS_DET_SD_BUSY()                is_det_sdcard_busy()
/*****************************************************************************
 * Module    : WS算法参数配置
 *****************************************************************************/
#define YLCRN_L3_EN                             (WIRELESS_MIC_YLCRN_L3_EN) //AI DNN大模型自研降噪算法
#define YLCRN_L3_DUMP_EN                        0               //打印降噪算法数据（调试用）

#define YLCRN_L2_EN                             (WIRELESS_MIC_YLCRN_L2_EN || ADAPTER_YLCRN_L2_EN)           //YLCRN_L2 DNN大模型自研降噪算法
#define YLCRN_L2_DUMP_EN                        0               //打印降噪算法数据（调试用）

//AI DNN小模型自研降噪算法
#define DNN_L3_EN                               WIRELESS_MIC_DNN_L3_EN               //DNN降噪使能
#define DNN_L3_LEVEL                            2900

//AINS4降噪算法
#define AINS4_EN                                (WIRELESS_MIC_AINS4_EN)
#define AINS4_NOISE_NT                          15500            //0~15000   最好每500一个step，值越大底噪越大，值越小底噪越小

//AI DNN小模型自研降噪算法
#define DNN_L1_EN                               WIRELESS_MIC_DNN_L1_EN               //DNN降噪使能
#define DNN_L1_LEVEL                            3                // range: -10 ~ 10 默认0

//混响算法
#define ECHO_EN                                 (WIRELESS_MIC_ECHO_EN || ADAPTER_ECHO_EN) //宏只能二选一，同时打开会编译报错
#define ECHO_LEVEL			                    70              //attenuation  range:0-90
#define ECHO_DRY_USER                           32767           //干度 range:0-32767
#define ECHO_WET_USER                           20000           //湿度 range:0-32767
#define ECHO_DELAY_MAX_LEVEL                    9               //echo延迟等级划分，按照固定的间隔划分成这么多等级，用于后续等级加减
#define ECHO_DELAY_DEFAULT_LEVEL                8               //echo第一次上电默认等级 range:1-ECHO_DELAY_MAX_LEVEL

//板式混响算法
#define PLAT_REVERB_EN                          WIRELESS_MIC_PLAT_REVERB_EN

//房间混响算法
#define ROOM_REVERB_EN                          (WIRELESS_MIC_ROOM_REVERB_EN||ADAPTER_ROOM_REVERB_EN)
#define ROOM_REVERB_LEVEL			            99              //衰减率range:0-100
#define ROOM_REVERB_DRY			                32767           //干度 range:0-65535 数值越大人声音越突出
#define ROOM_REVERB_WET			                26000           //湿度 range:0-32767 数值越大人混响效果越突出

//魔音算法
#define MAGIC_EN                                (WIRELESS_MIC_MAGIC_EN || ADAPTER_MAGIC_EN) //宏只能二选一，同时打开会编译报错
#define MAGIC_EFFECT_DEFAULT_LEVEL              1               //magic第一次上电默认效果等级，0:原声，1:男神，2:女神，3:娃娃音，4:魔兽

//AGC算法
#define AGC_EN                                  (WIRELESS_MIC_AGC_EN || ADAPTER_AGC_EN)

//NOTCH_L2啸叫算法
#define NOTCH_L2_EN                             (WIRELESS_MIC_NOTCH_L2_EN)

#define HOWLING_DNN_EN                          (WIRELESS_MIC_HOWLING_DNN_EN || ADAPTER_HOWLING_DNN_EN)
#define SOFT_EQ_EN                              ADAPTER_SOFT_EQ_EN

//随机相位算法
#define ALLPASS_FILTER_CHANGE_EN                (WIRELESS_ALLPASS_FILTER_CHANGE_EN || ADAPTER_ALLPASS_FILTER_CHANGE_EN)

#define PHASE_ROTATION_EN                       (WIRELESS_MIC_PHASE_ROTATION_EN)
#define YLCRN_HOWLING_EN                        WIRELESS_MIC_YLCRN_HOWLING_EN
//移频算法
#define FREQ_SHIFT2_EN                          (WIRELESS_MIC_FREQ_SHIFT2_EN || ADAPTER_FREQ_SHIFT2_EN)

#define VOICE_CHANGE_V2_EN                      (ADAPTER_VOICE_CHANGE_V2_EN || WIRELESS_VOICE_CHANGE_V2_EN)

#define GTCRN_QMF_HOWLING_EN                    (WIRELESS_MIC_GTCRN_QMF_HOWLING_EN)

//DNR FRE轻量降噪
#define DNR_FRE_EN                              (WIRELESS_MIC_DNR_FRE_EN || ADAPTER_DNR_FRE_EN)

#define YLCRN_16K_EN                            (WIRELESS_MIC_YLCRN_16K_EN)
/*****************************************************************************
* Module    : SDDAC配置控制
******************************************************************************/
#define DAC_DIFF_DIS                    0                           //DAC关掉VCMBUF模式（优化flash空间）
#define DAC_VCMBUF_DIS                  0                           //DAC关掉差分模式（优化flash空间）
#define DAC_CH_SEL                      xcfg_cb.dac_sel             //DAC_MONO ~ DAC_VCMBUF_DUAL
#define DAC_FAST_SETUP_EN               0                           //DAC快速上电，有噪声需要外部功放MUTE
#define DAC_MAX_GAIN                    xcfg_cb.dac_max_gain        //配置DAC最大模拟增益，默认设置为dac_vol_table[VOL_MAX]
#define DAC_24BITS_EN                   xcfg_cb.dac_24bits_en
#define DAC_OUT_SPR                     xcfg_cb.dac_spr_sel         //dac out sample rate
#define DAC_VCM_CAPLESS_EN              xcfg_cb.dac_vcm_less_en     //DAC VCM省电容方案,使用内部VCM
#define DAC_MAXOUT_EN                   xcfg_cb.dac_maxout_en       //DAC大功率模式
#define DAC_PULL_DOWN_DELAY             80                          //控制DAC隔直电容的放电时间, 无电容时可设为0，减少开机时间。
#define DAC_DNR_EN                      1                           //是否使能动态降噪
#define DAC_DRC_EN                      1                           //是否使能DRC功能(After EQ0)
#define DAC_OFF_FOR_BT_CONN_EN          xcfg_cb.dac_off_for_conn

#define DAC_PT_EN                       0                           //是否打开DAC产测校准功能
#define DAC_PT_NUM                      4                           //DAC产测使用EQ条数(最多6条)

/*****************************************************************************
* Module    : EQ配置
******************************************************************************/
#define EQ_MODE_EN                      0           //是否调节EQ MODE (POP, Rock, Jazz, Classic, Country)
#define EQ_DBG_IN_UART                  1           //是否使能HUART在线调节EQ

/*****************************************************************************
 * Module    : 调音工具配置
******************************************************************************/
#define EFFECT_DBG_ADJUST_EN            1          //是否使能音效离线调试
#define EFFECT_DBG_ADJUST_IN_UART       1          //是否使能UART在线调试音效,该功能需要打开EFFECT_DBG_ADJUST_EN和EQ_DBG_IN_UART

/*****************************************************************************
 * Module    : User按键配置 (可以同时选择多组按键)
 *****************************************************************************/
//按键通用配置
#define KEY_TBL_MAX_NB                  3
#define KEY_MSG_MAX_IDX                 6           //需要响应的按键消息
#define DOUBLE_KEY_TIME                 (xcfg_cb.double_key_time)                       //按键双击响应时间（单位50ms）
#define PWRON_PRESS_TIME                (500*xcfg_cb.pwron_press_time)                  //长按PWRKEY多长时间开机
#define PWROFF_PRESS_TIME               xcfg_cb.pwroff_press_time                       //长按PWRKEY多长时间关机

//pwrkey
#define PWRKEY_EN                       1           //PWRKEY的使用，0为不使用
#define PWRKEY_IS_PRESS()			    pwrkey_is_pressed()

//touch key
#define TKEY_EN                         0           //TouchKEY的使用，0为不使用
#define TKEY_SOFT_PWR_EN                0           //是否使用TouchKey进行软开关机
#define TKEY_LOWPWR_WAKEUP_DIS          0           //是否电池低电时关掉触摸唤醒, 无保护板的电池需要打开。
#define TKEY_INEAR_EN                   0           //是否使用TouchKey的入耳检测功能
#define TKEY_TEMP_EN                    0           //是否使用TouchKey的温度检测功能
#define TKEY_DEBUG_EN                   0           //仅调试使用，用于确认TKEY的参数
#define TKEY_IS_PRESS()			        (tkey_is_pressed() && TKEY_SOFT_PWR_EN)

//adkey
#define ADKEY_EN                        0           //ADKEY的使用， 0为不使用
#define ADKEY_CH                        ADCCH_PE7
#define ADKEY_MUX_SDCLK_EN              0           //是否使用复用SDCLK的ADKEY, 共用USER_ADKEY的按键table
#define ADKEY_MUX_LED_EN                0           //是否使用ADKEY与LED复用, 共用USER_ADKEY的流程(ADKEY与BLED配置同一IO)
#define ADKEY_PU10K_EN                  1           //ADKEY是否使用内部10K上拉, 按键数量及阻值见port_key.c
//adkey2
#define ADKEY2_EN                       0           //ADKEY2的使用，0为不使用
#define ADKEY2_CH                       ADCCH_PB1

//io key
#define IOKEY_EN                        0           //IOKEY的使用， 0为不使用
//knob
#define KNOB_KEY_EN                     0           //旋钮的使用，0为不使用
#define KNOB_KEY_LEVEL                  16          //旋钮的级数




/*****************************************************************************
 * Module    : LED指示灯配置
 *****************************************************************************/
#define LED_DISP_EN                     1           //是否使用LED指示灯(蓝灯)
#define LED_PWR_EN                      1           //充电及电源指示灯(红灯)
#define LED_LOWBAT_EN                   1                           //电池低电是否闪红灯
#define BLED_CHARGE_FULL                xcfg_cb.charge_full_bled    //充电满是否亮蓝灯
#define BT_RECONN_LED_EN                0//xcfg_cb.bt_reconn_led_en    //蓝牙回连状态是否不同的闪灯方式

#define LED_INIT()                      bled_func.port_init(&bled_gpio)
#define LED_SET_ON()                    bled_func.set_on(&bled_gpio)
#define LED_SET_OFF()                   bled_func.set_off(&bled_gpio)

#define LED_PWR_INIT()                  rled_func.port_init(&rled_gpio)
#define LED_PWR_SET_ON()                rled_func.set_on(&rled_gpio)
#define LED_PWR_SET_OFF()               rled_func.set_off(&rled_gpio)

/*****************************************************************************
 * Module    : 电量检测及低电
 *****************************************************************************/
#define VBAT_DETECT_EN                  1           //电池电量检测功能
#define VBAT_FILTER_USE_PEAK            0           //电池检测滤波方式: 0=取平均值，1=取峰值(适用于播音乐时电池波动较大的音箱方案).
#define LPWR_WARNING_VBAT               xcfg_cb.lpwr_warning_vbat   //低电提醒电压
#define LPWR_OFF_VBAT                   xcfg_cb.lpwr_off_vbat       //低电关机电压
#define LPWR_REDUCE_VOL_EN              0                           //低电是否降低音量
#define LPWR_WARING_TIMES               0xff                        //报低电次数

/*****************************************************************************
 * Module    : 充电功能选择
 *****************************************************************************/
#define CHARGE_EN                       1           //是否打开充电功能
#define CHARGE_TRICK_EN                 xcfg_cb.charge_trick_en     //是否打开涓流充电功能
#define CHARGE_DC_NOT_PWRON             xcfg_cb.charge_dc_not_pwron //DC插入，是否软开机。 1: DC IN时不能开机
#define CHARGE_VOLT_FOLLOW_EN           xcfg_cb.charge_voltage_follow // 打开电压跟随模式，适用于支持快充的充电仓
#define CHARGE_DC_IN()                  ((RTCCON >> 20) & 0x01)
#define CHARGE_INBOX()                  ((RTCCON >> 22) & 0x01)
#define CHARGE_LOW_POWER_EN             0           //是否打开充电低功耗模式,打开此宏时，不能 INTF_HUART 传输数据
#define CHARGE_USER_UI_CONFIG_EN        1           //是否使能充电客制化ui配置

//充电截止电流
#define CHARGE_STOP_CURR                xcfg_cb.charge_stop_curr
//充电截止电压：0:4.2v 1:4.35v 2:4.4v 3:4.45v
#define CHARGE_STOP_VOLT                0
//恒流充电（电池电压大于2.9v）电流
#define CHARGE_CONSTANT_CURR            xcfg_cb.charge_constant_curr
//涓流截止电压：0:2.9v; 1:3v
#define CHARGE_TRICK_STOP_VOLT          1
//涓流充电（电池电压小于2.9v）电流
#define CHARGE_TRICKLE_CURR             xcfg_cb.charge_trickle_curr
//恒压差充电差值选择：0:187.5mV  1:250mV  2:312mV  3:375mV
#define CHARGE_VOLT_FOLLOW_DIFF         3

/*****************************************************************************
 * Module    :  充电仓功能选择
 *****************************************************************************/
#define CHARGE_BOX_EN                   0               //是否打开充电仓功能
#define CHARGE_BOX_TYPE                 CBOX_NOR        //充电仓类型选择，0=普通充电仓，1=昇生微智能充电仓
#define CHARGE_BOX_INTF_SEL             INTF_NONE       //充电仓通信接口选择
#define CHARGE_BOX_KEEP_VOLT            xcfg_cb.ch_inbox_sel   //仓维持电压选择0=1.1V, 1=1.7V

/*****************************************************************************
 * Module    : usb device 功能选择
 *****************************************************************************/
#define USB_DET_VER_SEL                 0                           //USB插入检测方式,0-旧方式,1-新方式

#define UDE_PROD_NAME                   "USB Mirophone"    //最大36个字符
#define UDE_SUPPLIER                    "Generic"          //最大18个字符
#define UDE_SERIAL_NB                   "20250201905926"   //最大16个字符
#define UDE_PID                         0x0124             //最大4位
#define UDE_VID                         0x0C21             //最大4位
/*****************************************************************************
 * Module    : Sensor配置
 *****************************************************************************/
#define SC7A20_EN						0			//是否使能敲击芯片功能（需要配置I2C）

/*****************************************************************************
 * Module    : I2C配置
 *****************************************************************************/
#define I2C_HW_EN                       0           //是否使能硬件I2C功能
#define I2C_MAPPING                     I2CMAP_PA14PA13 //选择I2C mapping

#define I2C_SW_EN                       0           //是否使能软件I2C功能
#define I2C_MUX_SD_EN                   0           //是否I2C复用SD卡的IO

#if I2C_MUX_SD_EN
#define I2C_SCL_IN()                    SD_CMD_DIR_IN()
#define I2C_SCL_OUT()                   SD_CMD_DIR_OUT()
#define I2C_SCL_H()                     SD_CMD_OUT_H()
#define I2C_SCL_L()                     SD_CMD_OUT_L()
#define I2C_SDA_IN()                    SD_DAT_DIR_IN()
#define I2C_SDA_OUT()                   SD_DAT_DIR_OUT()
#define I2C_SDA_H()                     SD_DAT_OUT_H()
#define I2C_SDA_L()                     SD_DAT_OUT_L()
#define I2C_SDA_IS_H()                  SD_DAT_STA()
#else
#define I2C_SCL_IN()                    {GPIOFDIR |= BIT(0); GPIOFPU  |= BIT(0);}
#define I2C_SCL_OUT()                   {GPIOFDE |= BIT(0); GPIOFDIR &= ~BIT(0);}
#define I2C_SCL_H()                     {GPIOFSET = BIT(0);}
#define I2C_SCL_L()                     {GPIOFCLR = BIT(0);}
#define I2C_SDA_IN()                    {GPIOFDIR |= BIT(1); GPIOFPU  |= BIT(1);}
#define I2C_SDA_OUT()                   {GPIOFDE |= BIT(1); GPIOFDIR &= ~BIT(1);}
#define I2C_SDA_H()                     {GPIOFSET  = BIT(1);}
#define I2C_SDA_L()                     {GPIOFCLR = BIT(1);}
#define I2C_SDA_IS_H()                  (GPIOF & BIT(1))
#endif

#define I2C_SDA_SCL_OUT()               {I2C_SDA_OUT(); I2C_SCL_OUT();}
#define I2C_SDA_SCL_H()                 {I2C_SDA_H(); I2C_SCL_H();}

/*****************************************************************************
 * Module    : UART0配置
 *****************************************************************************/
#define UART0_EN                        0
#define UART0_BAUD_RATE                 115200
#define UART0_MAPPING                   UART0_PB4PB3

/*****************************************************************************
 * Module    : I2S配置
 *****************************************************************************/
#define I2S_EN                          (I2S_AUDIO_IN_EN||I2S_AUDIO_OUT_EN||I2S_AUDIO_IN_OUT_EN)  //是否使能I2S功能
#define I2S_DEVICE                      I2S_DEV_NO          //I2S设备选择
#define I2S_MAPPING_SEL                 I2S_IO_G3           //I2S IO口选择
#define I2S_BIT_MODE                    I2S_32BIT           //I2S数据位宽选择 16bit;32bit
#define I2S_DATA_MODE                   I2S_NORMAL          //I2S数据格式选择 left-justified mode; normal mode(left-justified mode 待调试)
#define I2S_DMA_EN                      1                   //I2S数据源选择 0:src; 1:dma(src 模式待调试)
#define I2S_MCLK_EN                     1                   //I2S_MASTER是否打开MCLK
#define I2S_MCLK_SEL                    2                   //I2S MCLK选择 0:64fs 1:128fs 2:256fs
#define I2S_PCM_MODE                    0                   //I2S是否打开PCM mode(PCM mdoe 待调试)
#define I2S_DAC_OUT_SET                 SPR_48000           //做从机时，配置dac采样率，做主机时可忽略(暂时支持48K采样率)
#define I2S_DMA_EN                      1                   //I2S数据传输都采用DMA方式
#define I2S_MODE_SEL                    I2S_MASTER          //I2S主从模式选择 0: master; 1:slave
#define I2S_SAMPLES_OUT_SET             I2S_48000           //I2S输出采样率设置(I2S_16000,I2S_48000)

#define I2S_AUDIO_IN_EN                 (WIRELESS_MIC_I2S_INPUT_EN)    //是否使能I2S输入音频
#define I2S_AUDIO_OUT_EN                (ADAPTER_I2S_OUTPUT_EN)        //是否使能适配器端IIS输出音频
#define I2S_AUDIO_IN_OUT_EN             (ADAPTER_I2S_IN_OUT_EN)

/*****************************************************************************
 * Module    : SPI配置
 *****************************************************************************/
#define SPI_HW_EN                       0              //是否使能硬件SPI功能（默认使用SPI1, CPOL=0,CPHA=0）
#define SPI_2WIRE_EN                    1              //1: 2线半双工模式（SPICLK、SPIDO和SPIDI复用）; 0: 3线全双工模式（独立的SPICLK、SPIDI、SPIDO）
#define SPI_3WIRE_DUAL_MODE_EN          0              //是否使能2bit数据位宽传输模式
#define SPI_MASTER_EN                   1              //0:SALAVE 1:MASTER
#define SPI_IRQ_EN                      0              //是否使能SPI中断功能(仅适用于DMA方式,TX或RX完成时,产生中断)
#define SPI_MAPPING                     SPI1MAP_G3     //选择SPI mapping
#define SPI_BAUD_RATE                   2000000        //SPI波特率2M

#define SPI_CS_EN                       0              //是否使能SPI CS功能
#define SPI_CS_GP                       A
#define SPI_CS_BIT                      BIT(15)


/*****************************************************************************
 * Module    : 提示音 功能选择
 *****************************************************************************/
#define WARNING_TONE_EN                 0            //是否打开提示音功能, 总开关
#define WARNING_BREAK_EN                1            //是否支持提示音打断功能（主要是打断开机提示音和TWS副耳断开提示音）
#define WARNING_FIXED_VOLUME            0            //提示音是否固定音量，不跟随系统音量调节
#define WARING_MAXVOL_TYPE              RES_TYPE_TONE            //最大音量提示音类型
#define WARNING_VOLUME                  xcfg_cb.warning_volume   //播放提示音的音量级数
#define LANG_SELECT                     LANG_EN_ZH   //提示音语言选择

#define WARNING_POWER_ON                1
#define WARNING_POWER_OFF               1
#define WARNING_FUNC_MUSIC              1
#define WARNING_FUNC_BT                 1
#define WARNING_FUNC_CLOCK              0
#define WARNING_FUNC_FMRX               0
#define WARNING_FUNC_AUX                1
#define WARNING_FUNC_USBDEV             0
#define WARNING_FUNC_SPEAKER            0
#define WARNING_LOW_BATTERY             1
#define WARNING_BT_WAIT_CONNECT         1
#define WARNING_BT_CONNECT              1
#define WARNING_BT_DISCONNECT           1
#define WARNING_BT_INCALL               1
#define WARNING_USB_SD                  0
#define WARNING_MAX_VOLUME              1
#define WARNING_MIN_VOLUME              0
#define WARNING_BT_HID_MENU             1            //BT HID MENU手动连接/断开HID Profile提示音
#define WARNING_BTHID_CONN              0            //BTHID模式是否有独立的连接/断开提示音
#define WARNING_BT_PAIR                 1            //BT PAIRING提示音


#define SW_VERSION		"V0.0.1"		//只能使用数字0-9,ota需要转码
#define HW_VERSION		"V0.0.1"		//只能使用数字0-9,ota需要转码
#include "config_extra.h"

#endif // USER_CONFIG_H
