#include "stm32f10x.h"
#include "key.h"
#include "sys.h" 
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK��ӢSTM32������
//������������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/3
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////  
								    
//������ʼ������
void KEY_Init(void) //IO��ʼ��
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//ʹ��PORTA,PORTEʱ��
	//��ʼ�� WK_UP-->GPIOA.0	  ��������
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0���ó����룬Ĭ������	  
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.0

}
//����������
//���ذ���ֵ
//mode:0,��֧��������;1,֧��������;
//0��û���κΰ�������
//1��KEY0����
//2��KEY1����
//3��KEY3���� WK_UP
//ע��˺�������Ӧ���ȼ�,KEY0>KEY1>KEY_UP!!
u8 KEY_Scan(u8 mode)
{	 
	static u8 key_up=1;//�������ɿ���־
	if(mode)key_up=1;  //֧������		  
	if(key_up&&(KEY1==0||KEY2==0||KEY11==0||KEY12==0||KEY13==0||KEY14==0))
	{
		delay_ms(10);//ȥ���� 
		key_up=0;
		if(KEY1==0)return KEY1_PRES;
		else if(KEY2==0)return KEY2_PRES;
		else if(KEY11==0)return KEY11_PRES;
		else if(KEY12==0)return KEY12_PRES;
		else if(KEY13==0)return KEY13_PRES;
		else if(KEY14==0)return KEY14_PRES;
	}else if(KEY1==1&&KEY2==1&&KEY11==1&&KEY12==1&&KEY13==1&&KEY14==1)key_up=1; 	    
 	return 0;// �ް�������
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
