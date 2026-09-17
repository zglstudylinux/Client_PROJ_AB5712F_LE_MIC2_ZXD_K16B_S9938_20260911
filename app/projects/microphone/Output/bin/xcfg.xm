depend(0x01020100);

config(SUB, "系统配置", "系统的相关配置");
config(LIST, "提示音语言选择", "选择系统的语言", LANG_ID, 4, "英文", "中文", "中英文(出厂默认英文)", "中英文(出厂默认中文)", 0);
config(CHECK, "POWKEY 10s复位系统", "是否长按POWKEY 10s复位系统，用于防止系统死机", POWKEY_10S_RESET, 1);
config(LISTVAL, "自动休眠时间", "设置自动休眠时间", SYS_SLEEP_TIME, SHORT, 30, 21, ("不休眠", 0), ("10秒钟后", 10), ("20秒钟后", 20), ("30秒钟后", 30), ("45秒钟后", 45), ("1分钟后", 60), ("2分钟后", 120), ("3分钟后", 180), ("4分钟后", 240), ("5分钟后", 300), ("6分钟后", 360), ("7分钟后", 420), ("8分钟后", 480), ("9分钟后", 540), ("10分钟后", 600), ("15分钟后", 900), ("20分钟后", 1200), ("25分钟后", 1500), ("30分钟后", 1800), ("45分钟后", 2700), ("1小时后", 3600));
config(LISTVAL, "自动关机时间", "设置自动关机时间", SYS_OFF_TIME, 	SHORT, 300, 18, ("不关机", 0), ("30秒钟后", 30), ("1分钟后", 60),  ("2分钟后", 120), ("3分钟后", 180), ("4分钟后", 240), ("5分钟后", 300), ("6分钟后", 360), ("7分钟后", 420), ("8分钟后", 480), ("9分钟后", 540), ("10分钟后", 600), ("15分钟后", 900), ("20分钟后", 1200), ("25分钟后", 1500), ("30分钟后", 1800), ("45分钟后", 2700), ("1小时后", 3600));
config(CHECK, "低电提示", "当电压比较低的时候，会有提示音警告", LOWPOWER_WARNING_EN, 1);
config(LIST, "低电语音提示电压", "选择低电语音提示电压", LPWR_WARNING_VBAT, 10, "2.8V", "2.9V", "3.0V", "3.1V", "3.2V", "3.3V", "3.4V", "3.5V", "3.6V", "3.7V", 4, LOWPOWER_WARNING_EN);
config(LIST, "低电关机电压", "选择低电关机电压", LPWR_OFF_VBAT, 11, "不关机", "2.8V", "2.9V", "3.0V", "3.1V", "3.2V", "3.3V", "3.4V", "3.5V", "3.6V", "3.7V", 2, LOWPOWER_WARNING_EN);
config(BYTE, "低电语音播报周期(秒)", "设置低电语音播报周期(秒)", LPWR_WARNING_PERIOD, 1, 240, 30, LOWPOWER_WARNING_EN);
config(LIST, "OSC基础电容", "配置24M晶振OSCI与OSCO基础电容（6PF）", OSC_BOTH_CAP, 2, "0PF", "6PF", 0);
config(BYTE, "自定义OSCI电容", "配置24M晶振OSCI负载电容（0.25PF）。电容大小：n * 0.25PF + 基础电容", UOSCI_CAP, 0, 63, 45);
config(BYTE, "自定义OSCO电容", "配置24M晶振OSCO负载电容（0.25PF）。电容大小：n * 0.25PF + 基础电容", UOSCO_CAP, 0, 63, 45);
config(CHECK,  "优先使用产测电容值",  "是否优先使用产测校准的OSC电容值，没过产测或者关闭时使用<自定义OSCI电容>、<自定义OSCO电容>",  FT_OSC_CAP_EN,  1);

config(CHECK, "HUART调试(EQ/FCC)", "是否使用HUART调试功能", HUART_EN, 0);
config(LISTVAL, "HUART串口选择", "选择HUART的IO", HUART_SEL, BIT, 4, 0, 10, ("PA7", 0), ("PB2", 1), ("PB3", 2), ("PE7", 3), ("PF0", 4), ("PA6", 5),("PB1", 6),("PB4", 7), ("PE6", 8), ("PF1", 9), HUART_EN);

config(SUB, "DAC配置", "DAC的相关配置");
config(LISTVAL, "DAC声道选择", "选择DAC的输出方式, 最大输出功率: 32Ω 11.3mW", DAC_SEL, BIT, 4, 4, 4,  ("差分单声道", 4), ("差分双声道", 5), ("VCMBUF单声道", 2), ("VCMBUF双声道", 3));
config(LISTVAL, "DAC输出采样率", "DAC选择输出的采样率", DAC_SPR_SEL, BIT, 2, 0, 2, ("44.1KHz", 0), ("48KHz", 1));
config(CHECK, "DAC使能24bit", "DAC使能24bit，默认使用24bit", DAC_24BITS_EN, 1);
config(CHECK, "DAC扩展模式", "提高DAC输出功率，功耗会增加", DAC_MAXOUT_EN, 0);
config(CHECK, "DAC省VCM方案", "是否使用内部VCM，SOP16, SSOP24, SSOP28使用内部VCM", DAC_VCM_LESS_EN, 0);
config(LEVEL, 0x00);
config(CHECK, "蓝牙连接时关闭DAC", "蓝牙连接过程关闭DAC，仅对差分/VCMVUF输出有效", DAC_OFF_FOR_CONN, 0);
config(LEVEL, 0x03);
config(CHECK, "功放MUTE功能", "是否使能功放MUTE功能", SPK_MUTE_EN, 0);
config(LISTVAL, "功放MUTE控制IO选择", "功放MUTE控制GPIO选择", SPK_MUTE_IO_SEL, BIT, 5, 0, 23, ("None", 0), ("PA3", 4), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5/WKO", 14), ("PE0", 17), ("PE1", 18), ("PE2", 19), ("PE4", 21), ("PE5", 22), ("PE6", 23), ("PE7", 24), ("PF0", 25), ("PF1", 26), ("PF2", 27), ("PF3", 28), SPK_MUTE_EN);
config(CHECK, "高MUTE", "是否为高MUTE，否则为低MUTE", HIGH_MUTE, 1, SPK_MUTE_EN);
config(BYTE, "功放MUTE延时(单位5ms)", "功放MUTE的延时控制，防止解MUTE时间不够导致声音不全。", LOUDSPEAKER_UNMUTE_DELAY, 0, 255, 6, SPK_MUTE_EN);
config(LEVEL, 0x00);
config(LIST, "功放AB/D控制模式", "功放AB/D控制模式选择", AMPABD_TYPE, 2, "独立IO电平控制", "mute脉冲控制", 0);
config(LISTVAL, "功放AB/D控制IO选择", "功放AB/D控制GPIO选择", AMPABD_IO_SEL, BIT, 5, 0, 23, ("None", 0), ("PA3", 4), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5/WKO", 14), ("PE0", 17), ("PE1", 18), ("PE2", 19), ("PE4", 21), ("PE5", 22), ("PE6", 23), ("PE7", 24), ("PF0", 25), ("PF1", 26), ("PF2", 27), ("PF3", 28));
config(LISTVAL, "耳机检测IO选择", "选择耳机检测IO选择", EARPHONE_DET_IOSEL, BIT, 6, 0, 26, ("None", 0), ("PA3", 4), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5/WKO", 14), ("PE0", 17), ("PE1", 18), ("PE2", 19), ("PE4", 21), ("PE5", 22), ("PE6", 23), ("PE7", 24), ("PF0", 25), ("PF1", 26), ("PF2", 27), ("PF3", 28), ("复用SDCLK检测", 60), ("复用SDCMD检测", 61), ("复用PWRKEY检测", 62));
config(LEVEL, 0x03);

config(SUB, "音量配置", "音量的相关配置");
config(LIST, "音乐衰减增益", "配置系统最大增益基础上衰减，自动调整音量表。", DAC_MAX_GAIN, 10, "0DB", "-1DB", "-2DB", "-3DB", "-4DB", "-5DB", "-6DB", "-7DB", "-8DB", "-9DB", 3);
config(LIST, "通话衰减增益", "配置通话时最大增益基础上衰减，自动调整音量表。", BT_CALL_MAX_GAIN, 10, "0DB", "-1DB", "-2DB", "-3DB", "-4DB", "-5DB", "-6DB", "-7DB", "-8DB", "-9DB", 1);
config(LIST, "音量级数", "选择系统音量级数", VOL_MAX, 2, "16级音量", "32级音量", 1);
config(BYTE, "开机默认音量", "开机默认音量级数", SYS_INIT_VOL, 0, 50, 19);
config(BYTE, "恢复默认音量阈值", "小于多少级音量后恢复成系统音量阈值", SYS_RECOVER_INITVOL, 0, 50, 5);
config(BYTE, "提示音播放音量", "提示音播放音量级数", WARNING_VOLUME, 0, 50, 21);

config(SUB, "电源配置", "电源的相关配置");
config(CHECK, "BUCK MODE", "是否设置为BUCK MODE", BUCK_MODE_EN, 0);
config(CHECK, "双BUCK模式", "是否打开VDDCORE BUCK", VDDCORE_BUCK_EN, 1, BUCK_MODE_EN);
config(CHECK, "VDDBT省电容", "是否VDDBT省电容，只能LDO模式才能省电容", VDDBT_CAPLESS_EN, 0, BUCK_MODE_EN);
config(LISTVAL, "VDDIO电压", "选择VDDIO电压, VDDAUD与VDDIO短接可能需要调VDDIO电压。", VDDIO_SEL, BIT, 4, 7, 9, ("None", 0), ("2.9V", 5), ("3.0V", 6), ("3.1V", 7), ("3.2V", 8), ("3.3V", 9), ("3.4V", 10), ("3.5V", 11), ("3.6V", 12));

config(SUB, "充电配置", "充电功能的相关配置");
config(CHECK, "充电使能", "是否打开充电功能", CHARGE_EN, 1);
config(CHECK, "涓流充电使能", "是否使能涓流充电", CHARGE_TRICK_EN, 1, CHARGE_EN);
config(CHECK, "插入DC禁止软开机", "PWRKEY软开机，DC Online时，禁止软开机", CHARGE_DC_NOT_PWRON, 1, CHARGE_EN);
config(CHECK, "充电电压跟随模式", "是否打开充电电压跟随模式，需支持快充的仓", CHARGE_VOLTAGE_FOLLOW, 0, CHARGE_EN);
config(LIST, "充电截止电流", "选择充电截止电流", CHARGE_STOP_CURR, 16, "0mA", "2.5mA", "5mA", "7.5mA", "10mA", "12.5mA", "15mA", "17.5mA", "20mA", "22.5mA", "25mA", "27.5mA", "30mA", "32.5mA", "35mA", "37.5mA", 4, CHARGE_EN);
config(LIST, "恒流充电电流", "恒流充电（电池电压大于2.9v）电流", CHARGE_CONSTANT_CURR, 33, "5mA", "10mA", "15mA", "20mA", "25mA", "30mA", "35mA", "40mA", "45mA", "50mA", "55mA", "60mA", "65mA", "70mA", "75mA", "80mA", "90mA", "100mA", "110mA", "120mA", "130mA", "140mA", "150mA", "160mA", "170mA", "180mA", "190mA", "200mA", "210mA", "220mA", "230mA", "240mA", "250mA", 7, CHARGE_EN);
config(LIST, "涓流充电电流", "涓流充电（电池电压小于2.9v）电流", CHARGE_TRICKLE_CURR, 8, "5mA", "10mA", "15mA", "20mA", "25mA", "30mA", "35mA", "40mA", 3, CHARGE_TRICK_EN);
config(LISTVAL, "充满电蓝灯亮", "设置充满电蓝灯亮时间", CHARGE_FULL_BLED, BIT, 3, 3, 8,  ("不亮蓝灯", 0), ("亮10秒", 1), ("亮20秒", 2), ("亮30秒", 3), ("1分钟", 4), ("2分钟", 5), ("3分钟", 6), ("常亮", 7), CHARGE_EN);
config(CHECK, "充满电自动关机", "电池充满自动关机功能，主要用于放入电池仓充电的选项", CH_FULL_AUTO_PWRDWN_EN, 1, CHARGE_EN);
config(CHECK, "充电仓功能", "是否使能充电仓功能", CHBOX_EN, 1, CHARGE_EN);
config(LISTVAL, "充电仓类型选择", "选择正确类型充电仓，确保充满关机和拿起开机功能正常", CH_BOX_TYPE_SEL, BIT, 2, 0, 4, ("兼容5V短暂掉0V后维持电压", 0), ("5V不掉电的充电仓", 1), ("5V掉电但有维持电压", 2), ("5V完全掉电无维持电压", 3), CHBOX_EN);
config(LISTVAL, "仓允许最低维持电压", "inbox信号电压，即充满维持电压的充电仓允许的最低维持电压", CH_INBOX_SEL, BIT, 1, 0, 2, ("1.1V", 0), ("1.7V", 1), CHBOX_EN);
config(CHECK, "从充电仓拿出自动开机", "设备从充电仓拿出自动开机，注意充满后电池仓自动断电的不能开此选项", CH_OUT_AUTO_PWRON_EN, 1 CHBOX_EN);
config(LISTVAL, "出仓或入仓VUSB漏电配置", "加速设备出仓时自动开机或入仓时唤醒充电仓", CH_LEAKAGE_SEL, BIT, 2, 2, 4, ("None", 0), ("漏电电流X1档", 1), ("漏电电流X2档", 2), ("漏电电流X3档", 3), CHARGE_EN);
config(CHECK, "入仓设备关机", "检测到入仓状态后, 关机并重启进入充电状态", CHG_INBOX_PWRDWN_EN, 1, CHBOX_EN);
config(BIT, "短暂掉0V的仓稳定检测时间", "短暂掉0V的仓稳定检测时间: 500ms + n*100ms", CHBOX_OUT_DELAY,  4, 0, 15, 0, CHBOX_EN);


config(LEVEL, 0x03);
config(SUB, "无线配置", "无线麦的相关配置");
config(TEXT, "设备名称", "无线设备的配对名称", WS_NAME, 32, "LE-MIC2");
config(MAC, "设备地址", "无线设备的MAC地址", BT_ADDR, 6, 41:42:00:00:00:00, 41:42:FF:FF:FF:FF, 41:42:00:00:00:01);
config(LEVEL, 0x03);

config(SUB, "RF参数", "射频RF参数");
config(S8, "降低预置RF参数发射功率", "微调<预置RF参数>或<FT的RF参数>的发射功率，每级可降低0.5dbm发射功率（负数则增加功率）", BT_RF_PWRDEC, -6, 32, 0);
config(CHECK,  "优先使用FT的RF参数",  "是否优先使用FT校准的RF参数，没过FT或关闭时使用<预置RF参数>或<自定义RF参数>",  FT_RF_PARAM_EN,  1);
config(CHECK, "自定义RF参数", "打开后，有FT的芯片优先使用<FT的RF参数>，否则使用<自定义RF参数>", BT_RF_PARAM_EN, 0);
config(LEVEL, 0x03);
config(BYTE, "GL_PA_GAIN",    "PA_GAIN, 参考值3",         RF_PA_GAIN,    0,  7,  7, BT_RF_PARAM_EN);
config(LEVEL, 0x03);
config(BYTE, "GL_MIX_GAIN",   "MIX_GAIN, 参考值3~5",      RF_MIX_GAIN,   0,  7,  6, BT_RF_PARAM_EN);
config(BYTE, "GL_DIG_GAIN",   "DIG_GAIN, 参考值30~59",    RF_DIG_GAIN,   24, 59, 55, BT_RF_PARAM_EN);
config(LEVEL, 0x00);
config(BYTE, "GL_PA_CAP",     "PA_CAP, 参考值12~14",      RF_PA_CAP,     0, 15, 4, BT_RF_PARAM_EN);
config(BYTE, "GL_MIX_CAP",    "MIX_CAP, 参考值7~9",       RF_MIX_CAP,    0, 15,  8, BT_RF_PARAM_EN);
config(BYTE, "GL_TX_DBM",     "Cable实测dbm值",           RF_TXDBM,      0, 12,  8, BT_RF_PARAM_EN);
config(BYTE, "GL_UDF0",       "UDF(保留使用)",            RF_UDF0,       0,  0,  0, BT_RF_PARAM_EN);
config(BYTE, "GL_UDF1",       "UDF(保留使用)",            RF_UDF1,       0,  0,  0, BT_RF_PARAM_EN);
config(LEVEL, 0x03);
config(LISTVAL, "降低回连功率", "是否降低回连手机或TWS的功率", BT_RF_PAGE_PWRDEC, BIT, 3, 1, 4, ("不降低", 0), ("降低3dbm", 1), ("降低6dbm", 2), ("降低9dbm", 3));
config(LISTVAL, "降低组队功率", "是否降低TWS搜索组队的功率", BLE_RF_PAGE_PWRDEC, BIT, 3, 0, 4, ("不降低", 0), ("降低3dbm", 1), ("降低6dbm", 2), ("降低9dbm", 3));
config(BYTE, "设置组队范围RSSI(-dbm)", "该值越小(-90dbm最小),组队范围约大", BLE_PAGE_RSSI_THR, 20, 90, 90);
config(LEVEL, 0x03);


config(SUB, "MIC参数", "蓝牙通话和ANC的MIC参数");
config(LISTVAL, "MIC插入检测配置", "选择MIC检测GPIO，或Disable MIC检测", MIC_DET_IOSEL, BIT, 6, 0, 23, ("None", 0), ("PA3", 4), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5", 14), ("PE0", 16), ("PE5", 21), ("PE6", 22), ("PE7", 23), ("PF0", 24), ("PF1", 25), ("PF2", 26), ("PF3", 27), ("复用SDCLK检测", 28), ("复用SDCMD检测", 29), ("复用PWRKEY检测", 30));
config(LISTVAL, "无线主MIC配置", "选择主MIC，注意配置对应MIC通路参数", BT_MMIC_CFG, BIT, 3, 4, 6,  ("None", 5), ("MIC0", 0), ("MIC1", 1), ("MIC2", 2), ("MIC3", 3), ("MIC4", 4), WIRELESS_DEVICE_EN);
config(LEVEL, 0x03);
config(LISTVAL, "MIC供电IO电压选择", "MIC供电IO电压选择", MIC_PWR_LEVEL, BIT, 3, 6, 8, ("1.8v", 0), ("2.4v", 1), ("2.5v", 2), ("2.6v", 3), ("2.7v", 4), ("2.8v", 5), ("2.9v", 6), ("3.0v", 7));

config(CHECK, "MIC参数配置", "是否打开MIC", MIC_EN, 0);
config(LISTVAL, "供电IO选择", "选择对应IO给MIC0供电", MIC_PWR_SEL, BIT, 2, 0, 4, ("None", 0), ("PF0", 1), ("PF1", 2),  ("PE7(MIC1~MIC4)", 3), MIC_EN);
config(LISTVAL, "MIC偏置电路配置", "MIC0电路BIAS配置，省电容，省电阻配置", MIC_BIAS_METHOD, BIT, 1, 1, 2, ("单端MIC外部电阻电容", 0), ("差分MIC", 1), MIC_EN);
config(BYTE, "MIC模拟增益", "MIC0模拟增益配置(3~42DB), Step 3DB", MIC_ANL_GAIN, 0, 13, 8, MIC_EN);
config(BYTE, "MIC通话数字增益", "通话MIC0数字增益配置(0~39DB), Step 1DB", BT_MIC_DIG_GAIN, 0, 39, 20, MIC_EN);

config(SUB, "AUX配置", "AUX模式的相关配置");
config(CHECK, "AUX输入功能", "是否需要AUX功能", ADAPTER_AUX_EN, 0);
config(LISTVAL, "AUX模拟增益", "AUX模拟增益选择", AUX_ANL_GAIN, BIT, 4, 3, 8, ("-12DB", 0), ("-9DB", 1), ("-6DB", 2), ("-3DB", 3), ("0DB", 4), ("+6DB", 5), ("+9DB", 6), ("+12DB", 7), ADAPTER_AUX_EN);
config(BIT, "AUX数字增益(0~39DB)", "SDADC数字增益, Step 1DB", AUX_DIG_GAIN, 6, 0, 39, 0, ADAPTER_AUX_EN);
config(LISTVAL, "AUXL通路选择", "AUX左声道通路选择,选择MIC输入增益会不同", AUXL_SEL, BIT, 3, 0, 5, ("无AUXL输入", 0), ("AUXL0_PE6", 1), ("AUXL1_PA6", 2), ("AUXL_MIC2", 3), ("AUXL_MIC3", 4), ADAPTER_AUX_EN);
config(LISTVAL, "AUXR通路选择", "AUX右声道通路选择,选择MIC输入增益会不同", AUXR_SEL, BIT, 3, 0, 5, ("无AUXR输入", 0), ("AUXR0_PE7", 1), ("AUXR1_PA7", 2), ("AUXR_MIC3", 3), ("AUXR_MIC2", 4), ADAPTER_AUX_EN);

config(SUB, "按键配置", "选择按键及按键功能配置");
config(LISTVAL, "软开机长按时间选择", "长按PWRKEY多长时间后软开机", PWRON_PRESS_TIME,  BIT, 3, 3, 8, ("0.1秒", 0), ("0.5秒", 1), ("1秒", 2), ("1.5秒", 3), ("2秒", 4), ("2.5秒", 5), ("3秒", 6), ("3.5秒", 7));
config(LISTVAL, "软关机长按时间选择", "长按PWRKEY多长时间后软关机", PWROFF_PRESS_TIME, BIT, 3, 3, 8, ("1.5秒", 0), ("2秒", 1), ("2.5秒", 2), ("3秒", 3), ("3.5秒", 4), ("4秒", 5), ("4.5秒", 6), ("5秒", 7));
config(LISTVAL, "双击响应时间选择", "选择双击按键响应间隔时间", DOUBLE_KEY_TIME, BIT, 3, 1, 8, ("200ms", 0), ("300ms", 1), ("400ms", 2), ("500ms", 3), ("600ms", 4), ("700ms", 5), ("800ms", 6), ("900ms", 7));
config(CHECK, "IOKEY按键定制", "是否配置IOKEY，不勾选使用SDK默认按键配置", IOKEY_CONFIG_EN, 0);
config(LISTVAL, "IOKEY按键1的IO", "IOKEY第1个按键的GPIO口选择", IOKEY_IO0, BIT, 5, 0, 18, ("None", 0), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5", 14), ("PE0", 17), ("PE4", 21), ("PE5", 22), ("PE6", 23), ("PE7", 24), ("PF0", 25), ("PF1", 26), IOKEY_CONFIG_EN);
config(LISTVAL, "IOKEY按键2的IO", "IOKEY第2个按键的GPIO口选择", IOKEY_IO1, BIT, 5, 0, 18, ("None", 0), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5", 14), ("PE0", 17), ("PE4", 21), ("PE5", 22), ("PE6", 23), ("PE7", 24), ("PF0", 25), ("PF1", 26), IOKEY_CONFIG_EN);
config(LISTVAL, "IOKEY按键3的IO", "IOKEY第3个按键的GPIO口选择", IOKEY_IO2, BIT, 5, 0, 18, ("None", 0), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5", 14), ("PE0", 17), ("PE4", 21), ("PE5", 22), ("PE6", 23), ("PE7", 24), ("PF0", 25), ("PF1", 26), IOKEY_CONFIG_EN);
config(LISTVAL, "IOKEY按键4的IO", "IOKEY第4个按键的GPIO口选择", IOKEY_IO3, BIT, 5, 0, 18, ("None", 0), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5", 14), ("PE0", 17), ("PE4", 21), ("PE5", 22), ("PE6", 23), ("PE7", 24), ("PF0", 25), ("PF1", 26), IOKEY_CONFIG_EN);
config(LISTVAL, "IOKEY按键5的IO", "IOKEY第5个按键的GPIO口选择", IOKEY_IO4, BIT, 5, 0, 18, ("None", 0), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5", 14), ("PE0", 17), ("PE4", 21), ("PE5", 22), ("PE6", 23), ("PE7", 24), ("PF0", 25), ("PF1", 26), IOKEY_CONFIG_EN);

config(SUB, "LED灯配置", "选择及配置LED状态指示灯");
config(CHECK, "系统指示灯(蓝灯)", "是否使用系统状态指示灯", LED_DISP_EN, 1);
config(CHECK, "电源状态灯(红灯)", "是否使用充电/电源指示灯", LED_PWR_EN, 1);
config(CHECK, "省电阻1个IO推两个灯", "省电阻1个IO推两个灯，不能兼容升级功能", PORT_2LED_RESLESS_EN, 1, LED_DISP_EN);
config(CHECK, "电池低电闪灯", "电池低电时，红灯或蓝灯是否闪烁？", RLED_LOWBAT_EN, 0, LED_DISP_EN);
config(LISTVAL, "蓝灯IO选择", "蓝灯的GPIO口选择", BLED_IO_SEL, BIT, 5, 25, 18, ("None", 0), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5", 14), ("PE0", 17), ("PE4", 21), ("PE5", 22), ("PE6", 23), ("PE7", 24), ("PF0", 25), ("PF1", 26), LED_DISP_EN);
config(LISTVAL, "红灯IO选择", "红灯的GPIO口选择", RLED_IO_SEL, BIT, 5, 25, 18, ("None", 0), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5", 14), ("PE0", 17), ("PE4", 21), ("PE5", 22), ("PE6", 23), ("PE7", 24), ("PF0", 25), ("PF1", 26), LED_PWR_EN);

config(CHECK, "开机状态配置LED", "是否配置开机LED指示灯状态", LED_PWRON_CONFIG_EN, 0, LED_DISP_EN);
config(LED, "开机", "配置开机状态下的闪灯状态", LED_POWERON, 0x02, 0x01, 10, 255, 		LED_PWRON_CONFIG_EN);

config(CHECK, "蓝牙初始状态配置LED", "是否配置蓝牙初始状态指示灯", LED_BTPAIR_CONFIG_EN, 0, LED_DISP_EN);
config(LED, "初始", "配置初始状态下的闪灯状态", LED_PAIR, 0x00, 0x02, 2, 86, 	LED_BTPAIR_CONFIG_EN);

config(CHECK, "蓝牙已连接状态配置LED", "是否配置蓝牙已连接状态指示灯", LED_BTCONN_CONFIG_EN, 0, LED_DISP_EN);
config(LED, "已连接", "配置已连接状态下的闪灯状态", LED_CONNECTED, 0x00, 0xff, 1, 0, 	LED_BTCONN_CONFIG_EN);

config(CHECK, "MUTE状态配置LED", "是否配置MUTE状态指示灯", LED_MUTE_CONFIG_EN, 0, LED_DISP_EN);
config(LED, "MUTE", "配置MUTE状态下的闪灯状态", LED_MUTE, 0x00, 0xff, 1, 0, 	LED_MUTE_CONFIG_EN);

config(CHECK, "低电状态配置LED", "是否配置低电状态指示灯", LED_LOWBAT_CONFIG_EN, 0, RLED_LOWBAT_EN);
config(LED, "低电", "配置低电状态下的闪灯状态", LED_LOWBAT, 0xaa, 0x00, 6, 0, 			LED_LOWBAT_CONFIG_EN);

config(SUB, "保持参数", "保持后不被擦出的区域");
config(KEEP);
config(CHECK, "无线适配器", "是否打开无线接收/发射适配器功能", WIRELESS_ADAPTER_EN, 0);
config(CHECK, "无线设备端", "是否打开无线MIC/无线耳机设备功能", WIRELESS_DEVICE_EN, 0);

config(LEVEL, 0x0E);
config(SUB, "产测参数", "产测校准的一些参数，不要改动！");
config(BYTE, "产测OSCI电容", "产测校准的24M晶振OSCI负载电容，单位0.25PF。不要改动默认值0。", OSCI_CAP, 0, 63, 0);
config(BYTE, "产测OSCO电容", "产测校准的24M晶振OSCO负载电容，单位0.25PF。不要改动默认值0。", OSCO_CAP, 0, 63, 0);


config(LEVEL, 0x100);
makecfgfile(xcfg.bin);
makecfgdef(xcfg.h);
xcopy(xcfg.h, ../../xcfg.h);
