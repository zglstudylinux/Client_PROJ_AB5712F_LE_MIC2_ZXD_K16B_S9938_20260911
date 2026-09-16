#include "include.h"

#if 0

//-----user config---------------------------------------------------------
//DUMP_BUF 测试程序,使用高速串口DMA方式导出数据,以下程序在530X上测试OK
//系统时钟是26M时，HUART 1.5M会异常。实测试系统时钟120M时 HUART 1.5M输出正常

#define HUART_BAUD_SEL_6M   1     //1 huart 6M波特率  //0 则表示1.5M波特率

#if HUART_BAUD_SEL_6M
#define HUART_BAUD          8000000
#else
#define HUART_BAUD          1500000
#endif
#define HUART_BAUD_VAL      (((48000000 + (HUART_BAUD / 2)) / HUART_BAUD) - 1)


void huart_module_init(void)   //huart DMA输出测试  //测试发现huart暂时只支持26M时钟
{
    printf("huart_module_init, baud cfg = %d\n",HUART_BAUD_VAL);
    FUNCMCON0 = 0x0f << 20 | 0x0f << 16;
#if 1  //PA7
    FUNCMCON0 = 0x01 << 20 | 0x01 << 16;
    GPIOADE |= BIT(7);
    GPIOADIR &= ~BIT(7);
    GPIOAFEN |= BIT(7);
#else   //PB3
    FUNCMCON0 = 0x03 << 20 | 0x03 << 16;
    GPIOBDE |= BIT(3);
    GPIOBDIR &= ~BIT(3);
    GPIOBFEN |= BIT(3);
#endif

    CLKGAT0 |= BIT(9);   //9 huart_clk_en  //11
    CLKCON1 = (CLKCON1 & ~(0x07 << 16)) | (0<<18) | (0x01 << 16);

    HSUT0BAUD = (HUART_BAUD_VAL << 16) | HUART_BAUD_VAL;
    HSUT0CON = 0;
    HSUT0CON |= BIT(9);        //BIT9 = 1, 2 bit stop bit  //0 : 1BIT   //打开2bit stop 不容易传输出错
    HSUT0CON |= BIT(1);       //TX EN
    HSUT0CON |= BIT(7);       //TX DMA MODE
    HSUT0CON |= BIT(17) | BIT(16);   //cfg sync
    HSUT0CPND = 0xffff;

    static u8 buf_first_kick[2] = {0x55,0xAA};  //kick for first Pending of DMA finish.
    HSUT0TXADR = (u32)buf_first_kick;
    HSUT0TXCNT = 2;
    printf("hsut0 init,HSUT0CON = 0x%X,HUART_BAUD_VAL = %d\n",HSUT0CON,HUART_BAUD_VAL);
    //printf("CLKCON0 = 0x%X, CLKCON2 = 0x%X\n",CLKCON0,CLKCON2);
}

AT(.com_text.dumpbuf_test)
void huart_wait_tx_finish(void)
{
    while(!(HSUT0CON&BIT(13))){
        my_printf("huart wait\n");
        WDT_CLR();
    }; //1 DMA finish  //0 dma not finish
}

AT(.com_text.dumpbuf_test)
void huart_putbuf(void *buf, u32 len)
{
    if (0 == len) {
        return;
    }
    huart_wait_tx_finish();     //等待上一次发送完成
    HSUT0TXADR = DMA_ADR(buf);
    HSUT0TXCNT = len;           //kick start auto clear tx pending
}

////测试 huart_putbuf 函数能否发送成功, dump_buf_init初始化前需要确认发送函数是否OK.
//void dumphuart_io_test(void)
//{
//    huart_module_init();
//    WDT_DIS();
//    //u8 dma_test_buf[] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
//    u8 dma_test_buf[] = {0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55};
//    while(1) {
//        huart_putbuf(dma_test_buf,8);
//        delay_5ms(10);
//        printf("->");
//    }
//}

////dump使用示例
//file_total 最大为6, 支持同时保存生成6个文件
//u8 dump_head_buf[14*6];   //管理文件头需要的buf
//void dump_buf_huart_init(void)
//{
//    printf("------> %s\n",__func__);
//    huart_module_init();    //初始化高速串口模块
//    dump_buf_init(dump_head_buf,6,huart_putbuf,huart_wait_tx_finish);  //初始化dump模块
//}

//
//#if 1
//u8 xdrc_dump_head_buf[14*2];   //管理包头需要的buf,uartDump工具目前支持最大同时保存生成6个文件,这里AUX只用到了1路
////static u8 xdrc_dump_buf[2][14+512*2 + 14+512] AT(.huart_dump);  //file0: LP+RP 双声道   //file1 MIX
//#endif
//
//void xdrc_huart_dump_init(void)
//{
//#if 1
//    printf("------> %s\n",__func__);
//    huart_module_init();    //初始化高速串口模块
//    dump_buf_init(xdrc_dump_head_buf,2, huart_putbuf, huart_wait_tx_finish);  //初始化dump模块
//#endif
//
//}
#endif  // HUART_DUMP_BUF_EN

//-------------------------------------------------
//#if 0  //TEST_HUART_DUMP 测试代码
////循环发送正弦波数组进行测试
//static u8 sinetbl_test[128] = {   //32 samples  //sine_16k_500hz_dual
//	0x00, 0x00, 0x00, 0x00, 0x84, 0x0C, 0x84, 0x0C, 0x8C, 0x18, 0x8C, 0x18, 0xA4, 0x23, 0xA4, 0x23,
//	0x5C, 0x2D, 0x5C, 0x2D, 0x56, 0x35, 0x56, 0x35, 0x44, 0x3B, 0x44, 0x3B, 0xEB, 0x3E, 0xEB, 0x3E,
//	0x26, 0x40, 0x26, 0x40, 0xEB, 0x3E, 0xEB, 0x3E, 0x43, 0x3B, 0x43, 0x3B, 0x57, 0x35, 0x57, 0x35,
//	0x5D, 0x2D, 0x5D, 0x2D, 0xA4, 0x23, 0xA4, 0x23, 0x8D, 0x18, 0x8D, 0x18, 0x83, 0x0C, 0x83, 0x0C,
//	0x00, 0x00, 0x00, 0x00, 0x7C, 0xF3, 0x7C, 0xF3, 0x74, 0xE7, 0x74, 0xE7, 0x5D, 0xDC, 0x5D, 0xDC,
//	0xA4, 0xD2, 0xA4, 0xD2, 0xA9, 0xCA, 0xA9, 0xCA, 0xBC, 0xC4, 0xBC, 0xC4, 0x16, 0xC1, 0x16, 0xC1,
//	0xD9, 0xBF, 0xD9, 0xBF, 0x15, 0xC1, 0x15, 0xC1, 0xBC, 0xC4, 0xBC, 0xC4, 0xA8, 0xCA, 0xA8, 0xCA,
//	0xA4, 0xD2, 0xA4, 0xD2, 0x5C, 0xDC, 0x5C, 0xDC, 0x74, 0xE7, 0x74, 0xE7, 0x7B, 0xF3, 0x7B, 0xF3
//};
//
//u8 dump_dma_buf[14+128];  //DMA需要的BUF
//void dump_buf_huart_test(void)
//{
//    //dumphuart_io_test();  //测试高速串口DMA发送是否正常
//	dump_buf_huart_init();
//    printf("dump run...\n");
//    int cnt =1024*3; //65536;//65536;  //65536*128 = 8M (8388608 byte)
//    WDT_DIS();
//
//#if 1
//    while(cnt--) {  //同时导出6个文件测试
//        dump_putbuf(dump_dma_buf,sinetbl_test,128,0);  //导出的数据会保存到文件0
//        dump_putbuf(dump_dma_buf,sinetbl_test,128,1);  //导出的数据会保存到文件1
//        dump_putbuf(dump_dma_buf,sinetbl_test,128,2);  //导出的数据会保存到文件2
//        dump_putbuf(dump_dma_buf,sinetbl_test,128,3);  //导出的数据会保存到文件3
//        dump_putbuf(dump_dma_buf,sinetbl_test,128,4);  //导出的数据会保存到文件4
//        dump_putbuf(dump_dma_buf,sinetbl_test,128,5);  //导出的数据会保存到文件5
//        //delay_5ms(10);
//        if(cnt % 1024 == 0) {
//            printf(".");
//        }
//    }
//#else
// while(cnt--) {  //同时导出6个文件测试(先dma到ram的方式)
//        dump_dma_wait();
//        dump_put2ram(dump_dma_buf,sinetbl_test,128,0); //文件0数据先准备到ram中待发送
//        dump_dma_kick(dump_dma_buf,14+128);            //KICK数据发送
//
//        dump_dma_wait();
//        dump_put2ram(dump_dma_buf,sinetbl_test,128,1); //文件1数据先准备到ram中待发送
//        dump_dma_kick(dump_dma_buf,14+128);            //KICK数据发送
//
//        dump_dma_wait();
//        dump_put2ram(dump_dma_buf,sinetbl_test,128,2); //文件2数据先准备到ram中待发送
//        dump_dma_kick(dump_dma_buf,14+128);            //KICK数据发送
//
//        dump_dma_wait();
//        dump_put2ram(dump_dma_buf,sinetbl_test,128,3); //文件3数据先准备到ram中待发送
//        dump_dma_kick(dump_dma_buf,14+128);            //KICK数据发送
//
//        dump_dma_wait();
//        dump_put2ram(dump_dma_buf,sinetbl_test,128,4); //文件4数据先准备到ram中待发送
//        dump_dma_kick(dump_dma_buf,14+128);            //KICK数据发送
//
//        dump_dma_wait();
//        dump_put2ram(dump_dma_buf,sinetbl_test,128,5); //文件5数据先准备到ram中待发送
//        dump_dma_kick(dump_dma_buf,14+128);            //KICK数据发送
////        delay_5ms(100);
//        if(cnt % 1024 == 0) {
//            printf(".");
//        }
//    }
//#endif
//    printf("dump_buf_test finish\n"); while(1);
//}
//#endif  //TEST_HUART_DUMP



































