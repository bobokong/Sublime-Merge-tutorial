#ifndef __ADC_H
#define __ADC_H	
#include "sys.h"

//STM32F103核心板例程
//库函数版本例程
/************** 嵌入式开发网  **************/
/********** mcudev.taobao.com  ********/

//ADC 代码	   
						  
////////////////////////////////////////////////////////////////////////////////// 

void Adc_Init(void);
u16  Get_Adc(u8 ch); 
u16 Get_Adc_Average(u8 ch,u8 times); 
extern float temp;
#endif 
