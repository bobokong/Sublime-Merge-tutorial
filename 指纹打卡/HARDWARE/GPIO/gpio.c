#include "gpio.h"
#include "adc.h"

#include "stdbool.h"
#include "uart_tft_bsp.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK ��ӢSTM32������
//LED��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/2
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 
extern char sendEnd[3];
extern vu8 ledFlag;
extern int8_t n0,n1,n2,n3,n4,n5,n6;

//��ʼ��PB5��PE5Ϊ�����.��ʹ���������ڵ�ʱ��		    
//LED IO��ʼ��
void LED_Init1(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure;

	
	  /* GPIOD Periph clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//ʹ��PAʱ��
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
//			temp=(float)adcx*(3.3/4096);//��ѹֵ��С��2V����
	temp=(float)Get_Adc_Average(ADC_Channel_1,10)*(3.3/4096);//��ѹֵ��С��2V����
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
				LED5=!LED5;//0Ϊ��
				TFT_Command1=0;
		break;
		case 2:
				LED6=!LED6;//0Ϊ��
				TFT_Command1=0;
		break;
		case 3:
				LED7=!LED7;//0Ϊ��
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

 
