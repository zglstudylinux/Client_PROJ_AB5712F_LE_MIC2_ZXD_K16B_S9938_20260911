#include "include.h"
#include "usb_enum.h"

#define TRACE_EN                0
#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif // TRACE_EN

const char ude_product_name[] = UDE_PROD_NAME;
const char ude_supplier[] = UDE_SUPPLIER;
const char ude_serial_number[] = UDE_SERIAL_NB;

usb_enum_t ue_vars AT(.usb.device.desc.buf);

AT(.text.usb_enum)
u32 upow_process(u32 groundNum, u32 topNum)
{
    u32 result = 1;
    int i;
    for(i = 0; i < topNum; i++)
    {
        result = result * groundNum;
    }
    return result;
}

AT(.text.usb_enum)
void ude_cfg_str_convert(u8 *in, u8 *out, int in_len, int out_len)
{
    int i, char_size;
    u16 unicode;
    int cnt = 0;

    out[cnt++] = out_len;
    out[cnt++] = 0x03;
    for (i = 0; i < in_len; ) {
        if (!in[i]) {
            break;
        }
        char_size = utf8_char_size(in[i]);
        unicode = utf8_convert_to_unicode(&in[i], char_size);
        out[cnt++] = (u8)unicode;
        out[cnt++] = (u8)(unicode >> 8);
        i += char_size;
        if (cnt >= out_len) {
            break;
        }
    }
    out[0] = cnt;                           //更新实际长度
}

#define OUT_SERIAL_MAX_SIZE 34
#define CUR_SERIAL_DECI_MAX 10
#define TOTAL_STR_HEX_MAX   10

AT(.text.usb_enum)
void ude_cfg_str_incre_convert(u8 *in_incre_addr,u8 *in_xcfg_serial_num, u8 *out_serial_num,
                                int in_incre_len, int in_xcfg_serial_len, int out_serial_len)
{
    int i, char_size;
    u16 unicode;
    int cnt = 0;

    memset(out_serial_num, 0, 34);//后面的字符也要清空
    for(i = 0; i < (OUT_SERIAL_MAX_SIZE-2)/2; i++){
        out_serial_num[i*2 + 2] = 0x30;//空字符的时候给0
    }

    out_serial_num[cnt++] = out_serial_len;
    out_serial_num[cnt++] = 0x03;
    for (i = 0; i < in_xcfg_serial_len; ) {
        if (!in_xcfg_serial_num[i]) {

            break;
        }
        char_size = utf8_char_size(in_xcfg_serial_num[i]);
        unicode = utf8_convert_to_unicode(&in_xcfg_serial_num[i], char_size);
        out_serial_num[cnt++] = (u8)unicode;
        out_serial_num[cnt++] = (u8)(unicode >> 8);
        i += char_size;
        if (cnt >= out_serial_len) {
            break;
        }
    }
    out_serial_num[0] = cnt;                           //更新实际长度

    u8 cur_serialNum_deci[CUR_SERIAL_DECI_MAX];
    u8 totalNum_str_hex[TOTAL_STR_HEX_MAX];
    u32 increNum = 0, curNum_hex = 0, totalNum = 0, index;
    increNum += (in_incre_addr[2] << 24);
    increNum += (in_incre_addr[3] << 16);
    increNum += (in_incre_addr[4] << 8);
    increNum += in_incre_addr[5];
    memset(cur_serialNum_deci, 0, sizeof(cur_serialNum_deci));
    memset(totalNum_str_hex, 0, sizeof(totalNum_str_hex));
    for(i = 0; i < CUR_SERIAL_DECI_MAX; i++){
        cur_serialNum_deci[CUR_SERIAL_DECI_MAX-1-i] = out_serial_num[OUT_SERIAL_MAX_SIZE-2-i*2] - 0x30;
    }
    for(i = 0; i < (CUR_SERIAL_DECI_MAX - 1); i++){
        index = (CUR_SERIAL_DECI_MAX-1-i);
        curNum_hex += upow_process(10, index) * cur_serialNum_deci[i];
        if(i == (CUR_SERIAL_DECI_MAX-1-1))
        {
            curNum_hex = curNum_hex + cur_serialNum_deci[CUR_SERIAL_DECI_MAX-1];//加最后一位数
        }
    }
    totalNum = increNum + curNum_hex;
    for(i = 0; i < TOTAL_STR_HEX_MAX; i++){
        u32 tempNum = totalNum % 10;
        totalNum = totalNum / 10;
        totalNum_str_hex[TOTAL_STR_HEX_MAX-1-i] = tempNum;
    }

    out_serial_num[0] = 0x22;                           //累加后字符长度为最大
    for(i = 0; i < TOTAL_STR_HEX_MAX; i++)
    {
       out_serial_num[OUT_SERIAL_MAX_SIZE-1-1-2*i] = totalNum_str_hex[TOTAL_STR_HEX_MAX-1-i] + 0x30;
    }
}

AT(.usbdev.com)
const u8 *get_usb_dev_product_name(void)
{
    return (const u8 *)ue_vars.product_name_buf;
}

AT(.text.usb_enum)
void dev_product_name_init(void)
{
    ude_cfg_str_convert((u8 *)ude_product_name, (u8 *)ue_vars.product_name_buf, sizeof(ude_product_name), sizeof(ue_vars.product_name_buf));
    TRACE("product_name\n");
    TRACE_R(ue_vars.product_name_buf, sizeof(ue_vars.product_name_buf));
}

AT(.usbdev.com)
const u8 *get_usb_dev_supplier(void)
{
    return (const u8 *)ue_vars.supplier_buf;
}

AT(.text.usb_enum)
void dev_supplier_init(void)
{
    
    ude_cfg_str_convert((u8 *)ude_supplier, (u8 *)ue_vars.supplier_buf, sizeof(ude_supplier), sizeof(ue_vars.supplier_buf));
    TRACE("supplier name\n");
    TRACE_R(ue_vars.supplier_buf, sizeof(ue_vars.supplier_buf));
}

AT(.usbdev.com)
const u8 *get_usb_dev_serial_number(void)
{
    return (const u8 *)ue_vars.serial_number_buf;
}

AT(.text.usb_enum)
void dev_serial_number_init(void)
{
    // if(USB_SERIAL_INCREASE_EN){
    //     ude_cfg_str_incre_convert((u8 *)xcfg_cb.bt_addr, (u8 *)xcfg_cb.ude_serial_number, (u8 *)ue_vars.serial_number_buf,
    //                             sizeof(xcfg_cb.bt_addr), sizeof(xcfg_cb.ude_serial_number), sizeof(ue_vars.serial_number_buf));
    // }else{
        ude_cfg_str_convert((u8 *)ude_serial_number, (u8 *)ue_vars.serial_number_buf, sizeof(ude_serial_number), sizeof(ue_vars.serial_number_buf));
        TRACE("serial number\n");
        TRACE_R(ue_vars.serial_number_buf, sizeof(ue_vars.serial_number_buf));
    // }
}

AT(.usbdev.com)
const u8 *get_usb_dev_vid_pid(void)
{
    return (const u8 *)ue_vars.vid_pid_buf;
}
#define PUT1_LE16(ptr, val)     do { \
                                u16 tmp = (u16)(val);       \
                                memcpy((void*)(ptr), &tmp, sizeof(u16)); \
                                } while(0)
AT(.text.usb_enum)
void dev_vid_pid_init(void)
{
    PUT1_LE16(&ue_vars.vid_pid_buf[0], UDE_VID);
    PUT1_LE16(&ue_vars.vid_pid_buf[2], UDE_PID);
}

void ude_info_init(void)
{
    memset(&ue_vars, 0x00, sizeof(ue_vars));

    dev_product_name_init();
    dev_supplier_init();
    dev_serial_number_init();
    dev_vid_pid_init();
}
