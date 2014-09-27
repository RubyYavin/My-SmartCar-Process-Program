/**
  ******************************************************************************
  * @file    led.h
  * @author  YANDLD
  * @version V2.4
  * @date    2013.5.23
  * @brief   ����ԭ�Ӻ˺��İ� BSP���� LED����
  ******************************************************************************
  */
#ifndef __LED_H__
#define __LED_H__

#include "gpio.h"

#ifdef __cplusplus
 extern "C" {
#endif

//LEDͷ�ļ�
//IO ��
#define LED1_PORT PTE//PTC
#define LED2_PORT PTE//PTC
#define LED3_PORT PTE//PTC
#define LED4_PORT PTD
//����
#define LED1_PIN  (4)//16
#define LED2_PIN  (5)//17
#define LED3_PIN  (6)//18
#define LED4_PIN  (1)
//λ��
#define LED1      PCout(LED1_PIN) 
#define LED2      PCout(LED2_PIN) 
#define LED3      PCout(LED3_PIN) 
#define LED4      PDout(LED4_PIN) 

//��������ʵ�ֵĺ����ӿ��б�
void LED_Init(void);

#ifdef __cplusplus
}
#endif

#endif
