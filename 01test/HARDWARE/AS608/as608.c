//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK ս��V3 STM32������
//ATK-AS608ָ��ʶ��ģ����������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/3/29
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights dataerved									  
////////////////////////////////////////////////////////////////////////////////// 	
#include <string.h>
#include "delay.h" 	
#include "uart_tft_bsp.h"
#include "as608.h"
#include "malloc.h"
//#include "SYN6288.h"
#include "gpio.h"
#include <stdio.h>
#include <stdarg.h>

u32 AS608Addr = 0XFFFFFFFF; //Ĭ��

SysPara AS608Para;//ָ��ģ��AS608����
u16 ValidN;//ģ������Чָ�Ƹ���
u8** kbd_tbl;
extern char sendEnd[3];
extern void SD_store(u16 ID);
u16 Set_flag=0;
u16 Add_flag=0;
u16 Del_flag=0;
u16 Ver_flag=0;
u16 ID=0;
u16 ID_flag=0;
//��ʼ��PC5Ϊ��������		    
//��������Ӧ״̬(������Ӧʱ����ߵ�ƽ�ź�)
void PS_StaGPIO_Init(void)
{   
  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//ʹ��GPIOCʱ��
  //��ʼ����״̬����GPIOC
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;//��������ģʽ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//50MHz
  GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIO	
}
//���ڷ���һ���ֽ�
static void MYUSART_SendData(u8 data)
{
	while((USART3->SR&0X40)==0); 
	USART3->DR = data;
}
//���Ͱ�ͷ
static void SendHead(void)
{
	MYUSART_SendData(0xEF);
	MYUSART_SendData(0x01);
}
//���͵�ַ
static void SendAddr(void)
{
	MYUSART_SendData(AS608Addr>>24);
	MYUSART_SendData(AS608Addr>>16);
	MYUSART_SendData(AS608Addr>>8);
	MYUSART_SendData(AS608Addr);
}
//���Ͱ���ʶ,
static void SendFlag(u8 flag)
{
	MYUSART_SendData(flag);
}
//���Ͱ�����
static void SendLength(int length)
{
	MYUSART_SendData(length>>8);
	MYUSART_SendData(length);
}
//����ָ����
static void Sendcmd(u8 cmd)
{
	MYUSART_SendData(cmd);
}
//����У���
static void SendCheck(u16 check)
{
	MYUSART_SendData(check>>8);
	MYUSART_SendData(check);
}
//�ж��жϽ��յ�������û��Ӧ���
//waittimeΪ�ȴ��жϽ������ݵ�ʱ�䣨��λ1ms��
//����ֵ�����ݰ��׵�ַ
static u8 *JudgeStr(u16 waittime)
{
	char *data;
	u8 str[8];
	str[0]=0xef;str[1]=0x01;str[2]=AS608Addr>>24;
	str[3]=AS608Addr>>16;str[4]=AS608Addr>>8;
	str[5]=AS608Addr;str[6]=0x07;str[7]='\0';
	USART3_RX_STA=0;
	while(--waittime)
	{
		delay_ms(1);
		if(USART3_RX_STA&0X8000)//���յ�һ������
		{
			USART3_RX_STA=0;
			data=strstr((const char*)USART3_RX_BUF,(const char*)str);
			if(data)
				return (u8*)data;	
		}
	}
	return 0;
}
//¼��ͼ�� PS_GetImage
//����:̽����ָ��̽�⵽��¼��ָ��ͼ�����ImageBuffer�� 
//ģ�鷵��ȷ����
u8 PS_GetImage(void)
{
  u16 temp;
  u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
	SendLength(0x03);
	Sendcmd(0x01);
  temp =  0x01+0x03+0x01;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}
//�������� PS_GenChar
//����:��ImageBuffer�е�ԭʼͼ������ָ�������ļ�����CharBuffer1��CharBuffer2			 
//����:BufferID --> charBuffer1:0x01	charBuffer1:0x02												
//ģ�鷵��ȷ����
u8 PS_GenChar(u8 BufferID)
{
	u16 temp;
  u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
	SendLength(0x04);
	Sendcmd(0x02);
	MYUSART_SendData(BufferID);
	temp = 0x01+0x04+0x02+BufferID;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}
//��ȷ�ȶ���öָ������ PS_Match
//����:��ȷ�ȶ�CharBuffer1 ��CharBuffer2 �е������ļ� 
//ģ�鷵��ȷ����
u8 PS_Match(void)
{
	u16 temp;
  u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
	SendLength(0x03);
	Sendcmd(0x03);
	temp = 0x01+0x03+0x03;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}
//����ָ�� PS_Search
//����:��CharBuffer1��CharBuffer2�е������ļ����������򲿷�ָ�ƿ�.�����������򷵻�ҳ�롣			
//����:  BufferID @ref CharBuffer1	CharBuffer2
//˵��:  ģ�鷵��ȷ���֣�ҳ�루����ָ��ģ�壩
u8 PS_Search(u8 BufferID,u16 StartPage,u16 PageNum,SearchResult *p)
{
	u16 temp;
  u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
	SendLength(0x08);
	Sendcmd(0x04);
	MYUSART_SendData(BufferID);
	MYUSART_SendData(StartPage>>8);
	MYUSART_SendData(StartPage);
	MYUSART_SendData(PageNum>>8);
	MYUSART_SendData(PageNum);
	temp = 0x01+0x08+0x04+BufferID
	+(StartPage>>8)+(u8)StartPage
	+(PageNum>>8)+(u8)PageNum;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
	{
		ensure = data[9];
		p->pageID   =(data[10]<<8)+data[11];
		p->mathscore=(data[12]<<8)+data[13];	
	}
	else
		ensure = 0xff;
	return ensure;	
}
//�ϲ�����������ģ�壩PS_RegModel
//����:��CharBuffer1��CharBuffer2�е������ļ��ϲ����� ģ��,�������CharBuffer1��CharBuffer2	
//˵��:  ģ�鷵��ȷ����
u8 PS_RegModel(void)
{
	u16 temp;
  u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
	SendLength(0x03);
	Sendcmd(0x05);
	temp = 0x01+0x03+0x05;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;		
}
//����ģ�� PS_StoreChar
//����:�� CharBuffer1 �� CharBuffer2 �е�ģ���ļ��浽 PageID ��flash���ݿ�λ�á�			
//����:  BufferID @ref charBuffer1:0x01	charBuffer1:0x02
//       PageID��ָ�ƿ�λ�úţ�
//˵��:  ģ�鷵��ȷ����
u8 PS_StoreChar(u8 BufferID,u16 PageID)
{
	u16 temp;
  u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
	SendLength(0x06);
	Sendcmd(0x06);
	MYUSART_SendData(BufferID);
	MYUSART_SendData(PageID>>8);
	MYUSART_SendData(PageID);
	temp = 0x01+0x06+0x06+BufferID
	+(PageID>>8)+(u8)PageID;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;	
}
//ɾ��ģ�� PS_DeletChar
//����:  ɾ��flash���ݿ���ָ��ID�ſ�ʼ��N��ָ��ģ��
//����:  PageID(ָ�ƿ�ģ���)��Nɾ����ģ�������
//˵��:  ģ�鷵��ȷ����
u8 PS_DeletChar(u16 PageID,u16 N)
{
	u16 temp;
  u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
	SendLength(0x07);
	Sendcmd(0x0C);
	MYUSART_SendData(PageID>>8);
	MYUSART_SendData(PageID);
	MYUSART_SendData(N>>8);
	MYUSART_SendData(N);
	temp = 0x01+0x07+0x0C
	+(PageID>>8)+(u8)PageID
	+(N>>8)+(u8)N;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}
//���ָ�ƿ� PS_Empty
//����:  ɾ��flash���ݿ�������ָ��ģ��
//����:  ��
//˵��:  ģ�鷵��ȷ����
u8 PS_Empty(void)
{
	u16 temp;
  u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
	SendLength(0x03);
	Sendcmd(0x0D);
	temp = 0x01+0x03+0x0D;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}
//дϵͳ�Ĵ��� PS_WriteReg
//����:  дģ��Ĵ���
//����:  �Ĵ������RegNum:4\5\6
//˵��:  ģ�鷵��ȷ����
u8 PS_WriteReg(u8 RegNum,u8 DATA)
{
	u16 temp;
  u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
	SendLength(0x05);
	Sendcmd(0x0E);
	MYUSART_SendData(RegNum);
	MYUSART_SendData(DATA);
	temp = RegNum+DATA+0x01+0x05+0x0E;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
//	if(ensure==0)
//		printf("\r\n���ò����ɹ���");
//	else
//		printf("\r\n%s",EnsureMessage(ensure));
	return ensure;
}
//��ϵͳ�������� PS_ReadSysPara
//����:  ��ȡģ��Ļ��������������ʣ�����С��)
//����:  ��
//˵��:  ģ�鷵��ȷ���� + ����������16bytes��
u8 PS_ReadSysPara(SysPara *p)
{
	u16 temp;
  u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
	SendLength(0x03);
	Sendcmd(0x0F);
	temp = 0x01+0x03+0x0F;
	SendCheck(temp);
	data=JudgeStr(1000);
	if(data)
	{
		ensure = data[9];
		p->PS_max = (data[14]<<8)+data[15];
		p->PS_level = data[17];
		p->PS_addr=(data[18]<<24)+(data[19]<<16)+(data[20]<<8)+data[21];
		p->PS_size = data[23];
		p->PS_N = data[25];
	}		
	else
		ensure=0xff;
//	if(ensure==0x00)
//	{
//		printf("\r\nģ�����ָ������=%d",p->PS_max);
//		printf("\r\n�Աȵȼ�=%d",p->PS_level);
//		printf("\r\n��ַ=%x",p->PS_addr);
//		printf("\r\n������=%d",p->PS_N*9600);
//	}
//	else 
//			printf("\r\n%s",EnsureMessage(ensure));
	return ensure;
}
//����ģ���ַ PS_SetAddr
//����:  ����ģ���ַ
//����:  PS_addr
//˵��:  ģ�鷵��ȷ����
u8 PS_SetAddr(u32 PS_addr)
{
	u16 temp;
  u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
	SendLength(0x07);
	Sendcmd(0x15);
	MYUSART_SendData(PS_addr>>24);
	MYUSART_SendData(PS_addr>>16);
	MYUSART_SendData(PS_addr>>8);
	MYUSART_SendData(PS_addr);
	temp = 0x01+0x07+0x15
	+(u8)(PS_addr>>24)+(u8)(PS_addr>>16)
	+(u8)(PS_addr>>8) +(u8)PS_addr;				
	SendCheck(temp);
	AS608Addr=PS_addr;//������ָ�������ַ
  data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;	
		AS608Addr = PS_addr;
//	if(ensure==0x00)
//		printf("\r\n���õ�ַ�ɹ���");
//	else
//		printf("\r\n%s",EnsureMessage(ensure));
	return ensure;
}
//���ܣ� ģ���ڲ�Ϊ�û�������256bytes��FLASH�ռ����ڴ��û����±�,
//	�ü��±��߼��ϱ��ֳ� 16 ��ҳ��
//����:  NotePageNum(0~15),Byte32(Ҫд�����ݣ�32���ֽ�)
//˵��:  ģ�鷵��ȷ����
u8 PS_WriteNotepad(u8 NotePageNum,u8 *Byte32)
{
	u16 temp;
  u8  ensure,i;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
	SendLength(36);
	Sendcmd(0x18);
	MYUSART_SendData(NotePageNum);
	for(i=0;i<32;i++)
	 {
		 MYUSART_SendData(Byte32[i]);
		 temp += Byte32[i];
	 }
  temp =0x01+36+0x18+NotePageNum+temp;
	SendCheck(temp);
  data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}
//������PS_ReadNotepad
//���ܣ�  ��ȡFLASH�û�����128bytes����
//����:  NotePageNum(0~15)
//˵��:  ģ�鷵��ȷ����+�û���Ϣ
u8 PS_ReadNotepad(u8 NotePageNum,u8 *Byte32)
{
	u16 temp;
  u8  ensure,i;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
	SendLength(0x04);
	Sendcmd(0x19);
	MYUSART_SendData(NotePageNum);
	temp = 0x01+0x04+0x19+NotePageNum;
	SendCheck(temp);
  data=JudgeStr(2000);
	if(data)
	{
		ensure=data[9];
		for(i=0;i<32;i++)
		{
			Byte32[i]=data[10+i];
		}
	}
	else
		ensure=0xff;
	return ensure;
}
//��������PS_HighSpeedSearch
//���ܣ��� CharBuffer1��CharBuffer2�е������ļ��������������򲿷�ָ�ƿ⡣
//		  �����������򷵻�ҳ��,��ָ����ڵ�ȷ������ָ�ƿ��� ���ҵ�¼ʱ����
//		  �ܺõ�ָ�ƣ���ܿ�������������
//����:  BufferID�� StartPage(��ʼҳ)��PageNum��ҳ����
//˵��:  ģ�鷵��ȷ����+ҳ�루����ָ��ģ�壩
u8 PS_HighSpeedSearch(u8 BufferID,u16 StartPage,u16 PageNum,SearchResult *p)
{
	u16 temp;
  u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
	SendLength(0x08);
	Sendcmd(0x1b);
	MYUSART_SendData(BufferID);
	MYUSART_SendData(StartPage>>8);
	MYUSART_SendData(StartPage);
	MYUSART_SendData(PageNum>>8);
	MYUSART_SendData(PageNum);
	temp = 0x01+0x08+0x1b+BufferID
	+(StartPage>>8)+(u8)StartPage
	+(PageNum>>8)+(u8)PageNum;
	SendCheck(temp);
	data=JudgeStr(2000);
 	if(data)
	{
		ensure=data[9];
		p->pageID 	=(data[10]<<8) +data[11];
		p->mathscore=(data[12]<<8) +data[13];
	}
	else
		ensure=0xff;
	return ensure;
}
//����Чģ����� PS_ValidTempleteNum
//���ܣ�����Чģ�����
//����: ��
//˵��: ģ�鷵��ȷ����+��Чģ�����ValidN
u8 PS_ValidTempleteNum(u16 *ValidN)
{
	u16 temp;
  u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
	SendLength(0x03);
	Sendcmd(0x1d);
	temp = 0x01+0x03+0x1d;
	SendCheck(temp);
  data=JudgeStr(2000);
	if(data)
	{
		ensure=data[9];
		*ValidN = (data[10]<<8) +data[11];
	}		
	else
		ensure=0xff;
	
//	if(ensure==0x00)
//	{
//		printf("\r\n��Чָ�Ƹ���=%d",(data[10]<<8)+data[11]);
//	}
//	else
//		printf("\r\n%s",EnsureMessage(ensure));
	return ensure;
}
//��AS608���� PS_HandShake
//����: PS_Addr��ַָ��
//˵��: ģ�鷵�µ�ַ����ȷ��ַ��	
u8 PS_HandShake(u32 *PS_Addr)
{
	SendHead();
	SendAddr();
	MYUSART_SendData(0X01);
	MYUSART_SendData(0X00);
	MYUSART_SendData(0X00);	
	delay_ms(200);
	if(USART3_RX_STA&0X8000)//���յ�����
	{		
		if(//�ж��ǲ���ģ�鷵�ص�Ӧ���				
					USART3_RX_BUF[0]==0XEF
				&&USART3_RX_BUF[1]==0X01
				&&USART3_RX_BUF[6]==0X07
			)
			{
				*PS_Addr=(USART3_RX_BUF[2]<<24) + (USART3_RX_BUF[3]<<16)
								+(USART3_RX_BUF[4]<<8) + (USART3_RX_BUF[5]);
				USART3_RX_STA=0;
				return 0;
			}
		USART3_RX_STA=0;					
	}
	return 1;		
}
//ģ��Ӧ���ȷ������Ϣ����
//���ܣ�����ȷ���������Ϣ������Ϣ
//����: ensure
const char *EnsureMessage(u8 ensure) 
{
	const char *p;
	switch(ensure)
	{
		case  0x00:
			p="OK";break;		
		case  0x01:
			p="���ݰ����մ���";break;
		case  0x02:
			p="��������û����ָ";break;
		case  0x03:
			p="¼��ָ��ͼ��ʧ��";break;
		case  0x04:
			p="ָ��ͼ��̫�ɡ�̫��������������";break;
		case  0x05:
			p="ָ��ͼ��̫ʪ��̫��������������";break;
		case  0x06:
			p="ָ��ͼ��̫�Ҷ�����������";break;
		case  0x07:
			p="ָ��ͼ����������������̫�٣������̫С��������������";break;
		case  0x08:
			p="ָ�Ʋ�ƥ��";break;
		case  0x09:
			p="û������ָ��";break;
		case  0x0a:
			p="�����ϲ�ʧ��";break;
		case  0x0b:
			p="����ָ�ƿ�ʱ��ַ��ų���ָ�ƿⷶΧ";
		case  0x10:
			p="ɾ��ģ��ʧ��";break;
		case  0x11:
			p="���ָ�ƿ�ʧ��";break;	
		case  0x15:
			p="��������û����Чԭʼͼ��������ͼ��";break;
		case  0x18:
			p="��д FLASH ����";break;
		case  0x19:
			p="δ�������";break;
		case  0x1a:
			p="��Ч�Ĵ�����";break;
		case  0x1b:
			p="�Ĵ����趨���ݴ���";break;
		case  0x1c:
			p="���±�ҳ��ָ������";break;
		case  0x1f:
			p="ָ�ƿ���";break;
		case  0x20:
			p="��ַ����";break;
		default :
			p="ģ�鷵��ȷ��������";break;
	}
 return p;	
}


//¼ָ��
//void Add_FR(void)
//{
//	u8 i,ensure ,processnum=0;
//	int ID=400;
//	char *str;
//	
//	str=mymalloc(SRAMIN,30);
//	while(1)
//	{
//		switch (processnum)
//		{
//			case 0:
//				i++;
//				Voice_play("�밴ָ��");
//				printf("add.t0.txt=\"�밴ָ��\"");
//				printf(sendEnd);
//				delay_s(2);
//				ensure=PS_GetImage();
//				if(ensure==0x00) 
//				{
//					ensure=PS_GenChar(CharBuffer1);//��������
//					if(ensure==0x00)
//					{
//						Voice_play("ָ������");
//						printf("add.t0.txt=\"ָ������\"");
//						printf(sendEnd);
//						delay_s(1);
//						i=0;
//						processnum=1;//�����ڶ���						
//					}
//					else
//					{
//						mymemset(str,0,50);
//						sprintf(str,"add.n0.val=%d",ensure);
//						UsartPrintf(USART2,str);
//						printf(sendEnd);
//						myfree(SRAMIN,str);
//					}
//				}
//				else 
//				{
//					mymemset(str,0,50);
//					sprintf(str,"add.n0.val=%d",ensure);
//					UsartPrintf(USART2,str);
//					printf(sendEnd);
//					myfree(SRAMIN,str);
//				}						
//				break;
//			
//			case 1:
//				i++;
//				Voice_play("���ٰ�һ��ָ��");
//				printf("add.t0.txt=\"���ٰ�һ��ָ��\"");
//				printf(sendEnd);
//				delay_s(2);
//				ensure=PS_GetImage();
//				if(ensure==0x00) 
//				{
//					ensure=PS_GenChar(CharBuffer2);//��������
//					if(ensure==0x00)
//					{
//						Voice_play("ָ������");
//						printf("add.t0.txt=\"ָ������\"");
//						printf(sendEnd);
//						delay_s(1);
//						i=0;
//						processnum=2;//����������
//					}else 
//					{
//						mymemset(str,0,50);
//						sprintf(str,"add.n0.val=%d",ensure);
//						UsartPrintf(USART2,str);
//						printf(sendEnd);
//						myfree(SRAMIN,str);
//					}	
//				}
//				else 
//				{
//					mymemset(str,0,50);
//					sprintf(str,"add.n0.val=%d",ensure);
//					UsartPrintf(USART2,str);
//					printf(sendEnd);
//					myfree(SRAMIN,str);
//				}		
//				break;

//			case 2:
//				Voice_play("�Ա�����ָ��");
//				printf("add.t0.txt=\"�Ա�����ָ��\"");
//				printf(sendEnd);
//				delay_s(2);
//				ensure=PS_Match();
//				if(ensure==0x00) 
//				{
//					Voice_play("�Աȳɹ�");
//					printf("add.t0.txt=\"�Աȳɹ�\"");
//					printf(sendEnd);
//					delay_s(2);
//					processnum=3;//�������Ĳ�
//				}
//				else 
//				{
//					Voice_play("�Ա�ʧ�ܣ�������¼��ָ��");
//					printf("add.t0.txt=\"�Ա�ʧ�ܣ�������¼��ָ��\"");
//					printf(sendEnd);
//					delay_s(3);
//					mymemset(str,0,50);
//					
//					sprintf(str,"add.n0.val=%d",ensure);
//					UsartPrintf(USART2,str);
//					printf(sendEnd);
//					myfree(SRAMIN,str);
//					i=0;
//					processnum=0;//���ص�һ��		
//				}
//				break;

//			case 3:
//				Voice_play("����ָ��ģ��");
//				printf("add.t0.txt=\"����ָ��ģ��\"");
//				printf(sendEnd);
//				delay_s(2);
//				ensure=PS_RegModel();
//				if(ensure==0x00) 
//				{
//					Voice_play("����ָ��ģ��ɹ�");
//					printf("add.t0.txt=\"����ָ��ģ��ɹ�\"");
//					printf(sendEnd);
//					delay_s(2);
//					processnum=4;//�������岽
//				}
//				else 
//				{
//					processnum=0;
//					mymemset(str,0,50);
//					sprintf(str,"add.n0.val=%d",ensure);
//					UsartPrintf(USART2,str);
//					printf(sendEnd);
//					myfree(SRAMIN,str);
//				}
//				break;
//				
//			case 4:	
//				Voice_play("�����봢��ID");
//				printf("add.t0.txt=\"�����봢��ID\"");
//				printf(sendEnd);
//				delay_s(2);
//			
//				printf("page ID");
//				printf(sendEnd);
//				delay_s(1);
//				do
//				{
//					if((alterInfo.rev_idle == 1) && (TFT_Command == 0x03))
//					{
//						ID=bytesToInt(&(alterInfo.alterBuf[3]));
//						alterInfo.rev_idle=0;
//						TFT_Command=0;
//					}
//					if(alterInfo.rev_idle)
//						alterInfo.rev_idle=0;
//				}
//				while(!(ID<AS608Para.PS_max));//����ID����С��ָ�����������ֵ
//				ensure=PS_StoreChar(CharBuffer2,ID);//����ģ��
//				
//				printf("page add");
//				printf(sendEnd);
//				
//				if(ensure==0x00) 
//				{
//					Voice_play("¼��ָ�Ƴɹ�");
//					printf("add.t0.txt=\"¼��ָ�Ƴɹ�\"");
//					printf(sendEnd);
//					GPIOA->BSRR = 0x00000100;
//					delay_s(1);
//					PS_ValidTempleteNum(&ValidN);//����ָ�Ƹ���
//					mymemset(str,0,50);
//					sprintf(str,"add.n0.val=%d",AS608Para.PS_max-ValidN);
//					UsartPrintf(USART2,str);
//					printf(sendEnd);
//					myfree(SRAMIN,str);
//					delay_ms(1500);
//					
//					printf("page main");
//					printf(sendEnd);					
//					processnum=0;
//					return ;
//				}
//				else 
//				{	
//					processnum=0;
//					mymemset(str,0,50);
//					sprintf(str,"add.n0.val=%d",ensure);
//					UsartPrintf(USART2,str);
//					printf(sendEnd);
//					myfree(SRAMIN,str);
//				}					
//				break;				
//		}
//		delay_ms(400);
//		if(i==5)//����5��û�а���ָ���˳�
//		{
//			TFT_Command=0;
//			printf("page main");
//			printf(sendEnd);
//			break;	
//		}				
//	}
//}
void Add_FR(void)
{	
	u8 i=0,ensure ,processnum=0;
	while(1)
	{
		switch (processnum)
		{
			case 0:
				i++;
				ensure=PS_GetImage();//�밴��ָ
				if(ensure==0x00) //��ȡ��ָ��
				{
					ensure=PS_GenChar(CharBuffer1);//��������
					if(ensure==0x00)
					{
						LED5=1;
						delay_ms(500);
						i=0;
						processnum=1;//�����ڶ���						
					}
//					else 
//						OLED_Show_Str(0, 33,"Finger no I",12);//ָ������				
				}
//				else 
//					//OLED_Show_Str(0, 20,"Finger X",12);//ָ��δ��ȡ						
				break;
			case 1:
				i++;
				ensure=PS_GetImage();//�ٰ�һ����ָ
				if(ensure==0x00) 
				{
					ensure=PS_GenChar(CharBuffer2);//��������			
					if(ensure==0x00)//ָ����ȷ
					{
						LED6=0;
						delay_ms(100);				
						i=0;
						processnum=2;//����������
					}//else OLED_Show_Str(0, 33,"Finger no II",12);	
				}//else OLED_Show_Str(0, 20,"Finger XX",12);		
				break;
			case 2:
				ensure=PS_Match();//�Ա�����ָ��
				if(ensure==0x00) //����ָ����һ����
				{
					delay_ms(100);				
					processnum=3;//�������Ĳ�
				}
				else 
				{
					i=0;
					processnum=0;//���ص�һ��		
				}
				delay_ms(500);
				break;

			case 3:
				ensure=PS_RegModel();//����һ��ָ��ģ��
				if(ensure==0x00) //����ָ��ģ��ɹ�
				{
					processnum=4;//�������岽
				}else 
				{
					processnum=0;//OLED_Show_Str(0, 20,"Creat Unsccess",12);
				}
				delay_ms(100);
				break;
				
			case 4:

//				OLED_Clear();				
//				OLED_Show_Str(20, 0,"Creat Mode ID    ",12);//����ID������Enter������
//				OLED_Show_Str(0, 20,"    0< ID <299   ",12);//���뷶Χ	
			//OLED_Show_Str(0, 33,"Input ID:",12);
//			while(!ID_flag)
//			{
//				GPIOA->BRR  = 0x00000100;
////			OLED_Show_IntNum(60, 33, ID,3,12);	
//			}
			//GPIOA->BSRR = 0x00000100;
//			OLED_Clear();	
				ensure=PS_StoreChar(CharBuffer2,ID);//����ģ��
				if(ensure==0x00) 
				{			
					LED5=0;
					LED6=0;
				delay_ms(500);
					PS_ValidTempleteNum(&ValidN);//����ָ�Ƹ���
					ID_flag=0;//ID_flag��0
					ID++;//ָ������һ��
					Set_flag=1;
					Add_flag=0;
					Del_flag=0;
					Ver_flag=0;
					return ;
				}else 
				{
					processnum=0;
				}					
				break;				
		}
		delay_ms(100);				
	}
}


//ɾ��ָ��
void Del_FR(void)
{
	u8  ensure;
	u16 num;
	char *str;
	_Bool ID_Flag=0;
	
	str=mymalloc(SRAMIN,30);
	//Voice_play("ɾ��ָ��");
	printf("delete.t0.txt=\"ɾ��ָ��\"");
	printf(sendEnd);
	delay_s(2);	
	//Voice_play("������ָ��ID");
	printf("delete.t0.txt=\"������ָ��ID\"");
	printf(sendEnd);
	delay_s(2);
	printf("page ID");
	printf(sendEnd);
	do
	{
		if((alterInfo.rev_idle == 1) && (TFT_Command == 0x03))
		{
			num=bytesToInt(&(alterInfo.alterBuf[3]));
			alterInfo.rev_idle=0;
			TFT_Command=0;
			ID_Flag=1;
		}
		if(alterInfo.rev_idle)
			alterInfo.rev_idle=0;
	}
	while(!ID_Flag);
	if(num==0xFFFF)
		goto MENU ; //������ҳ��
	else if(num==0xFF00)
		ensure=PS_Empty();//���ָ�ƿ�
	else 
		ensure=PS_DeletChar(num,1);//ɾ������ָ��
	
	printf("page delete");
	printf(sendEnd);
	if(ensure==0)
	{
		//Voice_play("ɾ��ָ�Ƴɹ�");
		printf("delete.t0.txt=\"ɾ��ָ�Ƴɹ�\"");
		printf(sendEnd);
		delay_s(2);		
	}
  else
  {
		mymemset(str,0,50);
		sprintf(str,"delete.n0.val=%d",ensure);
		UsartPrintf(USART2,str);
		printf(sendEnd);
		myfree(SRAMIN,str);
  }	
	PS_ValidTempleteNum(&ValidN);//����ָ�Ƹ���
	mymemset(str,0,50);
	sprintf(str,"delete.n0.val=%d",AS608Para.PS_max-ValidN);
	UsartPrintf(USART2,str);
	printf(sendEnd);
	myfree(SRAMIN,str);
	delay_ms(1200);
 MENU:
    printf("page main");
	printf(sendEnd);
}

//ˢָ��
//void press_FR(void)
//{
//	SearchResult seach;
//	u8 ensure;
//	char *str;
//	
//	str=mymalloc(SRAMIN,30);
//	ensure=PS_GetImage();
//	if(ensure==0x00)//��ȡͼ��ɹ� 
//	{	
//		ensure=PS_GenChar(CharBuffer1);
//		if(ensure==0x00) //���������ɹ�
//		{		
//			ensure=PS_HighSpeedSearch(CharBuffer1,0,AS608Para.PS_max,&seach);
//			if(ensure==0x00)//�����ɹ�
//			{
//				mymemset(str,0,50);
//				sprintf(str,"%d��ͬѧǩ���ɹ�",seach.pageID);
//				Voice_play(str);
//				myfree(SRAMIN,str);
//				
//				SD_store(seach.pageID);
//				switch(seach.pageID)
//				{
//					case 1:
//						LED1=0;
//						break;
//					case 2:
//						LED2=0;
//						break;
//					case 3:
//						LED3=0;
//						break;
//					case 4:
//						LED4=0;
//						break;
//					case 5:
//						LED5=0;
//						break;
//					default:break;
//				}
//				
//				delay_ms(500);
//				mymemset(str,0,50);;
//				printf("main.t0.txt=\"ID:%d :%d\"",seach.pageID,seach.mathscore);
//				printf(sendEnd);
//				myfree(SRAMIN,str);
//				delay_ms(1000);				
//			}				
//		}
//	}
//		
//}

//ˢָ��
void press_FR(void)
{
	SearchResult seach;
	u8 ensure;
//	char *str;
	ensure=PS_GetImage();
	if(ensure==0x00)//��ȡͼ��ɹ� 
	{	
		//OLED_Clear();
		ensure=PS_GenChar(CharBuffer1);
		if(ensure==0x00) //���������ɹ�
		{		//GPIOA->BSRR = 0x00000100;
			ensure=PS_HighSpeedSearch(CharBuffer1,0,300,&seach);	
				if(ensure==0x00)	
				{
					LED5=0;
					LED6=0;
					LED7=0;
					delay_ms(300);
					LED5=1;
					LED6=1;
					LED7=1;
					delay_ms(300);
					LED5=0;
					LED6=0;
					LED7=0;
					delay_ms(300);
					LED5=1;
					LED6=1;
					LED7=1;
					delay_ms(300);
				}
				else
				{
					LED5=0;
					LED7=0;
					delay_ms(300);
					LED5=1;
				
					LED7=1;
					delay_ms(300);
					LED5=0;
					LED7=0;
					delay_ms(300);
					LED5=1;
				
					LED7=1;
					delay_ms(300);
				}
	  }
		else
//			OLED_Show_Str(10, 0, "Create Fail",16);//����ʧ��
//	 delay_ms(1000);//��ʱ�������ʾ
//		OLED_Clear();
		return ;
	}		
}
