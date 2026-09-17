#ifndef _EFFECT_H
#define _EFFECT_H


#define BUF_OFS_CFG_MIC_EQ           		(0x00000016)
#define BUF_LEN_OFS_CFG_MIC_EQ       		(0x00000012)
#define SHOWBUF_OFS_CFG_MIC_EQ       		(0x00000016)
#define SHOWBUF_LEN_OFS_CFG_MIC_EQ   		(0x000000FA)

#define BUF_OFS_CFG_MIC_DRC          		(0x0000002A)
#define BUF_LEN_OFS_CFG_MIC_DRC      		(0x00000026)
#define SHOWBUF_OFS_CFG_MIC_DRC      		(0x0000002A)
#define SHOWBUF_LEN_OFS_CFG_MIC_DRC  		(0x000000FE)

#define BUF_OFS_CFG_ECHO             		(0x0000003E)
#define BUF_LEN_OFS_CFG_ECHO         		(0x0000003A)
#define SHOWBUF_OFS_CFG_ECHO         		(0x0000003E)
#define SHOWBUF_LEN_OFS_CFG_ECHO     		(0x00000102)

#define BUF_OFS_CFG_MAGIC            		(0x00000052)
#define BUF_LEN_OFS_CFG_MAGIC        		(0x0000004E)
#define SHOWBUF_OFS_CFG_MAGIC        		(0x00000052)
#define SHOWBUF_LEN_OFS_CFG_MAGIC    		(0x00000106)

#define BUF_OFS_CFG_MIX_EQ           		(0x00000066)
#define BUF_LEN_OFS_CFG_MIX_EQ       		(0x00000062)
#define SHOWBUF_OFS_CFG_MIX_EQ       		(0x00000066)
#define SHOWBUF_LEN_OFS_CFG_MIX_EQ   		(0x0000010A)

#define BUF_OFS_CFG_MIX_DRC          		(0x0000007A)
#define BUF_LEN_OFS_CFG_MIX_DRC      		(0x00000076)
#define SHOWBUF_OFS_CFG_MIX_DRC      		(0x0000007A)
#define SHOWBUF_LEN_OFS_CFG_MIX_DRC  		(0x0000010E)

#define BUF_OFS_CFG_USB_EQ           		(0x0000008E)
#define BUF_LEN_OFS_CFG_USB_EQ       		(0x0000008A)
#define SHOWBUF_OFS_CFG_USB_EQ       		(0x0000008E)
#define SHOWBUF_LEN_OFS_CFG_USB_EQ   		(0x00000112)

#define BUF_OFS_CFG_USB_DRC          		(0x000000A2)
#define BUF_LEN_OFS_CFG_USB_DRC      		(0x0000009E)
#define SHOWBUF_OFS_CFG_USB_DRC      		(0x000000A2)
#define SHOWBUF_LEN_OFS_CFG_USB_DRC  		(0x00000116)

#define BUF_OFS_CFG_USB1_EQ          		(0x000000B6)
#define BUF_LEN_OFS_CFG_USB1_EQ      		(0x000000B2)
#define SHOWBUF_OFS_CFG_USB1_EQ      		(0x000000B6)
#define SHOWBUF_LEN_OFS_CFG_USB1_EQ  		(0x0000011A)

#define BUF_OFS_CFG_USB1_DRC         		(0x000000CA)
#define BUF_LEN_OFS_CFG_USB1_DRC     		(0x000000C6)
#define SHOWBUF_OFS_CFG_USB1_DRC     		(0x000000CA)
#define SHOWBUF_LEN_OFS_CFG_USB1_DRC 		(0x0000011E)

#define BUF_OFS_CFG_USB0_EQ          		(0x000000DE)
#define BUF_LEN_OFS_CFG_USB0_EQ      		(0x000000DA)
#define SHOWBUF_OFS_CFG_USB0_EQ      		(0x000000DE)
#define SHOWBUF_LEN_OFS_CFG_USB0_EQ  		(0x00000122)

#define BUF_OFS_CFG_USB0_DRC         		(0x000000F2)
#define BUF_LEN_OFS_CFG_USB0_DRC     		(0x000000EE)
#define SHOWBUF_OFS_CFG_USB0_DRC     		(0x000000F2)
#define SHOWBUF_LEN_OFS_CFG_USB0_DRC 		(0x00000126)



#define ALL_MODE_NAME_SUM            		 0x0000172D


//EFFECT_MODE_IDX
enum {
	CFG_MIC_EQ = 0,
	CFG_MIC_DRC,
	CFG_ECHO,
	CFG_MAGIC,
	CFG_MIX_EQ,
	CFG_MIX_DRC,
	CFG_USB_EQ,
	CFG_USB_DRC,
	CFG_USB1_EQ,
	CFG_USB1_DRC,
	CFG_USB0_EQ,
	CFG_USB0_DRC,
	CFG_MAX,
};


typedef struct {
	char effect_cfg_name[12];
	u32  effect_res_offset;
	u32  effect_len_offset;
} effect_info_cfg_t;

extern const effect_info_cfg_t effect_info[CFG_MAX];

typedef struct {
	void (*effect_update_callback)(u8 *buf, u32 len, u8 params);//0:初始化 1:更新
} effect_update_callback_t;

extern const effect_update_callback_t effect_update_callback_tbl[CFG_MAX];

#endif