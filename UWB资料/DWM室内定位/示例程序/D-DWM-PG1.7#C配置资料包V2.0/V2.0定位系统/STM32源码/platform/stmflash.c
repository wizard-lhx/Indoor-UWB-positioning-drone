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
#define SIZE 100	 			  	//���鳤��
#define FLASH_SAVE_ADDR  0X0800FB38 				//����FLASH �����ַ(����Ϊż��������ֵҪ���ڱ�������ռ��FLASH�Ĵ�С+0X08000000)

u16 save_x[SIZE];				//FLASH��������
//////////////////////////////////////////////////////////////////////////////////

//����STM32��FLASH
void STMFLASH_Unlock(void)
{
  FLASH->KEYR=FLASH_KEY1;//д���������.
  FLASH->KEYR=FLASH_KEY2;
}
//flash����
void STMFLASH_Lock(void)
{
  FLASH->CR|=1<<7;//����
}
//�õ�FLASH״̬
u8 STMFLASH_GetStatus(void)
{	
	u32 res;		
	res=FLASH->SR; 
	if(res&(1<<0))return 1;		    //æ
	else if(res&(1<<2))return 2;	//��̴���
	else if(res&(1<<4))return 3;	//д��������
	return 0;						//�������
}
//�ȴ��������
//time:Ҫ��ʱ�ĳ���
//����ֵ:״̬.
u8 STMFLASH_WaitDone(u16 time)
{
	u8 res;
	do
	{
		res=STMFLASH_GetStatus();
		if(res!=1)break;//��æ,����ȴ���,ֱ���˳�.
		delay_us(1);
		time--;
	 }while(time);
	 if(time==0)res=0xff;//TIMEOUT
	 return res;
}
//����ҳ
//paddr:ҳ��ַ
//����ֵ:ִ�����
u8 STMFLASH_ErasePage(u32 paddr)
{
	u8 res=0;
	res=STMFLASH_WaitDone(0X5FFF);//�ȴ��ϴβ�������,>20ms    
	if(res==0)
	{ 
		FLASH->CR|=1<<1;//ҳ����
		FLASH->AR=paddr;//����ҳ��ַ 
		FLASH->CR|=1<<6;//��ʼ����		  
		res=STMFLASH_WaitDone(0X5FFF);//�ȴ���������,>20ms  
		if(res!=1)//��æ
		{
			FLASH->CR&=~(1<<1);//���ҳ������־.
		}
	}
	return res;
}
//��FLASHָ����ַд�����
//faddr:ָ����ַ(�˵�ַ����Ϊ2�ı���!!)
//dat:Ҫд�������
//����ֵ:д������
u8 STMFLASH_WriteHalfWord(u32 faddr, u16 dat)
{
	u8 res;	   	    
	res=STMFLASH_WaitDone(0XFF);	 
	if(res==0)//OK
	{
		FLASH->CR|=1<<0;//���ʹ��
		*(vu16*)faddr=dat;//д������
		res=STMFLASH_WaitDone(0XFF);//�ȴ��������
		if(res!=1)//�����ɹ�
		{
			FLASH->CR&=~(1<<0);//���PGλ.
		}
	} 
	return res;
} 
//��ȡָ����ַ�İ���(16λ����) 
//faddr:����ַ 
//����ֵ:��Ӧ����.
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr; 
}
#if STM32_FLASH_WREN	//���ʹ����д   
//������д��
//WriteAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��   
void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{ 			 		 
	u16 i;
	for(i=0;i<NumToWrite;i++)
	{
		STMFLASH_WriteHalfWord(WriteAddr,pBuffer[i]);
	    WriteAddr+=2;//��ַ����2.
	}  
} 
//��ָ����ַ��ʼд��ָ�����ȵ�����
//WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)
//pBuffer:����ָ��
//NumToWrite:����(16λ)��(����Ҫд���16λ���ݵĸ���.)
#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //�ֽ�
#else 
#define STM_SECTOR_SIZE	2048
#endif		 
u16 STMFLASH_BUF[STM_SECTOR_SIZE/2];//�����2K�ֽ�
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)	
{
	u32 secpos;	   //������ַ
	u16 secoff;	   //������ƫ�Ƶ�ַ(16λ�ּ���)
	u16 secremain; //������ʣ���ַ(16λ�ּ���)	   
 	u16 i;    
	u32 offaddr;   //ȥ��0X08000000��ĵ�ַ
	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return;//�Ƿ���ַ
	STMFLASH_Unlock();						//����
	offaddr=WriteAddr-STM32_FLASH_BASE;		//ʵ��ƫ�Ƶ�ַ.
	secpos=offaddr/STM_SECTOR_SIZE;			//������ַ  0~127 for STM32F103RBT6
	secoff=(offaddr%STM_SECTOR_SIZE)/2;		//�������ڵ�ƫ��(2���ֽ�Ϊ������λ.)
	secremain=STM_SECTOR_SIZE/2-secoff;		//����ʣ��ռ��С   
	if(NumToWrite<=secremain)secremain=NumToWrite;//�����ڸ�������Χ
	while(1) 
	{	
		STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//������������������
		for(i=0;i<secremain;i++)//У������
		{
			if(STMFLASH_BUF[secoff+i]!=0XFFFF)break;//��Ҫ����  	  
		}
		if(i<secremain)//��Ҫ����
		{
			STMFLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//�����������
			for(i=0;i<secremain;i++)//����
			{
				STMFLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//д����������  
		}else STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 				   
		if(NumToWrite==secremain)break;//д�������
		else//д��δ����
		{
			secpos++;				//������ַ��1
			secoff=0;				//ƫ��λ��Ϊ0 	 
		   	pBuffer+=secremain;  	//ָ��ƫ��
			WriteAddr+=secremain*2;	//д��ַƫ��(16λ���ݵ�ַ,��Ҫ*2)	   
		   	NumToWrite-=secremain;	//�ֽ�(16λ)���ݼ�
			if(NumToWrite>(STM_SECTOR_SIZE/2))secremain=STM_SECTOR_SIZE/2;//��һ����������д����
			else secremain=NumToWrite;//��һ����������д����
		}	 
	};	
	STMFLASH_Lock();//����
}
#endif

//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)   	
{
	u16 i;
	for(i=0;i<NumToRead;i++)
	{
	
		pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//��ȡ2���ֽ�.
		ReadAddr+=2;//ƫ��2���ֽ�.	
	}
}

//////////////////////////////////////////������///////////////////////////////////////////
//WriteAddr:��ʼ��ַ
//WriteData:Ҫд�������
void Test_Write(u32 WriteAddr,u16 WriteData)   	
{
	STMFLASH_Write(WriteAddr,&WriteData,1);//д��һ���� 
}



void FLASH_write(void)
{
	u16 o,q;
	/*
u16   Flash_Usart_BaudRate;   //�豸����ͨѶ������ 0��4800  1��9600 2��14400 3��19200 4��38400 5��56000 6��57600 7��115200  8��128000 9��256000
u16   Flash_Modbus_ID;        //Modbus ID�� 
u16   Flash_Device_Mode;       //�豸ģʽ 0����ǩ 1���λ�վ 2������վ
u16   Flash_Device_ID;         //��8λΪ�λ�վID����Χ0~6  ��8λΪ��ǩID 0~247    �������ڲ� ��ǩIDΪ0~247  �λ�վIDΪ248~245  ����վIDΪ255��
u16   Flash_structure_Mode;     //0:Ϊ��άƽ��3��վģʽ
u16   Flash_EN_X_Y_Z[7][4];   //1����վ+6�λ�վ��λ�ã�    ʹ�ܱ�־��0Ϊ��  1Ϊ��   X,Y,Z�����ֽ�
	*/
	save_x[0]=Flash_Usart_BaudRate;
	save_x[1]=Flash_Modbus_ADDR;   //Modbus ID�� 
	save_x[2]=Flash_structure_Mode;  //1:Ϊ��άƽ��
	save_x[3]=Flash_Device_Mode;
	save_x[4]=Flash_Device_ID;
	save_x[5]=Flash_Ranging_Mode;//��෽ʽ
	save_x[6]=FLASH_Channel;              //�����ŵ�
  save_x[7]=FLASH_Data_rat;             //���д�������     
  save_x[8]=FLASH_RX_ANT_DLY;           //������ʱ
  save_x[9]=FLASH_KALMAN_Q;       	    //�������˲�-Q
  save_x[10]=FLASH_KALMAN_R;					    //�������˲�-R
	save_x[11]=Flash_TAG_NUM;       //��ǩ����

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
��FLASH���в�����ȡ����
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
	Flash_Ranging_Mode=save_x[5];//��෽ʽ
	FLASH_Channel=save_x[6];              //�����ŵ�
  FLASH_Data_rat=save_x[7];             //���д�������     
  FLASH_RX_ANT_DLY=save_x[8];           //������ʱ
  FLASH_KALMAN_Q=save_x[9];       	    //�������˲�-Q
  FLASH_KALMAN_R=save_x[10];					    //�������˲�-R
	Flash_TAG_NUM=save_x[11];       //��ǩ����
	
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
			//��������д�¹̼�����ΪĬ������
			Flash_Usart_BaudRate=0x0007;
			Flash_Modbus_ADDR=0x0001;
			Flash_Device_Mode=0x0000;
			Flash_Device_ID=0x0000;
			Flash_structure_Mode=0x0000;
			
			Flash_Ranging_Mode=0;//��෽ʽ
			FLASH_Channel=1;              //�����ŵ�
			FLASH_Data_rat=0;             //���д�������     
			FLASH_RX_ANT_DLY=32975;           //������ʱ
			FLASH_KALMAN_Q=3;       	    //�������˲�-Q
			FLASH_KALMAN_R=10;					    //�������˲�-R
			Flash_TAG_NUM=1;              //��ǩ����
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












