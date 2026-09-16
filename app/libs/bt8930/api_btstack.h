/*
 *  api_btstack.h
 *
 *  Created by zoro on 2017-8-24.
 */
#ifndef _API_BTSTACK_H
#define _API_BTSTACK_H

//工作模式
#define MODE_NORMAL                     0       //正常连接模式，关闭DUT测试
#define MODE_NORMAL_DUT                 1       //正常连接模式，使能DUT测试
#define MODE_FCC_TEST                   2       //FCC测试模式，通过串口通信
#define MODE_BQB_RF_BREDR               3       //BR/EDR DUT测试模式，通过蓝牙通信
#define MODE_BQB_RF_BLE                 4       //BLE DUT测试模式，通过串口通信
#define MODE_IODM_TEST                  5       //IODM测试模式，通过串口通信

//删除配对信息TAG
#define BT_INFO_TAG_NOR                 "NULL"   //删除手机
#define BT_INFO_TAG_UNPAIR              "UPAR"   //删除TWS，并断开
#define BT_INFO_TAG_QTEST               "QTET"   //快测删除TWS配对信息
#define BT_INFO_TAG_IODM                "IODM"   //IODM删除TWS配对信息
#define BT_INFO_TAG_CHARGE_BOX          "CBOX"   //充电仓删除TWS配对信息
#define BT_INFO_TAG_USER                "USER"   //用户删除TWS配对信息

#define BD_ADDR_LENGTH 6
typedef uint8_t bd_addr_t[BD_ADDR_LENGTH];

//蓝牙状态
enum {
    BT_STA_OFF,                                 //蓝牙模块已关闭
    BT_STA_INITING,                             //初始化中
    BT_STA_IDLE,                                //蓝牙模块打开，未连接

    BT_STA_SCANNING,                            //扫描中
    BT_STA_DISCONNECTING,                       //断开中
    BT_STA_CONNECTING,                          //连接中

    BT_STA_CONNECTED,                           //已连接
    BT_STA_PLAYING,                             //播放
    BT_STA_INCOMING,                            //来电响铃
    BT_STA_OUTGOING,                            //正在呼出
    BT_STA_INCALL,                              //通话中
    BT_STA_OTA,                                 //OTA升级中
};

//蓝牙通知
enum {
    BT_NOTICE_WIRELESS_CONNECTED = 0x80,        //无线mic连接成功, param[0]=index, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_WIRELESS_DISCONNECT,              //无线mic断开连接, param[0]=index, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_WIRELESS_CONNECT_FAIL,            //无线mic连接失败, param[0]=index, param[1]=reason, param[7:2]=bd_addr

    BT_NOTICE_BROADCAST_SOURCE_TX_EN,
    BT_NOTICE_BROADCAST_SOURCE_TX_DIS,
    BT_NOTICE_BROADCAST_SINK_SYNC_SETUP,
    BT_NOTICE_BROADCAST_SINK_SYNC_LOST,

    BT_NOTICE_WIRELESS_END       = 0x9F,        //无线mic回调消息最后一个

    BT_NOTICE_BLE_CONNECTED      = 0xC0,        //ble连接成功, (param[0]|param[1])=con_handle
    BT_NOTICE_BLE_CON_PARAM_UPD,                //ble连接参数更新成功, (param[0]|param[1])=con_handle, param[2]=reason
    BT_NOTICE_BLE_DISCONNECT,                   //ble断开连接, (param[0]|param[1])=con_handle
};

//功能消息
enum {
    BT_FUNC_ADV_SET_ENABLE          = 0,        //打开/关闭广播
    BT_FUNC_ADV_SET_PARAM,
    BT_FUNC_ADV_SET_DATA,
    BT_FUNC_SET_SCAN_RSP_DATA,
    BT_FUNC_SCAN_SET_ENABLE,                    //打开/关闭扫描
    BT_FUNC_SCAN_SET_PARAM,                     //设置SCAN参数
    BT_FUNC_CONNECT_REQ,                        //发起连接
    BT_FUNC_DISCONNECT_REQ,                     //断开连接
    BT_FUNC_CON_PARAM_UPDATE,
    BT_FUNC_CON_SEND_CMD,
    BT_FUNC_HCI_SEND_CMD,
    BT_FUNC_SET_CON_ID,
    BT_FUNC_RESET_HASH,
    BT_CTL_FCC_TEST,
    BT_FUNC_PER_ADV_SET_ENABLE,
    BT_FUNC_PER_SCAN_SET_ENABLE,
    BT_FUNC_PER_SYNC_SET_ENABLE,
    BT_FUNC_PER_CH_MAP_UPDATE,
    BT_FUNC_BLE_AUDIO_CTRL,
    WIRELESS_CON_PARAM_UPDATE,
    WIRELESS_WL_SET_CON_FLAG,
    BT_CTL_MAX,
};

//蓝牙消息
enum bt_msg_t {
    BT_MSG_CTRL                 = 0,            //蓝牙控制消息
    BT_MSG_FUNC,
};

struct adv_param_t{
    uint16_t adv_int_min;
    uint16_t adv_int_max;
    uint8_t adv_type;
    uint8_t own_address_type;
    uint8_t direct_address_type;
    bd_addr_t direct_address;
    uint8_t channel_map;
    uint8_t filter_policy;
};

struct adv_data_t{
    uint8_t data[31];
    uint8_t data_len;
};

//feature
#define FEAT_D2A                BIT(7)      //Device to ADAPTER
#define FEAT_A2D                BIT(6)      //ADAPTER to Device
#define FEAT_BONDING            BIT(5)      //组队绑定，需要清除配对才能重新组队
#define FEAT_VERS               (0x7<<8)    //bit[10:8]
#define FEAT_RATE               (0x7<<11)   //bit[13:11]，传输速率
#define FEAT_HOP_V1             BIT(14)     //bit[14]，新版本跳频计算
#define FEAT_ADAPTER_SAVE       BIT(15)     //bit[15]，发射端算法音频参数由接收端发送同步

//ext_feature
#define EXT_FEAT_ADV_EXT        BIT(0)      //广播数据包拓展，多8字节可用于广播包发送及scan回调
#define EXT_FEAT_VERS           (0x7<<1)    //bit[3:1]，扩展feat，跟FEAT_VERS合并成64个组合
#define EXT_FEAT_2TNR_NB        (0x7<<4)    //bit[6:4]，2TNR时的最大可连接NB数

extern uint8_t cfg_bt_work_mode;

extern uint8_t cfg_bb_rf_def_txpwr;
extern bool cfg_wireless_role;
extern uint16_t cfg_wireless_feat;

extern uint8_t cfg_bt_rf_def_txpwr;
extern uint8_t cfg_wireless_tx_interval;
extern uint8_t cfg_wireless_con_interval;
extern uint8_t cfg_wireless_tx_retry;
extern uint8_t cfg_discon_auto_pwroff;
extern uint8_t cfg_bb_rf_freq_bands;
extern uint8_t cfg_wireless_codec[2];

#define wireless_role_is_adapter()              cfg_wireless_role
#define wireless_role_is_source()               cfg_wireless_role
#define wireless_mic_is_bonding()               (bool)(cfg_wireless_feat & FEAT_BONDING)

//control
void bb_run_loop(void);
void bt_fcc_init(void);
void bt_init(void);                             //初始化蓝牙变量
int bt_setup(void);                             //打开蓝牙模块
void bt_off(void);                              //关闭蓝牙模块
void bt_wakeup(void);                           //唤醒蓝牙模块
void bt_start_work(uint8_t opcode, uint8_t scan_en); //蓝牙开始工作，opcode: 0=回连, 1=不回连
void bt_send_msg_do(uint msg);                  //蓝牙控制消息，参数详见bt_msg_t
void bt_thread_check_trigger(void);
void bt_audio_bypass(void);                     //蓝牙SBC/SCO通路关闭
void bt_audio_enable(void);                     //蓝牙SBC/SCO通路使能
u32 bt_get_rand(void);

//status
uint bt_get_disp_status(void);                  //获取蓝牙的当前显示状态, V060
uint bt_get_status(void);                       //获取蓝牙的当前状态
bool bt_is_testmode(void);                      //判断当前蓝牙是否处于测试模式
bool bt_is_sleep(void);                         //判断蓝牙是否进入休眠状态
bool bt_is_allow_sleep(void);                   //判断蓝牙是否允许进入休眠状态
uint32_t bt_sleep_proc(void);
void bt_enter_sleep(void);
void bt_exit_sleep(void);

uint8_t bb_crc8(const uint8_t *data, size_t len);

#define bt_send_msg(ogf, ocf)                   bt_send_msg_do((ogf<<24) | (ocf))
#define bt_ctrl0_msg(param)                     bt_send_msg(BT_MSG_CTRL, param)
#define bt_func_msg(msg, param)                 bt_send_msg(BT_MSG_FUNC, (msg<<16) | (param))

#define ble_adv_set_enable(pscan, iscan)        bt_func_msg(BT_FUNC_ADV_SET_ENABLE, (pscan<<1)|(iscan))     //广播使能，pscan=可被连接，iscan=可被发现
#define ble_adv_set_interval(interval)          bt_func_msg(BT_FUNC_ADV_SET_PARAM,interval)                 //设置ADV 广播间隔
#define ble_scan_set_enable(en)                 bt_func_msg(BT_FUNC_SCAN_SET_ENABLE, en)                    //扫描使能
#define ble_scan_set_param(intv,window)         bt_func_msg(BT_FUNC_SCAN_SET_PARAM, (intv<<8)|window)       //设置ble_scan参数
#define ble_connect_req(ms)                     bt_func_msg(BT_FUNC_CONNECT_REQ, ms)                        //发起连接，ms=连接超时（单位毫秒，0xffff时一直连接不超时）
#define ble_disconnect_req(idx)                 bt_func_msg(BT_FUNC_DISCONNECT_REQ, idx)                    //发起断开，idx=连接通道
#define ble_hci_cmd_kick()                      bt_func_msg(BT_FUNC_HCI_SEND_CMD, 0)                        //处理HCI命令使能
#define ble_set_con_id(id)                      bt_func_msg(BT_FUNC_SET_CON_ID, id)                         //设置连接id
#define ble_set_con_flag(adv_flag,scan_flag)    bt_func_msg(WIRELESS_WL_SET_CON_FLAG, (adv_flag<<8)|scan_flag)//设置adv/scan类型，是用于t2r链路还是普通连接链路

///broadcast专用命令
#define ble_per_adv_set_enable(en)              bt_func_msg(BT_FUNC_PER_ADV_SET_ENABLE, en)                 //私有周期广播使能
#define ble_per_scan_set_enable(en)             bt_func_msg(BT_FUNC_PER_SCAN_SET_ENABLE, en)                //私有周期广播扫描使能
#define ble_per_sync_set_enable(en)             bt_func_msg(BT_FUNC_PER_SYNC_SET_ENABLE, en)                //私有周期广播同步使能，在使能后调用这个可以停止sync
#define ble_per_ch_map_update()                 bt_func_msg(BT_FUNC_PER_CH_MAP_UPDATE, 0)                   //Channel update

void ble_create_con_for_addr(uint8_t *addr, uint16_t timeout);
void ble_con_channel_assess(uint8_t chidx, bool rx_ok);
uint8_t ble_con_get_status(void);
void ble_close_all_con(void);

bool ble_con_user_cmd_tx_req(uint8_t index);        //请求发送用户私有命令，请求后库通过ble_con_user_cmd_get_tx_cb获取
void broadcast_source_user_cmd_tx_req(uint8_t index);        //请求发送用户私有命令，请求后库通过ble_con_user_cmd_get_tx_cb获取

uint16_t ble_single_link_duration_get(void);
void ble_audio_pwr_ctr_set(uint8_t idx, uint8_t pwr_level);
void ble_pscan_set_adv_ext_data(uint8_t *adv_ext_data, uint8_t adv_ext_len);       //无线麦扩展广播数据设置接口
void ble_ws_per_set_adv_ext_data(uint8_t *adv_ext_data, uint8_t adv_ext_len);      //广播麦专用扩展广播数据设置接口

//link info
uint  bt_get_link_info_addr(uint8_t *bd_addr, int order);
uint bt_get_link_info_nb(void);
void bt_delete_link_info(uint8_t *bd_addr);    //tag给NULL，ra_addr给0
void bt_delete_all_link_info(const char *tag, uint32_t ra_addr);

/*****************************************************************************
 * BLE连接相关
 *****************************************************************************/
//DUT测试设置
void ble_dut_set_tx_callback(void *handle);
void ble_dut_set_pkt_buf(void *buf);

bool gap_adv_param_set(struct adv_param_t* param);
bool gap_adv_data_set(struct adv_data_t* param);
bool gap_scan_rsp_data_set(struct adv_data_t* param);
void gap_adv_en_set(uint8_t adv_en);
void gap_update_conn_param(u16 con_handle, u16 interval, u16 latency, u16 timeout);
void gap_user_channel_send_buf(uint8_t index, uint8_t *buf, uint16_t buf_len);

#endif //_API_BTSTACK_H
