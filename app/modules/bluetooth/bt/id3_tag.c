#include "id3_tag.h"

#if BT_ID3_TAG_EN

#define ID3_BUF_LEN         512
u8 id3_buf[ID3_BUF_LEN] AT(.app.buf);
uint8_t cfg_bt_a2dp_id3_en = BT_ID3_TAG_EN;

//packet_type 0:single, 1:start, 2:continue, 3:end（处type0，其他需要拼包）
void bt_id3_tag_callback(u8 packet_type, u8 *buf, u16 size)
{
    // printf("size:%d\n",size);
    //拼包
    static u16 ptr = 0;
    if (packet_type != 0) {
        memcpy(&id3_buf[ptr], buf, size);
        ptr += size;
        if (packet_type != 3) {
            return;
        }
        buf = id3_buf;
    }
    ptr = 0;

    //格式
    //byte[0]   : num
    //byte[4:1] : index n
    //byte[6:5] : 编码格式（0x6a:UTF8）
    //byte[8:7] : lenght
    //byte[n:9] : value
    //...
    u16 offset = 0;
    u8 att_num = buf[offset++];

    while (att_num) {
        u32 index = ((u32)buf[offset + 3]) | (((u32)buf[offset + 2]) << 8) | (((u32)buf[offset + 1]) << 16) | (((u32)buf[offset + 0]) << 24);
        index = index;
        offset += 4;
        u16 character = ((u32)buf[offset + 1]) | (((u32)buf[offset + 0]) << 8);
        character = character;
        offset += 2;
        u16 length = ((u32)buf[offset + 1]) | (((u32)buf[offset + 0]) << 8);
        offset += 2;
//        printf("bt id3:%x %x %x\n", index, character, length);
//        switch (index) {
//            case BT_ID3_TITLE:
//                 printf("name: ");
//                 print_r(&buf[offset], length);
//                break;
//
//            case BT_ID3_ARTIST:
//                break;
//
//            default:
//                break;
//        }
        app_ctrl_id3_tag_process(index, &buf[offset], length);
        offset += length;
        att_num--;
    }
}

#endif
