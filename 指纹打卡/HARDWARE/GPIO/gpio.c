#include "gpio.h"
#include "adc.h"

#include "stdbool.h"
#include "uart_tft_bsp.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK 精英STM32开发板
//LED驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/2
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 
extern char sendEnd[3];
extern vu8 ledFlag;
extern int8_t n0,n1,n2,n3,n4,n5,n6;

//初始化PB5和PE5为输出口.并使能这两个口的时钟		    
//LED IO初始化
void LED_Init1(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure;

	
	  /* GPIOD Periph clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//使能PA时钟
  /* Configure PD0 and PD2 in output pushpull mode */
  //GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Pin = ((uint16_t)0x01F2);//Configure PA1 and PA8 in output pushpull mode
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	//GPIO_ResetBits(GPIOA,GPIO_Pin_8);
	GPIO_SetBits(GPIOA,GPIO_Pin_1);
	//GPIO_SetBits(GPIOA,GPIO_Pin_2);
	//GPIO_SetBits(GPIOA,GPIO_Pin_3);
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
	GPIO_SetBits(GPIOA,GPIO_Pin_5);
	GPIO_SetBits(GPIOA,GPIO_Pin_6);
	GPIO_SetBits(GPIOA,GPIO_Pin_7);
	GPIO_SetBits(GPIOA,GPIO_Pin_8);
}

void ledAuto(void)
{
//	adcx=Get_Adc_Average(ADC_Channel_1,10);
//			temp=(float)adcx*(3.3/4096);//电压值，小于2V开灯
	temp=(float)Get_Adc_Average(ADC_Channel_1,10)*(3.3/4096);//电压值，小于2V开灯
		if(temp<2)
		{
			LED5=0;
			LED6=0;
			LED7=0;
		}
		else
		{
			LED5=1;
			LED6=1;
			LED7=1;
		}
}

void ledSwitch(void)
{
	switch (TFT_Command1)
	{
		case 1:
				LED5=!LED5;//0为开
				TFT_Command1=0;
		break;
		case 2:
				LED6=!LED6;//0为开
				TFT_Command1=0;
		break;
		case 3:
				LED7=!LED7;//0为开
				TFT_Command1=0;
		break;
		default:break;
	}
}

void ledRefresh(void)
{
	if(LED5)
	{
		printf("led.p0.pic=4");
		printf(sendEnd);
	}
	else
	{
		printf("led.p0.pic=5");
		printf(sendEnd);
	}
	if(LED6)
	{
		printf("led.p1.pic=4");
		printf(sendEnd);
	}
	else
	{
		printf("led.p1.pic=5");
		printf(sendEnd);
	}
	if(LED7)
	{
		printf("led.p2.pic=4");
		printf(sendEnd);
	}
	else
	{
		printf("led.p2.pic=5");
		printf(sendEnd);
	}
}

 
