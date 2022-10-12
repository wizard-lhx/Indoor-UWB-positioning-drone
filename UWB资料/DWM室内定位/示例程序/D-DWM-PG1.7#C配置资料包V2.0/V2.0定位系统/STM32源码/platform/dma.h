#ifndef __DMA_H
#define	__DMA_H	   

#include "stm32f10x.h"


void MYDMA_Config1(void);
void MYDMA_Config3(void);
void MYDMA_Enable1(u16 len);
void MYDMA_Enable3(u16 len);
#endif
