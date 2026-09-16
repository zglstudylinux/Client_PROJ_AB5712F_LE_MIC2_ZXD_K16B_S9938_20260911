/*
 *  api_buffer.h
 *
 *  Created by zoro on 2025-3-1.
 */
#ifndef _API_BUFFER_H
#define _API_BUFFER_H

//循环buffer
typedef struct rbuf_tbl_tag {
    uint8_t *buf;
    uint32_t size;
} rbuf_tbl_t;

typedef struct rbuf_page_tag {
    uint8_t *buf;               //分页的buffer地址
    uint32_t begin;             //分页在buf中的起始位置
    uint32_t end;               //分页在buf中的结束位置
} rbuf_page_t;

typedef struct ring_buf_tag {
    uint32_t rpos;              //读地址位置
    uint32_t wpos;              //写地址位置
    volatile uint32_t count;    //写了多少byte

    uint32_t total;             //buf总大小
    uint16_t trig;              //写到多少byte开始可读
    uint8_t page_nb;            //buf分页总数
    rbuf_page_t page[1];        //分页描述，按实际page分配
} ring_buf_t;

typedef void (*rbuf_callback_t)(uint8_t *, uint8_t *, uint32_t);

void ring_buf_init(ring_buf_t *rbuf, const rbuf_tbl_t *rbuf_tbl, uint8_t page_nb, uint16_t trig);
bool ring_buf_put(ring_buf_t *rbuf, uint8_t *ptr, uint32_t len);
bool ring_buf_get_old(ring_buf_t *rbuf, rbuf_callback_t callback, uint32_t len);
bool ring_buf_get(ring_buf_t *rbuf, uint8_t *ptr, uint32_t len);
bool ring_buf_peek(ring_buf_t *rbuf, uint8_t *ptr, uint32_t len, uint32_t offset);
#define ring_buf_get_count(rbuf)        (rbuf)->count
#define ring_buf_get_total(rbuf)        (rbuf)->total
#define ring_buf_get_rpos(rbuf, ofs)    (rbuf)->rpos + (ofs)


//乒乓buffer
struct rw_tog {
    u8 *buf;
    u16 w_cnt;
    u16 r_cnt;
};

struct tog_bug_tag {
    u16 block_size;
    bool w_idx;
    bool r_idx;
    struct rw_tog tog[2];
};

void tog_buf_init(struct tog_bug_tag *tbuf, void *buffer, u16 block_size);  //初始化tog_buf，buffer size = 2*block_size
bool tog_buf_put(struct tog_bug_tag *tbuf, u8 *ptr, u16 size);              //写入到w_idx指向的buffer，写完w_cnt计数，w_cnt>=block_size时返回true
bool tog_buf_get(struct tog_bug_tag *tbuf, u8 *ptr, u16 size);              //获取r_idx指向的buffer，读完r_cnt计数，r_cnt>=block_size时返回true
void tog_buf_wr_toggle(struct tog_bug_tag *tbuf);                           //手动翻转w_idx
void tog_buf_rd_toggle(struct tog_bug_tag *tbuf);                           //手动翻转r_idx
void *tog_buf_get_wr_ptr(struct tog_bug_tag *tbuf);                         //获取当前写指针位置
void *tog_buf_get_rd_ptr(struct tog_bug_tag *tbuf);                         //获取当前读指针位置
#define tog_bug_get_w_block(tbuf)   (tbuf)->tog[(tbuf)->w_idx].buf          //获取当前写block指针
#define tog_bug_get_w_cnt(tbuf)     (tbuf)->tog[(tbuf)->w_idx].w_cnt        //获取当前写计数
#define tog_bug_get_r_cnt(tbuf)     (tbuf)->tog[(tbuf)->r_idx].r_cnt        //获取当前读计数

#endif // _API_BUFFER_H
