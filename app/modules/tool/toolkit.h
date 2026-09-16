#ifndef __TOOLKIT_H
#define __TOOLKIT_H

#include "toolkit_effect.h"

#define XTP_EFFECT_FRAME_HEAD_TAG              12
#define TOOLKIT_SEED                          0xffff
#define TOOLKIT_GET_BIN_PACKET_LEN             128

enum {
    TOOL_CMD_GNONE,
    TOOL_CMD_GET_BIN,
};

enum {
    REPLY_OK,
    REPLY_FAIL,
};

typedef struct PACKED{
    u8 head_tag[12];
    u16 size;
    u16 cmd;
    u8 reply;
    u8 RSVD[5];
    u32 filelen;
    u16 crc16;
} toolkit_reply_header;

typedef struct PACKED{
    u8 head_tag[12];
    u16 size;
    u8 cnt;
    u8 data[TOOLKIT_GET_BIN_PACKET_LEN];
    u16 crc16;
} toolkit_get_bin_header;

typedef struct {
    u8 cnt;
    u8* ptr;
    u32 len;
} toolkit_bin_info;

//enum {
//    TOOL_CHANNEL_ERROR = 0,
//    TOOL_CHANNEL_MIC,
//    TOOL_CHANNEL_MUSIC,
//};

void toolkit_init(void);
void toolkit_exit(void);
void toolkit_process(void);
void toolkit_ack_fail(void);

#endif // __TOOLKIT_H
