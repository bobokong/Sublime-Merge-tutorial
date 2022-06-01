#include "timer.h"
#include "fp_uart_bsp.h"
#include "uart_tft_bsp.h"

/***串口通信数据缓存***/
FP_InstructPackDef InstructPack;

unsigned char g_ucFPCheckInst  = 0;   /* 用于标志串口接收完成，未完成时为0，完成时修改为1 */
unsigned char BLH_Rec_STU = 1;   /* BLH3001串口1接收状态机状态 */

/********************************************************************************
 * FunctionName:USART2_IRQHandler
 * @brief:UART1中断服务函数
 * @param:None
 * @retval:None
********************************************************************************/
void BLH_UART1_DataRecive(uint8_t DataRec)
{
	static unsigned char offset = 0;
	static unsigned char Totlen = 0;
	
	offset++;

	if((offset >7) && (offset == Totlen))
	{
		InstructPack.Instruct[sizeof(FP_DataPackDef)-1] = DataRec;
	}
	else
	{
		InstructPack.Instruct[offset-1] = DataRec;
	}
	if(InstructPack.Instruct[0] != 0x3A)
	{
		offset = 0;
	}
	if(offset == 6)
	{
		Totlen = htons(InstructPack.DataPack.PackLen.Len)+8;//得到总共要接收的字节数
	}

	if((offset >= Totlen) && (offset >= 8))
	{
		Totlen = 0;
		offset = 0;
		g_ucFPCheckInst = 1;
	}
}

/********************************************************************************
 * FunctionName:
 * @brief:初始化外设电源控制引脚,并关闭外设电源
 * @param:None
 * @retval:None
********************************************************************************/
void BLHPOWER_Initize(void)
{
	__NOP();
}
/********************************************************************************
 * FunctionName:
 * @brief:打开指纹传感器电源
 * @param:None
 * @retval:None
********************************************************************************/
void BLHPOWER_ON(void)
{
	__NOP();
}
/********************************************************************************
 * FunctionName:
 * @brief:关闭指纹传感器电源
 * @param:None
 * @retval:None
********************************************************************************/
void BLHPOWER_OFF(void)
{
	__NOP();
}

/****************************************************************************
*函		数：FP_SendInstruct
*入   参: Direction-指令参数
*返		回: 无
*功   能: 将用户指令打包并发送
*作   者：YangGuo
*时   间: 2014-5-7
****************************************************************************/
static void FP_SendInstruct(FP_DirectionDef Direction)
{
	uint8_t XOR,SUM,i;
	
	XOR = 0;
	SUM = 0;
	InstructPack.DataPack.PackHeader  = 0X3A;//包头 1byte
	InstructPack.DataPack.Instruction = Direction.Instruct;//指令 1byte
	InstructPack.DataPack.Ackinf.Ack  = htons(Direction.Fun_Arg); //功能参数 2byte
	InstructPack.DataPack.PackLen.Len	= htons(Direction.ExfeildLen);//包长 2byte
	
	/**  数据校验  **/
	for(i=0;i<6;i++)
	{
		XOR ^= InstructPack.Instruct[i];
		SUM += InstructPack.Instruct[i];
	}
	InstructPack.Instruct[i++] = XOR; //第7个字节为前六个字节的异或值
	SUM += XOR;//前7byte的算术和
	
	/***数据格式转换,保证小端模式下填充到buffer中的字节顺序正确***/
	Direction.Exfield.ExArg[0] = htons(Direction.Exfield.ExArg[0]);
	Direction.Exfield.ExArg[1] = htons(Direction.Exfield.ExArg[1]);
	
	for(;i<Direction.ExfeildLen+7;i++)//扩展域包长为0时，次循环不执行
	{
		InstructPack.Instruct[i] =  Direction.Exfield.ExDat[i-7];//将扩展域的数据填充到待发送队列中
		SUM += InstructPack.Instruct[i];//把扩展域添加到算术和中
	}
	/** 和校验值固定存放在最后一个字节 **/
	InstructPack.Instruct[sizeof(FP_DataPackDef)-1]= ~SUM;//算术和取反，填充到待发队列中
	
	/** 发送数据 **/
	for(i=0;i<7;i++)
	{
		UART_TFT_SendByte(InstructPack.Instruct[i]);	//发送前7个固定字节
	}
	for(;i<Direction.ExfeildLen+7;i++)		//如果扩展域无数据,此循环不执行
	{
		UART_TFT_SendByte(InstructPack.Instruct[i]);	//发送扩展域数据
	}
	UART_TFT_SendByte(InstructPack.Instruct[sizeof(FP_DataPackDef)-1]);//发送和校验值
	
	BLH_Rec_STU = BLH_Rec_STU_PackHead;    /* 清除串口接收状态机 */
}

/****************************************************************************
*函		数：FP_GetSensorAck
*入   参: 无
*返		回: 状态码
*功   能: 获取/处理应答数数据
*作   者：YangGuo
*时   间: 2014-5-7
****************************************************************************/
static uint16_t FP_GetSensorAck(void)
{
	uint8_t XOR,SUM,i;
	uint8_t Temp;
	if(gUartReady)
	{
		gUartReady = 0;
		XOR = 0;
		SUM = 0;		
		for(i=0; i<6; i++)
		{
			XOR ^= InstructPack.Instruct[i];
			SUM += InstructPack.Instruct[i];
		}
		if(XOR != InstructPack.DataPack.XOR)
		{
			return ERR_XOR;
		}
		else//如果异或校验正确则检查扩展域
		{
			SUM += InstructPack.Instruct[i++];//第七个字节
			for(;i<htons(InstructPack.DataPack.PackLen.Len)+7;i++)
			{
				SUM += InstructPack.Instruct[i];
			}
			SUM = ~SUM;
			/***将接收到的数据转换为结构体定义的意义***/
			Temp = InstructPack.Instruct[sizeof(FP_DataPackDef)-1];//将最后一个字节SUM读入缓存
			InstructPack.Instruct[11] = InstructPack.Instruct[10];
			InstructPack.Instruct[10] = InstructPack.Instruct[9];
			InstructPack.Instruct[9] 	= InstructPack.Instruct[8];
			InstructPack.Instruct[8] 	= InstructPack.Instruct[7];
			InstructPack.Instruct[7] 	= Temp;
			if(SUM != InstructPack.DataPack.SUM)
			{
				return ERR_SUM;
			}
			else
			{
				return ERR_SUCCESS;
			}
		}
	}
	else
	{
		return ERR_FAIL;
	}
}


/****************************************************************************
*函		数：FP_AnalysisAckCode
*入   参: 指令数据
*返		回: 0-失败，1-成功，其他:错误代码
*功   能: 解析返回的数据
*作   者：2014-5-7
*时   间: 2013-12-05
****************************************************************************/
static unsigned int FP_AnalysisAckCode(unsigned char Direction)
{
	if(Direction == InstructPack.DataPack.Instruction)//应答指令和所发指令一致
	{
		if(0X80 & InstructPack.DataPack.Ackinf.Dat[0])//应答码高位为1，出错
		{
			return (0X0FFF & htons(InstructPack.DataPack.Ackinf.Ack));//返回错误代码
		}
		else
		{
			return 0X1;//应答正确
		}
	}
	else
	{
		return 0;//应答指令不匹配
	}
}

/****************************************************************************
*函		数：FP_Imagecheck
*入   参: Arg-图像检测参数 Error-错误代码指针
*返		回: FPR_OK-检测到手指 FPR_FAIL-无手指 FPR_TIME_OUT-传感器无应答
*功   能: 图像检测方式判断有无手指
*作   者：YangGuo
*时   间: 2014-5-7
****************************************************************************/
FPRESULT FP_Imagecheck(unsigned int Arg, unsigned int *Error)
{
	FP_DirectionDef   Direction;
	uint32_t RunOutTimer = 0;

	Direction.Instruct   = DIR_IMAGE_SAMPLE;
	Direction.Fun_Arg    = Arg; 
	Direction.ExfeildLen = 0;
	
	FP_SendInstruct(Direction);
	RunOutTimer = BLH3001_TIMEOUT;  /* 超时处理初始化 */
	while(1)
	{
		TMR3_RUNOUT_Timer(&RunOutTimer);  /* 超时处理 */
		if(RunOutTimer == 0)
		{
			return FPR_TIME_OUT;//无应答
		}
		
		if(ERR_SUCCESS == FP_GetSensorAck())
		{
			*Error = FP_AnalysisAckCode(DIR_IMAGE_SAMPLE);
			switch(*Error)
			{
				case 0x01:return FPR_OK;   /* 注册成功 */
				case 0x201:return FPR_TIME_OUT;  /* 注册超时 */
				
				default:return FPR_FAIL;  /* 注册失败 */
			}
		}
	}
}


/****************************************************************************
*函		数：FP_SendFunInst
*入   参: Direction-指令和功能参数 Error-错误代码指针
*返		回: FPR_OK-成功  FPR_FAIL-失败  FPR_TIME_OUT-传感器无应答
*功   能: 检测到手指图像后发送指令完成目标功能并返回结果
*作   者：YangGuo
*时   间: 2014-5-7
****************************************************************************/
FPRESULT FP_SendFunInst(FP_DirectionDef Direction, unsigned int *Error)
{
	uint32_t RunOutTimer;
	BLHPOWER_ON();
	
	FP_SendInstruct(Direction);
	RunOutTimer = BLH3001_TIMEOUT;  /* 超时处理初始化 */
	while(1)
	{
		TMR3_RUNOUT_Timer(&RunOutTimer);  /* 超时处理 */
		if(RunOutTimer == 0)
		{
			return FPR_TIME_OUT;//无应答
		}
		if(ERR_SUCCESS == FP_GetSensorAck())
		{
			*Error = FP_AnalysisAckCode(Direction.Instruct);
			switch(*Error)
			{
				case 0x1:return FPR_OK;
				case 0x301:return FPR_EXISTED;
				case 0x304:return FPR_FULL;
				default:return FPR_FAIL;
			}
		}
	}	
}

/****************************************************************************
*函		数：FP_Enroll_D
*入   参: StartID-起始I,珽ndID终止ID,FP_ID-匹配ID,Error-错误代码指针
*返		回: FPR_FAIL--注册失败,FPR_OK-注册成功
*功   能: 指纹注册函数,双指令模式(分三步)
*作   者：YangGuo
*时   间: 2014-5-7
****************************************************************************/
FPRESULT FP_Enroll_D(unsigned int StartID,unsigned int EndID,unsigned int *FP_ID, unsigned int *Error)
{
		uint8_t EnrollCnt;
		FPRESULT Res = FPR_NULL;
		unsigned int Arg[3]={ARG_D_FIRST_ENROLL,ARG_D_SECOND_ENROLL,ARG_D_FINAL_ENROLL};
		unsigned int Arg_ImageCheck;
		FP_DirectionDef Direction;
		uint32_t RUN_TIME_cnt;
		uint16_t err;  /* 超时处理变量 */
		
		FP_TerminateSensor(&err);
		
		BLHPOWER_ON();
		
		Direction.Instruct   				= DIR_ENROLL_FP;
		Direction.ExfeildLen 				= 4;
		Direction.Exfield.ExArg[0] 	= StartID;
		Direction.Exfield.ExArg[1] 	= EndID;
		EnrollCnt										=	0;
		/* 注册 */
		while(1)
		{
				Arg_ImageCheck = (EnrollCnt==0 ? ARG_D_IMAGE_SAMPLE : ARG_D_IMAGE_CHECK);
				if(EnrollCnt == 0)
				{
//					NY3P_Play(21,823, true);   /* 语音"请按手指" */
				}
				else if(EnrollCnt>0)
				{
//					NY3P_Play(22,987, true);   /* 语音"请再按手指" */
				}
					
				Res = FP_Imagecheck(Arg_ImageCheck,Error);
				if(Res == FPR_OK)													//检测到手指图像
				{
					Direction.Fun_Arg    = Arg[EnrollCnt];	//更新功能参数
					Res = FP_SendFunInst(Direction, Error);	//完成本次操作
					if( Res == FPR_OK)
					{
						EnrollCnt++;
						if(3 == EnrollCnt)
						{
							*FP_ID = htons(InstructPack.DataPack.ExField.ExArg[0]);//读取注册指纹ID
//							NY3P_Play(24,2160, true);   /* 语音"注册成功，请更换手指" */
							BLHPOWER_OFF();
							return FPR_OK;
						}
					}
					else
					{
						BLHPOWER_OFF();
						return Res;
					}
					RUN_TIME_cnt = 2;
					while(1)
					{
						TMR3_RUNOUT_Timer(&RUN_TIME_cnt);  /* 超时处理 */
						
						if(RUN_TIME_cnt == 0)
						{
							if(true == UserGetBLH_INT())
							{
								RUN_TIME_cnt = 2;
//								NY3P_Play(23,1880,true);  /* 语音"请抬起手指重新放入" */
							}
						}
						if(false == UserGetBLH_INT())
						{
							break;
						}
					}
				}
				else
				{
					BLHPOWER_OFF();
					return Res;
				}
		}
}

/****************************************************************************
*函		数：Fp_match_D
*入   参: StartID-起始ID,EndID终止ID,FP_ID-匹配ID,Error-错误代码指针
*返		回: FPR_FAIL--注册失败,FPR_OK-注册成功
*功   能: 指纹比对函数,双指令模式
*作   者：YangGuo
*时   间: 2014-5-7
****************************************************************************/
FPRESULT Fp_match_D(unsigned int StarID, unsigned int EndID ,unsigned int *FP_ID, unsigned int *Error)
{
		FPRESULT Res = FPR_NULL;
		FP_DirectionDef Direction;
		uint16_t err;
	
	FP_TerminateSensor(&err);
	
		BLHPOWER_ON();
	
		/* 初始化指令数据域 */
		Direction.Instruct   = DIR_COMPARISON_FP;	
		Direction.Fun_Arg    = ARG_D_COMPARISON_FP;
		Direction.ExfeildLen = 4;
		Direction.Exfield.ExArg[0] = StarID;
		Direction.Exfield.ExArg[1] = EndID;
	
		Res = FP_Imagecheck(ARG_D_IMAGE_SAMPLE, Error);
		if(Res == FPR_OK)										//检测到手指图像
		{
			Res = FP_SendFunInst(Direction, Error);//完成比对功能
			if(Res == FPR_OK)
			{
				//比对成功
				*FP_ID = htons(InstructPack.DataPack.ExField.ExArg[0]);//读取比对指纹的ID号
				BLHPOWER_OFF();
				return FPR_OK;
			}			
		}
		
		BLHPOWER_OFF();
		return Res;
}

/****************************************************************************
*函		数：Fp_Delete
*入   参: StartID-起始ID, EndID终止ID, Error-错误代码指针
*返		回: FPR_FAIL--失败,FPR_OK-成功
*功   能: 指纹删除函数
*作   者：YangGuo
*时   间: 2014-5-7
****************************************************************************/
FPRESULT Fp_Delete(unsigned int StartID, unsigned int EndID, unsigned int *Error)
{
	uint32_t RunOutTimer;
	FP_DirectionDef Direction;
	uint16_t err;
	
	FP_TerminateSensor(&err);
	
	BLHPOWER_ON();

	Direction.Instruct   				= DIR_DELETE_FP;
	Direction.Fun_Arg    				= ARG_DELETE_FP;
	Direction.ExfeildLen 				= 4;
	Direction.Exfield.ExArg[0] 	= StartID;
	Direction.Exfield.ExArg[1] 	= EndID;
	
	FP_SendInstruct(Direction);	
	RunOutTimer = BLH3001_TIMEOUT;  /* 超时处理初始化 */
	while(1)
	{
		TMR3_RUNOUT_Timer(&RunOutTimer);  /* 超时处理 */
		if(RunOutTimer == 0)
		{
			BLHPOWER_OFF();
			return FPR_TIME_OUT;//无应答
		}
		if(ERR_SUCCESS == FP_GetSensorAck())
		{
			*Error = FP_AnalysisAckCode(DIR_DELETE_FP);
			if(0x1 == *Error)
			{
				BLHPOWER_OFF();
				return FPR_OK;//删除成功
			}
			else
			{
				BLHPOWER_OFF();
				return FPR_FAIL;//删除失败
			}
		}			
	}
}

/****************************************************************************
*函		数：FP_GetFPQuatity
*入   参: StartID-起始ID,EndID终止ID,Quatity-指纹数量,Error-错误代码指针
*返		回: FPR_OK-查询成功,FPR_FAIL-查询失败,FPR_TIME_OUT-超时
*功   能:	查询指定范围内的指纹数量
*作   者：YangGuo
*时   间: 2014-5-7
****************************************************************************/
FPRESULT FP_GetFPQuatity(unsigned int StartID,unsigned int EndID, unsigned int *Q, unsigned int *Error)
{
	uint32_t RunOutTimer;
	FP_DirectionDef Direction;
	uint16_t err;
		
	FP_TerminateSensor(&err);
	
	BLHPOWER_ON();
	
	Direction.Instruct   				= DIR_SYS_CONFIG;
	Direction.Fun_Arg    				= ARG_FP_QUANTITY;
	Direction.ExfeildLen 				= 4;
	Direction.Exfield.ExArg[0] 	= StartID;
	Direction.Exfield.ExArg[1] 	= EndID;
	
	FP_SendInstruct(Direction);
	RunOutTimer = BLH3001_TIMEOUT;  /* 超时处理初始化 */
	while(1)
	{
		TMR3_RUNOUT_Timer(&RunOutTimer);  /* 超时处理 */
		if(RunOutTimer == 0)
		{
			BLHPOWER_OFF();
			return FPR_TIME_OUT;//无应答
		}
		if(ERR_SUCCESS == FP_GetSensorAck())
		{
			*Error = FP_AnalysisAckCode(DIR_SYS_CONFIG);
			if(0x1 == *Error)
			{
				*Q =  htons(InstructPack.DataPack.ExField.ExArg[0]);	//读取指纹数量		
				BLHPOWER_OFF();
				return FPR_OK;
			}
			else
			{			
				BLHPOWER_OFF();
				return FPR_FAIL;
			}
		}			
	}
}

/****************************************************************************
*函		数：FP_GetUnusedMin_ID
*入   参: StartID-起始ID,EndID终止ID,MinID-指定区间的未注册最小ID,Error-错误代码指针
*返		回: FPR_OK-查询成功,FPR_FAIL-查询失败,FPR_TIME_OUT-超时
*功   能:	查询指定范围内的未注册的最小的一个指纹ID
*作   者：YangGuo
*时   间: 2014-6-5
****************************************************************************/
FPRESULT FP_GetUnused_MinID(unsigned int StartID,unsigned int EndID, unsigned int *MinID, unsigned int *Error)
{
	uint32_t RunOutTimer;
	FP_DirectionDef Direction;
	uint16_t err;
	
	FP_TerminateSensor(&err);
	
	BLHPOWER_ON();
	
	Direction.Instruct   				= DIR_SYS_CONFIG;
	Direction.Fun_Arg    				= ARG_UNUSED_MINID;
	Direction.ExfeildLen 				= 4;
	Direction.Exfield.ExArg[0] 	= StartID;
	Direction.Exfield.ExArg[1] 	= EndID;
	
	FP_SendInstruct(Direction);
	RunOutTimer = BLH3001_TIMEOUT;  /* 超时处理初始化 */
	while(1)
	{
		TMR3_RUNOUT_Timer(&RunOutTimer);  /* 超时处理 */
		if(RunOutTimer == 0)
		{
			BLHPOWER_OFF();
			return FPR_TIME_OUT;//无应答
		}
		if(ERR_SUCCESS == FP_GetSensorAck())
		{
			*Error = FP_AnalysisAckCode(DIR_SYS_CONFIG);
			if(0x1 == *Error)
			{
				*MinID =  htons(InstructPack.DataPack.ExField.ExArg[0]);	//读取指纹数量		
				BLHPOWER_OFF();
				return FPR_OK;
			}
			else
			{			
				BLHPOWER_OFF();
				return FPR_FAIL;
			}
		}			
	}
}
/****************************************************************************
*函		数：FP_TerminateSensor
*入   参: Error-错误代码指针
*返		回: FPR_OK-成功,FPR_FAIL-失败,FPR_TIME_OUT-超时
*功   能:	发送终止指令,终止当前操作
*作   者：YangGuo
*时   间: 2014-5-7
****************************************************************************/
FPRESULT FP_TerminateSensor(uint16_t *Error)
{
	uint32_t RunOutTimer;
	FP_DirectionDef Direction;
	
	BLHPOWER_ON();
	
	Direction.Instruct   = DIR_TERMINATE;
	Direction.Fun_Arg    = ARG_TERMINATE;
	Direction.ExfeildLen = 0;
	
	FP_SendInstruct(Direction);
	RunOutTimer = BLH3001_TIMEOUT;  /* 超时处理初始化 */
	while(1)
	{
		TMR3_RUNOUT_Timer(&RunOutTimer);  /* 超时处理 */
		if(RunOutTimer == 0)
		{
			BLHPOWER_OFF();
			return FPR_TIME_OUT;//无应答
		}
		if(ERR_SUCCESS == FP_GetSensorAck())
		{
			*Error = FP_AnalysisAckCode(DIR_TERMINATE);
			if(0x1 == *Error)
			{
				BLHPOWER_OFF();
				return FPR_OK;//终止成功
			}
			else
			{
				BLHPOWER_OFF();
				return FPR_FAIL;//失败
			}
		}			
	}
}


/****************************************************************************
*函		数：DeleteSpecific_FP
*入   参: Error-错误代码指针
*返		回: RES_OK-成功,RES_FAIL-失败,RES_TIME_OUT-超时
*功   能:	删除比对通过的指纹
*作   者：YangGuo
*时   间: 2014-5-9
****************************************************************************/
FPRESULT  DeleteSpecific_FP(unsigned int *Error)
{
	FPRESULT Res;
	unsigned int FP_ID;
	
	BLHPOWER_ON();
	
	Res = Fp_match_D(FP_ID_START, FP_ID_END,&FP_ID,Error);//通过比对验证找出该用户指纹ID
	if(Res == RES_OK) //验证成功
	{
		Res = Fp_Delete(FP_ID, FP_ID, Error);
		if(Res == RES_OK)//删除成功
		{
			BLHPOWER_OFF();
			return Res;
		}
		else
		{
			BLHPOWER_OFF();
			return Res;
		}
	}
	else
	{
		BLHPOWER_OFF();
		return Res;
	}
}

/****************************************************************************
*函		数：GetRemanent_FP
*入   参: Rem_Num-剩余指纹数量指针 Error-错误代码指针
*返		回: RES_OK-成功,RES_FAIL-失败,RES_TIME_OUT-超时
*功   能:	查询传感器中剩余的指纹数量
*作   者：YangGuo
*时   间: 2014-5-9
****************************************************************************/
FPRESULT  GetRemanent_FP(unsigned int *Rem_Num, unsigned int *Error)
{
	FPRESULT   Res;
	unsigned int  Num;
	Res = FP_GetFPQuatity(FP_ID_START, FP_ID_END,&Num,Error);//查询传感器中已经注册的指纹数量
	if(Res == RES_OK)//成功
	{
		*Rem_Num = (FP_TOTLE_NUM - Num);//计算剩余指纹容量空间
		return Res;
	}
	else
	{
		return Res;
	}
}
/****************************************************************************
*函		数：BLH_INT_PinConfig
*入   参: None
*返		回: None
*功   能:	初始化指纺传感器触摸检测输出引脚
*作   者：ChenTian 
*时   间: 2014-5-9
****************************************************************************/
void BLH_INT_PinConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//读摸出感应状态(触摸感应时输出高电平信号)
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//使能PORTA,PORTE时钟
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;//GPIO_Pin_1KEY0-KEY1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;//输入下拉模式
 	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOE4,3
}
/****************************************************************************
*函		数：BLH3001_Initize
*入   参: None
*返		回: None
*功   能:	初始化指纹传感器相关的电源控制，中断输出唤醒功能，通信端口，超时定时器
*作   者：ChenTian 
*时   间: 2014-5-9
****************************************************************************/
void BLH3001_Initize(void)
{
	BLH_INT_PinConfig();
	BLHPOWER_Initize();/* 初始化电源引脚 */
}






















#if(TEPMLE_UPDOWNLOAD_EN)
/****************************************************************************
*函		数：FP_Download_Template
*入   参: FP_ID-指纹ID, Temp_ID-模板ID号, 
*						Ctrl-模板区域控制字(0-前256字节，1-后256字节)
*						pBuff-模板数据指针, Error-错误代码指针
*返		回: FPR_OK-下载成功,FPR_FAIL-下载失败,FPR_TIME_OUT-超时
*功   能:	下载指定模板
*作   者：YangGuo
*时   间: 2014-5-7
****************************************************************************/
FPRESULT FP_Download_Template(UINT16 FP_ID,UINT8 Temp_ID,UINT8 Ctrl,UINT8 *pBuff, UINT16 *Error)
{
	UINT8 *Ptr;
	UINT16 i;
	FP_DirectionDef Direction;
	
	Direction.Instruct   				= DIR_TEMPLATE_LOAD;
	Direction.Fun_Arg    				= ARG_TEMPLATE_DOWNLOAD;
	Direction.ExfeildLen 				= 4;
	Direction.Exfield.ExArg[0] 	= FP_ID;
// 	Direction.Exfield.ExDat[2] 	= Temp_ID;
// 	Direction.Exfield.ExDat[3] 	= Ctrl;
	Direction.Exfield.ExArg[1]  =  ((UINT16)(Temp_ID)&0X00FF)<<8|((UINT16)(Ctrl)&0X00FF);
	
	FP_SendInstruct(Direction);
	Start_TMR(BLH3001_TIMEOUT);	
	while(1)
	{
		if(Timer2OverFlag)
		{
			TIM2_Timing_Off();
			
			return FPR_TIME_OUT;//无应答
		}
		if(ERR_SUCCESS == FP_GetSensorAck())
		{
			TIM2_Timing_Off();
			*Error = FP_AnalysisAckCode(DIR_TEMPLATE_LOAD);
			if(0x1 == *Error)
			{
				//应答正确,准备传输模板数据
#if (TEMPLE_SIZE_ONE_TX ==512)				
				Ptr = (Ctrl==0 ? pBuff : pBuff+256);//根据所选下载数据区域设置指针
#elif (TEMPLE_SIZE_ONE_TX ==256)
				Ptr = pBuff;
#endif				
				for(i=0;i<=255;i++)
				{
					SendByte(Ptr[i]);
				}
				//模板发送完成等待应答
				Start_TMR(BLH3001_TIMEOUT);	
				while(1)
				{
						if(Timer2OverFlag)
						{
							TIM2_Timing_Off();
							
							return FPR_TIME_OUT;//无应答
						}
						if(ERR_SUCCESS == FP_GetSensorAck())
						{
							TIM2_Timing_Off();
							*Error = FP_AnalysisAckCode(DIR_TEMPLATE_LOAD);
							if(0x1 == *Error)
							{	
								return FPR_OK;
							}
							else
							{
								return FPR_FAIL;
							}
						}
				}
			}
			else
			{
				return FPR_FAIL;//失败
			}
		}		
	}
}

/****************************************************************************
*函		数：FP_Upload_Template
*入   参: FP_ID-指纹ID, Temp_ID-模板ID号, 
*						Ctrl-模板区域控制字(0-前256字节，1-后256字节)
*						Error-错误代码指针
*返		回: FPR_OK-上传成功,FPR_FAIL-上传失败,FPR_TIME_OUT-超时
*功   能:	上传指定模板
*作   者：YangGuo
*时   间: 2014-5-7
****************************************************************************/
FPRESULT FP_Upload_Template(UINT16 FP_ID,UINT8 Temp_ID,UINT8 Ctrl,UINT16 *Error)
{
	FP_DirectionDef Direction;
	
	Direction.Instruct   				= DIR_TEMPLATE_LOAD;
	Direction.Fun_Arg    				= ARG_TEMPLATE_UPLOAD;
	Direction.ExfeildLen 				= 4;
	Direction.Exfield.ExArg[0] 	= FP_ID;
	Direction.Exfield.ExArg[1]  =  ((UINT16)(Temp_ID)&0X00FF)<<8|((UINT16)(Ctrl)&0X00FF);
	
	FP_SendInstruct(Direction);
	Start_TMR(BLH3001_TIMEOUT);	
	while(1)
	{
		if(Timer2OverFlag)
		{
			TIM2_Timing_Off();
			
			return FPR_TIME_OUT;//无应答
		}
		if(ERR_SUCCESS == FP_GetSensorAck())
		{
			TIM2_Timing_Off();
			*Error = FP_AnalysisAckCode(DIR_TEMPLATE_LOAD);
			if(0x1 == *Error)
			{
				//应答成功,准备读取模板数据
				//使用一个全局数据在串口中断里接收模板
				//用户在串口中断处理中完成模板数据(256byte)接收后应该将gTempUplod置1
				Start_TMR(BLH3001_TIMEOUT);	
				while(!gTempUplod)//等待模板数据接收完成
				{
						if(Timer2OverFlag)
						{
							TIM2_Timing_Off();
							
							return FPR_TIME_OUT;//无应答
						}
				}
				//模板发送完成等待应答
				TIM2_Timing_Off();//数据在规定时间内接收完成，关闭定时器
				Start_TMR(BLH3001_TIMEOUT);	//重启定时器等待应答
				while(1)
				{
						if(Timer2OverFlag)
						{
							TIM2_Timing_Off();
							
							return FPR_TIME_OUT;//无应答
						}
						if(ERR_SUCCESS == FP_GetSensorAck())
						{
							TIM2_Timing_Off();
							*Error = FP_AnalysisAckCode(DIR_TEMPLATE_LOAD);
							if(0x1 == *Error)
							{	
								return FPR_OK;
							}
							else
							{
								return FPR_FAIL;
							}
						}
				}
			}
			else
			{
				return FPR_FAIL;//失败
			}
		}
	}
}

#endif



/************************************************************************************
@函数名称: UART3_IRQHandler
@函数功能: 指纹串口中断服务函数
@参    数: None
@返    回: None
@作    者: veiko
@公    司: 艺嵌科技
@时    间: 2015-7-16
************************************************************************************/
void UART3_IRQHandler(void)
{
	unsigned char data = 0;
	data = USART_ReceiveData(USART3);
	BLH_UART1_DataRecive(data);
}























