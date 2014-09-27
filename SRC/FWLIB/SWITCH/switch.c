#include "switch.h"
#include "gpio.h"

int SPEED_KEYS;
int PEOPLE_KEYS;
int RAMP_PEOPLE_KEYS=1;//1 - ����µ�������       0 - ������µ�������
int STOP_KEYS=1;//1 - ���������     0 - �����������
void switch_init()
{
	GPIO_InitTypeDef switch1,switch2,switch3,switch4,switch5,switch6,switch7,switch8;
	switch1.GPIOx = PTC;                             //C�˿�
	switch1.GPIO_InitState = Bit_RESET;                //��ʼ��������ߵ�ƽ
	switch1.GPIO_IRQMode = GPIO_IT_DISABLE;          //��ʱ���ж�
	switch1.GPIO_Pin = GPIO_Pin_15;                  //PC11����
	switch1.GPIO_Mode = GPIO_Mode_IPU;               //��������
	//ִ��GPIO��ʼ��
	GPIO_Init(&switch1); 
	
	switch2.GPIOx = PTC;                             //C�˿�
	switch2.GPIO_InitState = Bit_RESET;                //��ʼ��������ߵ�ƽ
	switch2.GPIO_IRQMode = GPIO_IT_DISABLE;          //��ʱ���ж�
	switch2.GPIO_Pin = GPIO_Pin_14;                  //PC10����
	switch2.GPIO_Mode = GPIO_Mode_IPU;               //��������
	//ִ��GPIO��ʼ��
	GPIO_Init(&switch2); 
	
	switch3.GPIOx = PTC;                             //C�˿�
	switch3.GPIO_InitState = Bit_RESET;                //��ʼ��������ߵ�ƽ
	switch3.GPIO_IRQMode = GPIO_IT_DISABLE;          //��ʱ���ж�
	switch3.GPIO_Pin = GPIO_Pin_13;                  //PC9����
	switch3.GPIO_Mode = GPIO_Mode_IPU;               //��������
	//ִ��GPIO��ʼ��
	GPIO_Init(&switch3); 
	
	switch4.GPIOx = PTC;                             //C�˿�
	switch4.GPIO_InitState = Bit_RESET;                //��ʼ��������ߵ�ƽ
	switch4.GPIO_IRQMode = GPIO_IT_DISABLE;          //��ʱ���ж�
	switch4.GPIO_Pin = GPIO_Pin_12;                  //PC8����
	switch4.GPIO_Mode = GPIO_Mode_IPU;               //��������
	//ִ��GPIO��ʼ��
	GPIO_Init(&switch4);   
	
	switch5.GPIOx = PTC;                             //C�˿�
	switch5.GPIO_InitState = Bit_RESET;                //��ʼ��������ߵ�ƽ
	switch5.GPIO_IRQMode = GPIO_IT_DISABLE;          //��ʱ���ж�
	switch5.GPIO_Pin = GPIO_Pin_11;                  //PC11����
	switch5.GPIO_Mode = GPIO_Mode_IPU;               //��������
	//ִ��GPIO��ʼ��
	GPIO_Init(&switch5); 
	
	switch6.GPIOx = PTC;                             //C�˿�
	switch6.GPIO_InitState = Bit_RESET;                //��ʼ��������ߵ�ƽ
	switch6.GPIO_IRQMode = GPIO_IT_DISABLE;          //��ʱ���ж�
	switch6.GPIO_Pin = GPIO_Pin_10;                  //PC10����
	switch6.GPIO_Mode = GPIO_Mode_IPU;               //��������
	//ִ��GPIO��ʼ��
	GPIO_Init(&switch6); 
	
	switch7.GPIOx = PTC;                             //C�˿�
	switch7.GPIO_InitState = Bit_RESET;                //��ʼ��������ߵ�ƽ
	switch7.GPIO_IRQMode = GPIO_IT_DISABLE;          //��ʱ���ж�
	switch7.GPIO_Pin = GPIO_Pin_9;                  //PC9����
	switch7.GPIO_Mode = GPIO_Mode_IPU;               //��������
	//ִ��GPIO��ʼ��
	GPIO_Init(&switch7); 
	
	switch8.GPIOx = PTC;                             //C�˿�
	switch8.GPIO_InitState = Bit_RESET;                //��ʼ��������ߵ�ƽ
	switch8.GPIO_IRQMode = GPIO_IT_DISABLE;          //��ʱ���ж�
	switch8.GPIO_Pin = GPIO_Pin_8;                  //PC8����
	switch8.GPIO_Mode = GPIO_Mode_IPU;               //��������
	//ִ��GPIO��ʼ��
	GPIO_Init(&switch8);   
}

int get_speed_switch()//��ó�ʼ�ٶȿ�����
{
	int temp=0;
	temp+=!GPIO_ReadInputDataBit(PTC,GPIO_Pin_15);
	temp+=!GPIO_ReadInputDataBit(PTC,GPIO_Pin_14)*2;
	temp+=!GPIO_ReadInputDataBit(PTC,GPIO_Pin_13)*4;
	temp+=!GPIO_ReadInputDataBit(PTC,GPIO_Pin_12)*8;
	return temp;
}
int is_open_ramp_people_function()//��������������
{
	int key=0;
	key+=!GPIO_ReadInputDataBit(PTC,GPIO_Pin_11);
	return key;
}
int is_open_stop_function()//����Ƿ���ͣ������
{
	int key=0;
	key+=!GPIO_ReadInputDataBit(PTC,GPIO_Pin_10);
	return key;
}