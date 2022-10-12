#include "Drv_PID.h"
#include "LX_FC_Fun.h"
#include "LX_FC_EXT_Sensor.h"
#include <stdio.h>
#include <math.h>

//飞机实际位置
u16 Position_X, Position_Y, Position_Z;

//飞机目标位置                          
u16 Position_XX[13] = {399, 355, 204, 132, 42,  42, 126, 275, 272, 353, 199, 204, 52};                
u16 Position_YY[13] = {346, 73,  231, 151, 380, 74, 299, 301, 142, 223, 374, 71, 220};                 

/*悬停控制(位置PID)*/
u8 PositionPID(int LIMIT1,u8 Position_i,u16 Position_ZZ) //PID输出限制，目标位置坐标号
{
	u8 PID_OVER_FLAG = 0;               //PID控制结束标志
	
	float VxErro,VyErro,VzErro;                //本次实际位置与目标位置误差
	static float lastVxErro,lastVyErro,lastVzErro; //上次实际位置与目标位置误差
	
	static float pidVx_pOut,pidVx_dOut,pidVx_iOut; //无人机x轴速度P,I,D分量输出
	static float pidVy_pOut,pidVy_dOut,pidVy_iOut; //无人机y轴速度P,I,D分量输出
	static float pidVz_pOut,pidVz_dOut,pidVz_iOut; //无人机z轴速度P,I,D分量输出
	static float pidVx_value,pidVy_value,pidVz_value;  //无人机x轴和y轴速度最终输出
	static unsigned char flag_Y,flag_X;            //积分分离标志位
	
	/***************X轴PID参数**ROLL************/
	float Vxkp=0.3f;//0.07
	float Vxki=0.00f;//0.001f;
	float Vxkd=0.01f;//-0.000531;0.027,0.005
	
	/***************Y轴PID参数*PITCH*************/
	float Vykp=0.3f;
	float Vyki=0.00f;//0.001f;
	float Vykd= 0.01f;//0.024
	/***************Z轴PID参数*PITCH*************/
	float Vzkp=0.4f;
	float Vzki=0.00f;//0.001f;
	float Vzkd= 0.01f;//0.024
	
	/*X轴位移速度调整*/
	VxErro=(float)(Position_Y - (Position_YY[Position_i])); //UWB坐标系和飞机坐标XY需调换
		
	float VxErroDelta=(VxErro-lastVxErro)/0.02f; //本次位置误差减上次位置误差
	lastVxErro=VxErro;                           //让上次误差变为本次误差
	
	/*X轴积分分离处理*/
	if(VxErro <= 50.0f&&VxErro >= -50.0f)																			 /*积分分离限度*/
	{
		flag_X = 1;
	}
	else
	{
		flag_X = 0;		
	}
	
	//P,I,D速度分量计算
	pidVx_pOut=Vxkp * VxErro;
	pidVx_dOut=Vxkd * VxErroDelta;
	pidVx_iOut+=Vxki * VxErro;
	
	//积分限幅
	if(pidVx_iOut>2.5f)//1.5
		pidVx_iOut=2.5f;
	if(pidVx_iOut<-2.5f)
		pidVx_iOut=-2.5f;
	
	//最终输出
	pidVx_value=pidVx_pOut+pidVx_dOut+flag_X*pidVx_iOut;
	
	//最后输出限幅防止太快
	if(pidVx_value>15)
		pidVx_value=15;
	if(pidVx_value<-15)
		pidVx_value=-15;
	
//	pidVx_value*=22;
		
	/***************Y轴PID调节***************/	
	/*Y轴位移速度调整*/
	
	VyErro=(float)(Position_X - Position_XX[Position_i]); //UWB坐标系和飞机坐标XY需调换
	
	float VyErroDelta=(VyErro-lastVyErro)/0.016f;
	lastVyErro=VyErro;
	
	/*Y轴积分分离处理*/
	if(VyErro <= 50.0f&&VyErro >= -50.0f)
	{
		flag_Y = 1;
	}
	else
	{
		flag_Y = 0;
	}
	
	pidVy_pOut=Vykp * VyErro;
	pidVy_dOut=Vykd * VyErroDelta;
	pidVy_iOut+=Vyki * VyErro;
	/*Y轴积分限幅处理*/
	if(pidVy_iOut>2.5f)
		pidVy_iOut=2.5f;
	if(pidVy_iOut<-2.5f)
		pidVy_iOut=-2.5f;
	
	pidVy_value=pidVy_pOut+pidVy_dOut+flag_Y*pidVy_iOut;
	
	/*Y轴输出限幅处理*/
	if(pidVy_value>15)
		pidVy_value=15;
	if(pidVy_value<-15)
		pidVy_value=-15;
	
	/***************Z轴PID调节***************/	
	/*Z轴位移速度调整*/
	
	VzErro=(float)(Position_ZZ - Position_Z); //UWB坐标系和飞机坐标XY需调换
	
	float VzErroDelta=(VzErro-lastVzErro)/0.016f;
	lastVzErro=VzErro;
	
	pidVz_pOut=Vzkp * VzErro;
	pidVz_dOut=Vzkd * VzErroDelta;
	pidVz_iOut+=Vzki * VzErro;
	
	pidVz_value=pidVz_pOut+pidVz_dOut;
	
	/*Z轴输出限幅处理*/
	if(pidVz_value>15)
		pidVz_value=15;
	if(pidVz_value<-15)
		pidVz_value=-15;
	
	//实时控制
	rt_tar.st_data.yaw_dps = 0;  //
	rt_tar.st_data.vel_x = (s16)pidVx_value;    //
	rt_tar.st_data.vel_y = (s16)(-pidVy_value);    //
	rt_tar.st_data.vel_z = (s16)pidVz_value;	 //

	if(pidVx_value < LIMIT1 && pidVy_value < LIMIT1 && pidVz_value < LIMIT1 &&
		pidVx_value > -LIMIT1 && pidVy_value > -LIMIT1 && pidVz_value > -LIMIT1)
	{
		PID_OVER_FLAG = 1;	
		rt_tar.st_data.vel_x = 0;    //
		rt_tar.st_data.vel_y = 0; 
		rt_tar.st_data.vel_z = 0;	//
	}
	
	return PID_OVER_FLAG;	
}

s16 Position_AG;

/*角度控制(角度PID)*/
u8 AnglePID(int LIMIT2) //PID输出限制
{
	Position_AG = fc_att.st_data.yaw_x100;
	u8 PID_OVER_FLAG2 = 0;
	static float lastAGErro; //上次角度误差
	static float pidAG_pOut,pidAG_dOut,pidAG_iOut; //角度P,I,D分量输出
	static float pidAG_value; //角度最终输出
//	static unsigned char flag_AG;
	
	/***************角度PID参数**ROLL************/
	float AGkp=0.1f;//
	float AGki=0.001f;//0.001f;
	float AGkd=0.007f;//-0.000531;0.027
	
	/***************角度PID******************/
	float AGErro=(float)(Position_AG / 100.0f);                       
	float AGErroDelta=(AGErro-lastAGErro)/0.016f;          //本次角度误差减上次角度误差
	lastAGErro=AGErro;                                     //上次角度误差等于本次角度误差

	pidAG_pOut=AGkp * AGErro;
	pidAG_dOut=AGkd * AGErroDelta;
	pidAG_iOut+=AGki * AGErro;
	
	//积分限幅
	if(pidAG_iOut>2.5f)//1.5
		pidAG_iOut=2.5f;
	if(pidAG_iOut<-2.5f)
		pidAG_iOut=-2.5f;
	
	pidAG_value = pidAG_pOut + pidAG_dOut + pidAG_iOut;        //角度输出值计算
	
	/*****************限幅处理*****************/	
	if(pidAG_value > 20)
		pidAG_value = 20;
	if(pidAG_value <- 20)
		pidAG_value =- 20;
	
	/*****************角度调节*****************/
	
	rt_tar.st_data.yaw_dps = (s16)pidAG_value;
	
	/****************调节结束，标志返回****************/
	
	if((Position_AG < LIMIT2) && (Position_AG > (0-LIMIT2)))
	{
		rt_tar.st_data.yaw_dps = 0;
		PID_OVER_FLAG2 = 1;
	}	

	return PID_OVER_FLAG2;
}
