#ifndef _API_USB_H
#define _API_USB_H

enum {
    USBCHK_ONLY_HOST,
    USBCHK_ONLY_DEVICE,
    USBCHK_OTG,
};

enum {
    USB_DISCONNECT,
    USB_UDISK_CONNECTED,
    USB_PC_CONNECTED,
};

//USB Device HID Comsumer usage id
#define UDE_HID_PLAYPAUSE       BIT(0)                  //0xcd
#define UDE_HID_VOLUP           BIT(1)                  //0xe9
#define UDE_HID_VOLDOWN         BIT(2)                  //0xea
#define UDE_HID_RESV            BIT(3)                  //0xcf
#define UDE_HID_HOME            BIT(4)                  //0x40
#define UDE_HID_NEXTFILE        BIT(5)                  //0xb5
#define UDE_HID_PREVFILE        BIT(6)                  //0xb6
#define UDE_HID_MUTE            BIT(7)                  //0xe2

void usb_init(void);
void usb_disable(void);
u8 usbchk_connect(u8 mode);
u8 usb_connect(void);
void udisk_remove(void);
void udisk_insert(void);
void udisk_invalid(void);

void udisk_suspend(void);
void udisk_resume(void);

void usb_device_enter(u8 enum_type);
void usb_device_exit(void);
void usb_device_process(void);
void ude_tmr_isr(void);
void pc_remove(void);
bool usb_device_hid_send(u16 hid_val, int auto_release);
u8 typec_computer_mobile_sta(void);

u16 usbmic_len_get(void);
#endif // _API_USB_H
