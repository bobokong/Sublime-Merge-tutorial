
#ifndef FP_SENSOR__H
#define FP_SENSOR__H

#include "stm32f10x.h"

#define FP_UART_Port    UART2

#include "stdbool.h"
//#include "timer_delay.h"

/* ָ�ƴ�����������Ӧ���������� */
#define BLH_POWER_Port      PC
#define BLH_POWER_Pin       BIT4

#define FP_INT_Port         PB
#define FP_INT_PinBit       BIT1
/* ��ȡ����������ŵ�ƽ��Ϊtrueʱ��ʾ����ָ����ָ��ģ�飬��ʱ���Խ��бȶԵ���ز��� */
#define UserGetBLH_INT()    ((GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)==1)? true:false)														 

#define FP_ID_START				(0)				/* ������ָ����ʼID */
#define FP_ID_END 				(999)			/* ������ָ����ֹID */

#define FP_TOTLE_NUM			(FP_ID_END-FP_ID_START+1)   /* ��������ָ������ */

 /********************************************************************************
 * ָ�ƴ���������ָ��״̬��״̬
 */
#define BLH_Rec_STU_PackHead      1
#define BLH_Rec_STU_BLH_Instruct  2
#define BLH_Rec_STU_BLH_FunPar1   3
#define BLH_Rec_STU_BLH_FunPar2   4
#define BLH_Rec_STU_PackLen1      5
#define BLH_Rec_STU_PackLen2      6
#define BLH_Rec_STU_XorPar        7
#define BLH_Rec_STU_ExDat1        8
#define BLH_Rec_STU_ExDat2        9
#define BLH_Rec_STU_ExDat3        10
#define BLH_Rec_STU_ExDat4        11
#define BLH_Rec_STU_SUMDat        12

/****************************************************************************
*Ϊ�˼��ݲ�ͬƽ̨(8/16/32λ��,��С��ģʽ),�����ݽṹ�����˵���,��SUM�����
*һ���ֽ�λ�õ�����XOR֮��λ�ã��Ӷ���֤��ͬλ��ƽ̨�ڴ��ֽڶ��뵼�µ�����.
*��˸Ķ�ֻ���ڶ�ȡӦ������ʱ�����һ���ֽں���չ�����ݽ���˳�򼴿�,������
*������ģ�����.ͬʱͨ���ṹ��ܷ���Ļ�ȡҪ��ȡ������(����֪��).������С��
*ת������,��֤�ڴ��/С��CPU�ϵļ���������.������������Ӱ��΢С��
*���ڽṹ������ݵ���,���ڻ������е��ֽ�˳���ڵ�������ָ������в���.
****************************************************************************/
#define LITTLE_ENDIAN

#if defined(BIG_ENDIAN) && !defined(LITTLE_ENDIAN)
#define htons(A)   		(A)
#define htonl(A)     	(A)
#define ntohs(A)   		(A)
#define ntohl(A)    	(A)
#elif defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)
#define htons(A)     ((((uint16_t)(A) & 0xff00) >> 8) | \
											(((uint16_t)(A) & 0x00ff) << 8))
#define htonl(A)     ((((uint32_t)(A) & 0xff000000) >> 24) | \
											(((uint32_t)(A) & 0x00ff0000) >> 8) | \
											(((uint32_t)(A) & 0x0000ff00) << 8) | \
											(((uint32_t)(A) & 0x000000ff) << 24))
#define ntohs htons
#define ntohl htohl
#else
#error "Either BIG_ENDIAN or LITTLE_ENDIAN must be #defined, but not both."
#endif

/******����ͨ�����ݰ��ṹ��*****/
typedef struct{
	/*** 1-4 byte***/
	unsigned char PackHeader;		//��ͷ-1byte
	unsigned char Instruction;	//ָ��-1byte
	
	union{
		unsigned char  Dat[2];
		unsigned short Ack;
			}Ackinf;								//Ӧ����Ϣ-2byte
			
	/*** 5-8 byte***/
	union{
		unsigned char  Dat[2];
		unsigned short Len;
	}PackLen;										//����-2byte
			
	unsigned char XOR;					//���У��-1byte
	unsigned char SUM;					//��У��-1byte
	/*** 9-12 byte***/
	union{
		unsigned char  ExDat[4];	//��չ��-4byte
		unsigned short ExArg[2];			
	}ExField;	
}FP_DataPackDef;

typedef union{
	FP_DataPackDef DataPack;
	unsigned char  Instruct[sizeof(FP_DataPackDef)];
}FP_InstructPackDef;


/******�û�����ָ�����ݽṹ��*****/
typedef struct{
	unsigned char  Instruct;	//ָ��1byte
	unsigned short Fun_Arg;		//���ܲ���1byte
	unsigned short ExfeildLen;//����2byte
	union{
		unsigned char  ExDat[4];//��չ��4byte
		unsigned short ExArg[2];
	}Exfield;
}FP_DirectionDef;



#define BLH3001_TIMEOUT					6*2//�������ȴ���ʱʱ��/500ms,�˲���ֵ��ӦС�ڴ�������ʱʱ��

#define	TEPMLE_UPDOWNLOAD_EN			0

#ifndef TEMPLE_SIZE_ONE_TX				
	#define TEMPLE_SIZE_ONE_TX       256 //һ�δ���ģ���������(256-С����)
#endif

/**********��Ҫ�û���д�ĺ���**********/
#define Start_TMR(x)		Timer3_Open()  /* ������ʱ����ʱ�� */
#define Close_TMR()			Timer3_Close() /* �رճ�ʱ����ʱ�� */
#define SendByte(x)			FP_SendByte(x) /* ͨ�����ڷ���һ���ֽڵ����� */ 

/***********��Ҫ�û�����ı���**************************/
#define gTMR_OverFlag		gTIM3_OverFlag
#define gUartReady			g_ucFPCheckInst


#if(TEPMLE_UPDOWNLOAD_EN)
extern unsigned char gTempUplod;    //ģ���ϴ���ɱ�־ 0-δ��� 1-���
extern unsigned char gTempBuff[256];//�����н���ģ��Ļ���,�������û������ⲿ����
#endif
/***************�Ѷ�����ⲿ��������************************/
extern FP_InstructPackDef InstructPack;

/*****************������ָ��*****************/
enum{
	DIR_TERMINATE     = 0X00,//��ָֹ��
	DIR_SYS_CONFIG    = 0X01,//ϵͳ����/��Ѱָ������ָ��
	DIR_IMAGE_SAMPLE	=	0X02,//ͼ��ɼ�
	DIR_ENROLL_FP     = 0X03,//ָ��ע��
	DIR_COMPARISON_FP = 0X04,//ָ�Ʊȶ�
	DIR_DELETE_FP			= 0X05,//ָ��ɾ��
	DIR_TEMPLATE_LOAD = 0X06,//ģ��/ͼ����
};

/*****************ָ�����*****************/
#define ARG_TERMINATE     			0X0000			//��ָֹ�����
#define ARG_FP_QUANTITY       	0X0020			//��ȡע��ָ������
#define ARG_UNUSED_MINID				0X0004      //��ȡָ������Ϊע�����СID��
#define ARG_DELETE_FP						0XE000			//ָ��ɾ��

#if (TEMPLE_SIZE_ONE_TX==512)
	#define ARG_TEMPLATE_UPLOAD   	0X00C8			//�ϴ�ָ��ָ��ģ��(��ģ��)
	#define ARG_TEMPLATE_DOWNLOAD 	0X0038			//����ָ��ָ֮��ģ��(��ģ��)
#elif (TEMPLE_SIZE_ONE_TX==256)
	#define ARG_TEMPLATE_UPLOAD   	0X00C0			//0x00C0(Сģ��)
	#define ARG_TEMPLATE_DOWNLOAD 	0X0030			//0C0030(Сģ��)
#endif

#define ARG_D_FIRST_ENROLL			0X06A0			//��һ��ע��
#define ARG_D_SECOND_ENROLL			0X0660			//�ڶ���ע��
#define ARG_D_FINAL_ENROLL			0X061C			//������ע��
#define ARG_D_COMPARISON_FP     0X06E0			//ָ�ƱȶԲ���(ƥ����+��ģ��)
#define ARG_D_IMAGE_SAMPLE      0XA200      //��ͼ
#define ARG_D_IMAGE_CHECK				0XB200			//ͼ����(��ָ�뿪���)

/*************������Ӧ����*************/
#define ERR_SUCCESS							0X000				 //�ɹ�
#define ERR_FAIL								0XFFF				 //ʧ��
#define ERR_XOR									0XF01				 //���У�����
#define ERR_SUM									0XF02				 //��У�����
#define ERR_DIR									0XF03        //ָ�����
#define ERR_ARG									0XF04				 //��������
#define ERR_NO_FS								0XF06				 //���ļ�ϵͳ
#define ERR_SYS									0XF07				 //ϵͳ����
#define	ERR_EX_FIELD_ARG				0XF08				 //��չ���������
#define ERR_COMS_INIT	  				0x101				 //��������ʼ��ʧ��
#define ERR_COMS_CORRECT   			0X102				 //��ѧ������У��ʧ��
#define ERR_TIME_OUT						0X201				 //��ָ��ⳬʱ
#define ERR_IMAGE_SAMPLE				0X202				 //��ͼʧ��
#define ERR_TEMPLATE_CREATE 		0X203        //ͼ��������
#define ERR_REGISTERED					0X301				 //ָ����ע��
#define ERR_COMPARE_TEMP1				0X302				 //��ģ��һƥ��ʧ��
#define ERR_COMPARE_TEMP2				0X303				 //��ģ���ƥ��ʧ��
#define ERR_FP_FULL							0X304				 //������ָ������
#define ERR_FP_EMPTY						0X401				 //��������ָ��		
#define ERR_FP_COMPARE					0X402				 //ָ�Ʊȶ�ʧ��
#define ERR_FP_DELETE						0X501				 //ָ��ɾ��ʧ��
#define	ERR_INVALID_TEMPLATE 		0X601				 //ָ��ģ����Ч


/********************************************************************************
 * �û��Զ���״̬��
 */
typedef enum{
	FPR_OK = 0,  		/* �ɹ� */
	FPR_FAIL,    		/* ʧ�� */
	FPR_TIME_OUT,		/* �ȴ���ʱ */
	FPR_USER_INTR,	/* �û��ж�(�����������ǿ��������ǰѭ��) */
	FPR_EXISTED,    /* ָ���Ѵ��� */
	FPR_FULL,       /* ָ��ע������ */
	FPR_NULL = 0XFF,/* ��ָ�� */
}FPRESULT;

#define RES_OK       (0)

/*ָ�ƴ洢��¼������Ͷ���*/
typedef enum{
	FP_USER_ADMINS = 'A',     /* ����Ա�˻����� */
	FP_USER_NORMAL = 'N',     /* ��ͨ�û��˻����� */
}FP_USER_NAME;   /* �˻����Ͷ��� */

void BLHPOWER_Initize(void);  /* ָ�ƴ�������Դ���ƺ������� */
void BLHPOWER_ON(void);
void BLHPOWER_OFF(void);
void BLH3001_Initize(void);
void BLH_INT_PinConfig(void);
FPRESULT FP_Imagecheck(unsigned int Arg, unsigned int *Error);  /* ָ�ƴ�����ͨ��Э�麯������ */
FPRESULT FP_SendFunInst(FP_DirectionDef Direction, unsigned int *Error);
FPRESULT FP_Enroll_D(unsigned int StartID,unsigned int EndID,unsigned int *FP_ID, unsigned int *Error); /* ָ�ƴ������û��ӿں������� */
FPRESULT Fp_match_D(unsigned int StarID, unsigned int EndID ,unsigned int *FP_ID, unsigned int *Error);
FPRESULT Fp_Delete(unsigned int StartID, unsigned int EndID, unsigned int *Error);
FPRESULT FP_GetFPQuatity(unsigned int StartID,unsigned int EndID, unsigned int *Q, unsigned int *Error);
FPRESULT FP_GetUnused_MinID(unsigned int StartID,unsigned int EndID, unsigned int *MinID, unsigned int *Error);
FPRESULT FP_TerminateSensor(uint16_t *Error);
FPRESULT FP_Download_Template(unsigned int FP_ID,unsigned char Temp_ID,unsigned char Ctrl,unsigned char *pBuff, unsigned int *Error);
FPRESULT FP_Upload_Template(unsigned int FP_ID,unsigned char Temp_ID,unsigned char Ctrl,unsigned int *Error);
FPRESULT DeleteSpecific_FP(unsigned int *Error);
FPRESULT GetRemanent_FP(unsigned int *Rem_Num, unsigned int *Error);

void BLH_UART1_DataRecive(uint8_t DataRec);
																 
#endif








