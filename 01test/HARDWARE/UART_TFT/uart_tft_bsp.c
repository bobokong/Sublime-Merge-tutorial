#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "uart_tft_bsp.h"
#include "timer.h"
#include "delay.h"
#include "gpio.h"
uint8_t TFT_Command = 0;  /* Ϊ0ʱ��ʾû������ */
uint8_t TFT_Command1 = 0;
unsigned char TFT_Command_data[50];
//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART2->SR&0X40)==0);//ѭ������,ֱ���������   
    USART2->DR = (u8) ch;      
	return ch;
}
#endif 
  

/************************************************************************************
@��������: UART_TFT_SendByte
@��������: ͨ��ָ��������UART_TFT����һ���ֽڵ�����
@��    ��: c -- Ҫ���͵��ֽ�����
@��    ��: None
@��    ��: veiko
@��    ˾: ��Ƕ�Ƽ�
@ʱ    ��: 2015-7-16
************************************************************************************/
void UART_TFT_SendByte(uint8_t c)
{
		USART_SendData(USART1,c);
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);		//�ȴ��������
}

/************************************************************************************
@��������: UART_TFT_SendString
@��������: ͨ��ָ��������UART_TFT����һ��ָ�����ȵ��ַ���
@��    ��: p -- ָ���ַ������ָ��
*          length -- Ҫ���͵��ֽ���
@��    ��: None
@��    ��: veiko
@��    ˾: ��Ƕ�Ƽ�
@ʱ    ��: 2015-7-16
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
Ӧ�ò�
��HMI�豸��صĺ���
***************************/

/************************************************************************************
@��������: HMI_SetPage
@��������: ����HMI�豸��ʾָ����ҳ
@��    ��: Page -- ҳ���(0-9)
*          length -- Ҫ���͵��ֽ���
@��    ��: None
@��    ��: veiko
@��    ˾: ��Ƕ�Ƽ�
@ʱ    ��: 2015-7-16
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

u8 USART3_RX_BUF[USART3_MAX_RECV_LEN]; 				//���ջ���,���USART3_MAX_RECV_LEN���ֽ�.
u8 USART3_TX_BUF[USART3_MAX_SEND_LEN]; 			  //���ͻ���,���USART3_MAX_SEND_LEN�ֽ�
vu16 USART3_RX_STA=0;   	


void USART3_IRQHandler(void)
{
	u8 res;	      
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)//���յ�����
	{	 
		res =USART_ReceiveData(USART3);		 
		if((USART3_RX_STA&(1<<15))==0)//�������һ������,��û�б�����,���ٽ�����������
		{ 
			if(USART3_RX_STA<USART3_MAX_RECV_LEN)	//�����Խ�������
			{
				TIM_SetCounter(TIM7,0);//���������          				//���������
				if(USART3_RX_STA==0) 				//ʹ�ܶ�ʱ��7���ж� 
				{
					TIM_Cmd(TIM7,ENABLE);//ʹ�ܶ�ʱ��7
				}
				USART3_RX_BUF[USART3_RX_STA++]=res;	//��¼���յ���ֵ	 
			}else 
			{
				USART3_RX_STA|=1<<15;				//ǿ�Ʊ�ǽ������
			} 
		}
	}  				 											 
}   


//��ʼ��IO ����2
//pclk1:PCLK1ʱ��Ƶ��(Mhz)
//bound:������	  
void uart3_init(u32 bound)
{  

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	// GPIOAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE); //����2ʱ��ʹ��

 	USART_DeInit(USART3);  //��λ����2
		 //USART3_TX   PB10
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PA2
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOB, &GPIO_InitStructure); //��ʼ��PA2
   
    //USART3_RX	  PB11
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOB, &GPIO_InitStructure);  //��ʼ��PA3
	
	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
  
	USART_Init(USART3, &USART_InitStructure); //��ʼ������2
  

	USART_Cmd(USART3, ENABLE);                    //ʹ�ܴ��� 
	
	//ʹ�ܽ����ж�
  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//�����ж�   
	
	//�����ж����ȼ�
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
	
	TIM7_Int_Init(99,7199);		//10ms�ж�
	USART3_RX_STA=0;		//����
	TIM_Cmd(TIM7,DISABLE);			//�رն�ʱ��7
}

//����2,printf ����
//ȷ��һ�η������ݲ�����USART3_MAX_SEND_LEN�ֽ�
void u3_printf(char* fmt,...)  
{  
	u16 i,j; 
	va_list ap; 
	va_start(ap,fmt);
	vsprintf((char*)USART3_TX_BUF,fmt,ap);
	va_end(ap);
	i=strlen((const char*)USART3_TX_BUF);		//�˴η������ݵĳ���
	for(j=0;j<i;j++)							//ѭ����������
	{
	  while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET); //ѭ������,ֱ���������   
		USART_SendData(USART3,USART3_TX_BUF[j]); 
	} 
}

//��ʼ�����ڣ�������DMAģʽ
void UsartInit(USART_TypeDef *USARTx,unsigned int baud)
{
	GPIO_InitTypeDef gpioInitStruct;
	USART_InitTypeDef usartInitStruct;
	NVIC_InitTypeDef nvicInitStruct;
	if(USARTx == USART1)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);    //����GPIOʱ��
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);   //����USART1ʱ��
		//PA9	TXD
		gpioInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;   //������������Ϊ���츴��
		gpioInitStruct.GPIO_Pin = GPIO_Pin_9;         
		gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &gpioInitStruct);
		
		//PA10	RXD
		gpioInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //���ý�������Ϊ��������
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
		//USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);			//ʹ��IDLE�ж�
		nvicInitStruct.NVIC_IRQChannelPreemptionPriority = 2;
		nvicInitStruct.NVIC_IRQChannelSubPriority = 3;
		nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&nvicInitStruct);
		
		usartInitStruct.USART_BaudRate = baud;
		usartInitStruct.USART_WordLength = USART_WordLength_8b;							          //8λ����λ
		usartInitStruct.USART_StopBits = USART_StopBits_1;								            //1λֹͣλ
		usartInitStruct.USART_Parity = USART_Parity_No;									              //��У��
		usartInitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		//��Ӳ������
		usartInitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;						        //���պͷ���
		
		USART_Init(USART1, &usartInitStruct);
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);				//ʹ�ܽ����ж�
		USART_Cmd(USART1, ENABLE);									//ʹ�ܴ���
	}
	else if(USARTx == USART2)
	{
		nvicInitStruct.NVIC_IRQChannel = USART2_IRQn;
		//USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);			//ʹ��IDLE�ж�
		nvicInitStruct.NVIC_IRQChannelPreemptionPriority = 1;
		nvicInitStruct.NVIC_IRQChannelSubPriority = 3;
		nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&nvicInitStruct);
		
		usartInitStruct.USART_BaudRate = baud;
		usartInitStruct.USART_WordLength = USART_WordLength_8b;							          //8λ����λ
		usartInitStruct.USART_StopBits = USART_StopBits_1;								            //1λֹͣλ
		usartInitStruct.USART_Parity = USART_Parity_No;									              //��У��
		usartInitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		//��Ӳ������
		usartInitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;						        //���պͷ���
		
		USART_Init(USART2, &usartInitStruct);
		USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);				//ʹ�ܽ����ж�
		USART_Cmd(USART2, ENABLE);									//ʹ�ܴ���
	}
	else if(USARTx == USART3)
	{		
		nvicInitStruct.NVIC_IRQChannel = USART3_IRQn;
		//USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);			//ʹ��IDLE�ж�
		nvicInitStruct.NVIC_IRQChannelPreemptionPriority = 2;
		nvicInitStruct.NVIC_IRQChannelSubPriority = 1;
		nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&nvicInitStruct);
		
		usartInitStruct.USART_BaudRate = baud;
		usartInitStruct.USART_WordLength = USART_WordLength_8b;							          //8λ����λ
		usartInitStruct.USART_StopBits = USART_StopBits_1;								            //1λֹͣλ
		usartInitStruct.USART_Parity = USART_Parity_No;									              //��У��
		usartInitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		//��Ӳ������
		usartInitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;						        //���պͷ���
		
		USART_Init(USART3, &usartInitStruct);
		USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);				//ʹ�ܽ����ж�
		USART_Cmd(USART3, ENABLE);									//ʹ�ܴ���
	}
	
}

/**
 *@breief	��ʽ����ӡ
 *@param	USARTx�����ں�
			fmt����������
 *@reval	None 
 *@notice	None
 */
void UsartPrintf(USART_TypeDef *USARTx, char *fmt,...)
{

	va_list ap;
	unsigned char UsartPrintfBuf[128];
	unsigned char *pStr = UsartPrintfBuf;
	
	va_start(ap, fmt);
	vsprintf((char *)UsartPrintfBuf, fmt, ap);							//��ʽ��
	va_end(ap);

	while(*pStr != 0)
	{
		USART_SendData(USARTx, *pStr++);
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
	}

}

//�����ַ�������������ʾ������Ϣ
void UsartSendString(USART_TypeDef *USARTx, char *str)
{
	unsigned short count = 0;
	unsigned short len = strlen(str);
	for(; count < len; count++)
	{
		USART_SendData(USARTx, *str++);									              //��������
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);		//�ȴ��������
	}
}

//���͹̶����ȵ�����
void UsartSendBytes(USART_TypeDef *USARTx, uint8_t *pData, uint16_t Size)
{
	uint16_t i;
	for(i=0;i<Size;i++)
	{
		USART_SendData(USARTx,pData[i]);
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);		//�ȴ��������
	}
}

void Usart_SendEnd(void)
{
	uint8_t i;
	
	for(i=0;i<3;i++)
	{
		USART_SendData(USART2,0xff);
		while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);		//�ȴ��������
	}
}


#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���	  
  
void uart1_init(u32 bound){
  //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.9
   
  //USART1_RX	  GPIOA.10��ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.10  

  //Usart1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(USART1, &USART_InitStructure); //��ʼ������1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
  USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���1 

}
void uart2_init(u32 bound){
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	// GPIOAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE); //����2ʱ��ʹ��

 	USART_DeInit(USART2);  //��λ����2
		 //USART2_TX   PA2
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA2
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA2
   
    //USART2_RX	  PA3
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA3
	
	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
  
	USART_Init(USART2, &USART_InitStructure); //��ʼ������2
  

	USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ��� 
	
	//ʹ�ܽ����ж�
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�����ж�   
	
	//�����ж����ȼ�
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ��� 

}

void USART1_IRQHandler(void)                	//����1�жϷ������
	{
	u8 Res;
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
		{
		Res =USART_ReceiveData(USART1);	//��ȡ���յ�������
		
		if((USART_RX_STA&0x8000)==0)//����δ���
			{
			if(USART_RX_STA&0x4000)//���յ���0x0d
				{
				if(Res!=0x0a)USART_RX_STA=0;//���մ���,���¿�ʼ
				else USART_RX_STA|=0x8000;	//��������� 
				}
				else //��û�յ�0X0D
				{	
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
					{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
					}		 
				}
			}   		 
     } 
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif
} 
#endif	


ALTER_INFO alterInfo;
_Bool isCanReceiveFlag=0;
//����2�����ж���Ӧ����
void USART2_IRQHandler(void)
{
	unsigned char data = 0,dataLen=0;
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)		//�����ж�
	{			
        data = USART_ReceiveData(USART2);
		if(data == 0xFD && alterInfo.rev_idle == 0)
		{
			//���յ�����֡����ʼ�ַ�0xFD���򿪽��տ���
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
				dataLen = alterInfo.alterBuf[2] + 3;//�����ܳ��ȣ�����������β
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
//	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)		//�����ж�
//	{			
//        data = USART_ReceiveData(USART2);
//		if(data == 0xFD && alterInfo.rev_idle == 0)
//		{
//			//���յ�����֡����ʼ�ַ�0xFD���򿪽��տ���
//			isCanReceiveFlag = 1;
//		}
//		if(isCanReceiveFlag == 1)
//		{
//			alterInfo.alterBuf[alterInfo.alterCount] = data;
//			alterInfo.alterCount++;
//			if(alterInfo.alterCount>3)
//			{
//				TFT_Command = alterInfo.alterBuf[1];
//				dataLen = alterInfo.alterBuf[2] + 3;//�����ܳ��ȣ�����������β
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
//				else if(alterInfo.alterCount>48)//��ֹ�������
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
//		USART2->DR;																//��ȡ����ע�⣺������Ҫ�������ܹ�����жϱ�־λ
//		USART_ClearFlag(USART2, USART_IT_IDLE);
//	}
//}

//����3�����ж���Ӧ����
//void USART3_IRQHandler(void)
//{
//	if(USART_GetFlagStatus(USART3, USART_FLAG_RXNE) != RESET)		//�����ж�
//	{
//		USART_ClearITPendingBit(USART3,USART_IT_RXNE);            //��������ж�
//	}
//}

DATA data;//����ת��
//intתΪ4�ֽ�
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








