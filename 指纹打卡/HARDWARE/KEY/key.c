#include "stm32f10x.h"
#include "key.h"
#include "sys.h" 
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK精英STM32开发板
//按键驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////  
								    
//按键初始化函数
void KEY_Init(void) //IO初始化
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能PORTA,PORTE时钟
	//初始化 WK_UP-->GPIOA.0	  下拉输入
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0设置成输入，默认下拉	  
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.0

}
//按键处理函数
//返回按键值
//mode:0,不支持连续按;1,支持连续按;
//0，没有任何按键按下
//1，KEY0按下
//2，KEY1按下
//3，KEY3按下 WK_UP
//注意此函数有响应优先级,KEY0>KEY1>KEY_UP!!
u8 KEY_Scan(u8 mode)
{	 
	static u8 key_up=1;//按键按松开标志
	if(mode)key_up=1;  //支持连按		  
	if(key_up&&(KEY1==0||KEY2==0||KEY11==0||KEY12==0||KEY13==0||KEY14==0))
	{
		delay_ms(10);//去抖动 
		key_up=0;
		if(KEY1==0)return KEY1_PRES;
		else if(KEY2==0)return KEY2_PRES;
		else if(KEY11==0)return KEY11_PRES;
		else if(KEY12==0)return KEY12_PRES;
		else if(KEY13==0)return KEY13_PRES;
		else if(KEY14==0)return KEY14_PRES;
	}else if(KEY1==1&&KEY2==1&&KEY11==1&&KEY12==1&&KEY13==1&&KEY14==1)key_up=1; 	    
 	return 0;// 无按键按下
}


//		switch(key)
//		{
//			case KEY1_PRES : sptr->Integral=sptr->Integral+0.1;break;
//			case KEY2_PRES : sptr->Integral=sptr->Integral-0.1;break;
//			case KEY11_PRES : dacval_num=dacval_num+1;break;
//			case KEY12_PRES : dacval_num=dacval_num-1;break;
//			case KEY13_PRES : sptr->Proportion=sptr->Proportion+0.1;break;    //pid.Kp
//			case KEY14_PRES : sptr->Proportion=sptr->Proportion-0.1;break;
//			case KEY13_PRES : pid.Kp=pid.Kp+0.1;break;    //pid.Kp
//			case KEY14_PRES : pid.Kp=pid.Kp-0.1;break;
//			default:
//				break;
//		}
