
#ifndef _outputdata_H
#define _outputdata_H

#include "sys.h"

#define OutData_UARTx       UART2
#define OutData_UARTx_MAP   UART2_RX_PD2_TX_PD3

extern float OutData[4];
void OutPut_Data(void); //��ҪUART_Init()�������ʹ�ã�
                            //�����ʹ��ʱ����UART��ʼ���ṹ��
void OutPut_DataInit(void);
#endif 
