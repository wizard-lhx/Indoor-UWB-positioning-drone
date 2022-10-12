#include <string.h>
#include <stdio.h>
#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_sleep.h"
#include "port.h"
#include "usart.h"
#include "main.h"
//#include "modbus.h"
#include "stmflash.h"
#include "delay.h"
#define SIZE 100	 			  	//数组长度
#define FLASH_SAVE_ADDR  0X0800FB38 				//设置FLASH 保存地址(必须为偶数，且其值要大于本代码所占用FLASH的大小+0X08000000)

u16 save_x[SIZE];				//FLASH整数缓存
//////////////////////////////////////////////////////////////////////////////////

//解锁STM32的FLASH
void STMFLASH_Unlock(void)
{
  FLASH->KEYR=FLASH_KEY1;//写入解锁序列.
  FLASH->KEYR=FLASH_KEY2;
}
//flash上锁
void STMFLASH_Lock(void)
{
  FLASH->CR|=1<<7;//上锁
}
//得到FLASH状态
u8 STMFLASH_GetStatus(void)
{	
	u32 res;		
	res=FLASH->SR; 
	if(res&(1<<0))return 1;		    //忙
	else if(res&(1<<2))return 2;	//编程错误
	else if(res&(1<<4))return 3;	//写保护错误
	return 0;						//操作完成
}
//等待操作完成
//time:要延时的长短
//返回值:状态.
u8 STMFLASH_WaitDone(u16 time)
{
	u8 res;
	do
	{
		res=STMFLASH_GetStatus();
		if(res!=1)break;//非忙,无需等待了,直接退出.
		delay_us(1);
		time--;
	 }while(time);
	 if(time==0)res=0xff;//TIMEOUT
	 return res;
}
//擦除页
//paddr:页地址
//返回值:执行情况
u8 STMFLASH_ErasePage(u32 paddr)
{
	u8 res=0;
	res=STMFLASH_WaitDone(0X5FFF);//等待上次操作结束,>20ms    
	if(res==0)
	{ 
		FLASH->CR|=1<<1;//页擦除
		FLASH->AR=paddr;//设置页地址 
		FLASH->CR|=1<<6;//开始擦除		  
		res=STMFLASH_WaitDone(0X5FFF);//等待操作结束,>20ms  
		if(res!=1)//非忙
		{
			FLASH->CR&=~(1<<1);//清除页擦除标志.
		}
	}
	return res;
}
//在FLASH指定地址写入半字
//faddr:指定地址(此地址必须为2的倍数!!)
//dat:要写入的数据
//返回值:写入的情况
u8 STMFLASH_WriteHalfWord(u32 faddr, u16 dat)
{
	u8 res;	   	    
	res=STMFLASH_WaitDone(0XFF);	 
	if(res==0)//OK
	{
		FLASH->CR|=1<<0;//编程使能
		*(vu16*)faddr=dat;//写入数据
		res=STMFLASH_WaitDone(0XFF);//等待操作完成
		if(res!=1)//操作成功
		{
			FLASH->CR&=~(1<<0);//清除PG位.
		}
	} 
	return res;
} 
//读取指定地址的半字(16位数据) 
//faddr:读地址 
//返回值:对应数据.
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr; 
}
#if STM32_FLASH_WREN	//如果使能了写   
//不检查的写入
//WriteAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数   
void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{ 			 		 
	u16 i;
	for(i=0;i<NumToWrite;i++)
	{
		STMFLASH_WriteHalfWord(WriteAddr,pBuffer[i]);
	    WriteAddr+=2;//地址增加2.
	}  
} 
//从指定地址开始写入指定长度的数据
//WriteAddr:起始地址(此地址必须为2的倍数!!)
//pBuffer:数据指针
//NumToWrite:半字(16位)数(就是要写入的16位数据的个数.)
#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //字节
#else 
#define STM_SECTOR_SIZE	2048
#endif		 
u16 STMFLASH_BUF[STM_SECTOR_SIZE/2];//最多是2K字节
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)	
{
	u32 secpos;	   //扇区地址
	u16 secoff;	   //扇区内偏移地址(16位字计算)
	u16 secremain; //扇区内剩余地址(16位字计算)	   
 	u16 i;    
	u32 offaddr;   //去掉0X08000000后的地址
	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return;//非法地址
	STMFLASH_Unlock();						//解锁
	offaddr=WriteAddr-STM32_FLASH_BASE;		//实际偏移地址.
	secpos=offaddr/STM_SECTOR_SIZE;			//扇区地址  0~127 for STM32F103RBT6
	secoff=(offaddr%STM_SECTOR_SIZE)/2;		//在扇区内的偏移(2个字节为基本单位.)
	secremain=STM_SECTOR_SIZE/2-secoff;		//扇区剩余空间大小   
	if(NumToWrite<=secremain)secremain=NumToWrite;//不大于该扇区范围
	while(1) 
	{	
		STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//读出整个扇区的内容
		for(i=0;i<secremain;i++)//校验数据
		{
			if(STMFLASH_BUF[secoff+i]!=0XFFFF)break;//需要擦除  	  
		}
		if(i<secremain)//需要擦除
		{
			STMFLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//擦除这个扇区
			for(i=0;i<secremain;i++)//复制
			{
				STMFLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//写入整个扇区  
		}else STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);//写已经擦除了的,直接写入扇区剩余区间. 				   
		if(NumToWrite==secremain)break;//写入结束了
		else//写入未结束
		{
			secpos++;				//扇区地址增1
			secoff=0;				//偏移位置为0 	 
		   	pBuffer+=secremain;  	//指针偏移
			WriteAddr+=secremain*2;	//写地址偏移(16位数据地址,需要*2)	   
		   	NumToWrite-=secremain;	//字节(16位)数递减
			if(NumToWrite>(STM_SECTOR_SIZE/2))secremain=STM_SECTOR_SIZE/2;//下一个扇区还是写不完
			else secremain=NumToWrite;//下一个扇区可以写完了
		}	 
	};	
	STMFLASH_Lock();//上锁
}
#endif

//从指定地址开始读出指定长度的数据
//ReadAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)   	
{
	u16 i;
	for(i=0;i<NumToRead;i++)
	{
	
		pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//读取2个字节.
		ReadAddr+=2;//偏移2个字节.	
	}
}

//////////////////////////////////////////测试用///////////////////////////////////////////
//WriteAddr:起始地址
//WriteData:要写入的数据
void Test_Write(u32 WriteAddr,u16 WriteData)   	
{
	STMFLASH_Write(WriteAddr,&WriteData,1);//写入一个字 
}



void FLASH_write(void)
{
	u16 o,q;
	/*
u16   Flash_Usart_BaudRate;   //设备串口通讯波特率 0：4800  1：9600 2：14400 3：19200 4：38400 5：56000 6：57600 7：115200  8：128000 9：256000
u16   Flash_Modbus_ID;        //Modbus ID号 
u16   Flash_Device_Mode;       //设备模式 0：标签 1：次基站 2：主基站
u16   Flash_Device_ID;         //高8位为次基站ID，范围0~6  低8位为标签ID 0~247    （程序内部 标签ID为0~247  次基站ID为248~245  主基站ID为255）
u16   Flash_structure_Mode;     //0:为二维平面3基站模式
u16   Flash_EN_X_Y_Z[7][4];   //1主基站+6次基站的位置，    使能标志：0为关  1为开   X,Y,Z各两字节
	*/
	save_x[0]=Flash_Usart_BaudRate;
	save_x[1]=Flash_Modbus_ADDR;   //Modbus ID号 
	save_x[2]=Flash_structure_Mode;  //1:为二维平面
	save_x[3]=Flash_Device_Mode;
	save_x[4]=Flash_Device_ID;
	save_x[5]=Flash_Ranging_Mode;//测距方式
	save_x[6]=FLASH_Channel;              //空中信道
  save_x[7]=FLASH_Data_rat;             //空中传输速率     
  save_x[8]=FLASH_RX_ANT_DLY;           //接收延时
  save_x[9]=FLASH_KALMAN_Q;       	    //卡尔曼滤波-Q
  save_x[10]=FLASH_KALMAN_R;					    //卡尔曼滤波-R
	save_x[11]=Flash_TAG_NUM;       //标签数量

	for(q=0;q<3;q++)
	{
			save_x[12+q]=Flash_MAJOR_BS_X_Y_Z[q];
	}
	for(o=0;o<7;o++)
	{
			for(q=0;q<4;q++)
			{
				save_x[15+o*4+q]=Flash_BS_EN_X_Y_Z[o][q];
			}
	}
	for(q=0;q<50;q++)
	{
		save_x[43+q]=((Flash_TAG_BUF[2*q+1]<<8)&0xFF00)|Flash_TAG_BUF[2*q];
	}
	save_x[99]=Flash_FLAG;
	STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)save_x,202);
}
/****************************************************
对FLASH进行参数读取操作
****************************************************/
void FLASH_read(void)
{
	u16 o,q;
	STMFLASH_Read(FLASH_SAVE_ADDR,(u16*)save_x,202);
	
	Flash_Usart_BaudRate=save_x[0];
	Flash_Modbus_ADDR=save_x[1];
	Flash_structure_Mode=save_x[2];
	Flash_Device_Mode=save_x[3];
	Flash_Device_ID=save_x[4];
	Flash_Ranging_Mode=save_x[5];//测距方式
	FLASH_Channel=save_x[6];              //空中信道
  FLASH_Data_rat=save_x[7];             //空中传输速率     
  FLASH_RX_ANT_DLY=save_x[8];           //接收延时
  FLASH_KALMAN_Q=save_x[9];       	    //卡尔曼滤波-Q
  FLASH_KALMAN_R=save_x[10];					    //卡尔曼滤波-R
	Flash_TAG_NUM=save_x[11];       //标签数量
	
	for(q=0;q<3;q++)
	{
			Flash_MAJOR_BS_X_Y_Z[q]=save_x[12+q];
	}
	
	for(o=0;o<7;o++)
	{
			for(q=0;q<4;q++)
			{
				Flash_BS_EN_X_Y_Z[o][q]=save_x[15+o*4+q];
			}
	}
	for(q=0;q<50;q++)
	{
		Flash_TAG_BUF[2*q+1]=(save_x[43+q]>>8)&0xFF;
		Flash_TAG_BUF[2*q]=save_x[43+q]&0xFF;
	}
	
		Flash_FLAG=save_x[99];

		if(Flash_FLAG!=0xab) 
		{
			Flash_FLAG=0xab;
			//若程序烧写新固件以下为默认设置
			Flash_Usart_BaudRate=0x0007;
			Flash_Modbus_ADDR=0x0001;
			Flash_Device_Mode=0x0000;
			Flash_Device_ID=0x0000;
			Flash_structure_Mode=0x0000;
			
			Flash_Ranging_Mode=0;//测距方式
			FLASH_Channel=1;              //空中信道
			FLASH_Data_rat=0;             //空中传输速率     
			FLASH_RX_ANT_DLY=32975;           //接收延时
			FLASH_KALMAN_Q=3;       	    //卡尔曼滤波-Q
			FLASH_KALMAN_R=10;					    //卡尔曼滤波-R
			Flash_TAG_NUM=1;              //标签数量
			for(q=0;q<3;q++)
			{
			Flash_MAJOR_BS_X_Y_Z[q]=0x0000;
			}
	
			for(o=0;o<7;o++)
			{
					for(q=0;q<4;q++)
					{
							Flash_BS_EN_X_Y_Z[o][q]=0x0000;
					}
			}
			for(q=0;q<100;q++)
			{
			  Flash_TAG_BUF[q]=0x0000;
			}
			FLASH_write();
		}
		
}












