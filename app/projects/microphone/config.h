/*****************************************************************************
 * Module    : Config
 * File      : config.h
 * Function  : SDK配置文件
 *****************************************************************************/

#ifndef USER_CONFIG_H
#define USER_CONFIG_H


//用户可选 CONFIG 方案配置
#define CONFIG_WIRELESS_LE_MIC      0          //无线话筒方案
#define CONFIG_CLIP_LE_MIC          1          //领夹麦方案
#define CONFIG_BROADCAST_MIC        2          //广播麦方案
#define CONFIG_LOW_LATENCY_MIC      3          //低延时一拖一方案
#define CONFIG_STEREO_MIC           4          //立体声一拖一方案
#define CONFIG_HEADSET              5          //耳机方案(一拖一双向传输)
#define CONFIG_INTERPHONE           6          //对讲机方案(一拖一双向传输)，配置工具都选择wireless_mic_emit烧录
#define CONFIG_INTERPHONE_LE_MIC    7          //一拖二双向无线麦方案(一拖二双向传输)，接收端目前只能搭配580x


#define USER_CONFIG                 CONFIG_WIRELESS_LE_MIC

#if (USER_CONFIG == CONFIG_WIRELESS_LE_MIC)
    #include "config_wireless_mic.h"
#elif (USER_CONFIG == CONFIG_CLIP_LE_MIC)
    #include "config_clip_mic.h"
#elif (USER_CONFIG == CONFIG_BROADCAST_MIC)
    #include "config_broadcast.h"
#elif (USER_CONFIG == CONFIG_LOW_LATENCY_MIC)
    #include "config_low_latency.h"
#elif (USER_CONFIG == CONFIG_STEREO_MIC)
    #include "config_stereo_mic.h"
#elif (USER_CONFIG == CONFIG_HEADSET)
    #include "config_headset.h"
#elif (USER_CONFIG == CONFIG_INTERPHONE)
    #include "config_interphone.h"
#elif (USER_CONFIG == CONFIG_INTERPHONE_LE_MIC)
    #include "config_interphone_le_mic.h"
#else
    #include "config_wireless_mic.h"
#endif


#endif
