#ifndef _USB_DEVICE_STORAGE_H
#define _USB_DEVICE_STORAGE_H

#include "usb_device_ep.h"

enum uds_state {
    UDS_STATE_COMMAND_PHASE = -10,      // This one isn't used anywhere
    UDS_STATE_DATA_PHASE,
    UDS_STATE_STATUS_PHASE,

    UDS_STATE_IDLE = 0,
    UDS_STATE_ABORT_BULK_OUT,
    UDS_STATE_RESET,
    UDS_STATE_INTERFACE_CHANGE,
    UDS_STATE_CONFIG_CHANGE,
    UDS_STATE_DISCONNECT,
    UDS_STATE_EXIT,
    UDS_STATE_TERMINATED
};

// Command Data Block
typedef struct _cdb_wrap_t {
    u32 addr;
    u16 len;
    u8  op;
    u8  subop;
    u8  subex;
} cdb_wrap_t;

typedef struct _diskio_t {
    void (*is_valid)(void);
    void (*read)(void);
    void (*write)(void);
} diskio_t;

typedef struct _lun_t {
    diskio_t *diskio;
    u32 file_length;
    u32 num_sectors;
    u32 sector_size;

    u32 sense_data;
    //u32 sense_data_info;
    //u32 unit_attention_data;

    u32 ro                          : 1,        //只读
        prevent_medium_removal      : 1;        //是否右键弹出
} lun_t;

typedef struct _uds_t {
    epcb_t *bulk_in;
    epcb_t *bulk_out;

    lun_t *luns;
    lun_t *curlun;

    u32 sense_data;                 //只保留一份Sense DATA

    u32 tag;
    u32 data_size;
    u32 residue;
    u8 flags;
    u8 lun;
    u8 nluns;

    u8 state;

    volatile u8 error       : 1,    //出错
        bulk_halt           : 1;    //Bulk Halt

    cdb_wrap_t cdb;                 //Command Data Block

    u32 rw_tick;
} uds_t;                            //usb device storage

void uds_init(uds_t *uds);
void uds_run_loop_execute(uds_t *uds);
void uds_add_lun(uds_t *uds, lun_t *lun);
void usb_bulk_reset(void);

#endif // _USB_DEVICE_STORAGE_H
