//#include "sys.h"
//#include "gpio.h"
#include "delay.h"
#include "pit.h"
//#include "ftm.h"
//#include "adc.h"
//#include "spi.h"
//#include "stdio.h"
#include "uart.h"
#include "ccd.h"
#include "motor.h"
#include "steer.h"
#include "encoder.h"
#include "switch.h"
#include "bell.h"
#include "oled.h"
#include "gyroscope.h"
#include "outputdata.h"

extern uint8_t ADC_Value[128];
extern uint8_t ADC2_Value[128];
extern uint8_t ADC3_Value[128];

extern int up_down_Offset[128];
extern int SPEED_KEYS;
extern int PEOPLE_KEYS;
extern int target_speed;
extern int super_speed; 
extern int top_speed;   //�����Զ�̬�ı�����ٶ� 
extern int middle_speed;   //�����Զ��е��ٶ�
extern int min_speed;   //�����Զ�̬�ܴﵽ����С�ٶ��ٶ� 
extern int Road_Middle;
extern int RAMP_PEOPLE_KEYS;//1 - ����µ�������       0 - ������µ�������
extern int STOP_KEYS;//����Ƿ���ͣ������

void set_speed();
void led_init();
int main(void)
{
	//ADC_InitTypeDef ADC_InitStruct1;//PTB10
	int i=0;
	uint8_t temp1_ADC_Value[128],temp2_ADC_Value[128],temp3_ADC_Value[128],temp_Road_Middle;
	PIT_InitTypeDef Time2,Time3;//��ʱ�� PIT2

	//��ʼ��ϵͳʱ�� ʹ���ⲿ50M���� PLL��Ƶ��100M
	SystemClockSetup(ClockSource_EX50M,CoreClock_200M);	
	UART_DebugPortInit(UART2_RX_PD2_TX_PD3,115200);
	UART_ITConfig(UART2,UART_IT_RDRF,ENABLE);//�������������ж�
	//OutPut_DataInit();
	DelayInit();
	steer_init();
	CCD1_init();
	CCD2_init();
	CCD3_init();
	motor_init();
	encoder_init();
	switch_init();//���뿪��
	oled_init();
	gyro_init();
	//led_init();

	SPEED_KEYS=get_speed_switch();
	RAMP_PEOPLE_KEYS=is_open_ramp_people_function();
	STOP_KEYS=is_open_stop_function();
	set_speed(40);//9ms 60; 3ms 20
	
	bell_init();
	
	//DelayMs(1500);
	
	bell_open();
	DelayMs(150);//50
	bell_close();
	
	DelayMs(150);//150
	
	bell_open();
	DelayMs(50);//100
	bell_close();
	
	DelayMs(100);//250
	
	bell_open();
	DelayMs(50);//150
	bell_close();
//set_motor_pwm(3900,0);	
//set_steer_angle(4750);
	DelayMs(1500);
	CCD_Value(1);//����CCD
	Time2.PITx = PIT2;          //PIT2ͨ��
	Time2.PIT_Interval = 9;   //��ʱ����3MS
	PIT_Init(&Time2);
	PIT_ITConfig(PIT2,PIT_IT_TIF,ENABLE);//*/
	//UART_printf("start\r\n");
	while(1)
	{
		 
/*		for(i=0;i<128;i++)
		{
			temp1_ADC_Value[i]=ADC_Value[i];
			temp2_ADC_Value[i]=ADC2_Value[i];
		}
		temp_Road_Middle=Road_Middle;
		
		for(i=0;i<128;i++)
		{
			UART_printf("%d ",temp1_ADC_Value[i]);
		}
		UART_printf("\r\n");
		//DelayMs(500);
		for(i=0;i<128;i++)
		{
			UART_printf("%d ",temp2_ADC_Value[i]);
		}
		UART_printf("\r\n");
		UART_printf("%d",Road_Middle);
		UART_printf("\r\n\r\n");
		*/
		Display_Info();
		Display_Pixel(7,ADC_Value,ADC2_Value);
		Picture(); 
	}

}
void led_init()
{
	GPIO_InitTypeDef led;
	led.GPIOx = PTC;                             //C�˿�
	led.GPIO_InitState = Bit_RESET;                //��ʼ��������ߵ�ƽ
	led.GPIO_IRQMode = GPIO_IT_DISABLE;          //��ʱ���ж�
	led.GPIO_Pin = GPIO_Pin_4;                  //PC11����
	led.GPIO_Mode = GPIO_Mode_IPU;               //��������
	//ִ��GPIO��ʼ��
	GPIO_Init(&led); 
}
void led_flash()
{
	volatile static int time_lot=0;
	if(time_lot<20)
		GPIO_SetBits(PTC,GPIO_Pin_4);  
	if(time_lot<40)
		GPIO_ResetBits(PTC,GPIO_Pin_4);  
	if(time_lot<60)
		GPIO_SetBits(PTC,GPIO_Pin_4);  
	if(time_lot<100)
		GPIO_ResetBits(PTC,GPIO_Pin_4);  
	if(time_lot>100)
		time_lot=0;
	GPIO_SetBits(PTC,GPIO_Pin_4);
	time_lot++;
}

void set_speed(int speed_range)
{
	if(0==SPEED_KEYS)
	{
		super_speed=40;
		top_speed=30;
		target_speed=20;
		middle_speed=20;
		min_speed=20;
	}
	if(1==SPEED_KEYS)//һ��
	{
		super_speed=speed_range+20+SPEED_KEYS*2;
		top_speed=speed_range+10+SPEED_KEYS*2;
		target_speed=speed_range;
		middle_speed=speed_range;
		min_speed=speed_range;
	}
	else if(2==SPEED_KEYS)//����
	{
		super_speed=speed_range+35+SPEED_KEYS*2;
		top_speed=speed_range+20+SPEED_KEYS*2;
		target_speed=speed_range+10;
		middle_speed=speed_range+10;
		min_speed=speed_range+10;
	}
	else if(4==SPEED_KEYS)//����
	{
		super_speed=speed_range+40+SPEED_KEYS*2;
		top_speed=speed_range+25+SPEED_KEYS*3;
		target_speed=speed_range+20;
		middle_speed=speed_range+20;
		min_speed=speed_range+20;
	}
	else if(8==SPEED_KEYS)//�ĵ�
	{
		super_speed=speed_range+60;
		top_speed=speed_range+50;
		target_speed=speed_range+25;
		middle_speed=speed_range+25;
		min_speed=speed_range+30;
	}
	else if(9==SPEED_KEYS)//�嵵
	{
		super_speed=speed_range+30;
		top_speed=speed_range+20;
		target_speed=speed_range+10;
		middle_speed=speed_range+10;
		min_speed=speed_range+35;
	}
	else
	{
		super_speed=speed_range+SPEED_KEYS*7;
		top_speed=speed_range+SPEED_KEYS*5;
		target_speed=speed_range+SPEED_KEYS*2;
		middle_speed=speed_range+SPEED_KEYS*2;
		min_speed=speed_range+SPEED_KEYS*3;
	}
}





void assert_failed(uint8_t* file, uint32_t line)
{
	//����ʧ�ܼ��
	UART_printf("assert_failed:line:%d %s\r\n",line,file);
	while(1);
}

