#include "include.h"
#include "usb_comm.h"
#include "usb_device_storage.h"
#include "usb_device_user.h"

#define TRACE_EN                0
#define TRACE_XLINK_EN          1
#define TRACE_CMD_EN            0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#if TRACE_XLINK_EN
#define TRACE_X(...)            printf(__VA_ARGS__)
#else
#define TRACE_X(...)
#endif

#define UM_GETID            0x5A                    //启用自定义命令，获取芯片型号及版本
#define UM_PORT_INIT        0x50                    //通信的波特率、超时等基本配置（主要针对COM模式）。
#define UM_UPDATE_KEY       0x55                    //更新命令密钥
#define UM_READ             0x52                    //读芯片RAM
#define UM_WRITE            0x57                    //写芯片RAM
#define UM_XEN              0x58                    //开启扩展命令功能
#define UM_QUIT             0x51                    //退出升级模式（主要针对USB模式）
#define UM_RESET            0x5E                    //复位

#define UMX_SET_STA          0x40                   //设置xlink状态
#define UMX_GET_INFO         0x41                   //获取xlink信息

#define UFOT_DOWNLOAD_LEN    512                    //USB升级单次传输长度


#define SS_INVALID_FIELD_IN_CDB                 0x052400
#define SS_UNRECOVERED_READ_ERROR               0x031100
#define SS_WIRTE_ERROR                          0x030c02
static u8 fot_flag = 0;

#if ADAPTER_AB_FOT_DEVICE_SUPPORT

typedef enum{
    USB_FOT_INIT = 0xA0,
    USB_FOT_IDLE,
    USB_FOT_WAKEUP,
    USB_FOT_REQUEST,
    USB_FOT_READ,
    USB_FOT_WRITE,
    USB_FOT_FINISH,
    USB_FOT_ERR,
} DEVICE_FOT_STA;


static struct udisk_fot_t {
    u8 device_sta;
    u8 rd_sta;
    u8 cnt;
    u32 hash;
    u32 file_size;
    u32 addr;
    uint len;
} usb_fot_cb;


static cbuf_cfg_t udisk_fot_in_cbuf;

//u8 usb_fot_buf_incache[UFOT_DOWNLOAD_LEN*3] AT(.bss.usb_fot.buf);
//u8 usb_fot_buf_outcache[UFOT_DOWNLOAD_LEN*2] AT(.bss.usb_fot.buf);
u8 usb_fot_buf_incache[UFOT_DOWNLOAD_LEN*3] AT(.fot_data.buf);
u8 usb_fot_buf_outcache[UFOT_DOWNLOAD_LEN*2] AT(.fot_data.buf);

void usb_fot_buf_set(u8 *fot_buf,u32 addr)
{
    cbuf_input_audio(fot_buf, UFOT_DOWNLOAD_LEN, &udisk_fot_in_cbuf);
}

//------------------------------------------------------
//adapter给devicec升级
//step1: fot file init(升级入口).
void usb_fot_device_init(void)
{
    memset(&usb_fot_cb, 0, sizeof(usb_fot_cb));
    memset(usb_fot_buf_incache,0,sizeof(usb_fot_buf_incache));
    memset(usb_fot_buf_outcache,0,sizeof(usb_fot_buf_outcache));
    cbuf_init(&(udisk_fot_in_cbuf), usb_fot_buf_incache, sizeof(usb_fot_buf_incache));

    if (bt_tws_is_connected()) {
        //需要对整个文件进行hash计算,对hash算法没有硬性要求,但要确保该hash值针对每一个不同的升级文件唯一存在即可
        usb_fot_cb.hash = 0xa3a6d255;

        //断开音频链路
        ble_audio_ctr_cb(0);
        //进入升级流程
        usb_fot_cb.device_sta = USB_FOT_INIT;
    }
}

//step2: Device wakeup.
bool usb_fot_device_wakeup(void)
{
    //作用是先唤醒副机,进入FOTA状态
    u8 tws_fot_start_flg[] = {0x80, 0x10, 0x01};
    if(bt_tws_is_connected()){
        tws_fot_fill_in_data(tws_fot_start_flg, 3);
        bt_tws_sync_fot_data();
        return true;
    }
    return false;
}

//step3: Send fot file hash to device.
void usb_fot_device_file_hash(void)
{
    TRACE("usb_fot_device_file_hash\n");

    fot_tws_update_file_hash_set(usb_fot_cb.hash);
}

//step4: Read fot file data to cache.
u8 usb_fot_device_file_read(void)
{
    usb_fot_cb.file_size = fot_get_down_length()  + UFOT_DOWNLOAD_LEN * 10;

    if(cbuf_total_samples_get(&udisk_fot_in_cbuf) >= UFOT_DOWNLOAD_LEN){
        usb_fot_cb.rd_sta = 0;
    } else {
        usb_fot_cb.rd_sta = 1;
    }

    return usb_fot_cb.rd_sta;
}

//step5: Send fot data to device, wait rsp and step4 again.
AT(.ude.fot)
void usb_fot_device_file_write(void)
{
    if(usb_fot_cb.rd_sta == 0) {

        cbuf_output_audio(usb_fot_buf_outcache, UFOT_DOWNLOAD_LEN, &udisk_fot_in_cbuf);

        TRACE("update:%x, %x\n", usb_fot_cb.addr, usb_fot_cb.file_size);
        fot_tws_update_data_write(usb_fot_buf_outcache, usb_fot_cb.addr, UFOT_DOWNLOAD_LEN);
        usb_fot_cb.addr += UFOT_DOWNLOAD_LEN;
    }
}

//step6: wait rsp and step4 again.
void usb_fot_device_update_idle(void)
{
    //从机请求下一包数据
    if(fot_tws_rsp_get()) {
        fot_tws_rsp_clear();
        if(usb_fot_cb.addr == usb_fot_cb.file_size) {
            usb_fot_cb.device_sta = USB_FOT_FINISH;
            fot_tws_sync_update_done();                    //已经取完升级文件数据,通知从机重启
            TRACE("update:%x, %x\n", usb_fot_cb.addr, usb_fot_cb.file_size);
            TRACE("USB_FOT_FINISH\n");
        } else {                                           //继续读取下一包升级文件数据
            usb_fot_cb.device_sta = USB_FOT_READ;
        }
    }
}

bool usb_fot_device_update_finished(void)
{
    return (usb_fot_cb.device_sta == USB_FOT_FINISH);
}

void usb_fot_device_process(void)
{
    //控制device端升级状态
    switch(usb_fot_cb.device_sta) {
        case USB_FOT_INIT:
            usb_fot_cb.device_sta = USB_FOT_WAKEUP;
            break;

        case USB_FOT_WAKEUP:
            if(usb_fot_device_wakeup()) {
                usb_fot_cb.device_sta = USB_FOT_REQUEST;
            } else {
                usb_fot_cb.device_sta = USB_FOT_ERR;
                TRACE("USB_FOT_ERR\n");
            }
            break;

        case USB_FOT_REQUEST:
            usb_fot_device_file_hash();
            usb_fot_cb.device_sta = USB_FOT_READ;
            break;

        case USB_FOT_READ:
            if(usb_fot_device_file_read() == 0) {
                usb_fot_cb.device_sta = USB_FOT_WRITE;
            }
            break;

        case USB_FOT_WRITE:
            usb_fot_device_file_write();
            usb_fot_cb.device_sta = USB_FOT_IDLE;  //进入IDLE等待下一包传输
            break;


        case USB_FOT_IDLE:
            usb_fot_device_update_idle();
            break;

        case USB_FOT_FINISH:
            break;

        case USB_FOT_ERR:
            break;

        default:
            break;
    }
}

bool usb_fot_device_update_success(void)
{
    if (bt_tws_is_connected()) {
        if (usb_fot_device_update_finished()){
            return true;
        } else {
            return false;
        }
    } else {
        return true;
    }

}

#endif //ADAPTER_AB_FOT_DEVICE_SUPPORT


void fot_init(void);
u32 fot_get_curaddr(void);
bool fot_write(void *buf, u32 addr, u32 len);
u8 fot_get_err(void);       //0:OK, 1:Done,  2~255:Error
bool is_fot_update_success(void);
u8 fot_get_state(void);

///获取Xlink进行双备份升级时的状态
AT(.ude.fot)
static uint do_host_get_fot_err(uds_t *uds)
{
    u8 *inbuf = uds->bulk_in->buf;
    memset(inbuf, 0, 4);
    u8 err_code = fot_get_err();
    u8 fot_state = fot_get_state();
    inbuf[0] = err_code;
    inbuf[1] = fot_state;
    return 4;
}

///开始升级
AT(.ude.fot)
void fot_write_start(uds_t *uds)
{
    if(!fot_flag){
        fot_init();
        fot_flag = 1;
#if ADAPTER_AB_FOT_DEVICE_SUPPORT
		usb_fot_device_init();
#endif // ADAPTER_AB_FOT_DEVICE_SUPPORT

    }
    epcb_t *bulk_out = uds->bulk_out;
    u8 *outbuf = bulk_out->buf;
    usb_ep_start_transfer(bulk_out, 512);
    while (!usb_ep_transfer(bulk_out));
#if ADAPTER_AB_FOT_DEVICE_SUPPORT
	usb_fot_buf_set(outbuf,fot_get_curaddr());
#endif // ADAPTER_AB_FOT_DEVICE_SUPPORT
    fot_write(outbuf,fot_get_curaddr(),512);
//    if(is_fot_update_success()){
//        my_printf("Xlink update successfully!\n");
//        WDT_RST();
//    }
}

///Host None xlink Fot_Update自定义命令入口
AT(.ude.fot)
void do_host_none_fot_update(uds_t *uds)
{
    TRACE("DO: %02x\n", uds->cdb.subop);

    switch (uds->cdb.subop) {
    case 0x23:
        if(fot_get_state() == 5){
            WDT_RST();
        }
        break;

    default:
        uds->sense_data = SS_INVALID_FIELD_IN_CDB;
        break;
    }
}

///Host Out Xlink Fot_Update自定义命令入口
AT(.ude.fot)
void do_host_out_fot_update(uds_t *uds)
{
    //printf("BO: %02x\n", uds->cdb.subop);
    switch (uds->cdb.subop) {
    case 0x21:
        fot_write_start(uds);
        break;

    default:
        uds->sense_data = SS_INVALID_FIELD_IN_CDB;
        break;
    }
}

///Host in Xlink Fot_Update自定义命令入口
AT(.ude.fot)
uint do_host_in_fot_update(uds_t *uds)
{
    TRACE("DI: %02x\n", uds->cdb.subop);

    switch (uds->cdb.subop) {
    case 0x22:
        return do_host_get_fot_err(uds);

    default:
        uds->sense_data = SS_INVALID_FIELD_IN_CDB;
        return 0;
    }
}
