#include "steer.h"
#include "ftm.h"
#include "gpio.h"
#include "stdbool.h"
#include "gyroscope.h"

#define STEER_MIDDLE 4000   //�����ֵ��ӦPWM      �� 5000    �� 3000
#define STEER_RANGE  1000    //���ת��   ������2���궨���ڻ�еӲ���й�
extern int system_speed;
extern int target_speed;
extern int SPEED_KEYS;
extern int RAMP_PEOPLE_KEYS;//1 - ����µ�������       0 - ������µ�������

extern int CCD_Forward;
extern int CCD_Left;//ccd ��ߵĺ��ߵ�������±�
extern int CCD_Right;//ccd �ұߵĺ��ߵ�������±�
extern int CCD_Middle;
extern int Black_Left;//ccd ��ߵĺ��������Ƿ�Ϊ��
extern int Black_Right;//ccd �ұߵĺ��������Ƿ�Ϊ��
extern int White_Middle;//middle is white?
extern int Thread_Left;//ccd ��ߵĺ��ߵ�������±�
extern int Thread_Right;//ccd �ұߵĺ��ߵ�������±�
bool True_Road=true;//road is true?
extern int last_CCD_Middle;
extern int last_CCD_Left;//ccd ��һ����ߵĺ��ߵ�������±�
extern int last_CCD_Right;//ccd ��һ���ұߵĺ��ߵ�������±�
extern uint8_t ADC_Value[128];
extern uint8_t ADC2_Value[128];
extern int up_down_Offset[128];
extern int upright_Offset[128];
extern uint8_t gyro_value;
extern uint8_t last_gyro_value;
extern int Road_Middle;
extern int Road_Width;
extern int ramp_before_steer_pwm;//����ǰ�Ķ���Ĵ��
extern int fast_de_speed;//����С��һ��ֵʱ ���ٵ�ת����
extern int min_speed;


bool turn_left=false,turn_right=false,hold_turn=false,hold_deadly=false,confirm=false,ramp=false;//�Ƿ�Ϊ ���� ���� ����ת�䷽��
int auxiliary_turn=0,ramp_time=255,is_straight=255,straight_time=10,delete_wan_counter=0;//׼ ���ʶ�� 1 ��2 ��
int steerKp=9,steerKd=9,steer_pwm=0,last_steer_pwm=0,last_steerKp,last_steerKp,people_wan=0,people_second_wan=0,people_third_wan=0;      //target_speed=42;      8,8 ����       14 ,17 �е��𵴣�����
FTM_InitTypeDef steer;


void steer_PID_test()
{
	volatile static int err=0,last_err=0,pwm,derr;
	err=64-Road_Middle; // CCD_Middle=CCD_Right-CCD_Left;
	derr=err-last_err;
	last_err=err;
	pwm=STEER_MIDDLE+err*abs(err)/2+derr*10; 
	if(pwm>STEER_MIDDLE+STEER_RANGE)
		pwm=STEER_MIDDLE+STEER_RANGE;
	else if(pwm<STEER_MIDDLE-STEER_RANGE)
		pwm=STEER_MIDDLE-STEER_RANGE;
	set_steer_angle(pwm);
}


void steer_PID()
{
	volatile static int err=0,last_err=0,derr,counter=255,speed_err=0,speed_last_err=0;
	volatile static int believe_people=0,threshold_count=0,temp=0,cross_road_counter=255;
	bool delete_wan=false,white_black_white=false;

/*	if((CCD_Middle>59 || (CCD_Right>59 && CCD_Right>CCD_Left) ) && hold_turn==false)//������  CCD_Middle=CCD_Right-CCD_Left;   > 0
	//if(((CCD_Middle>55 || (CCD_Right>55 && CCD_Right>CCD_Left) ) && hold_turn==false) || (CCD_Middle>20 && CCD_Left<5 && CCD_Forward<50 && hold_turn==false))//
	{
		turn_left=true;
		hold_turn=true;
	}
	if((CCD_Middle <-59 || (CCD_Left>59 && CCD_Left>CCD_Right) )&& hold_turn==false)//������   CCD_Middle=CCD_Right-CCD_Left <0  
	//if(((CCD_Middle <-55 || (CCD_Left>55 && CCD_Left>CCD_Right) )&& hold_turn==false) || (CCD_Middle <-20 && CCD_Right<5 && CCD_Forward<50 && hold_turn==false))
	{
		turn_right=true;
		hold_turn=true;
	}
	*/
	memory_turn();
	delete_wan=delete_wan_speed();
	if(CCD_Forward>50 && CCD_Left<55 && CCD_Right<55 && hold_turn==true )
		delete_wan=true;
	
	if(delete_wan)  //�Ѿ��뿪���
  {
		hold_turn=false;//ע��ֻ���������Ϊ false
		turn_left=false;
		turn_right=false;
		hold_deadly=false;
		is_straight=0;//���ڳ�����СKp,Kd
		if(counter>30)//��ֵΪ255
		{
			people_wan=0;
			people_second_wan=0;
			confirm=false;
		}
		if(counter>70)
		{
			people_second_wan=people_wan;
			people_third_wan=people_wan;
		}
		bell_close();
  } 
	if(hold_turn==false)
	{
		err=CCD_Middle; // CCD_Middle=CCD_Right-CCD_Left;
		derr=err-last_err;
		last_err=err;
	}
	//UART_printf("%d ",abs(derr));

	speed_Kp_Kd(err,derr);//�����ٶ� �ı�Kp��Kd
	if(CCD_Forward>66)
		is_straight=0;
	if(is_straight<straight_time)
		straight_speed_Kp_Kd();
	steer_pwm=err*steerKp+derr*steerKd;  //�� 5000    �� 3000
	//if(CCD_Left>40 && CCD_Right>40 && hold_turn==true)
	//if(CCD_Left>40 && CCD_Right>40 && hold_turn==true && hold_deadly==false)
	if(CCD_Forward<30 && hold_turn==true && hold_deadly==false)
	{
		hold_deadly=true;
		fast_de_speed=0;//����������ټ���
	}
	
	//��ֹ����
	if(temp=whereIsRoad(hold_turn))
	{
		if(temp==1)
			turn_left=true;
		else
			turn_right=true;
		hold_turn=true;
	}
	if(hold_turn==true)
	{
		if(delete_wan_counter>10)
			delete_wan_counter=0;
		wan_speed_Kp_Kd(err,derr);//�����ٶ� �ı�Kp��Kd
		if(is_straight<straight_time)
			straight_speed_Kp_Kd();
		if(turn_left==true)//˵��������  
		{
			err=CCD_Left+CCD_Right;
			if(err<last_err)
				err=last_err;
			last_err=err;
			steer_pwm=err*steerKp;
			if(hold_deadly==true)
				steer_pwm=hold_deadly_pwm();
			//if(SPEED_KEYS==4)
				//steer_pwm=err*steerKp+derr*steerKd;

		}
		else 
		{
			err=-CCD_Left-CCD_Right;
			if(err>last_err)
				err=last_err;
			last_err=err;
			steer_pwm=err*steerKp;
			if(hold_deadly==true)
				steer_pwm=-hold_deadly_pwm();
		}
	}
	
	//�� 5000    �� 3000
	
	//���ֵ����� �����������Ƚϳ����˸�����  �������������д
	temp=STEER_MIDDLE+steer_pwm;//���ıߴ�ǰ����ڷ�����
	
	//ʶ�������� ���ַ���
	if(people_wan==0 && counter>30)
	{
		believe_people=0;
		threshold_count=0;
		bell_close();
		if(CCD_Forward>55)
			people_wan=is_people_bent();
		if(people_wan!=0)
		{
			counter=0;
			people_second_wan=people_wan;
			people_third_wan=people_wan;
		}
	}
/*	else
	{
		if(people_wan==-1 && abs(last_CCD_Left-CCD_Left)<5 && CCD_Right<17 && CCD_Left>3 && threshold_count<7)
			believe_people++;
		if(people_wan==1 && abs(last_CCD_Right-CCD_Right)<5 && CCD_Left<17 && CCD_Right>3 && threshold_count<7)
			believe_people++;
		
		/*if(people_wan==-1 && threshold_count<7)
			UART_printf("-1   %d   %d   %d\r\n",abs(last_CCD_Left-CCD_Left),CCD_Left,CCD_Right);
		if(people_wan==1 && threshold_count<7)
			UART_printf("1   %d    %d   %d\r\n",abs(last_CCD_Right-CCD_Right),CCD_Left,CCD_Right);
		
		if(threshold_count<7 && believe_people>3)
			counter=0;
		threshold_count++;
	}*/
	//if(people_second() && ((CCD_Right<5 && CCD_Left>10) || (CCD_Right>10 && CCD_Left<5) ))
	if(people_second() && people_second_wan!=0)
	{
		believe_people=6;
		people_wan=people_second_wan;
		counter=0;
	}
	if(people_third() && people_third_wan!=0)
	{
		//people_third_wan
		believe_people=6;
		people_wan=people_third_wan;
		counter=0;
	}
	//UART_printf("%d \r\n",abs(last_CCD_Left-CCD_Left));
	if(people_wan!=0 && believe_people>3 && counter>5 && cross_road_counter>150 && RAMP_PEOPLE_KEYS==1)//ȷ���������� && believe_people>=2 fg5sgddqsm
	{
		temp=STEER_MIDDLE+people_wan*STEER_RANGE;
		hold_turn=true;
		//bell_open();
	}
	
	if(people_wan!=0 && believe_people<=3 && counter>30 )
	{
		people_wan=0;
		believe_people=0;
	}
	//����
	if(gyro_value>190 && last_gyro_value>190 && ramp_time>=90)//&& people_wan==0 && counter<50)
	{
		//temp=last_steer_pwm;
		if(ramp==true)
		{
			ramp=false;
		}
		else
		{
			ramp=true;	
			ramp_before_steer_pwm=last_steer_pwm;
			Road_Middle=CCD_Middle;//��������
			
		}
		ramp_time=0;
	}
	if(ramp==true && ramp_time<90 && RAMP_PEOPLE_KEYS==1)//�µ�����
	{
		if(ramp_time<30)
			temp=ramp_steer_pwm();
		else
			temp=STEER_MIDDLE+(CCD_Right-CCD_Left)*5;//;(64-ramp_middle())*30
		//UART_printf("%d  %d  %d\r\n",Road_Middle,Road_Width,ramp_middle());
		bell_open();
	}
	//UART_printf("%d \r\n",temp);
	steer_pwm=temp;
	steer_pwm=cross_road(&cross_road_counter);//ʮ�ִ���
	
	if(steer_pwm>STEER_MIDDLE+STEER_RANGE)//���ⳬ�������   
		steer_pwm=STEER_MIDDLE+STEER_RANGE;
	if(steer_pwm<STEER_MIDDLE-STEER_RANGE)//���ⳬ�������
		steer_pwm=STEER_MIDDLE-STEER_RANGE;
	
	counter++;
	ramp_time++;
	cross_road_counter++;
	is_straight++;
	delete_wan_counter++;
	last_steer_pwm=steer_pwm;
	set_steer_angle(steer_pwm);
}

void memory_turn()
{
	switch(SPEED_KEYS)
	{
		case 4:
			if((CCD_Middle>59 || (CCD_Right>59 && CCD_Right>CCD_Left) ) && hold_turn==false)//������  CCD_Middle=CCD_Right-CCD_Left;   > 0
			{
				turn_left=true;
				hold_turn=true;
				
			}
			if((CCD_Middle <-59 || (CCD_Left>59 && CCD_Left>CCD_Right) )&& hold_turn==false)//������   CCD_Middle=CCD_Right-CCD_Left <0  

			{
				turn_right=true;
				hold_turn=true;
				
			};
			break;    //�ĵ�
		case 8:
			if((CCD_Middle>60 || (CCD_Right>60 && CCD_Right>CCD_Left) ) && hold_turn==false)//������  CCD_Middle=CCD_Right-CCD_Left;   > 0
			{
				turn_left=true;
				hold_turn=true;
			}
			if((CCD_Middle <-60 || (CCD_Left>60 && CCD_Left>CCD_Right) )&& hold_turn==false)//������   CCD_Middle=CCD_Right-CCD_Left <0  

			{
				turn_right=true;
				hold_turn=true;
			};
			break;    //�ĵ�
		case 9:
			if((CCD_Middle>60 || (CCD_Right>60 && CCD_Right>CCD_Left) ) && hold_turn==false && delete_wan_counter>10)//������  CCD_Middle=CCD_Right-CCD_Left;   > 0
			{
				turn_left=true;
				hold_turn=true;
				delete_wan_counter=0;
			}
			if((CCD_Middle <-60 || (CCD_Left>60 && CCD_Left>CCD_Right) )&& hold_turn==false && delete_wan_counter>10)//������   CCD_Middle=CCD_Right-CCD_Left <0  

			{
				turn_right=true;
				hold_turn=true;
				delete_wan_counter=0;
			};
			break;    //�嵵
		default :
			if((CCD_Middle>59 || (CCD_Right>59 && CCD_Right>CCD_Left) ) && hold_turn==false)//������  CCD_Middle=CCD_Right-CCD_Left;   > 0
			{
				turn_left=true;
				hold_turn=true;
			}
			if((CCD_Middle <-59 || (CCD_Left>59 && CCD_Left>CCD_Right) )&& hold_turn==false)//������   CCD_Middle=CCD_Right-CCD_Left <0  

			{
				turn_right=true;
				hold_turn=true;
			};
			break; 
	}
}
bool delete_wan_speed()
{
	bool delete_wan=false;
	switch(SPEED_KEYS)
	{
		case 1:break;    //һ��
		case 2:break;    //����
		case 4:if(CCD_Forward>50 && CCD_Left<60 && CCD_Right<60 && hold_turn==true )
							delete_wan=true;
					 break;    //����
		case 8:if(CCD_Forward>50 && CCD_Left<59 && CCD_Right<59 && hold_turn==true )
							delete_wan=true;
					 break;    //�ĵ�
		case 9:if(CCD_Forward>40 && hold_turn==true && delete_wan_counter>10 && (CCD_Left>59 || CCD_Right>59)) //&& CCD_Left<61 && CCD_Right<61&& 
							delete_wan=true;
					 break;    //�嵵
		default :break;   
	}
	return delete_wan;
//	
}
void straight_speed_Kp_Kd()
{
	switch(SPEED_KEYS)
	{
		case 0:straight_time=0;break;    //һ��
		case 1:straight_time=10;break;    //һ��
		case 2:break;    //���� straight_time=10;
		case 4:;break;    //���� steerKp=8; steerKd=6;straight_time=0 
		case 8:steerKp=7; steerKd=4;straight_time=0;break;    //�ĵ�   7 10;steerKp=6; steerKd=4;
		case 9:steerKp=5; steerKd=5;straight_time=5;break;    //�嵵   7 10;
		default :steerKp=4;steerKd=5;straight_time=10;break;   
	}
}
void speed_Kp_Kd(int err,int derr)
{
	switch(SPEED_KEYS)
	{
		case 0:steerKp=3+(70-CCD_Forward)/22; steerKd=3+(70-CCD_Forward)/15;break;   //һ��
		case 1:steerKp=3+(70-CCD_Forward)/19; steerKd=6+(70-CCD_Forward)/5;break;   //һ��
		case 2:steerKp=2+(70-CCD_Forward)/15; steerKd=2+(70-CCD_Forward)/15;break;   //����
		case 4://����
			if(steerKp<=3+(70-CCD_Forward)/15)
				steerKp++; 
			else
				steerKp--;
			if(steerKd<=2+(70-CCD_Forward)/15)
				steerKd++;
			else
				steerKd--;
			break;   
		case 8://�ĵ�
			if(steerKp<=2+(70-CCD_Forward)/3)
				steerKp++; 
			else
				steerKp--;
			if(steerKd<=4+(70-CCD_Forward)/10)
				steerKd++;
			else
				steerKd--;
			break;   
		case 9://�嵵
			if(steerKp<=2+(70-CCD_Forward)/4)
				steerKp++; 
			else
				steerKp--;
			if(steerKd<=4+(70-CCD_Forward)/5)
				steerKd++;
			else
				steerKd--;
			break;   
		default :steerKp=3+(70-CCD_Forward)/12; steerKd=3+(70-CCD_Forward)/12;break;   
	}
}
void wan_speed_Kp_Kd(int err,int derr)
{
	switch(SPEED_KEYS)
	{
		case 0:steerKp=6;break;    //һ��
		case 1:steerKp=6;break;    //һ��
		case 2:steerKp=6;break;    //����
		case 4:steerKp=6+abs(derr)/5;steerKd=5;break;    //����
		case 8:steerKp=7+abs(derr)/5;break;    //�ĵ�
		case 9:steerKp=7+abs(derr)/4;break;    //�嵵
		default :steerKp=10;break;   
	}
}
int hold_deadly_pwm()
{
	int pwm=0;
	switch(SPEED_KEYS)
	{
		case 0:pwm=770;break;    //һ��
		case 1:pwm=600;break;    //һ��
		case 2:pwm=700;break;    //����
		case 4:pwm=750;break;    //����
		case 8:pwm=760;break;    //�ĵ�
		case 9:pwm=700;break;    //�嵵
		default :pwm=1000;break;   
	}
	return pwm;
}
int cross_road(int *cross_road_counter)
{
	volatile static int back_CCD_Right[4]={64},back_CCD_Left[4]={64},pwm[4]={STEER_MIDDLE};
	int i=0;
	if(back_CCD_Right[3]<15 && back_CCD_Left[3]<15)//��ʮ��
	{
		if(*cross_road_counter>150)
			*cross_road_counter=0;
		back_CCD_Right[3]=CCD_Right;
		back_CCD_Left[3]=CCD_Left;
		pwm[3]=steer_pwm;
		if(pwm[0]>STEER_MIDDLE+300)
			pwm[0]=STEER_MIDDLE+300;
		if(pwm[0]<STEER_MIDDLE-300)
			pwm[0]=STEER_MIDDLE-300;
		if(pwm[0]>STEER_MIDDLE)
			pwm[0]-=20;
		if(pwm[0]<STEER_MIDDLE)
			pwm[0]+=20;
		return pwm[0];
	}
	else
	{
		for(i=0;i<3;i++)
		{
			back_CCD_Right[i]=back_CCD_Right[i+1];
			back_CCD_Left[i]=back_CCD_Left[i+1];
			pwm[i]=pwm[i+1];
		}
		back_CCD_Right[3]=CCD_Right;
		back_CCD_Left[3]=CCD_Left;
		pwm[3]=steer_pwm;
		
		return steer_pwm;
	}
}
int whereIsRoad(bool hold_turn)
{
	//define road is true
	int up=0,down=0;
	int i=0,j=0,believe=0,peak=-1;
	for(i=0;i<128;i++)
	{
		if(up_down_Offset[i]==1)//������
		{
			up=i;
			
		}
		else if(up_down_Offset[i]==-1)//�½���
			down=i;

		if(down<up && down!=0 && up-down<65 && up-down>30)
		{
			peak=(up+down)/2;
			believe++;
			up=0;
			down=0;
		}
	}
	if(believe==1 && hold_turn==false )//�÷�ֹ����
	{
		if(peak>=64)
			return 1;
		if(peak<64)
			return -1;
	}
	else 
		return 0;
}
bool left_right_wrong()//�Ƿ���� �� �� ��
{	
	if(Thread_Left>Thread_Right+10)
		return true;
	else
		return false;

}
int ramp_steer_pwm()
{
	volatile static int pwm=0,label=0,last_pwm=0;
	if(label==0)
	{
		pwm=ramp_before_steer_pwm;
		last_pwm=pwm;
		label=1;
	}
	if(pwm>STEER_MIDDLE+300)
		pwm=STEER_MIDDLE+300;
	if(pwm<STEER_MIDDLE-300)
		pwm=STEER_MIDDLE-300;
	if(pwm>STEER_MIDDLE)
		pwm-=10;
	if(pwm<STEER_MIDDLE)
		pwm+=10;
	
	if(pwm>STEER_MIDDLE && SPEED_KEYS!=1)
		pwm+=5;
	if(pwm<STEER_MIDDLE && SPEED_KEYS!=1)
		pwm-=5;
	if(abs(pwm-STEER_MIDDLE)<50 && SPEED_KEYS!=1)
		pwm=last_pwm;
	if(abs(pwm-STEER_MIDDLE)<20)
		label=0;
	
	return pwm;
}
int ramp_middle()
{
	int middle=64,i=0;
	int left=64,right=64;
	for(i=64;i>0;i--)
		if(ADC_Value[i]==1)
			left=i;
		else 
			break;
	for(i=64;i<128;i++)
		if(ADC_Value[i]==1)
			right=i;
		else 
			break;
	if(left<44 && right<74)
		left=right-20;
	if(left>54 && right>84)
		right=20+left;
	return (left+right)/2;
}
int is_people_bent()//���� 1   ��-1    ����Ϊ 0
{
	int up=0,down=0,count=0;
	int i=0,believe=0,peak=-1;
	int people_wan=0;
	for(i=0;i<128;i++)
	{
		if(up_down_Offset[i]==1)//������
		{
			up=i;
			
		}
		else if(up_down_Offset[i]==-1)//�½���
			down=i;

		if(down<up+2 && down!=0 && up-down<15)
		{
			//UART_printf("%d  %d\r\n",down,up);
			peak=(up+down)/2;
			believe++;
			up=0;
			down=0;
		}
	}
	if(believe==1)//������
	{
		if(peak>=64)
			people_wan=-1;
		if(peak<64)
			people_wan=1;
	}
	else
		people_wan=0;
/*	if(CCD_Left<5 && CCD_Right>15)
		people_wan=-1;
	else if(CCD_Left>15 && CCD_Right<5)
		people_wan=1;
	else
		people_wan=0;*/
	return people_wan;
}
bool confirm_people_bent()//���� 1   ��-1    ����Ϊ 0
{
	int up=0,down=0;
	int i=0,believe=0;
	for(i=25;i<95;i++)
	{
		if(upright_Offset[i]==1)//������
		{
			up=i;
			
		}
		else if(upright_Offset[i]==-1)//�½���
			down=i;

		if(down<up && down!=0 && up-down<10)
		{
			
			believe++;
			
			up=0;
			down=0;
		}
	}
	//UART_printf("%d\r\n",believe);
	if(believe>=2)//ȷ��������
		return true;
	else
		return false;
}
bool people_second()
{
	int up=0,down=0,last_down=0;
	int i=0,believe=0;
	for(i=50;i<100;i++)
	{
		if(upright_Offset[i]==1)//������
		{
			up=i;
		}
		else if(upright_Offset[i]==-1)//�½���
			down=i;

		if(down<up && down!=0 && up-down<10)
		{
			believe++;
			last_down=down;
			up=0;
			down=0;
		}
	}
	if(believe==1)
	{
		for(i=last_down;i<100;i++)
			if(ADC2_Value[i]==1)
				break;
		if(i==100)
			believe++;
	}
	//UART_printf("%d\r\n",believe);
	if(believe>=2)//ȷ��������
		return true;
	else
		return false;
}
bool people_third()
{
	int up=0,down=0;
	int i=0,believe=0;
	for(i=50;i<100;i++)
	{
		if(upright_Offset[i]==1)//������
		{
			up=i;
		}
		else if(upright_Offset[i]==-1)//�½���
			down=i;

		if(up<down && up!=0)
		{
			believe++;
			up=0;
			down=0;
		}
	}
	//UART_printf("%d\r\n",believe);
	if(believe>=2)//ȷ��������
		return true;
	else
		return false;
}
void steer_init()
{
	GPIO_InitTypeDef n_1;//PTB10
	GPIO_InitTypeDef n_2;//PTB10
	steer.Frequency = 300;                // 300HZ
	steer.FTMxMAP = FTM2_CH0_PB18;          //FTM2_CH0_PB18 PB18����
	steer.FTM_Mode = FTM_Mode_EdgeAligned; //���ض���ģʽ
	steer.InitalDuty = STEER_MIDDLE;               //��ʼ�������40%��ռ�ձ�
	FTM_Init(&steer);
	
	//�������
	n_1.GPIOx = PTB;                             //C�˿�
	n_1.GPIO_InitState = Bit_RESET;                //��ʼ��������ߵ�ƽ
	n_1.GPIO_IRQMode = GPIO_IT_DISABLE;          //��ʱ���ж�
	n_1.GPIO_Pin = GPIO_Pin_17;                  //PB2����
	n_1.GPIO_Mode = GPIO_Mode_IPD;               //�������
	//ִ��GPIO��ʼ��
	GPIO_Init(&n_1);
	
	n_2.GPIOx = PTB;                             //C�˿�
	n_2.GPIO_InitState = Bit_RESET;                //��ʼ��������ߵ�ƽ
	n_2.GPIO_IRQMode = GPIO_IT_DISABLE;          //��ʱ���ж�
	n_2.GPIO_Pin = GPIO_Pin_19;                  //PB3����
	n_2.GPIO_Mode = GPIO_Mode_IPD;               //�������
	//ִ��GPIO��ʼ��
	GPIO_Init(&n_2);
	
}
void set_steer_angle(uint16_t steer)
{
	FTM_PWM_ChangeDuty(FTM2_CH0_PB18,steer);
}


void turn_where()//�������
{
	int temp=0;
	temp=CCD_Right-CCD_Left;
	if(turn_left==true)//˵��������   CCD_Middle=CCD_Right-CCD_Left;
	{
	/*	if(CCD_Middle==55)
			CCD_Middle=55;
		else if(temp<34)
			CCD_Middle=34+temp%34;
		else if(temp>54)*/
			CCD_Middle=56;
	}
	else//����
	{
		/*if(CCD_Middle==-55)
			CCD_Middle=-55;
		else if(temp>-34)
			CCD_Middle=-34-temp%34;
		else if(temp<-54)*/
			CCD_Middle=-56;
	}
}

void changeParameter(uint8_t receiveBuffer[])
{
	int i=0,temp=0;
	
	for(i=0;i<10;i++)
	{
		if(receiveBuffer[i]>47 && receiveBuffer[i]<58)//0~9    receiveBuffer=.70.15.20.
			temp=temp*10+receiveBuffer[i]-48;
		UART_SendData(UART2,receiveBuffer[i]);
		receiveBuffer[i]='a';
	}
	target_speed=temp/10000;//701520->70
	
	steerKp=temp%10000;//701520->1520
	steerKp=steerKp/100;//1520->15
	
	steerKd=temp%100;//701520->20

	UART_printf(" t_speed,Kp,Kd  %d  %d   %d\r\n",target_speed,steerKp,steerKd);
				NVIC_EnableIRQ(PIT2_IRQn);
	    EnableInterrupts();
}

