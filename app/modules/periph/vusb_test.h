#ifndef _VUSB_TEST_H
#define _VUSB_TEST_H

#define VUSB_TEST_BAUD          115200
#define VUSB_TEST_START_TO      10          ///单位5ms
#define VUSB_SET_CONFIG_TO      200         ///单位5ms

#define VUSB_TEST_WS_NAME_LEN   32
#define VUSB_TEST_MAX_LEN       sizeof(struct vusb_set_config_tag)
#define VUSB_TEST_HEAD          0x4567

enum{
    VUSB_TEST_START_REQ,
    VUSB_TEST_START_RSP,

    VUSB_GET_INFO_REQ,
    VUSB_GET_INFO_RSP,

    VUSB_SET_CONFIG_REQ,
    VUSB_SET_CONFIG_RSP,
};

typedef struct vusb_set_config_tag{
    ///无线麦interval;
    uint16_t tx_intv;
    uint16_t con_intv;
    uint16_t ws_feat;
    uint8_t codec[2];
    ///无线麦频段
    uint8_t freq_band;
    uint8_t retry;
    uint8_t discon_auto_pwroff;
	uint8_t config_rssi;
	uint8_t reserve[3];
    uint8_t ws_name_len;
    uint8_t ws_name[VUSB_TEST_WS_NAME_LEN];
} vusb_set_config_t;

typedef struct {
    uint16_t vusb_head;
    uint8_t reserve1;
    uint8_t crc8;
    uint8_t cmd;
    uint8_t len;
    uint8_t reserve2[2];
    uint8_t buf[VUSB_TEST_MAX_LEN];
} vusb_test_cmd_t;

typedef struct {
    vusb_test_cmd_t huart_test_cmd;
    uint8_t rx_kick_flag;
    uint8_t ws_name_set_flag;
    uint8_t ws_name_len;
    uint8_t ws_name[VUSB_TEST_WS_NAME_LEN];
    uint8_t ws_config_sucess_flag;
    uint8_t config_rssi;
    uint8_t vusb_start_test_flag;
} vusb_test_str_t;

void vusb_test_init(void);
void vusb_test_exit(void);
uint8_t vusb_test_rssi_get(void);
void vusb_test_rx_process(void);
uint8_t vusb_test_is_sucess(void);
uint8_t vusb_test_name_get(u8 *le_name);
void bsp_vusb_test_process(void);
bool vusb_test_huart_done(void);

void vusb_test_huart_init(void);
#endif
