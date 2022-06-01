#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "uart_tft_bsp.h"
#include "timer.h"
#include "delay.h"
#include "gpio.h"
uint8_t TFT_Command = 0;  /* 为0时表示没有命令 */
uint8_t TFT_Command1 = 0;
unsigned char TFT_Command_data[50];
//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART2->SR&0X40)==0);//循环发送,直到发送完毕   
    USART2->DR = (u8) ch;      
	return ch;
}
#endif 
  

/************************************************************************************
@函数名称: UART_TFT_SendByte
@函数功能: 通过指定串口向UART_TFT发送一个字节的数据
@参    数: c -- 要发送的字节内容
@返    回: None
@作    者: veiko
@公    司: 艺嵌科技
@时    间: 2015-7-16
************************************************************************************/
void UART_TFT_SendByte(uint8_t c)
{
		USART_SendData(USART1,c);
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);		//等待发送完成
}

/************************************************************************************
@函数名称: UART_TFT_SendString
@函数功能: 通过指定串口向UART_TFT发送一个指定长度的字符串
@参    数: p -- 指向字符数组的指针
*          length -- 要发送的字节数
@返    回: None
@作    者: veiko
@公    司: 艺嵌科技
@时    间: 2015-7-16
************************************************************************************/
void UART_TFT_SendString(uint8_t *p,uint16_t length)
{
	uint16_t i;
	for(i=0;i<length;i++)
	{
		UART_TFT_SendByte(*(p+i));
	}
}

/******************************************************************************************************************
应用层
与HMI设备相关的函数
***************************/

/************************************************************************************
@函数名称: HMI_SetPage
@函数功能: 设置HMI设备显示指定的页
@参    数: Page -- 页编号(0-9)
*          length -- 要发送的字节数
@返    回: None
@作    者: veiko
@公    司: 艺嵌科技
@时    间: 2015-7-16
************************************************************************************/
void HMI_SetPage(uint8_t Page)
{
	uint8_t arraySend[]= "page 5";
	
	arraySend[5] = Page + 0x30;
	
	UART_TFT_SendByte(0xff);
	UART_TFT_SendByte(0xff);
	UART_TFT_SendByte(0xff);
	UART_TFT_SendString(arraySend,6);
	UART_TFT_SendByte(0xff);
	UART_TFT_SendByte(0xff);
	UART_TFT_SendByte(0xff);
}

u8 USART3_RX_BUF[USART3_MAX_RECV_LEN]; 				//接收缓冲,最大USART3_MAX_RECV_LEN个字节.
u8 USART3_TX_BUF[USART3_MAX_SEND_LEN]; 			  //发送缓冲,最大USART3_MAX_SEND_LEN字节
vu16 USART3_RX_STA=0;   	


void USART3_IRQHandler(void)
{
	u8 res;	      
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)//接收到数据
	{	 
		res =USART_ReceiveData(USART3);		 
		if((USART3_RX_STA&(1<<15))==0)//接收完的一批数据,还没有被处理,则不再接收其他数据
		{ 
			if(USART3_RX_STA<USART3_MAX_RECV_LEN)	//还可以接收数据
			{
				TIM_SetCounter(TIM7,0);//计数器清空          				//计数器清空
				if(USART3_RX_STA==0) 				//使能定时器7的中断 
				{
					TIM_Cmd(TIM7,ENABLE);//使能定时器7
				}
				USART3_RX_BUF[USART3_RX_STA++]=res;	//记录接收到的值	 
			}else 
			{
				USART3_RX_STA|=1<<15;				//强制标记接收完成
			} 
		}
	}  				 											 
}   


//初始化IO 串口2
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率	  
void uart3_init(u32 bound)
{  

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	// GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE); //串口2时钟使能

 	USART_DeInit(USART3);  //复位串口2
		 //USART3_TX   PB10
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PA2
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOB, &GPIO_InitStructure); //初始化PA2
   
    //USART3_RX	  PB11
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOB, &GPIO_InitStructure);  //初始化PA3
	
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  
	USART_Init(USART3, &USART_InitStructure); //初始化串口2
  

	USART_Cmd(USART3, ENABLE);                    //使能串口 
	
	//使能接收中断
  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启中断   
	
	//设置中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
	
	TIM7_Int_Init(99,7199);		//10ms中断
	USART3_RX_STA=0;		//清零
	TIM_Cmd(TIM7,DISABLE);			//关闭定时器7
}

//串口2,printf 函数
//确保一次发送数据不超过USART3_MAX_SEND_LEN字节
void u3_printf(char* fmt,...)  
{  
	u16 i,j; 
	va_list ap; 
	va_start(ap,fmt);
	vsprintf((char*)USART3_TX_BUF,fmt,ap);
	va_end(ap);
	i=strlen((const char*)USART3_TX_BUF);		//此次发送数据的长度
	for(j=0;j<i;j++)							//循环发送数据
	{
	  while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕   
		USART_SendData(USART3,USART3_TX_BUF[j]); 
	} 
}

//初始化串口，不采用DMA模式
void UsartInit(USART_TypeDef *USARTx,unsigned int baud)
{
	GPIO_InitTypeDef gpioInitStruct;
	USART_InitTypeDef usartInitStruct;
	NVIC_InitTypeDef nvicInitStruct;
	if(USARTx == USART1)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);    //开启GPIO时钟
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);   //开启USART1时钟
		//PA9	TXD
		gpioInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;   //发送引脚设置为推挽复用
		gpioInitStruct.GPIO_Pin = GPIO_Pin_9;         
		gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &gpioInitStruct);
		
		//PA10	RXD
		gpioInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //设置接收引脚为浮空输入
		gpioInitStruct.GPIO_Pin = GPIO_Pin_10;
		gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &gpioInitStruct);
	}
	else if(USARTx == USART2)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
		//PA2	TXD
		gpioInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
		gpioInitStruct.GPIO_Pin = GPIO_Pin_2;
		gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &gpioInitStruct);
		
		//PA3	RXD
		gpioInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		gpioInitStruct.GPIO_Pin = GPIO_Pin_3;
		gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &gpioInitStruct);
	}
	else //if(USARTx == USART3)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
		//PB10	TXD
		gpioInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
		gpioInitStruct.GPIO_Pin = GPIO_Pin_10;
		gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &gpioInitStruct);
		
		//PB11	RXD
		gpioInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		gpioInitStruct.GPIO_Pin = GPIO_Pin_11;
		gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &gpioInitStruct);
	}
	
	if(USARTx == USART1)
	{
		nvicInitStruct.NVIC_IRQChannel = USART1_IRQn;
		//USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);			//使能IDLE中断
		nvicInitStruct.NVIC_IRQChannelPreemptionPriority = 2;
		nvicInitStruct.NVIC_IRQChannelSubPriority = 3;
		nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&nvicInitStruct);
		
		usartInitStruct.USART_BaudRate = baud;
		usartInitStruct.USART_WordLength = USART_WordLength_8b;							          //8位数据位
		usartInitStruct.USART_StopBits = USART_StopBits_1;								            //1位停止位
		usartInitStruct.USART_Parity = USART_Parity_No;									              //无校验
		usartInitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		//无硬件流控
		usartInitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;						        //接收和发送
		
		USART_Init(USART1, &usartInitStruct);
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);				//使能接收中断
		USART_Cmd(USART1, ENABLE);									//使能串口
	}
	else if(USARTx == USART2)
	{
		nvicInitStruct.NVIC_IRQChannel = USART2_IRQn;
		//USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);			//使能IDLE中断
		nvicInitStruct.NVIC_IRQChannelPreemptionPriority = 1;
		nvicInitStruct.NVIC_IRQChannelSubPriority = 3;
		nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&nvicInitStruct);
		
		usartInitStruct.USART_BaudRate = baud;
		usartInitStruct.USART_WordLength = USART_WordLength_8b;							          //8位数据位
		usartInitStruct.USART_StopBits = USART_StopBits_1;								            //1位停止位
		usartInitStruct.USART_Parity = USART_Parity_No;									              //无校验
		usartInitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		//无硬件流控
		usartInitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;						        //接收和发送
		
		USART_Init(USART2, &usartInitStruct);
		USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);				//使能接收中断
		USART_Cmd(USART2, ENABLE);									//使能串口
	}
	else if(USARTx == USART3)
	{		
		nvicInitStruct.NVIC_IRQChannel = USART3_IRQn;
		//USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);			//使能IDLE中断
		nvicInitStruct.NVIC_IRQChannelPreemptionPriority = 2;
		nvicInitStruct.NVIC_IRQChannelSubPriority = 1;
		nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&nvicInitStruct);
		
		usartInitStruct.USART_BaudRate = baud;
		usartInitStruct.USART_WordLength = USART_WordLength_8b;							          //8位数据位
		usartInitStruct.USART_StopBits = USART_StopBits_1;								            //1位停止位
		usartInitStruct.USART_Parity = USART_Parity_No;									              //无校验
		usartInitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		//无硬件流控
		usartInitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;						        //接收和发送
		
		USART_Init(USART3, &usartInitStruct);
		USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);				//使能接收中断
		USART_Cmd(USART3, ENABLE);									//使能串口
	}
	
}

/**
 *@breief	格式化打印
 *@param	USARTx：串口号
			fmt：不定长参
 *@reval	None 
 *@notice	None
 */
void UsartPrintf(USART_TypeDef *USARTx, char *fmt,...)
{

	va_list ap;
	unsigned char UsartPrintfBuf[128];
	unsigned char *pStr = UsartPrintfBuf;
	
	va_start(ap, fmt);
	vsprintf((char *)UsartPrintfBuf, fmt, ap);							//格式化
	va_end(ap);

	while(*pStr != 0)
	{
		USART_SendData(USARTx, *pStr++);
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
	}

}

//发送字符串，可用于显示调试信息
void UsartSendString(USART_TypeDef *USARTx, char *str)
{
	unsigned short count = 0;
	unsigned short len = strlen(str);
	for(; count < len; count++)
	{
		USART_SendData(USARTx, *str++);									              //发送数据
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);		//等待发送完成
	}
}

//发送固定长度的数据
void UsartSendBytes(USART_TypeDef *USARTx, uint8_t *pData, uint16_t Size)
{
	uint16_t i;
	for(i=0;i<Size;i++)
	{
		USART_SendData(USARTx,pData[i]);
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);		//等待发送完成
	}
}

void Usart_SendEnd(void)
{
	uint8_t i;
	
	for(i=0;i<3;i++)
	{
		USART_SendData(USART2,0xff);
		while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);		//等待发送完成
	}
}


#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	  
  
void uart1_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9
   
  //USART1_RX	  GPIOA.10初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

  //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART1, &USART_InitStructure); //初始化串口1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_Cmd(USART1, ENABLE);                    //使能串口1 

}
void uart2_init(u32 bound){
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	// GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE); //串口2时钟使能

 	USART_DeInit(USART2);  //复位串口2
		 //USART2_TX   PA2
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA2
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA2
   
    //USART2_RX	  PA3
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA3
	
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  
	USART_Init(USART2, &USART_InitStructure); //初始化串口2
  

	USART_Cmd(USART2, ENABLE);                    //使能串口 
	
	//使能接收中断
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断   
	
	//设置中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器 

}

void USART1_IRQHandler(void)                	//串口1中断服务程序
	{
	u8 Res;
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
		{
		Res =USART_ReceiveData(USART1);	//读取接收到的数据
		
		if((USART_RX_STA&0x8000)==0)//接收未完成
			{
			if(USART_RX_STA&0x4000)//接收到了0x0d
				{
				if(Res!=0x0a)USART_RX_STA=0;//接收错误,重新开始
				else USART_RX_STA|=0x8000;	//接收完成了 
				}
				else //还没收到0X0D
				{	
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
					{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
					}		 
				}
			}   		 
     } 
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
} 
#endif	


ALTER_INFO alterInfo;
_Bool isCanReceiveFlag=0;
//串口2接收中断响应函数
void USART2_IRQHandler(void)
{
	unsigned char data = 0,dataLen=0;
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)		//接收中断
	{			
        data = USART_ReceiveData(USART2);
		if(data == 0xFD && alterInfo.rev_idle == 0)
		{
			//接收到数据帧的起始字符0xFD，打开接收开关
			isCanReceiveFlag = 1;
		}
		if(isCanReceiveFlag == 1)
		{
			alterInfo.alterBuf[alterInfo.alterCount] = data;
			alterInfo.alterCount++;
			if(alterInfo.alterCount>4)
			{
				TFT_Command = alterInfo.alterBuf[1];
				TFT_Command1 = alterInfo.alterBuf[3];
				dataLen = alterInfo.alterBuf[2] + 3;//数据总长度，不包括数据尾
			isCanReceiveFlag =0;
				alterInfo.alterCount=0;
//						USART2->SR;	
//			USART2->DR;
//				USART_ClearFlag(USART2, USART_IT_IDLE);
			}
		}
		USART_ClearFlag(USART2, USART_FLAG_RXNE);
	}
}
//void USART2_IRQHandler(void)
//{
//	unsigned char data = 0,dataLen=0;
//	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)		//接收中断
//	{			
//        data = USART_ReceiveData(USART2);
//		if(data == 0xFD && alterInfo.rev_idle == 0)
//		{
//			//接收到数据帧的起始字符0xFD，打开接收开关
//			isCanReceiveFlag = 1;
//		}
//		if(isCanReceiveFlag == 1)
//		{
//			alterInfo.alterBuf[alterInfo.alterCount] = data;
//			alterInfo.alterCount++;
//			if(alterInfo.alterCount>3)
//			{
//				TFT_Command = alterInfo.alterBuf[1];
//				dataLen = alterInfo.alterBuf[2] + 3;//数据总长度，不包括数据尾
//				if((alterInfo.alterCount == (dataLen+1)) && (alterInfo.alterBuf[dataLen] == 0xFF))
//				{
//					alterInfo.rev_idle = 1;
//					isCanReceiveFlag = 0;
//					alterInfo.alterCount=0;
//				}
//				else if(alterInfo.alterCount>(dataLen+1))
//				{
//					alterInfo.rev_idle = 0;
//					isCanReceiveFlag = 0;
//					alterInfo.alterCount=0;
//				}
//				else if(alterInfo.alterCount>48)//防止数组溢出
//				{
//					alterInfo.rev_idle = 0;
//					isCanReceiveFlag = 0;
//					alterInfo.alterCount=0;
//				}
//			}
//		}
//		USART_ClearFlag(USART2, USART_FLAG_RXNE);
//	}
//	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
//	{	
////		alterInfo.rev_idle = 1;
////		alterInfo.alterCount = 0;		
//		USART2->SR;	
//		USART2->DR;																//读取数据注意：这句必须要，否则不能够清除中断标志位
//		USART_ClearFlag(USART2, USART_IT_IDLE);
//	}
//}

//串口3接收中断响应函数
//void USART3_IRQHandler(void)
//{
//	if(USART_GetFlagStatus(USART3, USART_FLAG_RXNE) != RESET)		//接收中断
//	{
//		USART_ClearITPendingBit(USART3,USART_IT_RXNE);            //清除串口中断
//	}
//}

DATA data;//数据转换
//int转为4字节
void IntToBytes(int temp)
{
//	char i;
	
	data.INT = temp;
//	for(i=0;i<4;i++)
//	{
//		sd_data.SdBuff[sd_data.SdBuffLen+i]=data.str[i];
//	}
//	sd_data.SdBuffLen += 4;
}

int bytesToInt(char *p)
{
	char i;
	for(i=0;i<4;i++)
	{
		data.str[i] = p[i];
	}
	
	return data.INT;
}








