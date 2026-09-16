/*
 *  tws_sync.h
 *
 *  Created by zoro on 2029-9-20.
 */
#ifndef __TWS_SYNC_H
#define __TWS_SYNC_H


#include "tws_res.h"


//提示音API函数
#define bsp_res_init()
#define bsp_res_process()
#define bsp_res_set_enable(en)
#define bsp_res_set_break(en)
#define bsp_res_is_full()                    false                      //提示音列表是否存满
#define bsp_res_is_empty()                   true                       //提示音列表是否播空
#define bsp_res_is_playing()                 false                      //是否有提示音正在播放
#define bsp_res_is_vol_busy()                false                      //提示音是否正在控制音量
#define bsp_res_cleanup()
#define bsp_res_search(idx)                  false                      //查找列表是否存在某个提示音
uint8_t bsp_res_play(uint8_t res_idx);                                  //播放一个提示音

#define bsp_res_tone_play(n)

#endif // __TWS_SYNC_H
