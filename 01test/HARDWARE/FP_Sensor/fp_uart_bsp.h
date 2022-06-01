
#ifndef FP_SENSOR__H
#define FP_SENSOR__H

#include "stm32f10x.h"

#define FP_UART_Port    UART2

#include "stdbool.h"
//#include "timer_delay.h"

/* 指纹传感器触摸感应输出检测引脚 */
#define BLH_POWER_Port      PC
#define BLH_POWER_Pin       BIT4

#define FP_INT_Port         PB
#define FP_INT_PinBit       BIT1
/* 获取触摸检测引脚电平，为true时表示有手指触摸指纹模块，此时可以进行比对等相关操作 */
#define UserGetBLH_INT()    ((GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)==1)? true:false)														 

#define FP_ID_START				(0)				/* 传感器指纹起始ID */
#define FP_ID_END 				(999)			/* 传感器指纹终止ID */

#define FP_TOTLE_NUM			(FP_ID_END-FP_ID_START+1)   /* 传感器的指纹容量 */

 /********************************************************************************
 * 指纹传感器返回指令状态机状态
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
*为了兼容不同平台(8/16/32位宽,大小端模式),将数据结构体做了调整,将SUM由最后
*一个字节位置调整到XOR之后位置，从而保证不同位宽平台内存字节对齐导致的问题.
*如此改动只需在读取应答数据时将最后一个字节和扩展域数据交换顺序即可,不用修
*改其他模块代码.同时通过结构体很方便的获取要读取的数据(见名知意).新增大小端
*转换功能,保证在大端/小端CPU上的兼容性问题.而对整体性能影响微小。
*由于结构体的数据调整,串口缓存其中的字节顺序在调整后与指令包略有差异.
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

/******串口通信数据包结构体*****/
typedef struct{
	/*** 1-4 byte***/
	unsigned char PackHeader;		//包头-1byte
	unsigned char Instruction;	//指令-1byte
	
	union{
		unsigned char  Dat[2];
		unsigned short Ack;
			}Ackinf;								//应答信息-2byte
			
	/*** 5-8 byte***/
	union{
		unsigned char  Dat[2];
		unsigned short Len;
	}PackLen;										//包长-2byte
			
	unsigned char XOR;					//异或校验-1byte
	unsigned char SUM;					//和校验-1byte
	/*** 9-12 byte***/
	union{
		unsigned char  ExDat[4];	//扩展域-4byte
		unsigned short ExArg[2];			
	}ExField;	
}FP_DataPackDef;

typedef union{
	FP_DataPackDef DataPack;
	unsigned char  Instruct[sizeof(FP_DataPackDef)];
}FP_InstructPackDef;


/******用户设置指令数据结构体*****/
typedef struct{
	unsigned char  Instruct;	//指令1byte
	unsigned short Fun_Arg;		//功能参数1byte
	unsigned short ExfeildLen;//包长2byte
	union{
		unsigned char  ExDat[4];//扩展域4byte
		unsigned short ExArg[2];
	}Exfield;
}FP_DirectionDef;



#define BLH3001_TIMEOUT					6*2//传感器等待超时时间/500ms,此参数值不应小于传感器超时时间

#define	TEPMLE_UPDOWNLOAD_EN			0

#ifndef TEMPLE_SIZE_ONE_TX				
	#define TEMPLE_SIZE_ONE_TX       256 //一次传送模板的数据量(256-小摸版)
#endif

/**********需要用户编写的函数**********/
#define Start_TMR(x)		Timer3_Open()  /* 启动超时处理定时器 */
#define Close_TMR()			Timer3_Close() /* 关闭超时处理定时器 */
#define SendByte(x)			FP_SendByte(x) /* 通过串口发送一个字节的数据 */ 

/***********需要用户定义的变量**************************/
#define gTMR_OverFlag		gTIM3_OverFlag
#define gUartReady			g_ucFPCheckInst


#if(TEPMLE_UPDOWNLOAD_EN)
extern unsigned char gTempUplod;    //模板上传完成标志 0-未完成 1-完成
extern unsigned char gTempBuff[256];//串口中接收模板的缓存,该数组用户需在外部定义
#endif
/***************已定义的外部变量声明************************/
extern FP_InstructPackDef InstructPack;

/*****************传感器指令*****************/
enum{
	DIR_TERMINATE     = 0X00,//终止指令
	DIR_SYS_CONFIG    = 0X01,//系统配置/查寻指纹数量指令
	DIR_IMAGE_SAMPLE	=	0X02,//图像采集
	DIR_ENROLL_FP     = 0X03,//指纹注册
	DIR_COMPARISON_FP = 0X04,//指纹比对
	DIR_DELETE_FP			= 0X05,//指纹删除
	DIR_TEMPLATE_LOAD = 0X06,//模板/图像传输
};

/*****************指令功能码*****************/
#define ARG_TERMINATE     			0X0000			//终止指令功能码
#define ARG_FP_QUANTITY       	0X0020			//读取注册指纹数量
#define ARG_UNUSED_MINID				0X0004      //读取指定区域为注册的最小ID号
#define ARG_DELETE_FP						0XE000			//指纹删除

#if (TEMPLE_SIZE_ONE_TX==512)
	#define ARG_TEMPLATE_UPLOAD   	0X00C8			//上传指定指纹模板(大模板)
	#define ARG_TEMPLATE_DOWNLOAD 	0X0038			//下载指定之指纹模板(大模板)
#elif (TEMPLE_SIZE_ONE_TX==256)
	#define ARG_TEMPLATE_UPLOAD   	0X00C0			//0x00C0(小模板)
	#define ARG_TEMPLATE_DOWNLOAD 	0X0030			//0C0030(小模板)
#endif

#define ARG_D_FIRST_ENROLL			0X06A0			//第一次注册
#define ARG_D_SECOND_ENROLL			0X0660			//第二次注册
#define ARG_D_FINAL_ENROLL			0X061C			//第三次注册
#define ARG_D_COMPARISON_FP     0X06E0			//指纹比对参数(匹配主+副模板)
#define ARG_D_IMAGE_SAMPLE      0XA200      //采图
#define ARG_D_IMAGE_CHECK				0XB200			//图像检测(手指离开检测)

/*************传感器应答码*************/
#define ERR_SUCCESS							0X000				 //成功
#define ERR_FAIL								0XFFF				 //失败
#define ERR_XOR									0XF01				 //异或校验错误
#define ERR_SUM									0XF02				 //和校验错误
#define ERR_DIR									0XF03        //指令错误
#define ERR_ARG									0XF04				 //参数错误
#define ERR_NO_FS								0XF06				 //无文件系统
#define ERR_SYS									0XF07				 //系统错误
#define	ERR_EX_FIELD_ARG				0XF08				 //扩展域参数错误
#define ERR_COMS_INIT	  				0x101				 //传感器初始化失败
#define ERR_COMS_CORRECT   			0X102				 //光学传感器校正失败
#define ERR_TIME_OUT						0X201				 //手指检测超时
#define ERR_IMAGE_SAMPLE				0X202				 //采图失败
#define ERR_TEMPLATE_CREATE 		0X203        //图像质量差
#define ERR_REGISTERED					0X301				 //指纹已注册
#define ERR_COMPARE_TEMP1				0X302				 //与模板一匹配失败
#define ERR_COMPARE_TEMP2				0X303				 //与模板二匹配失败
#define ERR_FP_FULL							0X304				 //传感器指纹已满
#define ERR_FP_EMPTY						0X401				 //传感器无指纹		
#define ERR_FP_COMPARE					0X402				 //指纹比对失败
#define ERR_FP_DELETE						0X501				 //指纹删除失败
#define	ERR_INVALID_TEMPLATE 		0X601				 //指定模板无效


/********************************************************************************
 * 用户自定义状态码
 */
typedef enum{
	FPR_OK = 0,  		/* 成功 */
	FPR_FAIL,    		/* 失败 */
	FPR_TIME_OUT,		/* 等待超时 */
	FPR_USER_INTR,	/* 用户中断(打断正常流程强制跳出当前循环) */
	FPR_EXISTED,    /* 指纹已存在 */
	FPR_FULL,       /* 指纹注册已满 */
	FPR_NULL = 0XFF,/* 无指纹 */
}FPRESULT;

#define RES_OK       (0)

/*指纹存储记录相关类型定义*/
typedef enum{
	FP_USER_ADMINS = 'A',     /* 管理员账户类型 */
	FP_USER_NORMAL = 'N',     /* 普通用户账户类型 */
}FP_USER_NAME;   /* 账户类型定义 */

void BLHPOWER_Initize(void);  /* 指纹传感器电源控制函数声明 */
void BLHPOWER_ON(void);
void BLHPOWER_OFF(void);
void BLH3001_Initize(void);
void BLH_INT_PinConfig(void);
FPRESULT FP_Imagecheck(unsigned int Arg, unsigned int *Error);  /* 指纹传感器通信协议函数声明 */
FPRESULT FP_SendFunInst(FP_DirectionDef Direction, unsigned int *Error);
FPRESULT FP_Enroll_D(unsigned int StartID,unsigned int EndID,unsigned int *FP_ID, unsigned int *Error); /* 指纹传感器用户接口函数声明 */
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








