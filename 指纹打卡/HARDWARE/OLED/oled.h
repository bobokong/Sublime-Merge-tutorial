
#ifndef __OLED_H
#define __OLED_H			  	 
#include "sys.h"
#include "stdlib.h"	    	

#define SCL_0() GPIO_ResetBits(GPIOD,GPIO_Pin_6)//CLK
#define SCL_1() GPIO_SetBits(GPIOD,GPIO_Pin_6)

#define SDA_0() GPIO_ResetBits(GPIOD,GPIO_Pin_7)//DIN
#define SDA_1() GPIO_SetBits(GPIOD,GPIO_Pin_7)

void OLED_GPIO_Init(void);
void Initial_LY096BG30(void);
//void picture_1(int Y,int K,int S,int L);
void picture_2(void);
void IIC_Start(void);
void IIC_Stop(void);
void Write_IIC_Command(unsigned char IIC_Command);
void Write_IIC_Data(unsigned char IIC_Data);
void Write_IIC_Byte(unsigned char IIC_Byte);
#endif  
	 
