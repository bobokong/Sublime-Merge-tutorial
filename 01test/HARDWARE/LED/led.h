#ifndef __LED_H
#define __LED_H	 
#include "sys.h"


//STM32F103���İ�����
//�⺯���汾����
/************** Ƕ��ʽ������  **************/
/********** mcudev.taobao.com ��Ʒ  ********/


//////////////////////////////////////////////////////////////////////////////////	 

//TM32������
//LED��������	   
					  
////////////////////////////////////////////////////////////////////////////////// 
#define LED PAout(8)// PA8
#define LED5 PAout(5)// PA5
#define LED6 PAout(6)// PA6
#define LED7 PAout(7)// PA7
void LED_Init(void);//��ʼ��
//void LED_Init1(void);//��ʼ��
		 				    
#endif
