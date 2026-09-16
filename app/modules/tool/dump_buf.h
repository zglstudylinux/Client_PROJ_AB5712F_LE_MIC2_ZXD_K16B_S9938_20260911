#ifndef __DUMP_BUF_H
#define __DUMP_BUF_H

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
//为了减少dma等待时间,可以先通过dump_put2ram把数据封包放到ram中,再调用dump_dma_kick一次性导出数据
void dump_put2ram(void *dump_dma_buf, void *tx_buf, int tx_buf_len, u8 file_idx);
void dump_dma_kick(void *dma_buf,u32 dma_len);  //kick一次dma发送
void dump_dma_wait(void);  //等待上一次发送完成

#endif
