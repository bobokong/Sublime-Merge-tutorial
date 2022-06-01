#ifndef __LED_H
#define __LED_H	 
#include "sys.h"


//STM32F103核心板例程
//库函数版本例程
/************** 嵌入式开发网  **************/
/********** mcudev.taobao.com 出品  ********/


//////////////////////////////////////////////////////////////////////////////////	 

//TM32开发板
//LED驱动代码	   
					  
////////////////////////////////////////////////////////////////////////////////// 
#define LED PAout(8)// PA8
#define LED5 PAout(5)// PA5
#define LED6 PAout(6)// PA6
#define LED7 PAout(7)// PA7
void LED_Init(void);//初始化
//void LED_Init1(void);//初始化
		 				    
#endif
