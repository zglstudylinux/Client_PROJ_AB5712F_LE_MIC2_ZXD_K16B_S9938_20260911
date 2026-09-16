#include "include.h"

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif // TRACE_EN

#if ADAPTER_USB_UPDATE_EN
typedef enum{
    CMD_UPDATE_REQ = 0x91,
    CMD_PKT_REQ,
    CMD_PKT_RSP,
    CMD_UPDATE_STA,
    CMD_DEV_RESET,
}UPDATE_CMD;

typedef enum{
    USB_UDP_INIT = 0,
    USB_UDP_IDLE,
    USB_UDP_REQ,
    USB_UDP_WRITE,
    USB_UDP_DEV_WAKEUP,
    USB_UDP_DEV_RSP,
    USB_UDP_DEV_SEND,
    USB_UDP_DEV_RESET,
    USB_UDP_FINISH,
    USB_UDP_ERR,
} USB_DEV_UPDATE_STA;

typedef enum {
    UPDATE_STA_OK = 0,
    UPDATE_STA_ERR,
}UPDATE_STA;

typedef enum {
    UPDATE_ROLE_ADPT = 0,
    UPDATE_ROLE_DEV,
}UPDATE_ROLE;

static struct usb_dev_update_t {
    u8  sta;
    u8  update_role;
    u8  dev_update_ok;
    u8  adpt_update_ok;
    u32 file_size;
    u32 file_hash;
    u32 addr;
    u32 remain;
    u32 rx_len;
    u32 ticks;
} usb_dev_cb;

#define cfg_update_role_is_adapter()    (usb_dev_cb.update_role == 0)

#define USB_DEV_PKT_FRAME    512
static u8 usb_dev_pkt_cache[USB_DEV_PKT_FRAME];

///将packet写入flash
void usb_dev_update_process(void)
{
    if(cfg_update_role_is_adapter()) {
        ///adapter升级
        if(usb_dev_cb.sta == USB_UDP_WRITE) {
            //写入flash
            if(!fot_write(usb_dev_pkt_cache, usb_dev_cb.addr, usb_dev_cb.rx_len)) {
                usb_dev_cb.sta = USB_UDP_ERR;
                TRACE("USB_DEV_ERR\n");
            }
            usb_dev_cb.addr += usb_dev_cb.rx_len;
            usb_dev_cb.remain -= usb_dev_cb.rx_len;
    //        TRACE_R(usb_dev_pkt_cache, usb_dev_cb.rx_len);
    //        TRACE("remain: %d, rx_len:%d, addr:%x\n", usb_dev_cb.remain, usb_dev_cb.rx_len, usb_dev_cb.addr);
            usb_dev_cb.rx_len = 0;
            if(usb_dev_cb.remain > 0) {
                usb_dev_cb.sta = USB_UDP_IDLE;
                usb_dev_update_pkt_req();          //继续请求下一包数据
            } else {
                uint8_t ret = fot_get_err();
                if(ret) {
                    TRACE("fot_get_err:%d\n", ret);
                } else {
                    #if ADAPTER_AB_FOT_DEVICE_SUPPORT
                    fot_tws_done();
                    #endif
                    TRACE("-->fot update ok\n");
                    usb_dev_update_sta_rsp(UPDATE_ROLE_ADPT, UPDATE_STA_OK);  //通知上位机升级完成
                    usb_dev_cb.adpt_update_ok = 1;
                    usb_dev_cb.sta = USB_UDP_FINISH;   //接收完成
                }
            }
        }
    } else {
        ///device升级
#if ADAPTER_AB_FOT_DEVICE_SUPPORT
        if(usb_dev_cb.sta == USB_UDP_DEV_WAKEUP) {
            if(usb_fot_device_wakeup()) {
                usb_dev_cb.sta = USB_UDP_DEV_RSP;
            } else {
                usb_dev_update_sta_rsp(UPDATE_ROLE_DEV, UPDATE_STA_ERR);
                usb_dev_cb.sta = USB_UDP_ERR;
                TRACE("USB_FOT_ERR\n");
            }
        } else if(usb_dev_cb.sta == USB_UDP_DEV_RSP) {
            //先传输文件hash值过去,并请求device端升级
            fot_tws_update_file_hash_set(usb_dev_cb.file_hash);
            //开始请求USB数据包,并进入idle
            usb_dev_update_pkt_req();
            usb_dev_cb.sta = USB_UDP_IDLE;
        } else if(usb_dev_cb.sta == USB_UDP_DEV_SEND) {
            //TWS传输数据
            fot_tws_update_data_write(usb_dev_pkt_cache, usb_dev_cb.addr, usb_dev_cb.rx_len);
            usb_dev_cb.addr += usb_dev_cb.rx_len;
            usb_dev_cb.remain -= usb_dev_cb.rx_len;
            usb_dev_cb.rx_len = 0;
            usb_dev_cb.sta = USB_UDP_IDLE;
        } else if(usb_dev_cb.sta == USB_UDP_IDLE) {
            //从机请求下一包数据
            if(fot_tws_rsp_get()) {
                fot_tws_rsp_clear();
                if(fot_tws_remote_err_get() != 0) {
                    usb_dev_update_sta_rsp(UPDATE_ROLE_DEV, UPDATE_STA_ERR);
                    return;
                }

                if(usb_dev_cb.remain > 0) {
                    usb_dev_update_pkt_req();          //继续请求下一包数据
                } else {
                    usb_dev_update_sta_rsp(UPDATE_ROLE_DEV, UPDATE_STA_OK);  //通知上位机升级完成
                    usb_dev_cb.dev_update_ok = 1;
                    usb_dev_cb.sta = USB_UDP_FINISH;   //接收完成
                    TRACE("update:%x, %x\n", usb_dev_cb.addr, usb_dev_cb.file_size);
                    TRACE("USB_UDP_FINISH\n");
                }
            }
        }
#endif
    }

    if(usb_dev_cb.sta == USB_UDP_DEV_RESET && tick_check_expire(usb_dev_cb.ticks, 1500)) {
        TRACE("ADAPTER RESET\n");
        WDT_RST();
    }
}


//-----------------------------------------------------------------------------------------

///解析RX数据包
void usb_dev_update_rx_pkt(uint8_t *ptr, uint8_t len)
{
    uint8_t cmd = ptr[1];
    uint8_t payload_len = ptr[2];

    if(cmd == CMD_UPDATE_REQ){
        usb_dev_update_init(ptr, len);
    } else if(cmd == CMD_PKT_RSP) {
        usb_dev_update_pkt_save(ptr + 3, payload_len);
    } else if(cmd == CMD_DEV_RESET) {
        usb_dev_update_reset();
    }
}

///请求数据包
void usb_dev_update_pkt_req(void)
{
    uint8_t data[10];
    uint8_t reporter_id = 0x03;
    uint32_t pkt_size = 0;
    uint32_t file_addr = usb_dev_cb.addr;

    if(usb_dev_cb.remain >= USB_DEV_PKT_FRAME) {
        pkt_size = USB_DEV_PKT_FRAME;
    } else {
        pkt_size = usb_dev_cb.remain;
    }

    data[0] = CMD_PKT_REQ;
    data[1] = 8;
    memcpy(data+2, &file_addr, 4);
    memcpy(data+6, &pkt_size, 4);

    usb_device_hid_send_user(reporter_id, data, sizeof(data), FOTA_DATA);

//    TRACE("pkt_req\n");
//    TRACE_R(data, 10);
}

///保存数据包
void usb_dev_update_pkt_save(u8 *ptr, uint8_t len)
{
    memcpy(usb_dev_pkt_cache + usb_dev_cb.rx_len, ptr, len);
    usb_dev_cb.rx_len += len;
    if(cfg_update_role_is_adapter()) {
		printf("adapter ota\n");
        if(usb_dev_cb.rx_len >= USB_DEV_PKT_FRAME || usb_dev_cb.rx_len == usb_dev_cb.remain) {
            usb_dev_cb.sta = USB_UDP_WRITE;
        } else if((usb_dev_cb.remain < USB_DEV_PKT_FRAME) && (usb_dev_cb.rx_len == usb_dev_cb.remain)) {
            usb_dev_cb.sta = USB_UDP_WRITE;
        }
    } else {
#if ADAPTER_AB_FOT_DEVICE_SUPPORT
		printf("device ota\n");
        if(usb_dev_cb.rx_len >= USB_DEV_PKT_FRAME || usb_dev_cb.rx_len == usb_dev_cb.remain) {
            usb_dev_cb.sta = USB_UDP_DEV_SEND;
        } else if((usb_dev_cb.remain < USB_DEV_PKT_FRAME) && (usb_dev_cb.rx_len == usb_dev_cb.remain)) {
            usb_dev_cb.sta = USB_UDP_DEV_SEND;
        }
#endif
    }
}

///升级状态响应
void usb_dev_update_sta_rsp(uint8_t update_role, uint8_t sta)
{
    uint8_t data[4];
    uint8_t reporter_id = 0x03;

    data[0] = CMD_UPDATE_STA;
    data[1] = 2;
    data[2] = update_role;
    data[3] = sta;
    usb_device_hid_send_user(reporter_id, data, sizeof(data), FOTA_DATA);

//    TRACE("pkt_req\n");
//    TRACE_R(data, 10);
}

///复位状态响应
void usb_dev_update_reset_rsp(void)
{
    uint8_t data[2];
    uint8_t reporter_id = 0x03;

    data[0] = CMD_DEV_RESET;
    data[1] = 0;
    usb_device_hid_send_user(reporter_id, data, sizeof(data), FOTA_DATA);
//    TRACE("pkt_req\n");
//    TRACE_R(data, 10);
}

//设备复位
void usb_dev_update_reset(void)
{
    TRACE("usb_dev_update_reset\n");

#if ADAPTER_AB_FOT_DEVICE_SUPPORT
    if(wireless_get_status() && usb_dev_cb.dev_update_ok) {
        fot_tws_sync_update_done();    //已经取完升级文件数据,通知从机重启
    } else {
        wireless_wdt_reset();
    }
#else
        wireless_wdt_reset();
#endif

    //主机先等一小段时间再复位
    usb_dev_cb.ticks = tick_get();
    usb_dev_cb.sta = USB_UDP_DEV_RESET;

    //响应复位状态给上位机
    usb_dev_update_reset_rsp();
}

void usb_dev_update_init(u8 *param, uint8_t len)
{
    fot_init();

    memset(&usb_dev_cb, 0, sizeof(usb_dev_cb));
    usb_dev_cb.file_size = little_endian_read_32(param, 3);
    usb_dev_cb.file_hash = little_endian_read_32(param, 7);
    usb_dev_cb.remain = usb_dev_cb.file_size ;
    usb_dev_cb.update_role = param[11]; //0=adapter,1=device

    if(cfg_update_role_is_adapter()) {
        TRACE("usb_dev_update_init adapter\n");
        //开始请求数据包
        usb_dev_update_pkt_req();
        usb_dev_cb.sta = USB_UDP_IDLE;
    } else {
#if ADAPTER_AB_FOT_DEVICE_SUPPORT
        TRACE("usb_dev_update_init device\n");
        //断开音频链路
        if(wireless_get_status()) {
            ble_audio_ctr_cb(0);
            usb_dev_cb.sta = USB_UDP_DEV_WAKEUP;
        } else {
            usb_dev_update_sta_rsp(UPDATE_ROLE_DEV, UPDATE_STA_ERR);
        }
#endif
    }

    TRACE("file_size: %x\n", usb_dev_cb.file_size);
    TRACE("file_hash: %x\n", usb_dev_cb.file_hash);
}
#else
void usb_dev_update_rx_pkt(uint8_t *ptr, uint8_t len){}
#endif
