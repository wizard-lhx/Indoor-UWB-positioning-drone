#ifndef __DRV_PID_H__
#define __DRV_PID_H__

#include "SysConfig.h"

#define PI 3.14159265

//飞机实际
extern u16 Position_X, Position_Y, Position_Z;
extern s16 Position_AG;

u8 PositionPID(int LIMIT1,u8 Position_i,u16 Position_ZZ);
u8 AnglePID(int LIMIT2); //PID输出限制

#endif
