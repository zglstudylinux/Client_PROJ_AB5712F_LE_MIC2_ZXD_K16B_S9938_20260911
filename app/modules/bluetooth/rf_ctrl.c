#include "include.h"
#include "api.h"


//----------------------------------------------------------------------------
//获取配置中的RF参数
const uint8_t *bt_rf_get_param(void)
{
    //优先使用FT参数，其次自定义参数，最后是库预置参数
    if(xcfg_cb.ft_rf_param_en && bt_get_ft_trim_value(&xcfg_cb.rf_pa_gain)) {
        return (const uint8_t *)&xcfg_cb.rf_pa_gain;
    } else if(xcfg_cb.bt_rf_param_en) {
        return (const uint8_t *)&xcfg_cb.rf_pa_gain;
    }
    return NULL;
}

#if BT_RF_EXT_CTL_EN
const uint8_t cfg_bb_rf_ctl = BIT(0);   //bit0=RF_EXT_CTL_EN


//此处添加外部PA/LNA控制IO，注意所有函数放公共区，不能加打印
AT(.com_text.isr.txrx)
void bb_rf_ext_ctl_cb(u32 rf_sta)
{
	if(rf_sta & BIT(8)) {			//tx on, enable PA
        RF_CTL_TX();
//	} else if(rf_sta & BIT(9)){	    //tx down, disable PA
	} else if(rf_sta & BIT(10)){	//rx on, enable LNA
	    RF_CTL_RX();
//	} else if(rf_sta & BIT(11)){	//rx down, disable LNA
	} else {						//idle
	    RF_CTL_IDLE();
	}
}

//初始化蓝牙时，初始化外部PA/LNA控制IO
void bb_rf_ext_ctl_init(void)
{
    RF_CTL_INIT();
}

//关闭蓝牙时，关闭外部PA/LNA
void bb_rf_ext_ctl_exit(void)
{
}
#endif
