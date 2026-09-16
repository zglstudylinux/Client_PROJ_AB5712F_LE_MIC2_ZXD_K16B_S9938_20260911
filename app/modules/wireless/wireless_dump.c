/*****************************************************************************
 * Module    : wireless_dump
 * File      : wireless_dump.c
 * Function  : 打印无线PER、RSSI等信息
 * Attention :
 *****************************************************************************/

#include "include.h"
#include "api.h"

#if WIRELESS_DUMP_EN

static struct {
    struct {
        uint pkt_cnt;
        uint pkt_err;
    } per;

    int8_t rssi;
    uint8_t chmap[5];
} wl_info[WIRELESS_CON_LINK_NB];

#define WL_RSSI_DFT         -99
#define WL_RSSI_WEIGHT      2

ALWAYS_INLINE void wireless_info_cleanup(u8 idx)
{
    wl_info[idx].per.pkt_cnt = 0;
    wl_info[idx].per.pkt_err = 0;
    wl_info[idx].rssi = WL_RSSI_DFT;
}

void wireless_dump_reset(u8 idx)
{
    wireless_info_cleanup(idx);
}

AT(.com_text.wireless.dump)
void wireless_dump_set_rx_status(u8 idx, u8 chidx, s8 rssi, u8 bfi)
{
    int16_t av_rssi = wl_info[idx].rssi;
    wl_info[idx].rssi = ((av_rssi << WL_RSSI_WEIGHT) - av_rssi + rssi + (1<<(WL_RSSI_WEIGHT-1))) >> WL_RSSI_WEIGHT;

    if(bfi) {
        wl_info[idx].per.pkt_err++;
    }
    wl_info[idx].per.pkt_cnt++;
}

void wireless_dump_set_chmap_cb(u8 idx, const u8 *chmap)
{
    if(idx >= 2) {
        return;
    }

    memcpy(wl_info[idx].chmap, chmap, 5);

    printf("LINK%d CHMAP_UPD: ", idx);
    for(uint i=0; i<5; i++) {
        for(uint j=0; j<8; j++) {
            if(chmap[i] & BIT(j)) {
                printf("%d, ", i*8+j);
            }
        }
    }
    printf("\n\n");
}

ALIGNED(512)
void wireless_dump_per(u8 idx)
{
    GLOBAL_INT_DISABLE();
    uint total = wl_info[idx].per.pkt_cnt;
    uint err = wl_info[idx].per.pkt_err;
    int32_t rssi = wl_info[idx].rssi;
    uint8_t chmap[5];

    memcpy(chmap, wl_info[idx].chmap, 5);
    wireless_info_cleanup(idx);
    GLOBAL_INT_RESTORE();

    if(total != 0) {
        printf("Link%d, PER=%3d%%, total=%3d, err=%3d, rssi=%d, ", idx, err*100/total, total, err, rssi);
        printf("CHMAP:%02x,%02x,%02x,%02x,%02x\n", chmap[0], chmap[1], chmap[2], chmap[3], chmap[4]);
    }
}

void wireless_dump_init(void)
{
    for(uint i=0; i<WIRELESS_CON_LINK_NB; i++) {
        wireless_dump_reset(i);
    }
}

AT(.text.func.process.wireless)
void wireless_dump_proc(void)
{
    static uint tick = 0;
    if(tick_check_expire(tick, 1000)) {
        tick = tick_get();

        for(uint i=0; i<WIRELESS_CON_LINK_NB; i++) {
            wireless_dump_per(i);
        }
    }
}

#endif // WIRELESS_DUMP_EN


