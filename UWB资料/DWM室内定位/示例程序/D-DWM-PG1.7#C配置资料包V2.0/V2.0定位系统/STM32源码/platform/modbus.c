#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_sleep.h"
#include "port.h"
#include "usart.h"
#include "stmflash.h"
#include "modbus.h"
#include "main.h"
#define ADDR 1   //ȡ��
#define NULL 0
#define MODBUS_LENGTH 150

u8 canbuf[8]={0xff,0x00,0x30,0x31,0x32,0x01,0x02,0x03};
unsigned int UsartNUM;
unsigned short modbus_reg[MODBUS_LENGTH];
const unsigned char auchCRCHi[] = /* CRC��λ�ֽڱ�*/
{ 	 
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40 
} ; 

const unsigned char auchCRCLo[] = /* CRC��λ�ֽڱ�*/ 
{ 
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC,
	0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 
	0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 
	0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10, 0xF0, 
	0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 
	0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 
	0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 
	0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 
	0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 
	0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 
	0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 
	0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 
	0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 
	0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 
	0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 0x44, 
	0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40
} ;	 

/******************************************************************************
												    CRCУ��
*******************************************************************************/
unsigned int CRC_Calculate(unsigned char *pdata,unsigned char num)
{
  unsigned char uchCRCHi = 0xFF ;               
	unsigned char  uchCRCLo = 0xFF ;               
	unsigned char uIndex ;                
	while(num --)                    
	{
		uIndex = uchCRCHi^*pdata++ ;           
		uchCRCHi = uchCRCLo^auchCRCHi[uIndex];
		uchCRCLo = auchCRCLo[uIndex];
	}
	return (uchCRCHi << 8 | uchCRCLo) ;
}
/******************************************************************************
												    ����Ӧ���ͺ���
*******************************************************************************/
void MODBUS_Return(unsigned char *buf,unsigned int length)
{
	if(UsartNUM==1) Usart1_SendString(buf,length);
	
	if(UsartNUM==3) 
	{	
		Usart3_SendString(buf,length);
	}
}

/******************************************************************************
												    ���ݽ��պ���
*******************************************************************************/
void MODBUS_Read(unsigned int startaddr,unsigned int number)
{
    //01 03 number 00 00 01 01 02 02 CRCH CRCL
    unsigned char send_length;
    unsigned char send_buf[300];
    unsigned char i;
    unsigned int crc;
	
    send_length = 0;
    send_buf[send_length++] = Flash_Modbus_ADDR;
    send_buf[send_length++] = 0x03;
    send_buf[send_length++] = number*2;
    for (i = 0;i < number;i++)
    {
        send_buf[send_length++] = modbus_reg[startaddr+i]/256;
        send_buf[send_length++] = modbus_reg[startaddr+i]%256;
    }
    crc = CRC_Calculate(send_buf,send_length);
    send_buf[send_length++] = crc/256;
    send_buf[send_length++] = crc%256;
    MODBUS_Return(send_buf,send_length);
}
/******************************************************************************
												    ����MODBUSЭ���ַ���
*******************************************************************************/
void MODBUS_WriteString(unsigned int addr,unsigned int code)
{
    unsigned char send_buf[8];
	unsigned int crc;
	send_buf[0] = Flash_Modbus_ADDR;
	send_buf[1] = 16;
	send_buf[2] = (addr / 256);
	send_buf[3] = (addr % 256);
	send_buf[4] = (code / 256);
	send_buf[5] = (code % 256);
	crc = CRC_Calculate(send_buf,6);
	send_buf[6] = (crc / 256);
  send_buf[7] = (crc % 256);
	MODBUS_Return(send_buf,8);
}

/******************************************************************************
												    MODBUS�������ݴ�����
*******************************************************************************/
void MODBUS(unsigned char *buf,unsigned int length,unsigned int NUM)
{
	unsigned int startaddr,number,code;
	unsigned int crc;
	UsartNUM=NUM;
	  crc = CRC_Calculate(buf,length-2);

    if (buf[0] == Flash_Modbus_ADDR&&buf[length-2] == (crc / 256)&&buf[length-1] == (crc % 256))
    {
			unsigned int Write_EN=0;
	
			MODBUS_datain();
        switch(buf[1])
        {
            case 0x03:    //��ȡ����&����
                startaddr = buf[2]*256 + buf[3];
                number = buf[4]*256 + buf[5];
								if ((startaddr+number) > MODBUS_LENGTH)
								{	
									break;
								}					
								MODBUS_Read(startaddr,number);
                break;
            case 0x06:      //дһ������
                startaddr = buf[2]*256 + buf[3];
                code = buf[4]*256 + buf[5];
								if ((startaddr) > MODBUS_LENGTH)
								{	
									break;
								}
                modbus_reg[startaddr] = code;
                MODBUS_Return(buf,length);
								Write_EN=1;
								break;
								
							
						case 0x10:     //д������
								startaddr = buf[2]*256 + buf[3];
								number = buf[4]*256 + buf[5];
								if ((startaddr+number) > MODBUS_LENGTH)
								{	
									break;
								}
								for (code = 0;code < number;code++)
								{
									modbus_reg[startaddr + code] = buf[7+code*2]*256 + buf[8+code*2];
								}			
								MODBUS_WriteString(startaddr,number);
								Write_EN=1;  //�ǵ��޸ģ�ֻ��FLASH�������˲���Ҫд��FLASH��
								break;	
						
						
            default:
                break;
        }
				MODBUS_dataout();
			  if(Write_EN==1) FLASH_write();
    }
}


/******************************************************************************
												    ���ڽ����¼�
*******************************************************************************/
void MODBUS_event() 
{
	unsigned int U1_EN=0;
	unsigned int U3_EN=0;
	if(usart1_rx_length)
	{
		switch(usart1_rx_buf[1])
		{
			case 0x03: if (usart1_rx_length==8) U1_EN=1;	
				break;
			case 0x06: if (usart1_rx_length==8) U1_EN=1;			
				break;
			case 0x10:if (usart1_rx_length==((usart1_rx_buf[5]*2)+9)) U1_EN=1;		
				break;
			default: 
				break;
		}
		if (time3_usart1 >= (10-Flash_Usart_BaudRate)*5) U1_EN=1;	
	}	
	if(usart3_rx_length)
	{
		switch(usart3_rx_buf[1])
		{
			case 0x03: if (usart3_rx_length==8) U3_EN=1;	
				break;
			case 0x06: if (usart3_rx_length==8) U3_EN=1;			
				break;
			case 0x10:if (usart3_rx_length==((usart3_rx_buf[5]*2)+9)) U3_EN=1;		
				break;
			default: 
				break;
		}
		if (time3_usart3 >= (10-Flash_Usart_BaudRate)*5) U3_EN=1;	
	}
	
	
	if (U1_EN==1)
		{

			//Usart3_SendString("123",3);
			MODBUS(usart1_rx_buf,usart1_rx_length,1);
			usart1_rx_length = 0;
			time3_usart1 = 0;
		}
		if (U3_EN==1)
		{

			//Usart3_SendString("123",3);
			MODBUS(usart3_rx_buf,usart3_rx_length,3);
			usart3_rx_length = 0;
			time3_usart3 = 0;
		}
}
/******************************************************************************
												    ���ݵ���MODBUS�Ĵ�����
*******************************************************************************/
void MODBUS_datain() 
{
	u16 o,q;
	
	modbus_reg[0]=Flash_Usart_BaudRate;
	modbus_reg[1]=Flash_Modbus_ADDR;
	modbus_reg[2]=(Flash_Ranging_Mode<<8&0xFF00)|Flash_structure_Mode;
	modbus_reg[3]=Flash_Device_Mode;
	modbus_reg[4]=Flash_Device_ID;
	modbus_reg[5]=(FLASH_Channel<<8&0xFF00)|FLASH_Data_rat;//�����ŵ�  ���д�������
  modbus_reg[6]=FLASH_KALMAN_Q;       	    //�������˲�-Q
	modbus_reg[7]=FLASH_KALMAN_R;       	    //�������˲�-R
  modbus_reg[8]=FLASH_RX_ANT_DLY;           //������ʱ

	
	
	for(q=0;q<3;q++)
		{
				modbus_reg[q+9]=Flash_MAJOR_BS_X_Y_Z[q];
		}
		for(o=0;o<7;o++)
		{
			for(q=0;q<4;q++)
			{
					modbus_reg[12+o*4+q]=Flash_BS_EN_X_Y_Z[o][q];
			}
		}
	modbus_reg[40]=Calculate_EN;
	modbus_reg[41]=Flash_TAG_NUM;
	modbus_reg[42]=Calculate_TAG_ID;
	modbus_reg[43]=Calculate_FLAG;
	modbus_reg[44]=Calculate_TAG_X;
	modbus_reg[45]=Calculate_TAG_Y;
	modbus_reg[46]=Calculate_TAG_Z; 
		
		for(o=0;o<8;o++)
	{
		modbus_reg[47+o]=Calculate_Station_TAG[o];
	}
	for(o=0;o<50;o++)
	{
		modbus_reg[55+o]=((Flash_TAG_BUF[2*o+1]<<8)&0xFF00)|Flash_TAG_BUF[2*o];
	}
	
	modbus_reg[105]=firmware_version; 	
	
	
	
}
/******************************************************************************
												   ��MODBUS�Ĵ������������
*******************************************************************************/
void MODBUS_dataout() 
{
		u16 o,q;
	  //��д�����ݲ���Ҫ��ֻ���Ĳ��ø��裡
		if(modbus_reg[0]<=9) Flash_Usart_BaudRate=modbus_reg[0];
		if(modbus_reg[1]<=255) Flash_Modbus_ADDR=modbus_reg[1];
	  if(((modbus_reg[2]>>8)&0xFF)<=1)Flash_Ranging_Mode=(modbus_reg[2]>>8)&0xFF; //��෽ʽ
		if((modbus_reg[2]&0xFF)<=1) Flash_structure_Mode=modbus_reg[2]&0xFF;
		if(modbus_reg[3]<=3) Flash_Device_Mode=modbus_reg[3];
		if((((modbus_reg[4]>>8)&0xFF)<=6)&&((modbus_reg[4]&0xFF)<=100)) Flash_Device_ID=modbus_reg[4];
	  if(((modbus_reg[5]>>8)&0xFF)<=5) FLASH_Channel=(modbus_reg[5]>>8)&0xFF;          //�����ŵ� 
	  if((modbus_reg[5]&0xFF)<=5) FLASH_Data_rat=modbus_reg[5]&0xFF;                   //���д�������

     FLASH_KALMAN_Q=modbus_reg[6];       	    //�������˲�-Q
	   FLASH_KALMAN_R=modbus_reg[7];       	    //�������˲�-R
     FLASH_RX_ANT_DLY=modbus_reg[8];           //������ʱ
	
		for(q=0;q<3;q++)
		{
				Flash_MAJOR_BS_X_Y_Z[q]=modbus_reg[q+9];
		}
		for(o=0;o<7;o++)
		{			
			for(q=0;q<4;q++)
			{  
				  if(q == 0) 
					{					
							if((modbus_reg[12+o*4+q]<=1))						
							Flash_BS_EN_X_Y_Z[o][q]=modbus_reg[12+o*4+q];
					}
					else
					{
							Flash_BS_EN_X_Y_Z[o][q]=modbus_reg[12+o*4+q];
					}
					
			}
		}
		
		if(modbus_reg[40]<=8) Calculate_EN=modbus_reg[40];			
	
		if((modbus_reg[41]>0)&&(modbus_reg[41]<=100))Flash_TAG_NUM=modbus_reg[41];
		
		for(q=0;q<50;q++)
		{
			if(((modbus_reg[55+q]>>8)&0xFF)<=99) 
			{
				Flash_TAG_BUF[2*q+1]=(modbus_reg[55+q]>>8)&0xFF;
			
			}
			if((modbus_reg[55+q]&0xFF)<=99) 
			{
				Flash_TAG_BUF[2*q]=modbus_reg[55+q]&0xFF;
			}
		}
		
		
}
/******************************************************************************
												    �Զ����XYZ������Ϣ
*******************************************************************************/
void MODBUS_xyz(u16 ID,u16 x,u16 y,u16 z,u16 *dis_bb)
{
    //01 03 number 00 00 01 01 02 02 CRCH CRCL
    unsigned char send_length;
    unsigned char send_buf[100];
    unsigned int crc;
		unsigned int i;
    send_length = 0;
    send_buf[send_length++] = Flash_Modbus_ADDR;
    send_buf[send_length++] = 0x03;
    send_buf[send_length++] = 24;
		
	 send_buf[send_length++] = ID/256;
	 send_buf[send_length++] = ID%256;
	
		send_buf[send_length++] = Calculate_FLAG/256;
	  send_buf[send_length++] = Calculate_FLAG%256;
	
    send_buf[send_length++] = x/256;
    send_buf[send_length++] = x%256;
   
		send_buf[send_length++] = y/256;
    send_buf[send_length++] = y%256;
		
		send_buf[send_length++] = z/256;
    send_buf[send_length++] = z%256;
    for(i=0;i<8;i++)
		{
		send_buf[send_length++] =dis_bb[i]/256;
    send_buf[send_length++] =dis_bb[i]%256;
		}
		
		crc = CRC_Calculate(send_buf,send_length);
    send_buf[send_length++] = crc/256;
    send_buf[send_length++] = crc%256;
    MODBUS_Return(send_buf,send_length);
}




