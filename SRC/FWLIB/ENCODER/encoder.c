#include "encoder.h"
#include "sys.h"
#include "lptm.h"
int counter;
void encoder_init()
{
	LPTM_InitTypeDef LPTM_InitStruct1;//������
	LPTM_InitStruct1.LPTMxMap = LPTM_CH2_PC5;
	LPTM_InitStruct1.LPTM_InitCompareValue = 200;          //��PCģʽ��������
	LPTM_InitStruct1.LPTM_Mode = LPTM_Mode_PC_FALLING;     //�½��ش�������ģʽ
	LPTM_Init(&LPTM_InitStruct1);
}
int get_encoder()
{
		//��ȡLPTMֵ
		counter = LPTM_GetTimerCounterValue(LPTMR0);
		//��ռ�����
		LPTM_ResetTimeCounter(LPTMR0);
	return counter;
}