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
extern SysPara AS608Para;//指纹模块AS608参数
uint8_t DIS_Buffer[64];//显示字符区位码缓冲区
u16 adcx;
float temp;
int main(void)
{		 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	delay_init();	    	 //延时函数初始化	
	KEY_Init();			  //初始化按键程序
	LED_Init1();			     //LED端口初始化
	EXTIX_Init();		 	//外部中断初始化
	//uart1_init(9600);	
	uart2_init(9600);
	uart3_init(57600);
	PS_StaGPIO_Init();	//初始化FR读状态引脚
	Adc_Init();		  		//ADC初始化
//	Lcd_Init();	 //1.44寸液晶屏--初始化配置
//	Lcd_Clear(GRAY0);//清屏
//	Gui_DrawFont_GBK16(0,80,RED,GRAY0,"   一级命令     ");
//  Gui_DrawFont_GBK16(0,96,BLUE,GRAY0,"  二级命令  ");
	while(PS_HandShake(&AS608Addr))//与AS608模块握手
	{
		delay_ms(400);		  
	}
	while(1)
	{
//		sprintf(DIS_Buffer,"%06.3f V",(float)TFT_Command1);             //格式化字符串
//		Gui_DrawFont_GBK16(0+64,96,BLUE,GRAY0,DIS_Buffer);//二级命令
//		sprintf(DIS_Buffer,"%06.3f V",(float)TFT_Command);             //格式化字符串
//		Gui_DrawFont_GBK16(0+64,80,BLUE,GRAY0,DIS_Buffer);//一级命令
		switch(TFT_Command)
		{
			case 0:
				press_FR();//刷指纹	
			break;
			case 1:
				printf("add.t0.txt=\"指纹添加\"");
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
					printf("add.t0.txt=\"指纹添加成功\"");
					printf(sendEnd);
				}
				break;
			case 3://切换模式，复位指令
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



