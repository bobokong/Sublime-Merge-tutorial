
#ifndef uart_tft_bsp___H
#define uart_tft_bsp___H

#include "sys.h"

#define USART3_MAX_RECV_LEN		400					//最大接收缓存字节数
#define USART3_MAX_SEND_LEN		400					//最大发送缓存字节数
#define USART3_RX_EN 			1					//0,不接收;1,接收.

extern u8  USART3_RX_BUF[USART3_MAX_RECV_LEN]; 		//接收缓冲,最大USART3_MAX_RECV_LEN字节
extern u8  USART3_TX_BUF[USART3_MAX_SEND_LEN]; 		//发送缓冲,最大USART3_MAX_SEND_LEN字节
extern vu16 USART3_RX_STA;   						//接收数据状态

void uart1_init(u32 bound);				//串口2初始化 
void uart2_init(u32 bound);
void uart3_init(u32 bound);
void u3_printf(char* fmt,...);

#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART2_RX 			1		//使能（1）/禁止（0）串口1接收
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收
	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA;         		//接收状态标记	
//如果想串口中断接收，请不要注释以下宏定义
void uart_init(u32 bound);

typedef struct
{

	char alterBuf[50];
	unsigned char alterCount;
	_Bool rev_idle;

} ALTER_INFO;

extern ALTER_INFO alterInfo;

typedef union Data
{
   int INT;
   uint8_t  str[4];
} DATA;//数据转换

extern DATA data;
void IntToBytes(int temp);
int bytesToInt(char *p);
void Usart_SendEnd(void);

#define TFT_CommandAddFP      1  /* 添加指纹 */
#define TFT_CommandDelFP      2  /* 删除指纹 */
#define TFT_CommandLedAuto	  3  /*自动调光*/
#define TFT_CommandLedSW      4  /*LED开关*/
#define TFT_CommandSetTime	  5

extern uint8_t TFT_Command;
extern uint8_t TFT_Command1;
/* 接口定义 */
#define UART_TFT_UART_Port    USART1

void UART_TFT_SendByte(uint8_t c);
void UART_TFT_SendString(uint8_t *p,uint16_t length);
extern void UART1_IRQHandler(void);

//初始化串口，不采用DMA模式
void UsartInit(USART_TypeDef *USARTx,unsigned int baud);

//发送字符串，调试功能时使用
void UsartSendString(USART_TypeDef *USARTx, char *str);

//将发送缓存中的数据添加校验和结束符，进行发送
void UsartSendOrder(USART_TypeDef *USARTx);

//发送固定长度的数据
void UsartSendBytes(USART_TypeDef *USARTx, uint8_t *pData, uint16_t Size);

//发送不定长度的数据
void UsartPrintf(USART_TypeDef *USARTx, char *fmt,...);


void HMI_SetPage(uint8_t Page);



#endif






