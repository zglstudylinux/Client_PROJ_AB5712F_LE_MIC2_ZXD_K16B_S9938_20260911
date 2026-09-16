#include "include.h"

#if 1  //DEBUG
#define dump_printf    my_printf
#else
#define dump_printf(...)
#endif
//----dump  api-------------------------------------------------------------
//head_buf:   dump模块需要的包头管理buf,长度为 file_total*14
//file_toal:  dump模块同时支持导出的文件个数. 目前PC端软件UartDump暂时只支持最多同时接收6路并保存.
//putbuf:     底层dump函数,这里一般为高速串口DMA发送函数
//wait:       dump等待函数,下一次dump_putbuf时,需要等待上一次putbuf结束
void dump_buf_init(u8 *head_buf, int file_toal,void (*putbuf)(void*buf,u32 len), void (*wait)(void));

//dump_dma_buf:长度需要大于或等于 sizeof(tx_buf) + 14, 其中14是包头的长度. tx_buf中的数据会先打包放到dump_dma_buf中待发送
//tx_buf:      导出的数据存放的buf
//tx_buf_len:  导出数据的长度
//file_idx: 文件序号,0表示UartDump收到数据后保存到文件0, 1表示保存到文件1...
void dump_putbuf(void *dump_dma_buf, void *tx_buf, int tx_buf_len, u8 file_idx);
//为了减少等待上次dma完成的时间,可以先通过dump_put2ram把数据封包放到ram中,再调用dump_dma_kick一次性导出数据
void dump_put2ram(void *dump_dma_buf, void *tx_buf, int tx_buf_len, u8 file_idx);
void dump_dma_kick(void *dma_buf,u32 dma_len);  //kick一次dma发送
void dump_dma_wait(void);  //等待上一次发送完成
//-----user config---------------------------------------------------------
#define DUMP_HEAD_NUM       6               // 支持同时导出几路数据(目前PC端工具UartDump最大只支持接收两路)
#define HEAD_TAG            0x37365455      //UT67
#define HEAD_LEN            14              //sizeof(DUMP_HEAD)

//dump数据包结构:
//4byte(tag) + 4byte(pkt_cnt) + 2byte(pkt_len) + 2byte(crc)+1byte(type)+1byte(head_sum) + dump实际数据
typedef struct _DUMP_HEAD {
    u32 tag;
    u32 pkt_cnt;
    u16 pkt_len;
    u16 crc;          //crc16
    u8  type;         //bit7 crc_en //bit[3:0]  file_idx
    u8  head_sum;
}PACKED DUMP_HEAD  ;  //#define PACKED  __attribute__((packed))

typedef struct _DUMP_SPI_PROTOCOL {
    DUMP_HEAD *head[DUMP_HEAD_NUM];
    void (*putbuf)(void* buf,u32 buf_len);
    void (*wait_put_finish)(void);
    bool init_ok;
}DUMP_PROTOCOL;
static DUMP_PROTOCOL dump;

AT(.text.dumpbuf)
void dump_buf_init(u8 *head_buf, int file_toal,void (*putbuf)(void*buf,u32 len), void (*wait)(void))
{
    if (NULL == head_buf ||  file_toal < 1) {
        dump_printf("dump_var_init param ERR\n");
        return;
    }
    if(file_toal > 6) {
        file_toal = 6;
    }
    dump_printf("dump_buf_init, file_total = %d\n",file_toal);
    memset(head_buf,0x00,file_toal*HEAD_LEN);
    memset(&dump,0x00,sizeof(DUMP_PROTOCOL));
    for(int i = 0; i < file_toal; i++) {
        dump.head[i] = (DUMP_HEAD*)(head_buf + i * HEAD_LEN);
        dump.head[i]->tag = HEAD_TAG;  //UT67
        dump.head[i]->pkt_len = 0;
        dump.head[i]->type = (u8)i & 0x0F;
    }
    dump.putbuf = putbuf;
    dump.wait_put_finish = wait;
    dump.init_ok = true;
    dump_printf("-->dump_buf_init ok\n");
}


AT(.com_text.dumpbuf.crc)
static u16 calc_sum(void *buf, uint len)
{
    u16 sum = 0;
    u8* buf8 = (u8*)buf;
    for(uint i = 0; i < len; i++) {
        sum += buf8[i];
    }
    return sum;
}

//dump_dma_buf 长度需要大于或等于 sizeof(tx_buf) + 14, 其中14是包头的长度
AT(.com_text.dumpbuf)
void dump_put2ram(void *dump_dma_buf, void *tx_buf, int tx_buf_len, u8 file_idx)
{
    if (dump.init_ok) {
        u8 *dma_buf = (u8 *)dump_dma_buf;
        memcpy(&dma_buf[HEAD_LEN], tx_buf, tx_buf_len);
        dump.head[file_idx]->pkt_cnt += 1;
        dump.head[file_idx]->pkt_len = tx_buf_len;
        dump.head[file_idx]->crc = calc_sum(&dma_buf[HEAD_LEN], tx_buf_len);
        dump.head[file_idx]->head_sum = calc_sum(&(dump.head[file_idx]->pkt_cnt),9);
        memcpy(dma_buf, dump.head[file_idx], HEAD_LEN);
    }
}

AT(.com_text.dumpbuf)
void dump_dma_kick(void *dma_buf,u32 dma_len)  //kick一次dma发送
{
    if (dump.init_ok) {
        dump.putbuf(dma_buf, dma_len);
    }
}

AT(.com_text.dumpbuf)
void dump_dma_wait(void)  //等待上一次发送完成
{
    if (dump.init_ok) {
        dump.wait_put_finish();  //等待上一次发送完成
    }
}

//dump_dma_buf 长度需要大于或等于 sizeof(tx_buf) + 14, 其中14是包头的长度
AT(.com_text.dumpbuf)
void dump_putbuf(void *dump_dma_buf, void *tx_buf, int tx_buf_len, u8 file_idx)
{
    dump_dma_wait();
    dump_put2ram(dump_dma_buf,tx_buf,tx_buf_len,file_idx);
    dump_dma_kick(dump_dma_buf, tx_buf_len + HEAD_LEN);
}





