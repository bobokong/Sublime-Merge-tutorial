#ifndef __GPIO_H
#define __GPIO_H	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK��ӢSTM32������
//LED��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/2
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
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
void LED_Init1(void);//��ʼ��
void ledSwitch(void);
void ledAuto(void);
void ledRefresh(void);


		 				    
#endif
