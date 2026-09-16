#ifndef _USB_DEVICE_EP_H
#define _USB_DEVICE_EP_H

#define EP_CNT                  4

enum {
    EP_CTRL,
    EP_BULK,
    EP_INT,
    EP_ISOC,
};

typedef struct _epcb_t {
    psfr_t sfr;

    u8 type;                        //EP Type: Ctrl, Bulk, Int, ISOC
    u8 index;                       //EP Index
    volatile u8  error      : 1,    //USB通信出错
        dir                 : 1,    //EP的方向
        halt                : 1,    //Halt标志
        first_pkt           : 1;    //Transfer first packet

    u8 *buf;                        //EP的BUF地址

    const u8 *xptr;                 //当前发送/接收指针
    u16 xlen;                       //当前发送/接收剩余
    u16 xcnt;                       //已发送/接收统计
    u16 epsize;                     //EP Size
} epcb_t;

typedef struct _eps_list_t {
    epcb_t *cur_ep;                 //当前操作的EP寄存器
    epcb_t *eptx[EP_CNT];           //EP TX
    epcb_t *eprx[EP_CNT];           //EP RX
} eps_list_t;

extern eps_list_t eps;

void usb_ep_init(epcb_t *epcb);
void usb_ep_halt(epcb_t *epcb);
void usb_ep_clear(epcb_t *epcb);

void usb_ep_var_init(epcb_t *epcb);
epcb_t *usb_set_cur_ep(epcb_t *epcb);

bool usb_ep_transfer(epcb_t *epcb);
bool usb_ep_start_transfer(epcb_t *epcb, uint len);
void usb_ep_do_transfer(epcb_t *epcb, uint len);

void usb_ep_reset(epcb_t *epcb);

#endif
