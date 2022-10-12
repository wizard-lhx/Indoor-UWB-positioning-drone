#ifndef __modbus_H
#define __modbus_H
#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_sleep.h"
#include "port.h"

void MODBUS(unsigned char *buf,unsigned int length,unsigned int UsartNUM);
void MODBUS_ReadFromEEPROM(void);
void MODBUS_event(void);
void MODBUS_datain(void); 
void MODBUS_dataout(void); 
void MODBUS_xyz(u16 ID,u16 x,u16 y,u16 z,u16 *dis_bb);
unsigned int CRC_Calculate(unsigned char *pdata,unsigned char num);
extern unsigned short modbus_reg[];



#endif	



