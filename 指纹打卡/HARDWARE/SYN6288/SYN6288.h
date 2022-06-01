#ifndef SYN8266___H
#define SYN8266___H

#include "sys.h"

#define VOICE_BUSY_PORT    GPIOA
#define VOICE_BUSY_PIN_BIT GPIO_Pin_8

#define Voice_ifBusy()      ((GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)==1)? 1:0)
//�͵�ƽ��ʾ CHIP ����, �ɽ�����λ�����͵����������
//�ߵ�ƽ��ʾ CHIP æ�����ڽ��������ϳɲ�����

void Voice_BusyPinInitize(void);
void Voice_play(u8 *puts);		//��������

#endif

