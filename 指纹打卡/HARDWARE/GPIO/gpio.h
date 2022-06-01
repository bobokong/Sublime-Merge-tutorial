#ifndef __GPIO_H
#define __GPIO_H	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK精英STM32开发板
//LED驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/2
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 
#define LED1 PBout(12)// PB5
#define LED2 PBout(13)// PE5
#define LED3 PBout(14)
#define LED4 PBout(15)
#define LED5 PAout(5)
#define LED6 PAout(6)
#define LED7 PAout(7)
#define LED8 PAout(8)
void LED_Init1(void);//初始化
void ledSwitch(void);
void ledAuto(void);
void ledRefresh(void);


		 				    
#endif
