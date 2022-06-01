#ifndef SYN8266___H
#define SYN8266___H

#include "sys.h"

#define VOICE_BUSY_PORT    GPIOA
#define VOICE_BUSY_PIN_BIT GPIO_Pin_8

#define Voice_ifBusy()      ((GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)==1)? 1:0)
//低电平表示 CHIP 空闲, 可接收上位机发送的命令和数据
//高电平表示 CHIP 忙，正在进行语音合成并播音

void Voice_BusyPinInitize(void);
void Voice_play(u8 *puts);		//播报语音

#endif

