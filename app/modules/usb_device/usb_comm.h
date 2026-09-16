#ifndef _USB_COMM_H
#define _USB_COMM_H

///应用层 USB公共配置

#if UDE_24BIT_STEREO

#define USB_MIC_Channel                 0x02      //双声道
#define USB_MIC_Channel_CFG             0x03
#define USB_MIC_FRAME                   0x03      //3byte/frame
#define USB_MIC_SAMPLE                  0x18      //24bit
#define USB_MIC_BUF_SIZE                288
#else

#define USB_MIC_Channel                 0x01      //单声道
#define USB_MIC_Channel_CFG             0x00
#define USB_MIC_FRAME                   0x02      //2byte/frame
#define USB_MIC_SAMPLE                  0x10      //16bit
#define USB_MIC_BUF_SIZE                128
#endif

#define EP_BULK_NUM             1
#define EP_CS_AC_NUM            1
#define EP_ISOOUT_NUM           3
#define EP_ISOIN_NUM            3
#define EP_HID_NUM              2
#define EP_HID2_NUM             1

//端点传输样点
#define USB_CTRL_SIZE           64
#define USB_BULK_SIZE           64
#define USB_ISOOUT_SIZE         192     //Audio speaker  samplerate * channel * pcmbits = 48 * 2 * 2
#define USB_ISOIN_SIZE          USB_MIC_BUF_SIZE     //MIC
#define USB_HID_IN_SIZE         64
#define USB_HID_OUT_SIZE        64

//Descriptor String Index
#define STR_LANGUAGE_ID                 0
#define STR_MANUFACTURER                1
#define STR_PRODUCT                     2
#define STR_SERIAL_NUM                  3

//端点功能配置
//可以remap不同的endpoint number
#define EP_BULK_CFG_NUM         1
#define EP_ISOOUT_CFG_NUM       3
#define EP_ISOIN_CFG_NUM        3
#define EP_HID_CFG_NUM          2

#define MASK_STORAGE            BIT(0)
#define MASK_SPEAKER            BIT(1)
#define MASK_HID                BIT(2)
#define MASK_MIC                BIT(3)
#define MASK_ISO_AUDIO          (MASK_SPEAKER | MASK_MIC)

#define UR_GET_STATUS                   0x00
#define UR_CLEAR_FEATURE                0x01
#define UR_SET_FEATURE                  0x03
#define UR_SET_ADDRESS                  0x05
#define UR_GET_DESCRIPTOR               0x06
#define UR_GET_CONFIGURATION            0x08
#define UR_SET_CONFIGURATION            0x09
#define UR_GET_INTERFACE                0x0a
#define UR_SET_INTERFACE                0x0b

//Recipient
#define REQ_DEVICE                      0
#define REQ_INTERFACE                   1
#define REQ_ENDPOINT                    2

//Feature
#define ENDPOINT_STALL                  0
#define REMOTE_WAKEUP                   1
#define TEST_MODE                       2

//Device Descriptor
#define DEVICE_DESCRIPTOR                   1
#define CONFIGURATION_DESCRIPTOR            2
#define STRING_DESCRIPTOR                   3
#define INTERFACE_DESCRIPTOR                4
#define ENDPOINT_DESCRIPTOR                 5
#define DEVICE_QUALIFIER_DESCRIPTOR         6
#define OTHER_SPEED_CONFIG_DESCRIPTOR       7
#define HID_DESCRIPTOR                      0x21    //获取HID描述符
#define HID_REPORT                          0x22    //获取HID报表

//Class Request
#define BULK_RESET                      0xff
#define GET_MAX_LUN                     0xfe

//Audio Class-Specific Request Codes
#define SET_CUR                         0x01
#define SET_MIN                         0x02
#define SET_MAX                         0x03
#define SET_RES                         0x04
#define GET_CUR                         0x81        //Current setting attribute
#define GET_MIN                         0x82        //Minimum setting attribute
#define GET_MAX                         0x83        //Maximum setting attribute
#define GET_RES                         0x84        //Resolution attribute
#define SET_IDLE                        0x0a        //USB HID SET_IDLE

//USB Audio Feature Unit ID
#define UID_SPK                         0x02
#define UID_MIC                         0x05

///Setup Packet Block
typedef struct _spb_wrap_t {
    union {
        struct {
            u8 recipient        : 5,
                type            : 2,
                dir             : 1;
        };
        u8 reqtype;
    };
    u8 req;
    u16 val;
    u16 index;
    u16 len;
} spb_wrap_t;

void ude_ep_reset(void); //ep0 枚举端点 标准通信端点复位
void ude_control_flow(void);
void ude_isoc_tx_process(void);
void ude_isoc_rx_process(void);
void ude_hid_get_data();
#endif
