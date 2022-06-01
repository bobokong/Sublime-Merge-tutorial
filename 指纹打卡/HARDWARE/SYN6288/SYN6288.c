#include "SYN6288.h"
#include "stdio.h"

void Voice_BusyPinInitize(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//读摸出感应状态(触摸感应时输出高电平信号)
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能PORTA,PORTE时钟
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_8;//GPIO_Pin_1KEY0-KEY1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;//输入下拉模式
 	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA8
}

void Voice_play(u8 *puts)		//播报语音
{	
	char str[700];
	uint16_t i=5,j;
	
	str[0] = 0xFD;
	str[3] = 0x01;
	str[4] = 0x01;	
    for (; *puts != 0;	puts++)  
	{
		str[i]=*puts;
		i++;//执行完多加了一次
	}
	str[1] = (i-2)/256;
	str[2] = (i-2)%256;
	str[i]=0;
	for(j=0;j<i;j++)
	{
		str[i] ^= str[j];
	}
	for(j=0;j<i+1;j++)
	{
		USART_SendData(USART1,str[j]);
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);		//等待发送完成
	}
}











