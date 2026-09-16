#include "include.h"

#define TRACE_EN                0
#if TRACE_EN
#define TRACE(...)              my_printf(__VA_ARGS__)
#define TRACE_R(...)            my_print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif

#if UDE_IAP_PROCESS_EN

#if MFI_NAME_EN
AT(.rodata.usbdev)
const u8 str_product[22] = {
	22, 				// Num bytes of this descriptor
	3,					// String descriptor
	'M',	0,
	'i',	0,
	'c',	0,
	'r',	0,
	'o',	0,
	'p',	0,
	'h',	0,
	'o',	0,
	'n',	0,
	'e',	0,
};
#endif

u8 certi_data_buf[APPLE_CERTI_SIZE] AT(.buf.iap2.device.desc.buf);   //认证信息buf
u8 mfi_data_cache[APPLE_CACHE_SIZE] AT(.buf.usb.hid_size);           //USB端点一次收到的数据
u8 ota_data_cache[APPLE_CACHE_SIZE] AT(.buf.iap2.cache);           //USB端点一次收到的数据
u8 device_data_cache[APPLE_CACHE_SIZE] AT(.buf.iap2.cache);           //USB端点一次收到的数据
u8 apple_data_pool[APPLE_DATA_POOL_SIZE] AT(.buf.iap2.data_pool);           //苹果下发数据缓存数据池
u8 device_data_pool[APPLE_DATA_POOL_SIZE] AT(.buf.iap2.data_pool);           //苹果下发数据缓存数据池
WEAK uint8_t ude_iap_process_en = 1;
extern uds_t uds_0 AT(.buf.udev.usb);
c9_upload_t c9_upload_vars AT(.buf.iap2.c9_vars);
mfi_iap2_cfg_t iap2_cfg AT(.buf.iap2.mfi_iap2);
static cbuf_cfg_t apple_rec_cbuf;
static cbuf_cfg_t apple_send_cbuf;
static cbuf_cfg_t apple_ota_cbuf;

typedef struct {
//    cbuf_cfg_t ios_len_cbuf;
    cbuf_cfg_t ios_data_cbuf;

    u8  w_idx;
    u8  r_idx;

    u16 ios_outlen_buf[100];
    u8 ios_cache[APPLE_CACHE_SIZE];
    u8 ios_data_pool[APPLE_DATA_POOL_SIZE];

} ios_data_cache_cb;

static ios_data_cache_cb ios_data_cache AT(.buf.iap2.data_pool);

u16 buf_size[25];
u16 buf_num;

AT(.rodata.iap)
static const u8 detect_sequence[6] = {0xFF,0x55,0x02,0x00,0xEE,0x10};

AT(.rodata.iap)
static const u8 iap_syn_sequence[26] = {0xFF,0x5A,0x00,0x1A,0x80,0x01,0x10,0x00,0xFC,0x01,0x05,0x04,0x00,0x13,0x88,0x00,0xFF,0x1E,0x03,0x01,0x00,0x01,0x02,0x02,0x01,0xF8};
//AT(.rodata.iap)
//static const u8 iap_syn_sequence[26] = {0xFF,0x5A,0x00,0x1A,0x80,0x01,0x10,0x00,0xFC,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x01,0x02,0x02,0x01,0xF8};

AT(.rodata.iap)
static const u8 iap_fourth_cmd[19] = {0xFF,0x5A,0x03,0xE0,0x40,0x02,0xF5,0x01,0x8C,0x40,0x40,0x03,0xD6,0xAA,0x01,0x03,0xD0,0x00,0x00};

AT(.rodata.iap)
static const u8 iap_challenage_cmd[19] = {0xFF,0x5A,0x00,0x94,0x40,0x03,0xF6,0x01,0xD9,0x40,0x40,0x00,0x8A,0xAA,0x03,0x00,0x84,0x00,0x00};


AT(.rodata.iap)
const u8 iap2_generial_info2[131] = {
    0x15, 0x00, 0x22, 0x32, 0x35, 0x39, 0x36, 0x34, 0x62, 0x32, 0x64, 0x33, 0x38, 0x65, 0x39, 0x34, 0x65, 0x65, 0x65, 0x00, 0x00,
    //固件版本："1.1.0"
    0x0A, 0x00, 0x04, 0x31, 0x2E, 0x31, 0x2E, 0x30, 0x00, 0x00,
    //硬件版本："1.1.0"
    0x0A, 0x00, 0x05, 0x31, 0x2E, 0x31, 0x2E, 0x30, 0x00, 0x00,
    //其他
    0x0C, 0x00, 0x06, 0xAE, 0x00, 0xAE, 0x02, 0xAE, 0x03, 0xEA, 0x03, 0x00,
    0x0A, 0x00, 0x07, 0xAE, 0x01, 0xEA, 0x00, 0xEA, 0x01, 0x00,
    0x05, 0x00, 0x08, 0x02, 0x00,
    0x06, 0x00, 0x09, 0x00, 0x64, 0x00,
    0x13, 0x00, 0x0A, 0x00, 0x05, 0x00, 0x00, 0xB1, 0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x05, 0x00, 0x02, 0x00, 0x00,
    0x07, 0x00, 0x0C, 0x65, 0x6E, 0x00, 0x00,
    0x07, 0x00, 0x0D, 0x65, 0x6E, 0x00, 0x00,
    0x18, 0x00, 0x10, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x00, 0x01, 0x69, 0x41, 0x50, 0x32, 0x48, 0x00, 0x00, 0x04, 0x00, 0x02, 0xAB
};


AT(.rodata.iap)
static const u8 iap_startPowerUpdates_cmd[32] = {0xFF,0x5A,0x00,0x20,0x40,0x30,0x17,0x01,0xff,0x40,0x40,0x00,0x16,0xae,0x00,0x00,0x04,0x00,0x00,0x00,0x04,0x00,0x01,0x00,0x04,0x00,0x04,0x00,0x04,0x00,0x05,0xa2};   //StartPowerUpdates

AT(.rodata.iap)
static const u8 iap_stopPowerUpdates_cmd[17] = {0xFF,0x5A,0x00,0x11,0x40,0x30,0x17,0x01,0xff,0x40,0x00,0x16,0xae,0x02,0x00,0x05,0xa2};

AT(.rodata.iap)
static const u8 powersource_update[27] = {0xFF,0x5A,0x00,0x1B,0x40,0x06,0xF9,0x01,0x4D,0x40,0x40,0x00,0x11,0xAE,0x03,0x00,0x06,0x00,0x00,0x09,0x60,0x00,0x05,0x00,0x01,0x01,0xC6};//2.4A

AT(.rodata.iap)
static const u8 ea_session_status[27] = {0xFF, 0x5A, 0x00, 0x1B, 0x40, 0x07, 0xFC, 0x01, 0x48, 0x40, 0x40, 0x00, 0x11, 0xEA, 0x03, 0x00, 0x06, 0x00, 0x00, 0x00, 0x08, 0x00, 0x05, 0x00, 0x01, 0x00, 0x6E};

AT(.rodata.iap)
static const u8 ea_session_packet[11] = {0xFF, 0x5A, 0x00, 0x4C, 0x40, 0x08, 0xFD, 0x02, 0x36, 0x00, 0x03};

//AT(.rodata.iap)
//static const u8 app_launch[37] = {0xFF, 0x5A, 0x00, 0x25, 0x40, 0x05, 0xC7, 0x01, 0x75, 0x40, 0x40, 0x00, 0x1B, 0xEA, 0x02, 0x00, 0x10, 0x00, 0x00, 0x63, 0x6F, 0x6D, 0x2E, 0x62, 0x6F, 0x79, 0x61, 0x2E, 0x65, 0x63, 0x6F, 0x00, 0x05, 0x00, 0x01, 0x00, 0xE6};

AT(.rodata.iap)
static const u8 iap_ack_cmd[9] = {0xFF, 0x5A, 0x00, 0x09, 0x40, 0x2f, 0x16, 0x01, 0x18};//第五位是SEQ，第六位是ACKnumber，最后一位是校验位

//AT(.rodata.iap)
//static const u8 iap_ack_cmd1[20] = {0x55, 0x12, 0x03, 0x00, 0x34, 0x00, 0x01,0x02, 0x14, 0x00, 0x01, 0x01, 0x1b, 0x00, 0x22, 0x00,0x01,0x01,0x04,0xfa};

//AT(.rodata.iap)
//static const u8 iap_test[9] = {0xFF, 0x5A, 0x00, 0x09, 0x40, 0x2f, 0x16, 0x01, 0x18};

WEAK uint8_t cfg_auth_cp_version = 1;       //0:Auth2.0  1:Auth3.0
WEAK uint8_t cfg_auth_cp_current = 1;       //0:1A充电电流 1:2.4A充电电流

///0x100减去前八位数据之和的一个校验位
AT(.usbdev.com.iap)
uint8_t check_before_eight(u8 *ptr, u16 len)
{
    uint16_t i ;
    uint8_t sum = 0;

    for(i=0; i<len; i++){
        sum += ptr[i];
    }
    return (uint8_t)(0x100 - sum);
}

void set_iap_send_flag(void)
{
    iap2_cfg.iap_data_send_flag = true;

}

AT(.com_text.usbdev)
void uds_iap_tmr_isr(void)
{
    if(iap2_cfg.time_out_tick) {
        iap2_cfg.time_out_tick--;
    }

    if((sys_cb.phone_type == APPLE) && !iap2_cfg.app_init_flag) {          //苹果手机5s还没认证成功的话就重新过一遍认证
        iap2_cfg.app_init_tick++;
        if(iap2_cfg.app_init_tick  >= 50) {
            iap2_cfg.app_init_tick = 0;
            uds_iap_reset();
            ude_iap_app_init();
        }
    } else if(!sys_cb.usb_detec_flag) {
        iap2_cfg.app_init_flag = false;
    } else {
        iap2_cfg.app_init_tick = 0;
    }

}

AT(.com_text.usbdev)
void uds_iap_reset(void)                  //端点reset后需要重新认证
{
    c9_upload_vars.iap_send_step = 0;
}

///发送数据给iphone
AT(.usbdev.com)
void ude_iap_transfer(uds_t *uds, uint len)
{
    iap2_cfg.time_out_tick = 10;
    usb_ep_start_transfer(uds->bulk_in, len);
    while ((!usb_ep_transfer(uds->bulk_in)) && (!uds->error) && (iap2_cfg.time_out_tick)){
        WDT_CLR();
        if((iap2_cfg.time_out_tick <= 1) && (sys_cb.phone_type == DEVICE_TYPE)) {   //认证数据交互即将超时，且没有通过第一次苹果认证
            sys_cb.phone_type = ANDROID;
            TRACE("Android PHONE\n");

        }
    }
    iap2_cfg.time_out_tick=0;
}

///获取MFI认证数据
AT(.usbdev.com.iap)
static u32 mfi_accessory_certificate_data_get(void)
{
    u32 i;
    u8 buf[2], *ceri_buf = &cfg_desc_buf[19];
    u16 length;
    u16 length_temp;
    u16 block_size;
    mfi_i2c_read_data(0x30, buf, 2);
    if(cfg_auth_cp_version){
        length = ((buf[0] << 8) + buf[1]);
        length_temp = length;
        block_size = length_temp / 128;
        memset(ceri_buf, 0, sizeof(cfg_desc_buf)-19);
        for (i = 0; i < block_size; i++){
            mfi_i2c_read_data(0x31 + i, &ceri_buf[i*128], 128);
            length_temp -= 128;
        }
        if(length_temp != 0){
            mfi_i2c_read_data(0x31 + i, &ceri_buf[i*128], length_temp);
        }
        return length;
    } else {
        block_size = (((u16)buf[0] << 8) + buf[1] + 127) / 128;
        memset(ceri_buf, 0, sizeof(cfg_desc_buf)-19);
        mfi_i2c_read_data(0x31, &ceri_buf[0], 128*block_size);
        return block_size * 128;
    }
}

AT(.usbdev.com.iap)
void ude_iap_transfer_cerificate_data(uds_t *uds)
{
    u32 length = 0, i, tx_len;
    u8 *txbuf = uds->bulk_in->buf;
    u8 *ceri_buf = cfg_desc_buf;

    if(cfg_auth_cp_version){
        length = mfi_accessory_certificate_data_get();
        length += 4;//3.0CP
    } else {
        length = mfi_accessory_certificate_data_get();
    }

    memcpy(ceri_buf, iap_fourth_cmd, sizeof(iap_fourth_cmd));
    length += 20;
    ceri_buf[2] = length >> 8;
    ceri_buf[3] = (u8)length;
    ceri_buf[6] = iap2_cfg.rec_seq;
    ceri_buf[8] = check_before_eight(ceri_buf, 8);
    ceri_buf[11] = (length + 6 - 20) >> 8;
    ceri_buf[12] = (u8)(length + 6 - 20);
    ceri_buf[15] = (length - 20) >> 8;
    ceri_buf[16] = (u8)(length - 20);
    ceri_buf[length - 1] = check_before_eight(ceri_buf, length-1);
    for (i = 0; i < length; ) {
        tx_len = length - i;
        if (tx_len > 512) {
            tx_len = 512;
        }
        memcpy(txbuf, ceri_buf, tx_len);
        ude_iap_transfer(uds, tx_len);
        i += tx_len;
        ceri_buf += tx_len;
    }
}


AT(.usbdev.com)
void ude_iap_update_packet_send(void *tcb, u8 ack, u8 *data, u16 len)
{
    uds_t *uds = (uds_t *)tcb;
    u8 *txbuf = uds->bulk_in->buf;
    u16 packet_len = 12 + len;

    memcpy(txbuf, ea_session_packet, sizeof(ea_session_packet));            //数据包头
    txbuf[3] = packet_len;
    txbuf[5] = iap2_cfg.send_seq++;
    txbuf[6] = ack;
    txbuf[7] = iap2_cfg.session_identifier;
    txbuf[8] = check_before_eight(txbuf, 8);
    txbuf[9] = (iap2_cfg.ea_identifiter >> 8);
    txbuf[10] = (iap2_cfg.ea_identifiter & 0xFF);
    if(len) {
        memcpy((&txbuf[11]), data, len);
    }
    memset(data, 0, len);
    len = 0;
    txbuf[(packet_len - 1)] = check_before_eight(txbuf, (packet_len - 1));
    ude_iap_transfer(uds, packet_len);
    TRACE("session send:%x, %x\n", (iap2_cfg.ea_identifiter >> 8), (iap2_cfg.ea_identifiter & 0xFF));
    return;
}

static u8 temp_data[2] = {0x00, 0x00};
AT(.usbdev.com)
void ude_iap_packet_send(void *tcb, u8 ack)
{
    u16 device_packet_len = cbuf_total_len_get(&(apple_send_cbuf));
    u16 device_packet_len1 = 2;
    u16 packet_len = 12 + device_packet_len;

    uds_t *uds = (uds_t *)tcb;
    u8 *txbuf = uds->bulk_in->buf;

    cbuf_output(device_data_cache, device_packet_len, &(apple_send_cbuf));
    packet_len = 12 + device_packet_len;
    memcpy(txbuf, ea_session_packet, sizeof(ea_session_packet));            //数据包头
    txbuf[2] = (u8)(packet_len >> 8);
    txbuf[3] = (u8)packet_len;
    txbuf[5] = iap2_cfg.send_seq++;
    txbuf[6] = ack;
    txbuf[7] = iap2_cfg.session_identifier;
    txbuf[8] = check_before_eight(txbuf, 8);
    txbuf[9] = (iap2_cfg.ea_identifiter >> 8);
    txbuf[10] = (iap2_cfg.ea_identifiter & 0xFF);
    if(device_packet_len) {
        memcpy((&txbuf[11]), device_data_cache, device_packet_len);
    } else {
        memcpy((&txbuf[11]), temp_data, device_packet_len1);
    }
    txbuf[(packet_len - 1)] = check_before_eight(txbuf, (packet_len - 1));
    ude_iap_transfer(uds, packet_len);
    memset(device_data_cache, 0, device_packet_len);
    device_packet_len = 0;

    TRACE("session send:%x, %x\n", (iap2_cfg.ea_identifiter >> 8), (iap2_cfg.ea_identifiter & 0xFF));
    return;
}

AT(.usbdev.com.iap) WEAK
void ude_iap_tx_process(void)
{
//    my_printf("1:%x,%x\n",iap2_cfg.iap_data_send_flag, iap2_cfg.ea_start_flag);
    if(iap2_cfg.ea_start_flag && iap2_cfg.iap_data_send_flag) {
        ude_iap_packet_send((void *)&uds_0, iap2_cfg.rec_seq);
    }

}


AT(.usbdev.com)
void apple_data_send(u8 *data, u16 len, u8 report_id, u8 type)
{
    if(sys_cb.phone_type == APPLE) {
        if(type) {                                                         //type,0:正常通信，1:升级
            iap2_cfg.apple_pyload[0] = report_id;
            if(len > APPLE_PYLOAD_SIZE) {
                memcpy((iap2_cfg.apple_pyload + 1), data, (APPLE_PYLOAD_SIZE - 1));
                iap2_cfg.data_len = APPLE_PYLOAD_SIZE;
            } else {
                memcpy(iap2_cfg.apple_pyload + 1, data, len);
                iap2_cfg.data_len = len + 1;
            }
            ude_iap_update_packet_send((void *)&uds_0, iap2_cfg.rec_seq, iap2_cfg.apple_pyload, iap2_cfg.data_len);
        } else {
            if(len > APPLE_PYLOAD_SIZE) {
                memcpy(iap2_cfg.apple_pyload, data, APPLE_PYLOAD_SIZE);
                iap2_cfg.data_len = APPLE_PYLOAD_SIZE;
            } else {
                memcpy(iap2_cfg.apple_pyload, data, len);
                iap2_cfg.data_len = len;
            }
            cbuf_input(iap2_cfg.apple_pyload, iap2_cfg.data_len, &(apple_send_cbuf));
        }

    }
}

AT(.rodata.iap)
const uint16_t ccitt_crc16_table[256] = {
0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6, 0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823, 0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70, 0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d, 0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a, 0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};


AT(.com_iap.crc_test)
uint16_t ccitt_crc16(uint8_t *data, uint16_t len)
{
    uint16_t crc16 = 0x0000;
    uint16_t crc_h8, crc_l8;

    while( len-- ) {
        crc_h8 = (crc16 >> 8);
        crc_l8 = (crc16 << 8);
        crc16 = crc_l8 ^ ccitt_crc16_table[crc_h8 ^ *data];
        data++;
    }
    return crc16;
}

AT(.usbdev.com.iap)
void iap_packet_analy(void)
{
    u16 recv_pkg_size = iap2_cfg.packet_size;

    if(iap2_cfg.iap_rec_packet_flag && (cbuf_total_len_get(&(apple_rec_cbuf)) >= recv_pkg_size)) {
        iap2_cfg.iap_rec_packet_flag = 0;

        cbuf_output(mfi_data_cache, recv_pkg_size, &(apple_rec_cbuf));

        if(recv_pkg_size > 15 && mfi_data_cache[11] == 0x55 && (mfi_data_cache[12] == 0x10 || mfi_data_cache[12] == 0x11 || mfi_data_cache[12] == 0x12)) {
            ios_data_cache.ios_outlen_buf[ios_data_cache.w_idx] = (recv_pkg_size - 12);
            cbuf_input((mfi_data_cache + 11), (recv_pkg_size - 12), &(ios_data_cache.ios_data_cbuf));
            ios_data_cache.w_idx++;
            if(ios_data_cache.w_idx >= 100) {
                ios_data_cache.w_idx = 0;
            }
        }

        iap2_cfg.control_byte = mfi_data_cache[4];
        iap2_cfg.rec_seq = mfi_data_cache[5];
        iap2_cfg.rec_ack = mfi_data_cache[6];
        iap2_cfg.session_identifier = mfi_data_cache[7];

        if(memcmp(mfi_data_cache, detect_sequence, 6) == 0){
            c9_upload_vars.iap_send_step = DEVICE_LINK_SYN;
            TRACE("DEVICE_LINK_SYN,APPLE PHONE\n");
        } else if((mfi_data_cache[0] != 0xFF) || (mfi_data_cache[1] != 0x5A)) {                      //数据包，包头不对直接修改为无效包，不做解析
            iap2_cfg.control_byte = APPLE_PACKET_IVALID;
        } else if(iap2_cfg.control_byte == APPLE_PACKET_SYN_ACK) {
            c9_upload_vars.iap_send_step = DEVICE_LINK_ACK;
            TRACE("APPLE_PACKET_SYN_ACK\n");
        } else if((iap2_cfg.control_byte == APPLE_PACKET_ACK) && (recv_pkg_size == 0x09)) {  //没有负载的ack，不回复
            c9_upload_vars.iap_send_step = DEVICE_IDLE;
            TRACE("APPLE_PACKET_ACK\n");
        } else if((iap2_cfg.control_byte == APPLE_PACKET_ACK) && (recv_pkg_size > 0x09)) {    //带有负载的ACK，需要解析数据包
            iap2_cfg.summary_id = mfi_data_cache[13];
            iap2_cfg.summary_id = ((iap2_cfg.summary_id << 8) | mfi_data_cache[14]);
            TRACE("iap2_cfg.summary_id:%x\n",iap2_cfg.summary_id);
            switch(iap2_cfg.summary_id) {
                case APPLE_REQ_AUTHENT_CERT:
                    c9_upload_vars.iap_send_step = DEVICE_AUTHENT_CERT;
                    break;
                case APPLE_REQ_AUTHENT_CHALL_RESP:
                    c9_upload_vars.iap_send_step = DEVICE_AUTHENT_CERT_RESPONSE;
                    break;
                case APPLE_AUTHENT_SUCCESSED:
                    c9_upload_vars.iap_send_step = DEVICE_LINK_ACK;
                    break;
                case APPLE_START_IDENTIFICATION:
                    c9_upload_vars.iap_send_step = DEVICE_IDENTIFICATION;
                    break;
                case APPLE_IDENTIFICATION_ACCEPTED:
                    c9_upload_vars.iap_send_step = DEVICE_START_POWER_UPDATE;                    //需要在认证通过后1s内发送电源更新参数
                    iap2_cfg.app_init_flag = true;                                             //苹果认证完成
                    break;
                case APPLE_START_EA_SESSION:
                    iap2_cfg.ea_identifiter = 0;
                    iap2_cfg.ea_identifiter = mfi_data_cache[24];
                    iap2_cfg.ea_identifiter = ((iap2_cfg.ea_identifiter << 8) | mfi_data_cache[25]);
                    iap2_cfg.ea_start_flag = true;
                    sys_cb.phone_type = APPLE;
#if MFI_NAME_EN
                    c9_upload_vars.iap_send_step = DEVICE_LINK_ACK;
#else
                    c9_upload_vars.iap_send_step = DEVICE_STATUS_EA_SESSION;
#endif
                    break;
                case APPLE_STOP_EA_SESSION:
                    iap2_cfg.ea_identifiter = 0;
                    iap2_cfg.ea_identifiter = mfi_data_cache[19];
                    iap2_cfg.ea_identifiter = ((iap2_cfg.ea_identifiter << 8) | mfi_data_cache[20]);
                    iap2_cfg.ea_start_flag = false;
#if MFI_NAME_EN
                    c9_upload_vars.iap_send_step = DEVICE_LINK_ACK;
#else
                    c9_upload_vars.iap_send_step = DEVICE_STATUS_EA_SESSION;
#endif
                    break;
                case APPLE_POWER_UPDATE:
                    iap2_cfg.mfi_inited_flag = true;
                    iap2_cfg.last_power_sta = sys_cb.ext_powet_flag;
//                    c9_upload_vars.iap_send_step = DEVICE_STOP_POWER_UPFATE;
                    c9_upload_vars.iap_send_step = DEVICE_LINK_ACK;
                    break;
                default:                           //暂时将EA协议心跳包放在这里处理
//                    iap2_cfg.ea_identifiter = mfi_data_cache[9];  //刷新一下端口号
//                    iap2_cfg.ea_identifiter = ((iap2_cfg.ea_identifiter << 8) | mfi_data_cache[10]);
                    if ((mfi_data_cache[11] == 0x03 && mfi_data_cache[12] >= 0x91 && mfi_data_cache[12] <= 0x95) || (iap2_cfg.update_data_flag && mfi_data_cache[11] != 0x55 && mfi_data_cache[12] != 0x12)) {
                    #if 0
                        printf("rcv:%x %d\n", recv_pkg_size, recv_pkg_size);
                        if (recv_pkg_size > 18 && mfi_data_cache[12] == 0x93) {
                            print_r(mfi_data_cache + 17, recv_pkg_size - 18);
                        } else {
                            print_r(mfi_data_cache + 11, recv_pkg_size - 11);
                        }
                    #endif
                        iap2_cfg.ea_identifiter = mfi_data_cache[9];  //刷新一下端口号
                        iap2_cfg.ea_identifiter = ((iap2_cfg.ea_identifiter << 8) | mfi_data_cache[10]);

                        if (mfi_data_cache[11] == 0x03 && mfi_data_cache[12] == 0x91) {
//                            set_update_flag(1);
                            iap2_cfg.update_data_flag = true;
//                            usb_dev_update_rx_iap_pkt((mfi_data_cache + 11), (recv_pkg_size - 12));
                        } else if(mfi_data_cache[11] == 0x03 && mfi_data_cache[12] == 0x95) {
//                            set_update_flag(0);
//                            usb_dev_update_reset(1);
                            iap2_cfg.update_data_flag = false;
                        } else if (recv_pkg_size > 18 && mfi_data_cache[11] == 0x03 && mfi_data_cache[12] == 0x93) {
                            iap2_cfg.crc_data = mfi_data_cache[16];
                            iap2_cfg.crc_data = ((iap2_cfg.crc_data << 8) | mfi_data_cache[15]);
//                            set_update_flag(1);
                            if(iap2_cfg.crc_data == ccitt_crc16((mfi_data_cache + 17), (recv_pkg_size - 18))) {
//                                usb_dev_update_rx_iap_pkt((mfi_data_cache + 17), (recv_pkg_size - 18));
                            } else {
                                memset(&mfi_data_cache[17], 0, (recv_pkg_size - 18));
//                                usb_dev_update_pkt_req();
                            }
                        }

                    } else {
                        if(!iap2_cfg.update_data_flag) {
                            iap2_cfg.ea_identifiter = mfi_data_cache[9];  //刷新一下端口号
                            iap2_cfg.ea_identifiter = ((iap2_cfg.ea_identifiter << 8) | mfi_data_cache[10]);
                        }
                        c9_upload_vars.iap_send_step = DEVICE_EA_SESSION_PROCESS;
                    }
                    break;
            }
            TRACE("c9_upload_vars.iap_send_step:%x\n",c9_upload_vars.iap_send_step);
        }
    }
}

AT(.com_text.iap_serial)
void get_mfi_serial_number(u8 *serial_number)
{
    u8 addr[6];
    u8 mac_data1, mac_data2;
//    bt_get_local_bd_addr(addr);
    for(int i = 0; i < 4; i++) {
        mac_data1 = ((addr[2 + i]&0xf0) >> 4);
        mac_data2 = (addr[2 + i]&0x0f);

        if(mac_data1 >= 0x00 && mac_data1 <= 0x09) {
            serial_number[0 + 2*i] = 0x30 + mac_data1;
        } else {
            serial_number[0 + 2*i] = 0x41 + (mac_data1- 0x09);
        }
        if(mac_data2 >= 0x00 && mac_data2 <= 0x09) {
            serial_number[1 + 2*i] = 0x30 + mac_data2;
        } else {
            serial_number[1 + 2*i] = 0x41 + (mac_data2- 0x09);
        }

    }
}

AT(.usbdev.com.iap) WEAK
void ude_iap_run_loop_execute(void *tcb)
{
    uds_t *uds = (uds_t *)tcb;
    u8 *txbuf = uds->bulk_in->buf;
    u8  databuf[2];
    u8 iap_step;
    u8  response_date_len[2];
    u8 serial_number[8];
    u32 challenge_data_len = 0;

    if(ude_cfgval_get() && (uds->state != UDI_STATE_DONE) && (ude_iap_process_en) && (LOAD_DONE == c9_upload_vars.upload_step)) {
#if MFI_POWER_EN
        if(iap2_cfg.mfi_inited_flag && (iap2_cfg.last_power_sta != sys_cb.ext_powet_flag)) {
            iap2_cfg.last_power_sta = sys_cb.ext_powet_flag;
            memcpy(txbuf, powersource_update, sizeof(powersource_update));  //在发送startpoweruodata之后需要马上更新电源参数给手机
            txbuf[5] = iap2_cfg.send_seq++;
            txbuf[6] = iap2_cfg.rec_seq;                                    //rx收到的ack作为发送数据的seq
            txbuf[8] = check_before_eight(txbuf, 8);
            if(iap2_cfg.last_power_sta) {
                txbuf[19] = 0x09;
                txbuf[20] = 0x60;
            } else {
                txbuf[19] = 0x00;
                txbuf[20] = 0x00;
            }
            txbuf[26] = check_before_eight(txbuf, 26);
            ude_iap_transfer(uds, 27);
        }
#endif
        if(c9_upload_vars.iap_send_step != DEVICE_IDLE) {
            iap_step = c9_upload_vars.iap_send_step;
            c9_upload_vars.iap_send_step = DEVICE_IDLE;
            switch(iap_step){
                case DEVICE_LINK_INIT:
                    {
                        memcpy(&cfg_desc_buf[19], certi_data_buf, c9_upload_vars.load_info_len);
                        memcpy(txbuf, detect_sequence, sizeof(detect_sequence));
                        uds->data_size = sizeof(detect_sequence);
                        ude_iap_transfer(uds, sizeof(detect_sequence));
                    }
                    break;
                case DEVICE_LINK_SYN:
                    {
                        memcpy(&txbuf[0], iap_syn_sequence, sizeof(iap_syn_sequence));
                        txbuf[25] = check_before_eight(txbuf, 25);
                        iap2_cfg.send_seq = 1;
                        uds->data_size = sizeof(iap_syn_sequence);
                        ude_iap_transfer(uds, sizeof(iap_syn_sequence));
                        c9_upload_vars.iap_send_step = DEVICE_IDLE;
                    }
                    break;
                case DEVICE_LINK_ACK:
                    {
                        memcpy(txbuf, iap_ack_cmd, sizeof(iap_ack_cmd));
                        txbuf[5] = iap2_cfg.send_seq;
                        txbuf[6] = iap2_cfg.rec_seq;
                        txbuf[7] = iap2_cfg.session_identifier;
                        txbuf[8] = check_before_eight(txbuf, 8);
                        uds->data_size = sizeof(iap_ack_cmd);
                        ude_iap_transfer(uds, 9);
                    }
                    break;
                case DEVICE_AUTHENT_CERT:
                    {
                        mfi_i2c_init();
                        iap2_cfg.send_seq++;
                        ude_iap_transfer_cerificate_data(uds);                    //第四次收发
                    }
                    break;
                case DEVICE_IDENTIFICATION:
                    {
                        get_mfi_serial_number(serial_number);
                        for(int i = 0; i < 8; i++) {
                            iap_audio_certen_tbl[91 + i] = serial_number[i];
                        }
                        memcpy(txbuf, iap_audio_certen_tbl, sizeof(iap_audio_certen_tbl));
                        txbuf[5] = iap2_cfg.rec_ack + 1;                        //rx收到的ack作为发送数据的seq
                        iap2_cfg.send_seq = txbuf[5] + 1;                       //后续发包该seq + 1
                        txbuf[6] = iap2_cfg.rec_seq;
                        txbuf[8] = check_before_eight(txbuf, 8);
                        txbuf[248] = check_before_eight(txbuf, 248);
                        ude_iap_transfer(uds, 249);                             //第六次收发
                    }
                    break;
                case DEVICE_AUTHENT_CERT_RESPONSE:
                    {
                        memcpy(txbuf, iap_ack_cmd, sizeof(iap_ack_cmd));   //发送challenage之前需要提前发送一个ack，避免获取数据时间过长，ack回复超时
                        txbuf[5] = iap2_cfg.send_seq++;
                        txbuf[6] = iap2_cfg.rec_seq;
                        txbuf[8] = check_before_eight(txbuf, 8);
                        uds->data_size = sizeof(iap_ack_cmd);
                        ude_iap_transfer(uds, 9);

                        memcpy(txbuf, &mfi_data_cache[19], 32);                            //第五次收发（challenage）
                        mfi_i2c_write_data(0x21, txbuf, 32);                      //write challenage data
                        mfi_i2c_read_data(0x20, databuf, 2);                      //read challenage length
                        txbuf[0] = 0x01;
                        iap2_cfg.send_seq++;
                        mfi_i2c_write_data(0x10, txbuf, 1);                       //write authentication control and status
                        if(cfg_auth_cp_version){
                            mfi_i2c_read_data(0x10, txbuf, 1);                   //read authentication status
                            mfi_i2c_read_data(0x11, response_date_len, 2);         //read challenage response data length
                            mfi_i2c_read_data(0x12, &txbuf[19], 64);             //read response data
                            challenge_data_len = ((response_date_len[0]<<8) + response_date_len[1]) + 20;
                            memcpy(txbuf, iap_challenage_cmd, sizeof(iap_challenage_cmd));
                            txbuf[2] = challenge_data_len >> 8;
                            txbuf[3] = (u8)challenge_data_len;
                            txbuf[6] = iap2_cfg.rec_seq;
                            txbuf[8] = check_before_eight(txbuf, 8);
                            txbuf[11] = (challenge_data_len - 10) >> 8;
                            txbuf[12] = (u8)(challenge_data_len - 10);
                            txbuf[15] = (challenge_data_len - 16) >> 8;
                            txbuf[16] = (u8)(challenge_data_len - 16);
                            txbuf[83] = check_before_eight(txbuf, 83);
                            ude_iap_transfer(uds, 84);
                        } else {
                            mfi_i2c_read_data(0x12, &txbuf[19], 128);              //read response data
                            memcpy(txbuf, iap_challenage_cmd, sizeof(iap_challenage_cmd));
                            txbuf[6] = iap2_cfg.rec_seq;
                            txbuf[8] = check_before_eight(txbuf, 8);
                            txbuf[147] = check_before_eight(txbuf, 147);
                            ude_iap_transfer(uds, 148);
                        }
                        TRACE("5\n");
                    }
                    break;
                case DEVICE_POWER_SOURCE_UPDATE:
                    {
                        memcpy(txbuf, powersource_update, sizeof(powersource_update));
                        txbuf[5] = iap2_cfg.send_seq++;
                        txbuf[6] = iap2_cfg.rec_seq;                                //rx收到的ack作为发送数据的seq
                        txbuf[8] = check_before_eight(txbuf, 8);
#if MFI_POWER_EN
                        if(sys_cb.ext_powet_flag) {
                            txbuf[19] = 0x09;
                            txbuf[20] = 0x60;
                        } else {
                            txbuf[19] = 0x00;
                            txbuf[20] = 0x00;
                        }
#endif
                        txbuf[26] = check_before_eight(txbuf, 26);
                        ude_iap_transfer(uds, 27);                            //第八次收发
                    }
                    break;
                case DEVICE_REQUEST_APP_LAUNCH:
    //                memcpy(txbuf, app_launch, sizeof(app_launch));
    //                txbuf[5] = iap2_cfg.send_seq++;
    //                txbuf[6] = iap2_cfg.rec_seq;                                //rx收到的ack作为发送数据的seq
    //                txbuf[8] = check_before_eight(txbuf, 8);
    //                txbuf[36] = check_before_eight(txbuf, 36);
    //                ude_iap_transfer(uds, 37);
                    break;
                case DEVICE_STATUS_EA_SESSION:
                    {
                        memcpy(txbuf, ea_session_status, sizeof(ea_session_status));
                        txbuf[5] = iap2_cfg.send_seq++;
                        txbuf[6] = iap2_cfg.rec_seq;                                //rx收到的ack作为发送数据的seq
                        txbuf[8] = check_before_eight(txbuf, 8);
                        txbuf[19] = iap2_cfg.ea_identifiter >> 8;
                        txbuf[20] = (iap2_cfg.ea_identifiter & 0xFF);
                        txbuf[25] = (!iap2_cfg.ea_start_flag);                      //SessionClose
                        txbuf[26] = check_before_eight(txbuf, 26);
                        ude_iap_transfer(uds, 27);
                    }
                    break;
                case DEVICE_START_POWER_UPDATE:
                    {
                        memcpy(txbuf, iap_startPowerUpdates_cmd, sizeof(iap_startPowerUpdates_cmd));
                        txbuf[5] = iap2_cfg.send_seq++;;
                        txbuf[6] = iap2_cfg.rec_seq;                                    //rx收到的ack作为发送数据的seq
                        txbuf[8] = check_before_eight(txbuf, 8);
                        txbuf[31] = check_before_eight(txbuf, 31);
                        ude_iap_transfer(uds, 32);

                        memcpy(txbuf, powersource_update, sizeof(powersource_update));  //在发送startpoweruodata之后需要马上更新电源参数给手机
                        txbuf[5] = iap2_cfg.send_seq++;
                        txbuf[6] = iap2_cfg.rec_seq;                                    //rx收到的ack作为发送数据的seq
                        txbuf[8] = check_before_eight(txbuf, 8);
#if MFI_POWER_EN
                        if(sys_cb.ext_powet_flag) {
                            txbuf[19] = 0x09;
                            txbuf[20] = 0x60;
                        } else {
                            txbuf[19] = 0x00;
                            txbuf[20] = 0x00;
                        }
#endif
                        txbuf[26] = check_before_eight(txbuf, 26);
                        ude_iap_transfer(uds, 27);
                    }
                    break;
                case DEVICE_STOP_POWER_UPFATE:
                    {
                        memcpy(txbuf, iap_stopPowerUpdates_cmd, sizeof(iap_stopPowerUpdates_cmd));
                        txbuf[5] = iap2_cfg.send_seq++;;
                        txbuf[6] = iap2_cfg.rec_seq;                                //rx收到的ack作为发送数据的seq
                        txbuf[8] = check_before_eight(txbuf, 8);
                        txbuf[16] = check_before_eight(txbuf, 16);
                        ude_iap_transfer(uds, 17);
                    }
                    break;
                case DEVICE_EA_SESSION_PROCESS:
                    {
                        while((cbuf_total_len_get(&(ios_data_cache.ios_data_cbuf))) && (ios_data_cache.r_idx != ios_data_cache.w_idx)) {
                            cbuf_output(ios_data_cache.ios_cache, ios_data_cache.ios_outlen_buf[ios_data_cache.r_idx], &(ios_data_cache.ios_data_cbuf));
//                            user_cfdl_data_in(ios_data_cache.ios_cache, ios_data_cache.ios_outlen_buf[ios_data_cache.r_idx]);
                            ios_data_cache.r_idx++;
                            WDT_CLR();
                            if(ios_data_cache.r_idx >= 100) {
                                ios_data_cache.r_idx = 0;
                            }
//                            user_cfdl_run();
                        }
                        iap2_cfg.iap_data_send_flag = true;
                        ude_iap_tx_process();
                        }
                    break;
                case DEVICE_IDLE:
    //                memset(txbuf, 0, uds->data_size);
                    break;
                default:
                    break;
            }
            if(c9_upload_vars.iap_send_step != DEVICE_LINK_SYN) {
                c9_upload_vars.iap_send_step = DEVICE_IDLE;
            }
        }
    }
}


AT(.usbdev.com)
void ude_iap_get_data_callback(u8 *ptr, u8 len)
{
    iap2_cfg.iap_data_send_flag = false;
    memcpy(ota_data_cache, ptr, len);

    u8 *ptr_iap = ota_data_cache;
    cbuf_input(ota_data_cache, len, &(apple_rec_cbuf));

    if(memcmp(ptr_iap, detect_sequence, 6) == 0) {
        iap2_cfg.packet_size = 6;
    } else if((ptr_iap[0] == 0xFF) && (ptr_iap[1] == 0x5A)){
        iap2_cfg.packet_size = ptr_iap[2];
        iap2_cfg.packet_size = ((iap2_cfg.packet_size << 8) | ptr_iap[3]);
    }

    iap2_cfg.iap_rec_packet_flag = 1;
    iap_packet_analy();

}

///库函数回调
AT(.usbdev.com)
void ude_iap_app_init(void)
{
    memset(&c9_upload_vars, 0, sizeof(c9_upload_t));
    memset(&iap2_cfg, 0, sizeof(iap2_cfg));
    memset(&certi_data_buf, 0, sizeof(certi_data_buf));
    memset(&mfi_data_cache, 0, sizeof(mfi_data_cache));
    memset(&ota_data_cache, 0, sizeof(ota_data_cache));
    memset(&device_data_cache, 0, sizeof(device_data_cache));
    memset(&apple_data_pool, 0, sizeof(apple_data_pool));
    memset(&device_data_pool, 0, sizeof(device_data_pool));
    memset(&apple_rec_cbuf, 0, sizeof(apple_rec_cbuf));
    memset(&apple_send_cbuf, 0, sizeof(apple_send_cbuf));
    memset(&apple_ota_cbuf, 0, sizeof(apple_ota_cbuf));
    memset(&ios_data_cache, 0, sizeof(ios_data_cache));


    cbuf_init(&(apple_rec_cbuf), apple_data_pool, APPLE_DATA_POOL_SIZE);
    cbuf_init(&(apple_send_cbuf), device_data_pool, APPLE_DATA_POOL_SIZE);
    cbuf_init(&(ios_data_cache.ios_data_cbuf), ios_data_cache.ios_data_pool, APPLE_DATA_POOL_SIZE);

    if(0 == auth_cp_oneline_en){
        c9_upload_vars.upload_step = LOAD_DONE;
    }
}

#endif
