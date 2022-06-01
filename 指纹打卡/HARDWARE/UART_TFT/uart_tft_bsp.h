
#ifndef uart_tft_bsp___H
#define uart_tft_bsp___H

#include "sys.h"

#define USART3_MAX_RECV_LEN		400					//�����ջ����ֽ���
#define USART3_MAX_SEND_LEN		400					//����ͻ����ֽ���
#define USART3_RX_EN 			1					//0,������;1,����.

extern u8  USART3_RX_BUF[USART3_MAX_RECV_LEN]; 		//���ջ���,���USART3_MAX_RECV_LEN�ֽ�
extern u8  USART3_TX_BUF[USART3_MAX_SEND_LEN]; 		//���ͻ���,���USART3_MAX_SEND_LEN�ֽ�
extern vu16 USART3_RX_STA;   						//��������״̬

void uart1_init(u32 bound);				//����2��ʼ�� 
void uart2_init(u32 bound);
void uart3_init(u32 bound);
void u3_printf(char* fmt,...);

#define USART_REC_LEN  			200  	//�����������ֽ��� 200
#define EN_USART2_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����
#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����
	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART_RX_STA;         		//����״̬���	
//����봮���жϽ��գ��벻Ҫע�����º궨��
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
} DATA;//����ת��

extern DATA data;
void IntToBytes(int temp);
int bytesToInt(char *p);
void Usart_SendEnd(void);

#define TFT_CommandAddFP      1  /* ���ָ�� */
#define TFT_CommandDelFP      2  /* ɾ��ָ�� */
#define TFT_CommandLedAuto	  3  /*�Զ�����*/
#define TFT_CommandLedSW      4  /*LED����*/
#define TFT_CommandSetTime	  5

extern uint8_t TFT_Command;
extern uint8_t TFT_Command1;
/* �ӿڶ��� */
#define UART_TFT_UART_Port    USART1

void UART_TFT_SendByte(uint8_t c);
void UART_TFT_SendString(uint8_t *p,uint16_t length);
extern void UART1_IRQHandler(void);

//��ʼ�����ڣ�������DMAģʽ
void UsartInit(USART_TypeDef *USARTx,unsigned int baud);

//�����ַ��������Թ���ʱʹ��
void UsartSendString(USART_TypeDef *USARTx, char *str);

//�����ͻ����е��������У��ͽ����������з���
void UsartSendOrder(USART_TypeDef *USARTx);

//���͹̶����ȵ�����
void UsartSendBytes(USART_TypeDef *USARTx, uint8_t *pData, uint16_t Size);

//���Ͳ������ȵ�����
void UsartPrintf(USART_TypeDef *USARTx, char *fmt,...);


void HMI_SetPage(uint8_t Page);



#endif






