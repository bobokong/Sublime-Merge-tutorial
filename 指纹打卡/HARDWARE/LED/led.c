#include "led.h"

//STM32F103核心板例程
//库函数版本例程
/************** 嵌入式开发网  **************/
/********** mcudev.taobao.com 出品  ********/

//////////////////////////////////////////////////////////////////////////////////	 
//STM32开发板
//LED驱动代码	   
								  
////////////////////////////////////////////////////////////////////////////////// 	   

//初始化PA8为输出口.并使能时钟		    
//LED IO初始化
void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //使能PA端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				      //LED0-->PA8 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);					       //根据设定参数初始化PA8
 GPIO_SetBits(GPIOA,GPIO_Pin_8);						           //PA8 输出高
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
