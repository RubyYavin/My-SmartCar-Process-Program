/**
  ******************************************************************************
  * @file    rtc.h
  * @author  YANDLD
  * @version V2.4
  * @date    2013.5.23
  * @brief   ����K60�̼��� ʵʱʱ������ ͷ�ļ�
  ******************************************************************************
  */
#ifndef __RTC_H_
#define	__RTC_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "sys.h"
//ʱ��ṹ��
typedef struct 
{
	uint8_t Hour;
	uint8_t Minute;
	uint8_t Second;			
	uint8_t Month;
	uint8_t Date;
	uint8_t Week;	
	uint16_t Year;
	uint32_t TSRValue;
}RTC_CalanderTypeDef;		

//������ʵ�ֵĺ����ӿ��б�
void RTC_Init(void);
void RTC_SecondIntProcess(RTC_CalanderTypeDef * RTCx);
uint8_t RTC_SetData(RTC_CalanderTypeDef * RTCx);

#ifdef __cplusplus
}
#endif

#endif
