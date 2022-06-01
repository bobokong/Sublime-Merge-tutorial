#include "gpio.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "timer.h"
#include "math.h"
#include "uart_tft_bsp.h"
#include "AS608.h"
#include <string.h>
#include <stdio.h>
#include "ff.h"
#include "exfuns.h" 
#include "adc.h"
#include "GUI.h"
#include "Lcd_Driver.h"
#include "exti.h"
u8 model_flag=0;
char sendEnd[]={0xff,0xff,0xff};
extern SysPara AS608Para;//ָ��ģ��AS608����
uint8_t DIS_Buffer[64];//��ʾ�ַ���λ�뻺����
u16 adcx;
float temp;
int main(void)
{		 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	delay_init();	    	 //��ʱ������ʼ��	
	KEY_Init();			  //��ʼ����������
	LED_Init1();			     //LED�˿ڳ�ʼ��
	EXTIX_Init();		 	//�ⲿ�жϳ�ʼ��
	//uart1_init(9600);	
	uart2_init(9600);
	uart3_init(57600);
	PS_StaGPIO_Init();	//��ʼ��FR��״̬����
	Adc_Init();		  		//ADC��ʼ��
//	Lcd_Init();	 //1.44��Һ����--��ʼ������
//	Lcd_Clear(GRAY0);//����
//	Gui_DrawFont_GBK16(0,80,RED,GRAY0,"   һ������     ");
//  Gui_DrawFont_GBK16(0,96,BLUE,GRAY0,"  ��������  ");
	while(PS_HandShake(&AS608Addr))//��AS608ģ������
	{
		delay_ms(400);		  
	}
	while(1)
	{
//		sprintf(DIS_Buffer,"%06.3f V",(float)TFT_Command1);             //��ʽ���ַ���
//		Gui_DrawFont_GBK16(0+64,96,BLUE,GRAY0,DIS_Buffer);//��������
//		sprintf(DIS_Buffer,"%06.3f V",(float)TFT_Command);             //��ʽ���ַ���
//		Gui_DrawFont_GBK16(0+64,80,BLUE,GRAY0,DIS_Buffer);//һ������
		switch(TFT_Command)
		{
			case 0:
				press_FR();//ˢָ��	
			break;
			case 1:
				printf("add.t0.txt=\"ָ�����\"");
				printf(sendEnd);
				if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)==1)
				{
					LED5=0;
					LED6=1;
					LED7=1;
					Add_FR();
					LED5=1;
					LED6=1;
					TFT_Command=0;
					printf("add.t0.txt=\"ָ����ӳɹ�\"");
					printf(sendEnd);
				}
				break;
			case 3://�л�ģʽ����λָ��
				LED5=1;
				LED6=1;
				LED7=1;
				TFT_Command=0;
				model_flag=!model_flag;
			break;
			case 4:
				ledSwitch();
				ledRefresh();
			break;
			default:break;
		}
		if(model_flag)
		{
			ledAuto();
			ledRefresh();
		}
	}
}



