#include "timer.h"
#include "fp_uart_bsp.h"
#include "uart_tft_bsp.h"

/***����ͨ�����ݻ���***/
FP_InstructPackDef InstructPack;

unsigned char g_ucFPCheckInst  = 0;   /* ���ڱ�־���ڽ�����ɣ�δ���ʱΪ0�����ʱ�޸�Ϊ1 */
unsigned char BLH_Rec_STU = 1;   /* BLH3001����1����״̬��״̬ */

/********************************************************************************
 * FunctionName:USART2_IRQHandler
 * @brief:UART1�жϷ�����
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
		Totlen = htons(InstructPack.DataPack.PackLen.Len)+8;//�õ��ܹ�Ҫ���յ��ֽ���
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
 * @brief:��ʼ�������Դ��������,���ر������Դ
 * @param:None
 * @retval:None
********************************************************************************/
void BLHPOWER_Initize(void)
{
	__NOP();
}
/********************************************************************************
 * FunctionName:
 * @brief:��ָ�ƴ�������Դ
 * @param:None
 * @retval:None
********************************************************************************/
void BLHPOWER_ON(void)
{
	__NOP();
}
/********************************************************************************
 * FunctionName:
 * @brief:�ر�ָ�ƴ�������Դ
 * @param:None
 * @retval:None
********************************************************************************/
void BLHPOWER_OFF(void)
{
	__NOP();
}

/****************************************************************************
*��		����FP_SendInstruct
*��   ��: Direction-ָ�����
*��		��: ��
*��   ��: ���û�ָ����������
*��   �ߣ�YangGuo
*ʱ   ��: 2014-5-7
****************************************************************************/
static void FP_SendInstruct(FP_DirectionDef Direction)
{
	uint8_t XOR,SUM,i;
	
	XOR = 0;
	SUM = 0;
	InstructPack.DataPack.PackHeader  = 0X3A;//��ͷ 1byte
	InstructPack.DataPack.Instruction = Direction.Instruct;//ָ�� 1byte
	InstructPack.DataPack.Ackinf.Ack  = htons(Direction.Fun_Arg); //���ܲ��� 2byte
	InstructPack.DataPack.PackLen.Len	= htons(Direction.ExfeildLen);//���� 2byte
	
	/**  ����У��  **/
	for(i=0;i<6;i++)
	{
		XOR ^= InstructPack.Instruct[i];
		SUM += InstructPack.Instruct[i];
	}
	InstructPack.Instruct[i++] = XOR; //��7���ֽ�Ϊǰ�����ֽڵ����ֵ
	SUM += XOR;//ǰ7byte��������
	
	/***���ݸ�ʽת��,��֤С��ģʽ����䵽buffer�е��ֽ�˳����ȷ***/
	Direction.Exfield.ExArg[0] = htons(Direction.Exfield.ExArg[0]);
	Direction.Exfield.ExArg[1] = htons(Direction.Exfield.ExArg[1]);
	
	for(;i<Direction.ExfeildLen+7;i++)//��չ�����Ϊ0ʱ����ѭ����ִ��
	{
		InstructPack.Instruct[i] =  Direction.Exfield.ExDat[i-7];//����չ���������䵽�����Ͷ�����
		SUM += InstructPack.Instruct[i];//����չ����ӵ���������
	}
	/** ��У��ֵ�̶���������һ���ֽ� **/
	InstructPack.Instruct[sizeof(FP_DataPackDef)-1]= ~SUM;//������ȡ������䵽����������
	
	/** �������� **/
	for(i=0;i<7;i++)
	{
		UART_TFT_SendByte(InstructPack.Instruct[i]);	//����ǰ7���̶��ֽ�
	}
	for(;i<Direction.ExfeildLen+7;i++)		//�����չ��������,��ѭ����ִ��
	{
		UART_TFT_SendByte(InstructPack.Instruct[i]);	//������չ������
	}
	UART_TFT_SendByte(InstructPack.Instruct[sizeof(FP_DataPackDef)-1]);//���ͺ�У��ֵ
	
	BLH_Rec_STU = BLH_Rec_STU_PackHead;    /* ������ڽ���״̬�� */
}

/****************************************************************************
*��		����FP_GetSensorAck
*��   ��: ��
*��		��: ״̬��
*��   ��: ��ȡ/����Ӧ��������
*��   �ߣ�YangGuo
*ʱ   ��: 2014-5-7
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
		else//������У����ȷ������չ��
		{
			SUM += InstructPack.Instruct[i++];//���߸��ֽ�
			for(;i<htons(InstructPack.DataPack.PackLen.Len)+7;i++)
			{
				SUM += InstructPack.Instruct[i];
			}
			SUM = ~SUM;
			/***�����յ�������ת��Ϊ�ṹ�嶨�������***/
			Temp = InstructPack.Instruct[sizeof(FP_DataPackDef)-1];//�����һ���ֽ�SUM���뻺��
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
*��		����FP_AnalysisAckCode
*��   ��: ָ������
*��		��: 0-ʧ�ܣ�1-�ɹ�������:�������
*��   ��: �������ص�����
*��   �ߣ�2014-5-7
*ʱ   ��: 2013-12-05
****************************************************************************/
static unsigned int FP_AnalysisAckCode(unsigned char Direction)
{
	if(Direction == InstructPack.DataPack.Instruction)//Ӧ��ָ�������ָ��һ��
	{
		if(0X80 & InstructPack.DataPack.Ackinf.Dat[0])//Ӧ�����λΪ1������
		{
			return (0X0FFF & htons(InstructPack.DataPack.Ackinf.Ack));//���ش������
		}
		else
		{
			return 0X1;//Ӧ����ȷ
		}
	}
	else
	{
		return 0;//Ӧ��ָ�ƥ��
	}
}

/****************************************************************************
*��		����FP_Imagecheck
*��   ��: Arg-ͼ������� Error-�������ָ��
*��		��: FPR_OK-��⵽��ָ FPR_FAIL-����ָ FPR_TIME_OUT-��������Ӧ��
*��   ��: ͼ���ⷽʽ�ж�������ָ
*��   �ߣ�YangGuo
*ʱ   ��: 2014-5-7
****************************************************************************/
FPRESULT FP_Imagecheck(unsigned int Arg, unsigned int *Error)
{
	FP_DirectionDef   Direction;
	uint32_t RunOutTimer = 0;

	Direction.Instruct   = DIR_IMAGE_SAMPLE;
	Direction.Fun_Arg    = Arg; 
	Direction.ExfeildLen = 0;
	
	FP_SendInstruct(Direction);
	RunOutTimer = BLH3001_TIMEOUT;  /* ��ʱ�����ʼ�� */
	while(1)
	{
		TMR3_RUNOUT_Timer(&RunOutTimer);  /* ��ʱ���� */
		if(RunOutTimer == 0)
		{
			return FPR_TIME_OUT;//��Ӧ��
		}
		
		if(ERR_SUCCESS == FP_GetSensorAck())
		{
			*Error = FP_AnalysisAckCode(DIR_IMAGE_SAMPLE);
			switch(*Error)
			{
				case 0x01:return FPR_OK;   /* ע��ɹ� */
				case 0x201:return FPR_TIME_OUT;  /* ע�ᳬʱ */
				
				default:return FPR_FAIL;  /* ע��ʧ�� */
			}
		}
	}
}


/****************************************************************************
*��		����FP_SendFunInst
*��   ��: Direction-ָ��͹��ܲ��� Error-�������ָ��
*��		��: FPR_OK-�ɹ�  FPR_FAIL-ʧ��  FPR_TIME_OUT-��������Ӧ��
*��   ��: ��⵽��ָͼ�����ָ�����Ŀ�깦�ܲ����ؽ��
*��   �ߣ�YangGuo
*ʱ   ��: 2014-5-7
****************************************************************************/
FPRESULT FP_SendFunInst(FP_DirectionDef Direction, unsigned int *Error)
{
	uint32_t RunOutTimer;
	BLHPOWER_ON();
	
	FP_SendInstruct(Direction);
	RunOutTimer = BLH3001_TIMEOUT;  /* ��ʱ�����ʼ�� */
	while(1)
	{
		TMR3_RUNOUT_Timer(&RunOutTimer);  /* ��ʱ���� */
		if(RunOutTimer == 0)
		{
			return FPR_TIME_OUT;//��Ӧ��
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
*��		����FP_Enroll_D
*��   ��: StartID-��ʼI,�EndID��ֹID,FP_ID-ƥ��ID,Error-�������ָ��
*��		��: FPR_FAIL--ע��ʧ��,FPR_OK-ע��ɹ�
*��   ��: ָ��ע�ắ��,˫ָ��ģʽ(������)
*��   �ߣ�YangGuo
*ʱ   ��: 2014-5-7
****************************************************************************/
FPRESULT FP_Enroll_D(unsigned int StartID,unsigned int EndID,unsigned int *FP_ID, unsigned int *Error)
{
		uint8_t EnrollCnt;
		FPRESULT Res = FPR_NULL;
		unsigned int Arg[3]={ARG_D_FIRST_ENROLL,ARG_D_SECOND_ENROLL,ARG_D_FINAL_ENROLL};
		unsigned int Arg_ImageCheck;
		FP_DirectionDef Direction;
		uint32_t RUN_TIME_cnt;
		uint16_t err;  /* ��ʱ������� */
		
		FP_TerminateSensor(&err);
		
		BLHPOWER_ON();
		
		Direction.Instruct   				= DIR_ENROLL_FP;
		Direction.ExfeildLen 				= 4;
		Direction.Exfield.ExArg[0] 	= StartID;
		Direction.Exfield.ExArg[1] 	= EndID;
		EnrollCnt										=	0;
		/* ע�� */
		while(1)
		{
				Arg_ImageCheck = (EnrollCnt==0 ? ARG_D_IMAGE_SAMPLE : ARG_D_IMAGE_CHECK);
				if(EnrollCnt == 0)
				{
//					NY3P_Play(21,823, true);   /* ����"�밴��ָ" */
				}
				else if(EnrollCnt>0)
				{
//					NY3P_Play(22,987, true);   /* ����"���ٰ���ָ" */
				}
					
				Res = FP_Imagecheck(Arg_ImageCheck,Error);
				if(Res == FPR_OK)													//��⵽��ָͼ��
				{
					Direction.Fun_Arg    = Arg[EnrollCnt];	//���¹��ܲ���
					Res = FP_SendFunInst(Direction, Error);	//��ɱ��β���
					if( Res == FPR_OK)
					{
						EnrollCnt++;
						if(3 == EnrollCnt)
						{
							*FP_ID = htons(InstructPack.DataPack.ExField.ExArg[0]);//��ȡע��ָ��ID
//							NY3P_Play(24,2160, true);   /* ����"ע��ɹ����������ָ" */
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
						TMR3_RUNOUT_Timer(&RUN_TIME_cnt);  /* ��ʱ���� */
						
						if(RUN_TIME_cnt == 0)
						{
							if(true == UserGetBLH_INT())
							{
								RUN_TIME_cnt = 2;
//								NY3P_Play(23,1880,true);  /* ����"��̧����ָ���·���" */
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
*��		����Fp_match_D
*��   ��: StartID-��ʼID,EndID��ֹID,FP_ID-ƥ��ID,Error-�������ָ��
*��		��: FPR_FAIL--ע��ʧ��,FPR_OK-ע��ɹ�
*��   ��: ָ�ƱȶԺ���,˫ָ��ģʽ
*��   �ߣ�YangGuo
*ʱ   ��: 2014-5-7
****************************************************************************/
FPRESULT Fp_match_D(unsigned int StarID, unsigned int EndID ,unsigned int *FP_ID, unsigned int *Error)
{
		FPRESULT Res = FPR_NULL;
		FP_DirectionDef Direction;
		uint16_t err;
	
	FP_TerminateSensor(&err);
	
		BLHPOWER_ON();
	
		/* ��ʼ��ָ�������� */
		Direction.Instruct   = DIR_COMPARISON_FP;	
		Direction.Fun_Arg    = ARG_D_COMPARISON_FP;
		Direction.ExfeildLen = 4;
		Direction.Exfield.ExArg[0] = StarID;
		Direction.Exfield.ExArg[1] = EndID;
	
		Res = FP_Imagecheck(ARG_D_IMAGE_SAMPLE, Error);
		if(Res == FPR_OK)										//��⵽��ָͼ��
		{
			Res = FP_SendFunInst(Direction, Error);//��ɱȶԹ���
			if(Res == FPR_OK)
			{
				//�ȶԳɹ�
				*FP_ID = htons(InstructPack.DataPack.ExField.ExArg[0]);//��ȡ�ȶ�ָ�Ƶ�ID��
				BLHPOWER_OFF();
				return FPR_OK;
			}			
		}
		
		BLHPOWER_OFF();
		return Res;
}

/****************************************************************************
*��		����Fp_Delete
*��   ��: StartID-��ʼID, EndID��ֹID, Error-�������ָ��
*��		��: FPR_FAIL--ʧ��,FPR_OK-�ɹ�
*��   ��: ָ��ɾ������
*��   �ߣ�YangGuo
*ʱ   ��: 2014-5-7
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
	RunOutTimer = BLH3001_TIMEOUT;  /* ��ʱ�����ʼ�� */
	while(1)
	{
		TMR3_RUNOUT_Timer(&RunOutTimer);  /* ��ʱ���� */
		if(RunOutTimer == 0)
		{
			BLHPOWER_OFF();
			return FPR_TIME_OUT;//��Ӧ��
		}
		if(ERR_SUCCESS == FP_GetSensorAck())
		{
			*Error = FP_AnalysisAckCode(DIR_DELETE_FP);
			if(0x1 == *Error)
			{
				BLHPOWER_OFF();
				return FPR_OK;//ɾ���ɹ�
			}
			else
			{
				BLHPOWER_OFF();
				return FPR_FAIL;//ɾ��ʧ��
			}
		}			
	}
}

/****************************************************************************
*��		����FP_GetFPQuatity
*��   ��: StartID-��ʼID,EndID��ֹID,Quatity-ָ������,Error-�������ָ��
*��		��: FPR_OK-��ѯ�ɹ�,FPR_FAIL-��ѯʧ��,FPR_TIME_OUT-��ʱ
*��   ��:	��ѯָ����Χ�ڵ�ָ������
*��   �ߣ�YangGuo
*ʱ   ��: 2014-5-7
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
	RunOutTimer = BLH3001_TIMEOUT;  /* ��ʱ�����ʼ�� */
	while(1)
	{
		TMR3_RUNOUT_Timer(&RunOutTimer);  /* ��ʱ���� */
		if(RunOutTimer == 0)
		{
			BLHPOWER_OFF();
			return FPR_TIME_OUT;//��Ӧ��
		}
		if(ERR_SUCCESS == FP_GetSensorAck())
		{
			*Error = FP_AnalysisAckCode(DIR_SYS_CONFIG);
			if(0x1 == *Error)
			{
				*Q =  htons(InstructPack.DataPack.ExField.ExArg[0]);	//��ȡָ������		
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
*��		����FP_GetUnusedMin_ID
*��   ��: StartID-��ʼID,EndID��ֹID,MinID-ָ�������δע����СID,Error-�������ָ��
*��		��: FPR_OK-��ѯ�ɹ�,FPR_FAIL-��ѯʧ��,FPR_TIME_OUT-��ʱ
*��   ��:	��ѯָ����Χ�ڵ�δע�����С��һ��ָ��ID
*��   �ߣ�YangGuo
*ʱ   ��: 2014-6-5
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
	RunOutTimer = BLH3001_TIMEOUT;  /* ��ʱ�����ʼ�� */
	while(1)
	{
		TMR3_RUNOUT_Timer(&RunOutTimer);  /* ��ʱ���� */
		if(RunOutTimer == 0)
		{
			BLHPOWER_OFF();
			return FPR_TIME_OUT;//��Ӧ��
		}
		if(ERR_SUCCESS == FP_GetSensorAck())
		{
			*Error = FP_AnalysisAckCode(DIR_SYS_CONFIG);
			if(0x1 == *Error)
			{
				*MinID =  htons(InstructPack.DataPack.ExField.ExArg[0]);	//��ȡָ������		
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
*��		����FP_TerminateSensor
*��   ��: Error-�������ָ��
*��		��: FPR_OK-�ɹ�,FPR_FAIL-ʧ��,FPR_TIME_OUT-��ʱ
*��   ��:	������ָֹ��,��ֹ��ǰ����
*��   �ߣ�YangGuo
*ʱ   ��: 2014-5-7
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
	RunOutTimer = BLH3001_TIMEOUT;  /* ��ʱ�����ʼ�� */
	while(1)
	{
		TMR3_RUNOUT_Timer(&RunOutTimer);  /* ��ʱ���� */
		if(RunOutTimer == 0)
		{
			BLHPOWER_OFF();
			return FPR_TIME_OUT;//��Ӧ��
		}
		if(ERR_SUCCESS == FP_GetSensorAck())
		{
			*Error = FP_AnalysisAckCode(DIR_TERMINATE);
			if(0x1 == *Error)
			{
				BLHPOWER_OFF();
				return FPR_OK;//��ֹ�ɹ�
			}
			else
			{
				BLHPOWER_OFF();
				return FPR_FAIL;//ʧ��
			}
		}			
	}
}


/****************************************************************************
*��		����DeleteSpecific_FP
*��   ��: Error-�������ָ��
*��		��: RES_OK-�ɹ�,RES_FAIL-ʧ��,RES_TIME_OUT-��ʱ
*��   ��:	ɾ���ȶ�ͨ����ָ��
*��   �ߣ�YangGuo
*ʱ   ��: 2014-5-9
****************************************************************************/
FPRESULT  DeleteSpecific_FP(unsigned int *Error)
{
	FPRESULT Res;
	unsigned int FP_ID;
	
	BLHPOWER_ON();
	
	Res = Fp_match_D(FP_ID_START, FP_ID_END,&FP_ID,Error);//ͨ���ȶ���֤�ҳ����û�ָ��ID
	if(Res == RES_OK) //��֤�ɹ�
	{
		Res = Fp_Delete(FP_ID, FP_ID, Error);
		if(Res == RES_OK)//ɾ���ɹ�
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
*��		����GetRemanent_FP
*��   ��: Rem_Num-ʣ��ָ������ָ�� Error-�������ָ��
*��		��: RES_OK-�ɹ�,RES_FAIL-ʧ��,RES_TIME_OUT-��ʱ
*��   ��:	��ѯ��������ʣ���ָ������
*��   �ߣ�YangGuo
*ʱ   ��: 2014-5-9
****************************************************************************/
FPRESULT  GetRemanent_FP(unsigned int *Rem_Num, unsigned int *Error)
{
	FPRESULT   Res;
	unsigned int  Num;
	Res = FP_GetFPQuatity(FP_ID_START, FP_ID_END,&Num,Error);//��ѯ���������Ѿ�ע���ָ������
	if(Res == RES_OK)//�ɹ�
	{
		*Rem_Num = (FP_TOTLE_NUM - Num);//����ʣ��ָ�������ռ�
		return Res;
	}
	else
	{
		return Res;
	}
}
/****************************************************************************
*��		����BLH_INT_PinConfig
*��   ��: None
*��		��: None
*��   ��:	��ʼ��ָ�Ĵ�������������������
*��   �ߣ�ChenTian 
*ʱ   ��: 2014-5-9
****************************************************************************/
void BLH_INT_PinConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//��������Ӧ״̬(������Ӧʱ����ߵ�ƽ�ź�)
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//ʹ��PORTA,PORTEʱ��
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;//GPIO_Pin_1KEY0-KEY1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;//��������ģʽ
 	GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIOE4,3
}
/****************************************************************************
*��		����BLH3001_Initize
*��   ��: None
*��		��: None
*��   ��:	��ʼ��ָ�ƴ�������صĵ�Դ���ƣ��ж�������ѹ��ܣ�ͨ�Ŷ˿ڣ���ʱ��ʱ��
*��   �ߣ�ChenTian 
*ʱ   ��: 2014-5-9
****************************************************************************/
void BLH3001_Initize(void)
{
	BLH_INT_PinConfig();
	BLHPOWER_Initize();/* ��ʼ����Դ���� */
}






















#if(TEPMLE_UPDOWNLOAD_EN)
/****************************************************************************
*��		����FP_Download_Template
*��   ��: FP_ID-ָ��ID, Temp_ID-ģ��ID��, 
*						Ctrl-ģ�����������(0-ǰ256�ֽڣ�1-��256�ֽ�)
*						pBuff-ģ������ָ��, Error-�������ָ��
*��		��: FPR_OK-���سɹ�,FPR_FAIL-����ʧ��,FPR_TIME_OUT-��ʱ
*��   ��:	����ָ��ģ��
*��   �ߣ�YangGuo
*ʱ   ��: 2014-5-7
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
			
			return FPR_TIME_OUT;//��Ӧ��
		}
		if(ERR_SUCCESS == FP_GetSensorAck())
		{
			TIM2_Timing_Off();
			*Error = FP_AnalysisAckCode(DIR_TEMPLATE_LOAD);
			if(0x1 == *Error)
			{
				//Ӧ����ȷ,׼������ģ������
#if (TEMPLE_SIZE_ONE_TX ==512)				
				Ptr = (Ctrl==0 ? pBuff : pBuff+256);//������ѡ����������������ָ��
#elif (TEMPLE_SIZE_ONE_TX ==256)
				Ptr = pBuff;
#endif				
				for(i=0;i<=255;i++)
				{
					SendByte(Ptr[i]);
				}
				//ģ�巢����ɵȴ�Ӧ��
				Start_TMR(BLH3001_TIMEOUT);	
				while(1)
				{
						if(Timer2OverFlag)
						{
							TIM2_Timing_Off();
							
							return FPR_TIME_OUT;//��Ӧ��
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
				return FPR_FAIL;//ʧ��
			}
		}		
	}
}

/****************************************************************************
*��		����FP_Upload_Template
*��   ��: FP_ID-ָ��ID, Temp_ID-ģ��ID��, 
*						Ctrl-ģ�����������(0-ǰ256�ֽڣ�1-��256�ֽ�)
*						Error-�������ָ��
*��		��: FPR_OK-�ϴ��ɹ�,FPR_FAIL-�ϴ�ʧ��,FPR_TIME_OUT-��ʱ
*��   ��:	�ϴ�ָ��ģ��
*��   �ߣ�YangGuo
*ʱ   ��: 2014-5-7
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
			
			return FPR_TIME_OUT;//��Ӧ��
		}
		if(ERR_SUCCESS == FP_GetSensorAck())
		{
			TIM2_Timing_Off();
			*Error = FP_AnalysisAckCode(DIR_TEMPLATE_LOAD);
			if(0x1 == *Error)
			{
				//Ӧ��ɹ�,׼����ȡģ������
				//ʹ��һ��ȫ�������ڴ����ж������ģ��
				//�û��ڴ����жϴ��������ģ������(256byte)���պ�Ӧ�ý�gTempUplod��1
				Start_TMR(BLH3001_TIMEOUT);	
				while(!gTempUplod)//�ȴ�ģ�����ݽ������
				{
						if(Timer2OverFlag)
						{
							TIM2_Timing_Off();
							
							return FPR_TIME_OUT;//��Ӧ��
						}
				}
				//ģ�巢����ɵȴ�Ӧ��
				TIM2_Timing_Off();//�����ڹ涨ʱ���ڽ�����ɣ��رն�ʱ��
				Start_TMR(BLH3001_TIMEOUT);	//������ʱ���ȴ�Ӧ��
				while(1)
				{
						if(Timer2OverFlag)
						{
							TIM2_Timing_Off();
							
							return FPR_TIME_OUT;//��Ӧ��
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
				return FPR_FAIL;//ʧ��
			}
		}
	}
}

#endif



/************************************************************************************
@��������: UART3_IRQHandler
@��������: ָ�ƴ����жϷ�����
@��    ��: None
@��    ��: None
@��    ��: veiko
@��    ˾: ��Ƕ�Ƽ�
@ʱ    ��: 2015-7-16
************************************************************************************/
void UART3_IRQHandler(void)
{
	unsigned char data = 0;
	data = USART_ReceiveData(USART3);
	BLH_UART1_DataRecive(data);
}























