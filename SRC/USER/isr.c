#include "isr.h"
//#include "adc.h"
#include "ccd.h"
#include "steer.h"
//#include "lptm.h"
#include "encoder.h"
#include "switch.h"
#include "motor.h"
#include "stdbool.h"
#include "gyroscope.h"
#include "outputdata.h"
extern uint8_t ADC_Value[128];
extern int ADC_Offset[128];
extern int up_down_Offset[128];
extern uint8_t CCD_Threshold;//ccd ����ֵ
extern int CCD_Left;//ccd ��ߵĺ��ߵ�������±�
extern int CCD_Right;//ccd �ұߵĺ��ߵ�������±�
extern int CCD_Middle;
extern int steerKp;
extern int steerKd;
extern int people_wan;
uint16_t time_count=0;
float OutData[4];
bool stop_yes=false;
extern int system_speed;
extern int target_speed;
extern int top_speed;
extern int middle_speed;
extern int min_speed;
extern int ADC_MAX;
extern int ADC_MIN;
extern int SPEED_KEYS;
extern int STOP_KEYS;
extern uint8_t gyro_value;
extern int Road_Middle;
extern int ramp_time;
extern bool ramp;


int isrcount=0;
int pit3_count=0;
uint8_t receiveBuffer[10];
bool should_stop();
bool should_stop_second();
void stop();

void UART2_RX_TX_IRQHandler(void)
{
	uint8_t ch='e';
	UART_SendDataIntProcess(UART2);
	NVIC_DisableIRQ(PIT2_IRQn);	
  if(UART_ReceiveData(UART2,&ch))
		{	
			switch(ch)
			{
				case '1':EnableInterrupts();break;         //ʹ���жϣ�����
				case '2':stop();break; //���жϣ��ػ�
				case '3':top_speed++;middle_speed++;min_speed++;break;             //�ٶȼ�1
				case '4':top_speed+=5;middle_speed+=5;min_speed+=5;break;          //�ٶȼ�5
				case '5':top_speed--;middle_speed--;min_speed--;break;             //�ٶȼ�1
				case '6':top_speed-=5;middle_speed-=5;min_speed-=5;break;//�ٶȼ�5
				case '7':steerKp++;break;                  //ServerKP��1
				case '8':steerKp--;break;                  //ServerKP��1
				case '9':steerKd++;break;                  //ServerKD��1
				case 'a':steerKd--;break;                  //ServerKD��1
				default :break;
			}
		}
		if(ch!='2')
			NVIC_EnableIRQ(PIT2_IRQn);
}
void PIT2_IRQHandler(void)
{
	int i;
	CCD_Value(1);
	//CCD3_Value(1);
	Adapted_Black_Thread();//����Ӧ����
	Adapted2_Black_Thread();//����Ӧ����
	//Adapted3_Black_Thread();//����Ӧ����
	get_gyro_value();//��������ǵ�ֵ
	//UART_printf("%d\r\n",gyro_value);
	//road_middle_width();
	steer_PID();
	//steer_PID_test();
	system_speed=get_encoder();
	if(stop_yes==false)
		motor_PID();
	if(time_count>1000 && stop_yes==false)//5���Ժ� �����յ��߼��
	{
		stop_yes=should_stop();
		//if(stop_yes==false)
		//	stop_yes=should_stop_second();
		if(stop_yes==true)
			time_count=0;
	}
	if(ramp==true && ramp_time<90)//���·�ֹɲ��
		stop_yes=false;
	if(stop_yes==true && STOP_KEYS==1)
	{
			people_wan=0;
			stop();	
	}
	//if(time_count>1667)//2667 24��ɲ��
		//stop_yes=true;
	time_count++;
	//UART_printf("%d  ",gyro_value);
	
	//OutData[0]=Road_Middle;
	//OutData[1]=64;
	//OutPut_Data();
	
	
	/*	if(CCD_Middle>=0)
			UART_printf("%d       %d        %d\r\n",CCD_Left,CCD_Middle,CCD_Right);
		else 
			UART_printf("%d       -%d        %d\r\n",CCD_Left,abs(CCD_Middle),CCD_Right);
		i=0;
		for(i=0;i<128;i++)
		{
			UART_printf("%d ",ADC_Value[i]);
		}
		UART_printf("\r\n");*/
		/*Adapted_Black_Thread();
		steer_PID();
	  	for(i=0;i<128;i++)
				UART_printf("%d ",ADC_Value[i]);
		UART_printf("\r\n");
		UART_printf("CCD_Threshold      %d\r\n",CCD_Threshold);
		UART_printf("\r\n");
		*/
		//UART_printf("   %d",system_speed);
		//UART_printf("%x",255);
	  //UART_printf("%d       %d        %d\r\n",CCD_Left,abs(CCD_Middle),CCD_Right);
		//UART_printf("CCD_Right          %d\r\n",CCD_Right);
		//UART_printf("CCD_Threshold      %d\r\n",CCD_Threshold);
	 //UART_printf("system_speed        %d\r\n",system_speed);
	 //UART_printf("target_speed:  %d\r\n",target_speed);
	 //UART_printf("\r\n");
	 //UART_printf("\r\n");
	 //count=0;			
	//	UART_printf("  %d \r\n",CCD_Threshold);
	//	UART_printf("%d\r\n",SPEED_KEYS);
	//UART_printf("%d     %d      %d        %d       %d\r\n",CCD_Left,CCD_Middle,CCD_Right,system_speed,target_speed);
	//�رո��ж�
	//NVIC_DisableIRQ(PIT2_IRQn);//UART2_RX_TX_IRQn
	PIT_ClearITPendingBit(PIT2,PIT_IT_TIF);
}

void PIT3_IRQHandler(void)//STOP
{
	//UART_printf("hello\r\n");
	CCD_Value(1);
	Adapted_Black_Thread();//����Ӧ����
	Adapted2_Black_Thread();//����Ӧ����
	steer_PID();
	system_speed=get_encoder();
	if(time_count<40)
		motor_PID();
	else//�ſ�ʼ����
	{
		bell_open();
		if(system_speed*system_speed*system_speed<=10000 && system_speed>5)
			set_motor_pwm(0,system_speed*system_speed*system_speed);
		else if(system_speed*system_speed*system_speed>10000 && system_speed>5)
			set_motor_pwm(0,10000);
		else 
		{
			set_motor_pwm(0,0);
			bell_close();
			time_count=0;
			//UART_printf("stoping,very good!\r\n");
			NVIC_DisableIRQ(PIT3_IRQn);
		}
	}
	time_count++;
	
	PIT_ClearITPendingBit(PIT3,PIT_IT_TIF);
}

bool should_stop()
{
  int i=0,num=0,believe=0,max[24],index[24];
	for(i=0;i<24;i++)
	{
		max[i]=0;
		index[i]=0;
	}
	for(i=1;i<128;i++)//ADC_Offset[0]=0
	{
		if(abs(ADC_Offset[i])>(ADC_MAX-ADC_MIN)/4 && num<24)
		{
			max[num]=ADC_Offset[i];
			index[num]=i;
			num++;
		}
		else if(abs(ADC_Offset[i])>(ADC_MAX-ADC_MIN)/5 && num<24 && i<126)
		{
			if(ADC_Offset[i]*ADC_Offset[i+1]>0) //&& ADC_Offset[i+1]*ADC_Offset[i+2]>0
			{
				max[num]=ADC_Offset[i+1];
				index[num]=i+1;
				num++;
			}
		}
	}
	for(i=0;i<num-1;i++)
	{
		if(num<2)//����3���أ�����
			break;
		if((max[i]>0 && max[i+1]<0) || (max[i]<0 && max[i+1]>0) )
			if(index[i+1]-index[i]>2 && index[i+1]-index[i]<27)
				believe++;
	}
	
	if(believe>2)//���յ�
		return true;
	else
		return false;
}
bool should_stop_second()
{
	int up=0,down=0;
	int i=0,believe=0;
	for(i=0;i<128;i++)
	{
		if(up_down_Offset[i]==1)//������
		{
			up=i;
		}
		else if(up_down_Offset[i]==-1)//�½���
			down=i;

		if(up<down-5 && up!=0 && down-up<27)
		{
			believe++;
			up=0;
			down=0;
		}
	}
	//UART_printf("%d\r\n",believe);
	if(believe==3)//ȷ�����յ�
		return true;
	else
		return false;
}
void stop()
{
	if(time_count<40)
		motor_PID();
	else//�ſ�ʼ����
	{
		bell_open();
		if(system_speed>2)
			set_motor_pwm(0,2000);
		if(system_speed<=2) 
		{
			set_motor_pwm(0,0);
			bell_close();
			NVIC_DisableIRQ(PIT2_IRQn);
		}
	}
}
