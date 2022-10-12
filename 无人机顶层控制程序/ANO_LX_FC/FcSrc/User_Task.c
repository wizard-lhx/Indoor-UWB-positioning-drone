#include "User_Task.h"
#include "Drv_RcIn.h"
#include "Drv_PID.h"
#include "Drv_Laser.h"
#include "Drv_Motor.h"
#include "LX_FC_Fun.h"
#include "ANO_LX.h"

u8 First_Position = 10;
u8 Second_Position = 9;
u8 Task = 1;//任务一二
u8 Mode = 1;//六种图形飞法
u8 Clever = 1;//是否使用openmv
u8 flag1 = 0, flag2 = 0;

//选择飞行顺序
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
    //一键起飞/降落例程
    //////////////////////////////////////////////////////////////////////
    //用静态变量记录一键起飞/降落指令已经执行。
    static u8 one_key_takeoff_f = 1, one_key_land_f = 1, one_key_mission_f = 0;
    static u8 mission_step;
    //判断有遥控信号才执行
    if (rc_in.fail_safe == 0)
    {
		//判断第7通道拨杆位置 
        if (rc_in.rc_ch.st_data.ch_[ch_7_aux3] > 800 && rc_in.rc_ch.st_data.ch_[ch_7_aux3] < 1200)
        {
			//任务一
            Task = 1;
        }
		else if (rc_in.rc_ch.st_data.ch_[ch_7_aux3] > 1800 && rc_in.rc_ch.st_data.ch_[ch_7_aux3] < 2200)
		{
			//任务二
			Task = 2;
		}
		//判断第8,9通道拨杆位置 
        if (rc_in.rc_ch.st_data.ch_[ch_8_aux4] > 800 && rc_in.rc_ch.st_data.ch_[ch_8_aux4] < 1200
			&& rc_in.rc_ch.st_data.ch_[ch_9_aux5] > 800 && rc_in.rc_ch.st_data.ch_[ch_9_aux5] < 1200)
        {
			//红三角
            Mode = 1;
        }
		else if (rc_in.rc_ch.st_data.ch_[ch_8_aux4] > 800 && rc_in.rc_ch.st_data.ch_[ch_8_aux4] < 1200
			&& rc_in.rc_ch.st_data.ch_[ch_9_aux5] > 1300 && rc_in.rc_ch.st_data.ch_[ch_9_aux5] < 1700)
		{
			//红正方
			Mode = 2;
		}
		else if (rc_in.rc_ch.st_data.ch_[ch_8_aux4] > 800 && rc_in.rc_ch.st_data.ch_[ch_8_aux4] < 1200
			&& rc_in.rc_ch.st_data.ch_[ch_9_aux5] > 1800 && rc_in.rc_ch.st_data.ch_[ch_9_aux5] < 2200)
		{
			//红圆
			Mode = 3;
		}
		else if (rc_in.rc_ch.st_data.ch_[ch_8_aux4] > 1800 && rc_in.rc_ch.st_data.ch_[ch_8_aux4] < 2200
			&& rc_in.rc_ch.st_data.ch_[ch_9_aux5] > 800 && rc_in.rc_ch.st_data.ch_[ch_9_aux5] < 1200)
        {
			//蓝三角
            Mode = 4;
        }
		else if (rc_in.rc_ch.st_data.ch_[ch_8_aux4] > 1800 && rc_in.rc_ch.st_data.ch_[ch_8_aux4] < 2200
			&& rc_in.rc_ch.st_data.ch_[ch_9_aux5] > 1300 && rc_in.rc_ch.st_data.ch_[ch_9_aux5] < 1700)
		{
			//蓝正方
			Mode = 5;
		}
		else if (rc_in.rc_ch.st_data.ch_[ch_8_aux4] > 1800 && rc_in.rc_ch.st_data.ch_[ch_8_aux4] < 2200
			&& rc_in.rc_ch.st_data.ch_[ch_9_aux5] > 1800 && rc_in.rc_ch.st_data.ch_[ch_9_aux5] < 2200)
		{
			//蓝圆
			Mode = 6;
		}
		//判断第10通道拨杆位置 
        if (rc_in.rc_ch.st_data.ch_[ch_10_aux6] > 800 && rc_in.rc_ch.st_data.ch_[ch_10_aux6] < 1200)
        {
			//不用openmv
			Clever = 1;
        }
		else if (rc_in.rc_ch.st_data.ch_[ch_7_aux3] > 1800 && rc_in.rc_ch.st_data.ch_[ch_7_aux3] < 2200)
		{
			//使用openmv
			Clever = 0;
		}
		Motor_DT_Transmit(0, Task);
		//发送一次指令
		if(Task == 1)
		{
			if(flag1 == 0)
			{
				Motor_DT_Transmit(0, Task);
				flag1++;
			}
		}		
		//发送一次指令并判断任务
		if(Task == 2)
		{   
			if(flag2 == 0)
			{
				Motor_DT_Transmit(0, Task);
				flag2++;
			}
			Mode_Chosen(Mode);			
		}
		
        //判断第6通道拨杆位置 1800<CH_6<2200
        if (rc_in.rc_ch.st_data.ch_[ch_6_aux2] > 1800 && rc_in.rc_ch.st_data.ch_[ch_6_aux2] < 2200)
        {
            //还没有执行
            if (one_key_takeoff_f == 0)
            {
                //标记已经执行
                one_key_takeoff_f =
                    //执行一键起飞
                    OneKey_Takeoff(100); //参数单位：厘米； 0：默认上位机设置的高度。
            }
        }
        else
        {
            //复位标记，以便再次执行
            one_key_takeoff_f = 0;
        }
        //
        //判断第6通道拨杆位置 800<CH_6<1200
        if (rc_in.rc_ch.st_data.ch_[ch_6_aux2] > 800 && rc_in.rc_ch.st_data.ch_[ch_6_aux2] < 1200)
        {
            //还没有执行
            if (one_key_land_f == 0)
            {
                //标记已经执行
                one_key_land_f =
                    //执行一键降落
                    OneKey_Land();
            }
        }
        else
        {
            //复位标记，以便再次执行
            one_key_land_f = 0;
        }
        //判断第6通道拨杆位置 1300<CH_6<1700
        if (rc_in.rc_ch.st_data.ch_[ch_6_aux2] > 1300 && rc_in.rc_ch.st_data.ch_[ch_6_aux2] < 1700)
        {
            //还没有执行
            if (one_key_mission_f == 0)
            {
                //标记已经执行
                one_key_mission_f = 1;
                //开始流程
                mission_step = 1;
            }
        }
        else
        {
            //复位标记，以便再次执行
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
					//定点模式
					mission_step += LX_Change_Mode(2);
				}
				break;
				case 2:
				{
					//解锁
					mission_step += FC_Unlock();
				}
				break;
				case 3:
				{
					//等2秒
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
					//起飞
					mission_step += OneKey_Takeoff(140);//参数单位：厘米； 0：默认上位机设置的高度。
				}
				break;
				case 5:
				{
					//等5秒
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
					//等5秒(测试用)
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
						//调节结束开启激光
						Laser_OnOff(1);
						Motor_DT_Transmit(0x02, Task);
					}
				}
				break;
				case 9:
				{
					//等16秒
					if(time_dly_cnt_ms<16000)
					{
						time_dly_cnt_ms+=20;//ms
					}
					else
					{
						//关闭激光
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
					//等1秒
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
					//等5秒(测试用)
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
						//调节结束开启激光
						Laser_OnOff(1);
						Motor_DT_Transmit(0x02, Task);
					}			
				}
				break;
				case 16:
				{
					//等16秒
					if(time_dly_cnt_ms<16000)
					{
						time_dly_cnt_ms+=20;//ms
					}
					else
					{
						//关闭激光
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
					//等1秒
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
