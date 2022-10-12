#ifndef __MAIN_H
#define __MAIN_H
#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_sleep.h"
#include "port.h"
/* Example application name and version to display on LCD screen. ��ҵҺ��������ʾ��ʾ��Ӧ�ó������ƺͰ汾*/
#define APP_NAME "DS TWR INIT v1.1"

/* Inter-ranging delay period, in milliseconds. �����ʱ�䣬�Ժ���Ϊ��λ��*/
#define RNG_DELAY_MS 5
#define firmware_version 20 //�̼��汾�� 
/* Default communication configuration. We use here EVK1000's default mode (mode 3).Ĭ��ͨ�����á�����������ʹ��EVK1000��ģʽ3��*/
static dwt_config_t config = {
    2,               /* Channel number.ͨ���š�*/
    DWT_PRF_64M,     /* Pulse repetition frequency.�����ظ�Ƶ��*/
    DWT_PLEN_1024,   /* Preamble length. ǰ�����ȡ� */
    DWT_PAC32,       /* Preamble acquisition chunk size. Used in RX only. ǰ����ȡ���С��������RX�� */
    9,               /* TX preamble code. Used in TX only. TXǰ���롣ֻ��TXʹ�á� */
    9,               /* RX preamble code. Used in RX only. RXǰ���롣������RX��*/
    1,               /* Use non-standard SFD (Boolean)  ʹ�÷Ǳ�׼��SFD��������*/
    DWT_BR_110K,     /* Data rate. �������ʡ� */
    DWT_PHRMODE_STD, /* PHY header mode. PHYͷģʽ�� */
    (1025 + 64 - 32) /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only.
												SFD��ʱ��ǰ������+ 1 +SFD����-PAC��С����������RX��*/
};
/* Default antenna delay values for 64 MHz PRF. See NOTE 1 below. 64 MHz PRF��Ĭ�������ӳ�ֵ���������ע��1��*/
#define TX_ANT_DLY 0
//#define RX_ANT_DLY 32950
	
#define FINAL_MSG_TS_LEN 4  //ʱ�����ݳ���
extern u32 MODBUS_BaudRate[10];
extern int16_t SYS_dis_buf_t[7]; //����վ��ȡ���ľ������ݻ�����
extern u16 ERROR_FLAG;  //��������������־λ���ﵽһ����������

extern  u16   Flash_Usart_BaudRate;   //�豸����ͨѶ������ 0��4800  1��9600 2��14400 3��19200 4��38400 5��56000 6��57600 7��115200  8��128000 9��256000
extern  u16   Flash_Modbus_ADDR;        //Modbus ID�� 
extern  u16   Flash_structure_Mode;     //0:Ϊ��άƽ�� 3��վģʽ
extern  u16   Flash_Ranging_Mode;        //���ģʽ
extern  u16   Flash_Device_Mode;       //�豸ģʽ 0����ǩ 1���λ�վ 2������վ
extern  u16   Flash_Device_ID;         //��8λΪ�λ�վID����Χ0~6  ��8λΪ��ǩID 0~99    �������ڲ� ��ǩIDΪ0~247  �λ�վIDΪ248~245  ����վIDΪ255��
extern  u16   Flash_MAJOR_BS_X_Y_Z[3];   //����վ��λ�ã�      X,Y,Z�����ֽ�
extern  u16   Flash_BS_EN_X_Y_Z[7][4];   //7�λ�վ��λ�ã�    ʹ�ܱ�־��0Ϊ��  1Ϊ��   X,Y,Z�����ֽ�
extern  u16 	Flash_TAG_NUM;					//������ǩID
extern  u8 	  Flash_TAG_BUF[100];      //��ǩID�б�

extern  u16   Flash_FLAG;	 //��־λ
extern  u16   FLASH_Channel;              //�����ŵ�
extern  u16   FLASH_Data_rat;             //���д�������     
extern  u16   FLASH_RX_ANT_DLY;           //������ʱ
extern  u16   FLASH_KALMAN_Q;       	    //�������˲�-Q
extern  u16   FLASH_KALMAN_R;					    //�������˲�-R

extern  u16   Calculate_EN;       //����ʹ��
extern  u16		Calculate_FLAG;			//����״̬��־
extern  u16		Calculate_TAG_ID;						//����ִ�еı�ǩID
extern  u16		Calculate_TAG_X;		//������ǩ��X��
extern  u16		Calculate_TAG_Y;		//������ǩ��Y��
extern  u16		Calculate_TAG_Z;     //������ǩ��Z��
extern  u16		Calculate_Station_TAG[8]; //������ǩ�ľ���

extern  unsigned char USART_STR_BUF[300];   //�����õ��ַ����������
 void GPIO_Toggle(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
#endif


