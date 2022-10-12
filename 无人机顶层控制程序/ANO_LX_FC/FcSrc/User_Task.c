#include "User_Task.h"
#include "Drv_RcIn.h"
#include "Drv_PID.h"
#include "Drv_Laser.h"
#include "Drv_Motor.h"
#include "LX_FC_Fun.h"
#include "ANO_LX.h"

u8 First_Position = 10;
u8 Second_Position = 9;
u8 Task = 1;//����һ��
u8 Mode = 1;//����ͼ�ηɷ�
u8 Clever = 1;//�Ƿ�ʹ��openmv
u8 flag1 = 0, flag2 = 0;

//ѡ�����˳��
void Mode_Chosen(u8 mode)
{
	switch(mode)
	{
		case 1:
		{
			First_Position = 1;
			Second_Position = 2;
		}
		break;
		case 2:
		{
			First_Position = 9;
			Second_Position = 10;
		}
		break;
		case 3:
		{
			First_Position = 5;
			Second_Position = 6;
		}
		break;
		case 4:
		{
			First_Position = 4;
			Second_Position = 3;
		}
		break;
		case 5:
		{
			First_Position = 11;
			Second_Position = 12;
		}
		break;
		case 6:
		{
			First_Position = 7;
			Second_Position = 8;
		}
		break;
	}
}

void UserTask_OneKeyCmd(void)
{
    //////////////////////////////////////////////////////////////////////
    //һ�����/��������
    //////////////////////////////////////////////////////////////////////
    //�þ�̬������¼һ�����/����ָ���Ѿ�ִ�С�
    static u8 one_key_takeoff_f = 1, one_key_land_f = 1, one_key_mission_f = 0;
    static u8 mission_step;
    //�ж���ң���źŲ�ִ��
    if (rc_in.fail_safe == 0)
    {
		//�жϵ�7ͨ������λ�� 
        if (rc_in.rc_ch.st_data.ch_[ch_7_aux3] > 800 && rc_in.rc_ch.st_data.ch_[ch_7_aux3] < 1200)
        {
			//����һ
            Task = 1;
        }
		else if (rc_in.rc_ch.st_data.ch_[ch_7_aux3] > 1800 && rc_in.rc_ch.st_data.ch_[ch_7_aux3] < 2200)
		{
			//�����
			Task = 2;
		}
		//�жϵ�8,9ͨ������λ�� 
        if (rc_in.rc_ch.st_data.ch_[ch_8_aux4] > 800 && rc_in.rc_ch.st_data.ch_[ch_8_aux4] < 1200
			&& rc_in.rc_ch.st_data.ch_[ch_9_aux5] > 800 && rc_in.rc_ch.st_data.ch_[ch_9_aux5] < 1200)
        {
			//������
            Mode = 1;
        }
		else if (rc_in.rc_ch.st_data.ch_[ch_8_aux4] > 800 && rc_in.rc_ch.st_data.ch_[ch_8_aux4] < 1200
			&& rc_in.rc_ch.st_data.ch_[ch_9_aux5] > 1300 && rc_in.rc_ch.st_data.ch_[ch_9_aux5] < 1700)
		{
			//������
			Mode = 2;
		}
		else if (rc_in.rc_ch.st_data.ch_[ch_8_aux4] > 800 && rc_in.rc_ch.st_data.ch_[ch_8_aux4] < 1200
			&& rc_in.rc_ch.st_data.ch_[ch_9_aux5] > 1800 && rc_in.rc_ch.st_data.ch_[ch_9_aux5] < 2200)
		{
			//��Բ
			Mode = 3;
		}
		else if (rc_in.rc_ch.st_data.ch_[ch_8_aux4] > 1800 && rc_in.rc_ch.st_data.ch_[ch_8_aux4] < 2200
			&& rc_in.rc_ch.st_data.ch_[ch_9_aux5] > 800 && rc_in.rc_ch.st_data.ch_[ch_9_aux5] < 1200)
        {
			//������
            Mode = 4;
        }
		else if (rc_in.rc_ch.st_data.ch_[ch_8_aux4] > 1800 && rc_in.rc_ch.st_data.ch_[ch_8_aux4] < 2200
			&& rc_in.rc_ch.st_data.ch_[ch_9_aux5] > 1300 && rc_in.rc_ch.st_data.ch_[ch_9_aux5] < 1700)
		{
			//������
			Mode = 5;
		}
		else if (rc_in.rc_ch.st_data.ch_[ch_8_aux4] > 1800 && rc_in.rc_ch.st_data.ch_[ch_8_aux4] < 2200
			&& rc_in.rc_ch.st_data.ch_[ch_9_aux5] > 1800 && rc_in.rc_ch.st_data.ch_[ch_9_aux5] < 2200)
		{
			//��Բ
			Mode = 6;
		}
		//�жϵ�10ͨ������λ�� 
        if (rc_in.rc_ch.st_data.ch_[ch_10_aux6] > 800 && rc_in.rc_ch.st_data.ch_[ch_10_aux6] < 1200)
        {
			//����openmv
			Clever = 1;
        }
		else if (rc_in.rc_ch.st_data.ch_[ch_7_aux3] > 1800 && rc_in.rc_ch.st_data.ch_[ch_7_aux3] < 2200)
		{
			//ʹ��openmv
			Clever = 0;
		}
		Motor_DT_Transmit(0, Task);
		//����һ��ָ��
		if(Task == 1)
		{
			if(flag1 == 0)
			{
				Motor_DT_Transmit(0, Task);
				flag1++;
			}
		}		
		//����һ��ָ��ж�����
		if(Task == 2)
		{   
			if(flag2 == 0)
			{
				Motor_DT_Transmit(0, Task);
				flag2++;
			}
			Mode_Chosen(Mode);			
		}
		
        //�жϵ�6ͨ������λ�� 1800<CH_6<2200
        if (rc_in.rc_ch.st_data.ch_[ch_6_aux2] > 1800 && rc_in.rc_ch.st_data.ch_[ch_6_aux2] < 2200)
        {
            //��û��ִ��
            if (one_key_takeoff_f == 0)
            {
                //����Ѿ�ִ��
                one_key_takeoff_f =
                    //ִ��һ�����
                    OneKey_Takeoff(100); //������λ�����ף� 0��Ĭ����λ�����õĸ߶ȡ�
            }
        }
        else
        {
            //��λ��ǣ��Ա��ٴ�ִ��
            one_key_takeoff_f = 0;
        }
        //
        //�жϵ�6ͨ������λ�� 800<CH_6<1200
        if (rc_in.rc_ch.st_data.ch_[ch_6_aux2] > 800 && rc_in.rc_ch.st_data.ch_[ch_6_aux2] < 1200)
        {
            //��û��ִ��
            if (one_key_land_f == 0)
            {
                //����Ѿ�ִ��
                one_key_land_f =
                    //ִ��һ������
                    OneKey_Land();
            }
        }
        else
        {
            //��λ��ǣ��Ա��ٴ�ִ��
            one_key_land_f = 0;
        }
        //�жϵ�6ͨ������λ�� 1300<CH_6<1700
        if (rc_in.rc_ch.st_data.ch_[ch_6_aux2] > 1300 && rc_in.rc_ch.st_data.ch_[ch_6_aux2] < 1700)
        {
            //��û��ִ��
            if (one_key_mission_f == 0)
            {
                //����Ѿ�ִ��
                one_key_mission_f = 1;
                //��ʼ����
                mission_step = 1;
            }
        }
        else
        {
            //��λ��ǣ��Ա��ٴ�ִ��
            one_key_mission_f = 0;
        }
        //
        if (one_key_mission_f == 1)
        {
			static u16 time_dly_cnt_ms;
			//
			switch(mission_step)
			{
				case 0:
				{
					//reset
					time_dly_cnt_ms = 0;
				}
				break;
				case 1:
				{
					//����ģʽ
					mission_step += LX_Change_Mode(2);
				}
				break;
				case 2:
				{
					//����
					mission_step += FC_Unlock();
				}
				break;
				case 3:
				{
					//��2��
					if(time_dly_cnt_ms<2000)
					{
						time_dly_cnt_ms+=20;//ms
					}
					else
					{
						time_dly_cnt_ms = 0;
						mission_step += 1;
					}
				}
				break;
				case 4:
				{
					//���
					mission_step += OneKey_Takeoff(140);//������λ�����ף� 0��Ĭ����λ�����õĸ߶ȡ�
				}
				break;
				case 5:
				{
					//��5��
					if(time_dly_cnt_ms<5000)
					{
						time_dly_cnt_ms+=20;//ms
					}
					else
					{
						time_dly_cnt_ms = 0;
						mission_step += 1;
					}					
				}
				break;
				case 6:
				{
					mission_step += PositionPID(3, First_Position, 150);
				}
				break;
				case 7:
				{
					//��5��(������)
					if(time_dly_cnt_ms<5000)
					{
						time_dly_cnt_ms+=20;//ms
					}
					else
					{
						time_dly_cnt_ms = 0;
						mission_step += 1;
					}					
				}
				break;
				case 8:
				{
					mission_step += PositionPID(3, First_Position, 80);
					if(mission_step == 9)
					{
						//���ڽ�����������
						Laser_OnOff(1);
						Motor_DT_Transmit(0x02, Task);
					}
				}
				break;
				case 9:
				{
					//��16��
					if(time_dly_cnt_ms<16000)
					{
						time_dly_cnt_ms+=20;//ms
					}
					else
					{
						//�رռ���
						Laser_OnOff(0);
						Motor_DT_Transmit(0x00, Task);
						time_dly_cnt_ms = 0;
						mission_step += 1;
					}					
				}
				break;
				case 10:
				{
					mission_step += PositionPID(3, First_Position, 150);
				}
				break;
				case 11:
				{
					mission_step += AnglePID(5);
				}
				break;
				case 12:
				{
					//��1��
					if(time_dly_cnt_ms<1000)
					{
						time_dly_cnt_ms+=20;//ms
					}
					else
					{
						time_dly_cnt_ms = 0;
						mission_step += 1;
					}	
				}
				break;
				case 13:
				{
					mission_step += PositionPID(3, Second_Position, 150);
				}
				break;
				case 14:
				{
					//��5��(������)
					if(time_dly_cnt_ms<1000)
					{
						time_dly_cnt_ms+=20;//ms
					}
					else
					{
						time_dly_cnt_ms = 0;
						mission_step += 1;
					}					
				}
				break;
				case 15:
				{
					mission_step += PositionPID(3, Second_Position, 80);
					if(mission_step == 16)
					{
						//���ڽ�����������
						Laser_OnOff(1);
						Motor_DT_Transmit(0x02, Task);
					}			
				}
				break;
				case 16:
				{
					//��16��
					if(time_dly_cnt_ms<16000)
					{
						time_dly_cnt_ms+=20;//ms
					}
					else
					{
						//�رռ���
						Laser_OnOff(0);
						Motor_DT_Transmit(0x00, Task);
						time_dly_cnt_ms = 0;
						mission_step += 1;
					}					
				}
				break;
				case 17:
				{
					mission_step += PositionPID(3, Second_Position, 150);
				}
				break;
				case 18:
				{
					mission_step += AnglePID(5);
				}
				break;
				case 19:
				{
					//��1��
					if(time_dly_cnt_ms<1000)
					{
						time_dly_cnt_ms+=20;//ms
					}
					else
					{
						time_dly_cnt_ms = 0;
						mission_step += 1;
					}
				}
				break;
				case 20:
				{
					mission_step += PositionPID(3,0,150);
				}
				break;
				case 21:
				{
					OneKey_Land();
				}
				break;
			}
        }
        else
        {
            mission_step = 0;
        }
    }
    ////////////////////////////////////////////////////////////////////////
}
