#include "led.h"

//STM32F103���İ�����
//�⺯���汾����
/************** Ƕ��ʽ������  **************/
/********** mcudev.taobao.com ��Ʒ  ********/

//////////////////////////////////////////////////////////////////////////////////	 
//STM32������
//LED��������	   
								  
////////////////////////////////////////////////////////////////////////////////// 	   

//��ʼ��PA8Ϊ�����.��ʹ��ʱ��		    
//LED IO��ʼ��
void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //ʹ��PA�˿�ʱ��
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				      //LED0-->PA8 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);					       //�����趨������ʼ��PA8
 GPIO_SetBits(GPIOA,GPIO_Pin_8);						           //PA8 �����
}
// void LED_Init1(void)
//{
// 
//	GPIO_InitTypeDef  GPIO_InitStructure;

//	
//	  /* GPIOD Periph clock enable */
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//???PA???
//  /* Configure PD0 and PD2 in output pushpull mode */
//  //GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
//	GPIO_InitStructure.GPIO_Pin = ((uint16_t)0x01F2);//Configure PA1 and PA8 in output pushpull mode
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//  GPIO_Init(GPIOA, &GPIO_InitStructure);
//	//GPIO_ResetBits(GPIOA,GPIO_Pin_8);
//	GPIO_SetBits(GPIOA,GPIO_Pin_1);
//	//GPIO_SetBits(GPIOA,GPIO_Pin_2);
//	//GPIO_SetBits(GPIOA,GPIO_Pin_3);
//	GPIO_SetBits(GPIOA,GPIO_Pin_4);
//	GPIO_SetBits(GPIOA,GPIO_Pin_5);
//	GPIO_SetBits(GPIOA,GPIO_Pin_6);
//	GPIO_SetBits(GPIOA,GPIO_Pin_7);
//	GPIO_SetBits(GPIOA,GPIO_Pin_8);
//}
