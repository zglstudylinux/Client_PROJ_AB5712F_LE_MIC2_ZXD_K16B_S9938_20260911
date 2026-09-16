#ifndef _USB_ENUM_H
#define _USB_ENUM_H

//Descriptor String Index
#define STR_LANGUAGE_ID                 0
#define STR_MANUFACTURER                1
#define STR_PRODUCT                     2
#define STR_SERIAL_NUM                  3
#define STR_MICROSOFT_OS                0xEE        //Microsoft OS Descriptors

#define MASK_STORAGE            BIT(0)
#define MASK_SPEAKER            BIT(1)
#define MASK_HID                BIT(2)
#define MASK_MIC                BIT(3)
#define MASK_ISO_AUDIO          (MASK_SPEAKER | MASK_MIC)
#define MASK_BULK               MASK_STORAGE

typedef struct{
    u8 product_name_buf[74];            //36个字符
    u8 supplier_buf[38];                //18个字符
    u8 serial_number_buf[34];           //16个字符
    u8 vid_pid_buf[4];

    // u8 user_define_hid_config[64];
    // u8 user_define_hid_report[64];
}usb_enum_t;

void dev_product_name_init(void);
void dev_supplier_init(void);
void dev_serial_number_init(void);
void dev_vid_pid_init(void);
void ude_info_init(void);
#endif
