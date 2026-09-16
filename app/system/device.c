#include "include.h"


AT(.com_text.detect)
u8 get_usbtf_muxio(void)
{
#if SD_USB_MUX_IO_EN
	return 1;
#else
	return 0;
#endif
}

//AT(.com_text.const)
//const char usb_detect_str[] = "USB STA:%d\r\n";
//AT(.com_text.const)
//const char usb_insert_str[] = "udisk insert\n";
//AT(.com_text.const)
//const char usb_remove_str[] = "udisk remove\n";

