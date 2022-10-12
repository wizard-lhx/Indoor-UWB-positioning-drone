
 /*! ----------------------------------------------------------------------------
 *V2.0
 *֧��һ��һ��࣬���վ���ǩ��ά��λ
 * @author ���������Ƽ����޹�˾
 */
 
#include <string.h>
#include <stdio.h>
#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_sleep.h"
#include "port.h"
#include "usart.h"
#include "stmflash.h"
#include "main.h"
#include "delay.h"
#include "modbus.h"
#include "timer.h"
#include "loc.h"
#include "oled.h"
#include "dma.h"

typedef signed long long int64;
typedef unsigned long long uint64;
typedef unsigned long long uint64;

/******************************************************************************
														�ڴ�FLASH������
*******************************************************************************/
u16   Flash_Usart_BaudRate;       //�豸����ͨѶ������ 0��4800  1��9600 2��14400 3��19200 4��38400 5��56000 6��57600 7��115200  8��128000 9��256000
u16   Flash_Modbus_ADDR;        	//Modbus ID�� 
u16   Flash_structure_Mode;     	//0:Ϊ���ģʽ 1:��άģʽ 3����ά�Զ�ģʽ
u16   Flash_Ranging_Mode; //���ģʽ
u16   Flash_Device_Mode;       		//�豸ģʽ 0����ǩ 1���λ�վ 2������վ
u16   Flash_Device_ID;        	  //��8λΪ�λ�վID����Χ0~6  ��8λΪ��ǩID 0~99    �������ڲ� ��ǩIDΪ0~247  �λ�վIDΪ248~254  ����վIDΪ255��
u16   Flash_MAJOR_BS_X_Y_Z[3];    //����վ��λ�ã�      X,Y,Z�����ֽ�
u16   Flash_BS_EN_X_Y_Z[7][4];    //7�λ�վ��λ�ã�    ʹ�ܱ�־��0Ϊ��  1Ϊ��   X,Y,Z�����ֽ�
u16 	Flash_TAG_NUM;					//������ǩID
u8 	  Flash_TAG_BUF[100];      //��ǩID�б�

u16   Flash_FLAG;	 								//��־λ
u16   FLASH_Channel;              //�����ŵ�
u16   FLASH_Data_rat;             //���д�������     
u16   FLASH_RX_ANT_DLY;           //������ʱ
u16   FLASH_KALMAN_Q;       	    //�������˲�-Q
u16   FLASH_KALMAN_R;					    //�������˲�-R

/******************************************************************************
														 ����ʹ�ñ���
*******************************************************************************/
u16   Calculate_EN;       			//����ʹ��
u16		Calculate_FLAG;						//����״̬��־
u16		Calculate_TAG_ID;						//����ִ�еı�ǩID
u16		Calculate_TAG_X;					//������ǩ��X��
u16		Calculate_TAG_Y;					//������ǩ��Y��
u16		Calculate_TAG_Z;     			//������ǩ��Z��
u16		Calculate_Station_TAG[8]; //������ǩ�ľ���
u16   Calculate_Last_Dist[100];      //�ش�����ʹ�õĻ��棬���ڴλ�վ������վģʽ��õı�ǩ������һ�λش�����ǩ
u16   Calculate_Last_XYZ[100][3];       //�ش�����ʹ�õĻ��棬���ڴλ�վ������վģʽ��õı�ǩ������һ�λش�����ǩ
u16   Calculate_Receive_MODE=0;      //��ǩ���յ��ľ������
u16   Calculate_Receive_Dist=0;      //��ǩ���յ��ľ������
u16   Calculate_Receive_XYZ[3];      //��ǩ���յ��ľ������
u16   Calculate_Receive_FLAG=0;      //��ǩ���յ�����Ϣ��־λ
u16   Calculate_TAG_FLAG=0;      //��λģʽ�У�ѭ����ǩID�ı�־λ
u16   Calculate_KEY_NUM_FLAG=0;      //����ɨ����̵ļƴα�־λ
/******************************************************************************
														ϵͳ����ѭ����־λ
*******************************************************************************/
u8 SYS_Calculate_ACTIVE_FLAG=0;   				//ϵͳѭ����־λ������ຯ��
u8 SYS_Calculate_PASSIVE_FLAG=0;   				//ϵͳѭ����־λ������ຯ��
u8 SYS_BS_FLAG=0;    				//ϵͳѭ����־λ-�λ�վ
u8 SYS_MAJOR_BS_FLAG=0;     //ϵͳѭ����־λ-����վ
u8 SYS_BS_TAG_FLAG=0;       //�λ�վ�յ���Ҫ����ǩ��ID
u8 SYS_BS_MESSAGE_FLAG=0;       //����վ��ϵ�λ�վ��־λ
u16 SYS_BS_MESSAGE_Timer_FLAG=0;       //����վ�ȴ��λ�վ�ظ�����������ݴ���������

/******************************************************************************
														ϵͳ�����Զ���
*******************************************************************************/

u32 MODBUS_BaudRate[10]={4800,9600,14400,19200,38400,56000,57600,115200,128000,256000};    //�������б�
u32 AIR_Chan[6]={1,2,3,4,5,7};    //�����ŵ�
int16_t SYS_dis_buf_t[7];  	//����վ��ȡ���ľ������ݻ�����
u16 TAG_ID=0;   							//����ǩID
u16 ERROR_FLAG;  						//��������������־λ���ﵽһ����������
uint16_t LED_FLAG=0;     		//ϵͳָʾ�Ƽ�¼��־λ
uint16_t KEY_FLAG=0;     		//ϵͳ������¼��־λ
unsigned char USART_STR_BUF[300];   //�����õ��ַ����������
u16 DWM1000_SET_DATA[3];     //DWM1000������ò����Ƿ񱻸ı�Ļ�������
/******************************************************************************
														DWM1000���������
*******************************************************************************/
// DWM1000ͨѶ���ݰ�                 A_ID  B_ID  ֡    ����  
static uint8 Send_get_dist_msg[] = {0X00, 0x00, 0x00, 0xCA,'1','1','1','1','2','2','2','2','3','3','3','3','4','4','4','4','5','5','5','5','6','6','6','6','D','D','D','D'};
	
#define RX_BUF_LEN 32               //DWM1000�������ݰ�����
static uint8 rx_buffer[RX_BUF_LEN]; //DWM1000�������ݰ�������
	
static int16_t dist[20];    //LP��ͨ�˲�����
static double tof;             //����
static double distance,dist2;  //���۾��� ���������
int32_t dis;             		//�����뻺��   
uint32 frame_len;        		//DWM1000�շ����ݰ����Ȼ���
static uint32 Time_ts[6];  					//����ʱ�仺���¼
	
/* Frame sequence number, incremented after each transmission. ֡���кţ�ÿ�δ��������� */
static uint32 frame_seq_nb = 0;

/* Hold copy of status register state here for reference, so reader can examine it at a breakpoint. 
�����ﱣ��״̬�Ĵ���״̬�ĸ����Թ��ο����Ա���߿����ڶϵ��ϼ������*/
static uint32 status_reg = 0;
/* UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) conversion factor.
UWB΢�루UUS�����豸ʱ�䵥λ��DTU��Լ15.65 ps����ת��ϵ����
 * 1 uus = 512 / 499.2 �s and 1 �s = 499.2 * 128 dtu. 
  1 UUS��512��499.2��1��499.2��128 DTU��*/
#define UUS_TO_DWT_TIME 65536
#define SPEED_OF_LIGHT 299702547

/******************************************************************************
														��̬��������
*******************************************************************************/
static uint64 get_tx_timestamp_u64(void);
static uint64 get_rx_timestamp_u64(void);
static void final_msg_set_ts(uint8 *ts_field, uint64 ts);
static void final_msg_set_dist(uint8 *ts_field, uint32 dist);
void OLED_display(void);//OLED��ʾ����
void OLED_display_data(int16 dist,int16 X,int16 Y,u16 MODE);  //���������ʾ����

/******************************************************************************
												    ��վ���б�ǩ�����в��
*******************************************************************************/
int32_t DW1000send(uint8_t A_ID,uint8_t B_ID,uint8_t MODE) //����ģʽ
{
		   
			  uint32 i;
				if(SYS_Calculate_ACTIVE_FLAG==0)
				{
						for(i=0;i<30;i++)
						{
								Send_get_dist_msg[i]=0x00;
						}
						Send_get_dist_msg[0] =  A_ID;	//UWB POLL ������
						Send_get_dist_msg[1] =  B_ID;//UWB Fianl ������
						Send_get_dist_msg[2] = frame_seq_nb;
						Send_get_dist_msg[3]=0XAB; 
						Send_get_dist_msg[4]=0;    //��ʼ��
						if(MODE==1)
						{
							Send_get_dist_msg[4]=1;     //�ش������Ϣ
							Send_get_dist_msg[5]=Calculate_Last_Dist[B_ID]/256;		//������һ�εľ�����Ϣ
							Send_get_dist_msg[6]=Calculate_Last_Dist[B_ID]%256;		//������һ�εľ�����Ϣ
						}
            else if(MODE==2)		
						{
							Send_get_dist_msg[4]=2;     //�ش������Ϣ
							Send_get_dist_msg[7]=Calculate_Last_XYZ[B_ID][0]/256;		//������һ�ε�������Ϣ
							Send_get_dist_msg[8]=Calculate_Last_XYZ[B_ID][0]%256;		//������һ�ε�������Ϣ				
							Send_get_dist_msg[9]=Calculate_Last_XYZ[B_ID][1]/256;		//������һ�ε�������Ϣ
							Send_get_dist_msg[10]=Calculate_Last_XYZ[B_ID][1]%256;		//������һ�ε�������Ϣ										
						}							
						dwt_writetxdata(sizeof(Send_get_dist_msg), Send_get_dist_msg, 0);//��Poll�����ݴ���DW1000�����ڿ�������ʱ����ȥ
						dwt_writetxfctrl(sizeof(Send_get_dist_msg), 0);//���ó�����������ݳ���
						dwt_setrxaftertxdelay(0);
						dwt_setrxtimeout(60000);						//���ý��ճ�ʱʱ��
						dwt_starttx(DWT_START_TX_IMMEDIATE| DWT_RESPONSE_EXPECTED);//�������ͣ�������ɺ�ȴ�һ��ʱ�俪�����գ��ȴ�ʱ����dwt_setrxaftertxdelay������;
						SYS_Calculate_ACTIVE_FLAG=1;
				}			
				if(SYS_Calculate_ACTIVE_FLAG==1)
				{			
			//		printf("%d\n",SYS_Calculate_ACTIVE_FLAG); 
						if((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR))//���ϲ�ѯоƬ״ֱ̬���ɹ����ջ��߷�������
						{
							SYS_Calculate_ACTIVE_FLAG=2;
						}
						else return 0;
				}
				if(SYS_Calculate_ACTIVE_FLAG==2)
				{
	//				printf("%d\n",SYS_Calculate_ACTIVE_FLAG); 
						if(frame_seq_nb<0xFFFFFFFF)frame_seq_nb++;
						else frame_seq_nb=0;
						if (status_reg & SYS_STATUS_RXFCG)//����ɹ�����
						{									
								SYS_Calculate_ACTIVE_FLAG=3;
						}
						else 
						{
							 
								dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);		
							SYS_Calculate_ACTIVE_FLAG=0;
							  return	0;	
						}
				}
								
        if(SYS_Calculate_ACTIVE_FLAG==3)
				{
			//		printf("%d\n",SYS_Calculate_ACTIVE_FLAG); 
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);//����Ĵ�����־λ
            frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;	//��ý��յ������ݳ���
            dwt_readrxdata(rx_buffer, frame_len, 0);   //��ȡ��������
            
						if ((rx_buffer[3]==0xBC)&&((rx_buffer[0]==B_ID)&&(rx_buffer[1]==A_ID)))//�жϽ��յ��������Ƿ���response����
            {  
								SYS_Calculate_ACTIVE_FLAG=4;
						}
						else 
						{
							dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);	
							SYS_Calculate_ACTIVE_FLAG=0;
							return 0;							
						}		
				}
				if(SYS_Calculate_ACTIVE_FLAG==4)
				{
			//		printf("%d\n",SYS_Calculate_ACTIVE_FLAG); 
							  for(i=0;i<30;i++)
								{
									 Send_get_dist_msg[i]=rx_buffer[i];
								}								
                Time_ts[0] = get_tx_timestamp_u64();										//���POLL����ʱ��T1
                Time_ts[3] = get_rx_timestamp_u64();										//���RESPONSE����ʱ��T4
                final_msg_set_ts(&Send_get_dist_msg[4],Time_ts[0]);//��T1д�뷢������
                final_msg_set_ts(&Send_get_dist_msg[16],Time_ts[3]);//��T4д�뷢������
								Send_get_dist_msg[0] =  A_ID;	//UWB POLL ������
								Send_get_dist_msg[1] =  B_ID;//UWB Fianl ������
							  Send_get_dist_msg[3]=0XCD; 
                Send_get_dist_msg[2] = frame_seq_nb;
                dwt_writetxdata(sizeof(Send_get_dist_msg), Send_get_dist_msg, 0);//����������д��DW1000
                dwt_writetxfctrl(sizeof(Send_get_dist_msg), 0);//�趨�������ݳ���
								dwt_setrxaftertxdelay(0);
								dwt_setrxtimeout(5000);						//���ý��ճ�ʱʱ��
                dwt_starttx(DWT_START_TX_IMMEDIATE| DWT_RESPONSE_EXPECTED);//�趨Ϊ�ӳٷ���
								SYS_Calculate_ACTIVE_FLAG=5;						
					}
				
					if(SYS_Calculate_ACTIVE_FLAG==5)						
					{
								//printf("%d\n",SYS_Calculate_ACTIVE_FLAG); 
								if ((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR))//���ϲ�ѯоƬ״ֱ̬���ɹ����ջ��߷�������
								{ 
									SYS_Calculate_ACTIVE_FLAG=6;
								}
								else return 0;
					}
					if(SYS_Calculate_ACTIVE_FLAG==6)
					{
						//	printf("%d\n",SYS_Calculate_ACTIVE_FLAG); 	
							if(frame_seq_nb<0xFFFFFFFF)frame_seq_nb++;
							else frame_seq_nb=0;
							if (status_reg & SYS_STATUS_RXFCG)//����ɹ�����
							{	
									SYS_Calculate_ACTIVE_FLAG=7;	
							}
							else 
							{
									
									dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);	
									SYS_Calculate_ACTIVE_FLAG=0;								
									return	0;								
							}
					}
					if(SYS_Calculate_ACTIVE_FLAG==7)
					{
							dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);//����Ĵ�����־λ
							frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;	//��ý��յ������ݳ���
							dwt_readrxdata(rx_buffer, frame_len, 0);   //��ȡ��������						
							if ((rx_buffer[3]==0xDE)&&((rx_buffer[0]==B_ID)&&(rx_buffer[1]==A_ID)))//�жϽ��յ��������Ƿ���response����
							{
									SYS_Calculate_ACTIVE_FLAG=8;
							}
							else 
							{
									dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);	
									SYS_Calculate_ACTIVE_FLAG=0;
									return 0;
							}
					}
					if(SYS_Calculate_ACTIVE_FLAG==8)
					{
							uint32 Time_ts_F[6];
							double Ra, Rb, Da, Db;
               int64 tof_dtu;
							 for(i=0;i<30;i++)
							{								
									Send_get_dist_msg[i]=rx_buffer[i];
							}
							final_msg_get_ts(&rx_buffer[4], &Time_ts_F[0]);
							final_msg_get_ts(&rx_buffer[8], &Time_ts_F[1]);
							final_msg_get_ts(&rx_buffer[12], &Time_ts_F[2]);
							final_msg_get_ts(&rx_buffer[16], &Time_ts_F[3]);
							final_msg_get_ts(&rx_buffer[24], &Time_ts_F[5]);									
							Time_ts[0]= (uint32)Time_ts_F[0];
							Time_ts[1]= (uint32)Time_ts_F[1];
							Time_ts[2]= (uint32)Time_ts_F[2];
							Time_ts[3]= (uint32)Time_ts_F[3];
							Time_ts[4]= (uint32)get_tx_timestamp_u64();	
							Time_ts[5]= (uint32)Time_ts_F[5];

							Ra = (double)(Time_ts[3] - Time_ts[0]);//Tround1 = T4 - T1  
              Rb = (double)(Time_ts[5] - Time_ts[2]);//Tround2 = T6 - T3 
              Da = (double)(Time_ts[4] - Time_ts[3]);//Treply2 = T5 - T4  
              Db = (double)(Time_ts[2] - Time_ts[1]);//Treply1 = T3 - T2  
              tof_dtu = (int64)((Ra * Rb - Da * Db) / (Ra + Rb + Da + Db));//���㹫ʽ
              tof = tof_dtu * DWT_TIME_UNITS;
              distance = tof * SPEED_OF_LIGHT;//����=����*����ʱ��
							dist2 = distance - dwt_getrangebias(config.chan,(float)distance, config.prf);//�����ȥ����ϵ��	
							dis = dist2*100;//dis Ϊ��λΪcm�ľ���		
							SYS_Calculate_ACTIVE_FLAG=0;
							return dis;			
					}				
					return 0;   
}



/******************************************************************************
												    ��ǩӦ���վ�����в��
*******************************************************************************/
int32_t DW1000receive(uint8_t B_ID) //����
{
				uint32 i;
	      if(SYS_Calculate_PASSIVE_FLAG==0)  //�򿪽���
				{
						ERROR_FLAG=0;        //�����־��0		
						dwt_setrxtimeout(0);//�趨���ճ�ʱʱ�䣬0λû�г�ʱʱ��
						dwt_rxenable(0);//�򿪽���
						SYS_Calculate_PASSIVE_FLAG=1;
				}
				if(SYS_Calculate_PASSIVE_FLAG==1)  //�ȴ�����
				{
						if((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR))//���ϲ�ѯоƬ״ֱ̬�����ճɹ����߳��ִ���
						{
							SYS_Calculate_PASSIVE_FLAG=2;
						}
						else return 0;
				}					
						
				if(SYS_Calculate_PASSIVE_FLAG==2)  //��֤�Ƿ�ɹ�����
				{
						if (status_reg & SYS_STATUS_RXFCG)//�ɹ�����
						{
								SYS_Calculate_PASSIVE_FLAG=3;
						}
						else
						{
								/* Clear RX error events in the DW1000 status register. */
								dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
								SYS_Calculate_PASSIVE_FLAG=0;							
								return 0;
						}
				}
				if(SYS_Calculate_PASSIVE_FLAG==3)   //�ж��Ƿ�Ϊ��Ч���ݰ�
				{
							dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG);//�����־λ
							frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;//��ý������ݳ���
							dwt_readrxdata(rx_buffer, frame_len, 0);//��ȡ��������
							if (rx_buffer[3]==0xAB&&B_ID==rx_buffer[1])//�ж�����
							{       
									SYS_Calculate_PASSIVE_FLAG=4;							
							}
							else 
							{	
								  SYS_Calculate_PASSIVE_FLAG=0;
									return 0;
							}
					}
					if(SYS_Calculate_PASSIVE_FLAG==4)  //�������ݺ�򿪽���
					{
							for(i=0;i<30;i++)
							{
									Send_get_dist_msg[i]=rx_buffer[i];
							}											
							if(rx_buffer[0]==0xFF) //�ж��Ƿ�Ϊ����վ����Ϣ
							{
								Calculate_Receive_MODE=rx_buffer[4];
								Calculate_Receive_Dist=rx_buffer[5]*256+rx_buffer[6];	
								Calculate_Receive_XYZ[0]=rx_buffer[7]*256+rx_buffer[8];	
								Calculate_Receive_XYZ[1]=rx_buffer[9]*256+rx_buffer[10];	
							}
							Time_ts[1] = get_rx_timestamp_u64();//���Poll������ʱ��T2
							final_msg_set_ts(&Send_get_dist_msg[8],Time_ts[1]);//��T2д�뷢������
							Send_get_dist_msg[0]=B_ID;
							Send_get_dist_msg[1]=rx_buffer[0];
							Send_get_dist_msg[2] = frame_seq_nb;
							Send_get_dist_msg[3]=0XBC; 
							dwt_writetxdata(sizeof(Send_get_dist_msg), Send_get_dist_msg, 0);//д�뷢������
							dwt_writetxfctrl(sizeof(Send_get_dist_msg), 0);//�趨���ͳ���
							dwt_setrxaftertxdelay(0);//���÷��ͺ������գ����趨�ӳ�ʱ��
							dwt_setrxtimeout(5000);						//���ý��ճ�ʱʱ��
							dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);//�������ͣ��ȴ�����      
							SYS_Calculate_PASSIVE_FLAG=5;						
					}							
					if(SYS_Calculate_PASSIVE_FLAG==5)  //�ȴ�����
					{
								if ((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR))///���ϲ�ѯоƬ״ֱ̬�����ճɹ����߳��ִ���
								{ 
										SYS_Calculate_PASSIVE_FLAG=6;
								}
								else return 0;
					}
					if(SYS_Calculate_PASSIVE_FLAG==6)  //��֤�Ƿ�ɹ�����
					{							
							if(frame_seq_nb<0xFFFFFFFF)frame_seq_nb++;
							else frame_seq_nb=0;
							if (status_reg & SYS_STATUS_RXFCG)//���ճɹ�
							{
									SYS_Calculate_PASSIVE_FLAG=7;
							}
							else
							{
                   /* Clear RX error events in the DW1000 status register. */
                   dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);			
									SYS_Calculate_PASSIVE_FLAG=0;
									return 0;
							}
					}
					if(SYS_Calculate_PASSIVE_FLAG==7)  //�ж��Ƿ�Ϊ��Ч����
					{															
							dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);//�����־λ
							frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;//���ݳ���
							dwt_readrxdata(rx_buffer, frame_len, 0);//��ȡ��������
							if ((rx_buffer[3]==0xCD)&&(B_ID==rx_buffer[1]))//�ж��Ƿ�ΪFianl��
							{
								SYS_Calculate_PASSIVE_FLAG=8;
							}
							else 
							{
								SYS_Calculate_PASSIVE_FLAG=0;
								return 0;
							}
					}						
					if(SYS_Calculate_PASSIVE_FLAG==8)  //��������
					{						
							for(i=0;i<30;i++)
							{
									Send_get_dist_msg[i]=rx_buffer[i];
							}
												
              /* Retrieve response transmission and final reception timestamps. */
							Send_get_dist_msg[0]=B_ID;
							Send_get_dist_msg[1]=rx_buffer[0];
							Send_get_dist_msg[3]=0XDE; 
							Time_ts[2] = get_tx_timestamp_u64();//���response����ʱ��T3
							Time_ts[5] = get_rx_timestamp_u64();//���final����ʱ��T6
							final_msg_set_ts(&Send_get_dist_msg[12],Time_ts[2]);//��T3д�뷢������
							final_msg_set_ts(&Send_get_dist_msg[24],Time_ts[5]);//��T6д�뷢������
							dwt_writetxdata(sizeof(Send_get_dist_msg), Send_get_dist_msg, 0);//д�뷢������
							dwt_writetxfctrl(sizeof(Send_get_dist_msg), 0);//�趨���ͳ���
							dwt_starttx(DWT_START_TX_IMMEDIATE );//�趨Ϊ�ӳٷ���
							SYS_Calculate_PASSIVE_FLAG=9;
					}						
					if(SYS_Calculate_PASSIVE_FLAG==9)  //��֤�Ƿ������
					{						
							if (dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS)//���ϲ�ѯоƬ״ֱ̬���������
							{ 
									dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);//�����־λ										
									SYS_Calculate_PASSIVE_FLAG=0;
									GPIO_Toggle(LED_GPIO,LED);
								  if(rx_buffer[0]==0xFF) 
									{
										Calculate_Receive_FLAG++; //�����������վͨѶ������OLED��ʾ
										return 2;
									}
								  
								return 1;
							}
							else return 0;
					 }									
				 return 0;	
}


/******************************************************************************
												    ����վͨѶ�λ�վ���в�ಢ�ȴ���������
*******************************************************************************/
int32_t DW1000send_dist_msg(uint8_t A_ID,uint8_t B_ID,uint8_t C_ID) //ͨѶ�λ�վ���
{
				uint32 i=0;
		   if(SYS_BS_MESSAGE_FLAG==0)
			 {
				
						SYS_BS_MESSAGE_Timer_FLAG=0;//��־λ����
						for(i=0;i<30;i++)
						{
								Send_get_dist_msg[i]=0x00;
						}
						Send_get_dist_msg[0] =  A_ID;	//UWB POLL ������
						Send_get_dist_msg[1] =  B_ID;//UWB Fianl ������
						Send_get_dist_msg[2] = frame_seq_nb;
						Send_get_dist_msg[3]=0XEF; 
						Send_get_dist_msg[4]=C_ID;
						dwt_writetxdata(sizeof(Send_get_dist_msg), Send_get_dist_msg, 0);//��Poll�����ݴ���DW1000�����ڿ�������ʱ����ȥ
						dwt_writetxfctrl(sizeof(Send_get_dist_msg), 0);//���ó�����������ݳ���
						dwt_setrxaftertxdelay(0);
						dwt_starttx(DWT_START_TX_IMMEDIATE);//��������
						dwt_setrxtimeout(6000);//�趨���ճ�ʱʱ�䣬0λû�г�ʱʱ��				
						deca_sleep(7);//���߹̶�ʱ��	
						SYS_BS_MESSAGE_FLAG=1;					
			 }
			 if(SYS_BS_MESSAGE_FLAG==1)
			 {
					dwt_rxenable(0);//�򿪽���
				  SYS_BS_MESSAGE_FLAG=2;
			 }
			 if(SYS_BS_MESSAGE_FLAG==2)
			 {				
				  
						if((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR))//���ϲ�ѯоƬ״ֱ̬���ɹ����ջ��߷�������
						{
							
								SYS_BS_MESSAGE_FLAG=3;							
						}	
						else return 0;
				}
			 if(SYS_BS_MESSAGE_FLAG==3)
			 {
						if(frame_seq_nb<0xFFFFFFFF)frame_seq_nb++;
						else frame_seq_nb=0;
						if (status_reg & SYS_STATUS_RXFCG)//����ɹ�����
						{
							SYS_BS_MESSAGE_FLAG=4;
						}
						else 
						{
								dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);		
								if(SYS_BS_MESSAGE_Timer_FLAG>7)  //������������7�����½��գ�����7������ͨѶ���
								{
										SYS_BS_MESSAGE_FLAG=0;	
								}
								else
								{
										SYS_BS_MESSAGE_FLAG=1;	//�����������½���
										SYS_BS_MESSAGE_Timer_FLAG++;
								}						
								//SYS_BS_MESSAGE_FLAG=0;//����ʼ�� ��Ϊ�ȴ����Ź��̣��в�������յ���Ч���ݰ�
								return 0;
						}
				}
			  if(SYS_BS_MESSAGE_FLAG==4)
				{	
								dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);//����Ĵ�����־λ
								frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;	//��ý��յ������ݳ���
								dwt_readrxdata(rx_buffer, frame_len, 0);   //��ȡ��������
								if ((rx_buffer[3])==0xFF&&((rx_buffer[0]==B_ID)&&(rx_buffer[1]==A_ID)))//�жϽ��յ��������Ƿ���response����
								{     
								
										SYS_BS_MESSAGE_FLAG=5;									
								}
								else 
								{
									ERROR_FLAG=0;        //�����־��0
										dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);		
									//SYS_BS_MESSAGE_FLAG=0;//����ʼ�� ��Ϊ�ȴ����Ź��̣��в�������յ���Ч���ݰ�											
										if(SYS_BS_MESSAGE_Timer_FLAG>7)  //������������7�����½��գ�����7������ͨѶ���
										{
												SYS_BS_MESSAGE_FLAG=0;	
										}
										else
										{
											ERROR_FLAG=0;//���ܵ��λ�վ��ͨѶ��¼��֤����վ���ڣ������־����
											  SYS_BS_MESSAGE_FLAG=1;	//�����������½���
												SYS_BS_MESSAGE_Timer_FLAG++;
										}
										return 0;
								}
				}
				if(SYS_BS_MESSAGE_FLAG==5)
				{
							uint32 rec_dist;
							final_msg_get_ts(&rx_buffer[4], &rec_dist);
							SYS_BS_MESSAGE_FLAG=0;
							return rec_dist;
				}
				return 0;
			
}


/******************************************************************************
												    �λ�վӦ������վ���в�ಢ��������
*******************************************************************************/
int32_t DW1000send_dist_msg_last(uint8_t A_ID,uint8_t B_ID,uint32_t dist) //�λ�վ��Ӧ����վ
{
		   
			  uint32 i;
				for(i=0;i<30;i++)
				{
						Send_get_dist_msg[i]=0x00;
				}
				Send_get_dist_msg[0] =  A_ID;	//UWB POLL ������
				Send_get_dist_msg[1] =  B_ID;//UWB Fianl ������
			  Send_get_dist_msg[2] = frame_seq_nb;
			  Send_get_dist_msg[3]=0XFF; 
				final_msg_set_dist(&Send_get_dist_msg[4], dist);
        dwt_writetxdata(sizeof(Send_get_dist_msg), Send_get_dist_msg, 0);//��Poll�����ݴ���DW1000�����ڿ�������ʱ����ȥ
        dwt_writetxfctrl(sizeof(Send_get_dist_msg), 0);//���ó�����������ݳ���
				dwt_setrxaftertxdelay(0);
				dwt_setrxtimeout(5000);						//���ý��ճ�ʱʱ��
        dwt_starttx(DWT_START_TX_IMMEDIATE);//��������   				
				return 0;
			
}


/******************************************************************************
												    �λ�վ�ȴ�����վ�´��������źţ�����ǲ�����Ҳ���б�����࣬���ڻ�վ�궨����
*******************************************************************************/
uint8_t DW1000rec_dist_msg(uint8_t B_ID) //�λ�վ�ȴ��ź� 
{
		uint32 i;
		if(SYS_BS_FLAG==0)
		{
        dwt_setrxtimeout(65535);//�趨���ճ�ʱʱ�䣬0λû�г�ʱʱ��
        dwt_rxenable(0);//�򿪽���
				SYS_BS_FLAG=1;
		}
		if(SYS_BS_FLAG==1)
		{
			
        if((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR))//���ϲ�ѯоƬ״ֱ̬�����ճɹ����߳��ִ���
        { 
					SYS_BS_FLAG=2;
				}
				else return 0;
			
		}		
		if(SYS_BS_FLAG==2)
		{
        if (status_reg & SYS_STATUS_RXFCG)//�ɹ�����
        {
						SYS_BS_FLAG=3;	 
				}
        else
        {
            /* Clear RX error events in the DW1000 status register. */
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
					SYS_BS_FLAG=0;
					return 0;
        }
		}
		if(SYS_BS_FLAG==3)
		{
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG);//�����־λ
				frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;//��ý������ݳ���
				dwt_readrxdata(rx_buffer, frame_len, 0);//��ȡ��������			 
        if (rx_buffer[3]==0xEF&&B_ID==rx_buffer[1])//�ж�
        {
						SYS_BS_FLAG=4;
						return rx_buffer[4];
         }
				else 
				{
						SYS_BS_FLAG=0;
						return 0;
				}	
		}					
		return 0;		
}

/******************************************************************************
												         ��ǩģʽ����
*******************************************************************************/
void MODE_TAG(void)
{
	    int32_t dis_buf=0;							
			dis_buf=DW1000receive(Flash_Device_ID&0x00FF);	
      if(Calculate_Receive_FLAG>6)
			{
				OLED_display_data(Calculate_Receive_Dist,Calculate_Receive_XYZ[0],Calculate_Receive_XYZ[1],Calculate_Receive_MODE);				//��Ļ��ʾ�Ῠס������ͨѶ		������Ļ��ʾ�ή�Ͳ������
				Calculate_Receive_FLAG=0;
			}				
			if(dis_buf==2)  //�������
			{
				u16 len=0;
				if(Calculate_Receive_MODE==1)//���ģʽ
				{
					char a[5]="DIST=";	
					char b[5];		  				
					char c[8]="cm    ";		
					char show_buf[18]="";	
					sprintf(b, "%d", Calculate_Receive_Dist);
					strcat(show_buf,a); 
					strcat(show_buf,b); 
					strcat(show_buf,c); 
					len=strlen(show_buf);
					Usart1_SendString(show_buf,len);
					Usart3_SendString(show_buf,len);
				}
				if(Calculate_Receive_MODE==2)//��λģʽ
				{
					char a[2]="X=";	
					char b[5];		  				
					char c[8]="cm ";					
					char d[2]="Y=";	
					char e[5];		  				
					char f[8]="cm    ";							
					char show_buf[18]="";				
					sprintf(b, "%d", Calculate_Receive_XYZ[0]);				
					strcat(show_buf,a); 
					strcat(show_buf,b); 
					strcat(show_buf,c); 				
					sprintf(e, "%d", Calculate_Receive_XYZ[1]);	
					strcat(show_buf,d); 
					strcat(show_buf,e); 
					strcat(show_buf,f); 
					len=strlen(show_buf);
					Usart1_SendString(show_buf,len);
					Usart3_SendString(show_buf,len);
				}
				
			}
			if(dis_buf!=0)//���ɹ�
			{   		
					ERROR_FLAG=0;        //�����־��0				
					if(LED_FLAG>3)
					{
							GPIO_Toggle(LED_GPIO,LED);
					    LED_FLAG=0;						  
					} 
					else LED_FLAG++;
						
			}
			if(ERROR_FLAG>1500)   //��෢������
			{				
				SYS_Calculate_PASSIVE_FLAG=0;
				ERROR_FLAG=0;        //�����־��0
			}					
}
/******************************************************************************
												         �λ�վ����
*******************************************************************************/
void MODE_SUB_ANCHOR(void)
{
								if(SYS_BS_FLAG!=4) 
								{							
									ERROR_FLAG=0;        //�����־��0
									SYS_BS_TAG_FLAG=DW1000rec_dist_msg(((Flash_Device_ID>>8)&0xFF)+248);									
							  }
								if(SYS_BS_FLAG==4)
								{
									int32_t dis_buf=0;
									dis_buf=DW1000send(((Flash_Device_ID>>8)&0xFF)+248,SYS_BS_TAG_FLAG,0);																		
									if(dis_buf!=0)
									{
											DW1000send_dist_msg_last(((Flash_Device_ID>>8)&0xFF)+248,255,dis_buf);	//���;���
										  if(LED_FLAG>6)
											{
													GPIO_Toggle(LED_GPIO,LED);
													LED_FLAG=0;
											}
											else LED_FLAG++;
											SYS_BS_FLAG=0;	
									}																
									
								}
									if(ERROR_FLAG>1500)   //��෢������
									{				
											if(SYS_BS_FLAG==4) DW1000send_dist_msg_last(((Flash_Device_ID>>8)&0xFF)+248,255,0);	//����Ƕ�λģʽ����Ȼ���ʧ�ܣ�Ҳ������Ϣ
										  SYS_BS_FLAG=0;	
										  SYS_Calculate_ACTIVE_FLAG=0;
										  ERROR_FLAG=0;        //�����־��0
									}						
									
}
/******************************************************************************
												         ����վ����
*******************************************************************************/

void MODE_MAJOR_ANCHOR(void)
{
							if(Flash_structure_Mode==0) //���ģʽ
							{
								int8_t o=0;
								if((SYS_MAJOR_BS_FLAG!=0)&&(Flash_TAG_BUF[0] != TAG_ID))//�������б�ǩID���޸ľ�ȡ��������¿�ʼ  ��ֹ�����̱��޸�
									{
											SYS_MAJOR_BS_FLAG=0;							
									}
									if(SYS_MAJOR_BS_FLAG==0)
									{
											Calculate_FLAG=0;                                    //״̬��־Ϊ���
											if(Calculate_EN==0x01||Calculate_EN==0x02||Calculate_EN==0x03||Calculate_EN==0x04)		
											{														
													TAG_ID=Flash_TAG_BUF[Calculate_TAG_FLAG];												
												  Calculate_TAG_ID=TAG_ID;
													SYS_MAJOR_BS_FLAG=1;								
													ERROR_FLAG=0;                     //�����־Ϊ��0												
											}										
									}
									if(SYS_MAJOR_BS_FLAG==1) //����վ���ǩ���
									{										   		
											SYS_dis_buf_t[0]=DW1000send(255,TAG_ID,1);		//����Ĵ���MODBUS														
											if(SYS_dis_buf_t[0]!=0)
											{
												Calculate_FLAG|=0x01<<0;		//�ɹ���ำ���־	
													SYS_MAJOR_BS_FLAG=2;
													//deca_sleep(RNG_DELAY_MS);//���߹̶�ʱ��
											}
										
									}		
									if(SYS_MAJOR_BS_FLAG==2)
									{			
										if((Calculate_FLAG>>0)&0x01) //���ɹ����˲�ˢ�£����ʧ�ܱ�����һ�ε�����	
										{											
												dist[0] = LP(SYS_dis_buf_t[0],8*TAG_ID);//LP Ϊ��ͨ�˲����������ݸ��ȶ�	
										}
										Calculate_Last_Dist[TAG_ID]=dist[0];//����ش���ǩ��������ֵ										
									  OLED_display_data(Calculate_Last_Dist[TAG_ID],0,0,1);  //OLED��ʾ����
										
										Calculate_TAG_X=0; //���赽�Ĵ���
										Calculate_TAG_Y=0; //���赽�Ĵ���
										Calculate_TAG_Z=0; //���赽�Ĵ���		
									
										Calculate_Station_TAG[0]=dist[0];//����A��վ���ֵ
		
											for(o=0;o<7;o++)   //����B~G��վ���ֵ
											{
												
													Calculate_Station_TAG[o+1]=0;																								
											}									
											
											if(Calculate_EN==0x03||Calculate_EN==0x04)//�Զ����
											{
												MODBUS_xyz(Calculate_TAG_ID,Calculate_TAG_X,Calculate_TAG_Y,Calculate_TAG_Z,Calculate_Station_TAG);									
											}
											
											if(Calculate_EN==0x01||Calculate_EN==0x03)//���μ�������ʹ��
											{												
												Calculate_EN=0;
											}	
											
											if(Calculate_TAG_FLAG<(Flash_TAG_NUM-1))  //ID�б�ɨ�� ��ID���
											{
												Calculate_TAG_FLAG++;
											}
											else
											{
												Calculate_TAG_FLAG=0;
											}
											
												if(LED_FLAG>6)
											{
													GPIO_Toggle(LED_GPIO,LED);
													LED_FLAG=0;
											}
											else LED_FLAG++;
											SYS_MAJOR_BS_FLAG=0;
										}
									if(ERROR_FLAG>1000)   //��෢������
									{				
										SYS_MAJOR_BS_FLAG=2;
										SYS_Calculate_ACTIVE_FLAG=0;//ϵͳѭ����־λ��������־λ
										ERROR_FLAG=0;        //�����־��0

									}
							}						
							if(Flash_structure_Mode==1)//��ά��λģʽ
							 {									
									int8_t o=0,i=0;								  
								  if((SYS_MAJOR_BS_FLAG!=0)&&(Flash_TAG_BUF[Calculate_TAG_FLAG]!= TAG_ID))//�������б�ǩID���޸ľ�ȡ��������¿�ʼ  ��ֹ�����̱��޸�
									{
											SYS_MAJOR_BS_FLAG=0;													
									}
									/*****************************************
													         �ȴ�ָ��
									*****************************************/
									if(SYS_MAJOR_BS_FLAG==0)
									{
										Calculate_FLAG=0;                                    //״̬��־Ϊ���
											if(Calculate_EN==0x01||Calculate_EN==0x02||Calculate_EN==0x03||Calculate_EN==0x04)		
											{
													TAG_ID=Flash_TAG_BUF[Calculate_TAG_FLAG];
												  Calculate_TAG_ID=TAG_ID;																						
													SYS_MAJOR_BS_FLAG=1;			
													ERROR_FLAG=0;                                      //�����־Ϊ��0
											}										
									}
									/*****************************************
													��ʼ��λ������վ���ǩ���
									*****************************************/
									if(SYS_MAJOR_BS_FLAG==1) //����վ���ǩ���
									{
											SYS_dis_buf_t[0]=DW1000send(255,TAG_ID,2);		//����Ĵ���MODBUS																		
											if(SYS_dis_buf_t[0]!=0)
											{
												  Calculate_FLAG|=0x01<<0;		//�ɹ���ำ���־	
													SYS_MAJOR_BS_FLAG=2;												
													//deca_sleep(RNG_DELAY_MS);//���߹̶�ʱ��
													ERROR_FLAG=0;                                      //�����־Ϊ��0
											}		
																		
									}					
									/*****************************************
													 �λ�վ���ǩ������ν���
									*****************************************/
									for(i=0;i<7;i++) //�λ�վ���ǩ��� 
								  {
											if(SYS_MAJOR_BS_FLAG==2+i) //�λ�վ���ǩ���
											{													                          										
												  if(Flash_BS_EN_X_Y_Z[i][0]==1)
													{																										
															SYS_dis_buf_t[1+i]=DW1000send_dist_msg(255,248+i,TAG_ID);
															if(SYS_dis_buf_t[1+i]!=0)		
															{
																  Calculate_FLAG|=0x01<<(1+i);		//�ɹ���ำ���־	
																	SYS_MAJOR_BS_FLAG++;															
															    //deca_sleep(RNG_DELAY_MS);//���߹̶�ʱ��
																	ERROR_FLAG=0;                                      //�����־Ϊ��0
															}																						
													}
													else
													{
															SYS_MAJOR_BS_FLAG++;
													}
													
											}
											
									}								
									/*****************************************
													   ���������������ݲ����
									*****************************************/
						      if(SYS_MAJOR_BS_FLAG==9)
									{
										  double clua_x_y[2];
										  u8 i;
										  u8 cla_flag=0;
									    
											for(i=0;i<8;i++)
											{					
													if((Calculate_FLAG>>i)&0x01) //���ɹ����˲�ˢ�£����ʧ�ܱ�����һ�ε�����
														dist[i]=LP(SYS_dis_buf_t[i],i+8*TAG_ID);//LP Ϊ��ͨ�˲����������ݸ��ȶ�	
											}
									
											cla_flag=PersonPosition(Flash_MAJOR_BS_X_Y_Z[0],Flash_MAJOR_BS_X_Y_Z[1],dist[0],(Calculate_FLAG>>0)&0x01,
																							Flash_BS_EN_X_Y_Z[0][1],Flash_BS_EN_X_Y_Z[0][2],dist[1],(Calculate_FLAG>>1)&0x01,
																							Flash_BS_EN_X_Y_Z[1][1],Flash_BS_EN_X_Y_Z[1][2],dist[2],(Calculate_FLAG>>2)&0x01,
																							Flash_BS_EN_X_Y_Z[2][1],Flash_BS_EN_X_Y_Z[2][2],dist[3],(Calculate_FLAG>>3)&0x01,
																							Flash_BS_EN_X_Y_Z[3][1],Flash_BS_EN_X_Y_Z[3][2],dist[4],(Calculate_FLAG>>4)&0x01,
																							Flash_BS_EN_X_Y_Z[4][1],Flash_BS_EN_X_Y_Z[4][2],dist[5],(Calculate_FLAG>>5)&0x01,
																							Flash_BS_EN_X_Y_Z[5][1],Flash_BS_EN_X_Y_Z[5][2],dist[6],(Calculate_FLAG>>6)&0x01,
											                        Flash_BS_EN_X_Y_Z[6][1],Flash_BS_EN_X_Y_Z[6][2],dist[7],(Calculate_FLAG>>7)&0x01,
																							clua_x_y);  //������㺯��
									
										 if(cla_flag!=0) //״̬��ʾ�������
										  {
												  
												  Calculate_FLAG&=~(0x01<<8);		//����ʧ�ܸ����־	   
													Calculate_TAG_X=0; //���赽�Ĵ���
													Calculate_TAG_Y=0; //���赽�Ĵ���
													Calculate_TAG_Z=0; //���赽�Ĵ���		
												  Calculate_Last_XYZ[TAG_ID][0]=0;//�ش���ǩ����ֵ
												  Calculate_Last_XYZ[TAG_ID][1]=0;//�ش���ǩ����ֵ
												  Calculate_Last_XYZ[TAG_ID][2]=0;//�ش���ǩ����ֵ
												  Calculate_Station_TAG[0]=dist[0];												
													for(o=0;o<7;o++)   //���赽�Ĵ���  �������ʧ�ܣ�������վ����ֵ�ճ����
													{
													if(Flash_BS_EN_X_Y_Z[o][0]==1) Calculate_Station_TAG[o+1]=dist[o+1];								
													else Calculate_Station_TAG[o+1]=0;							
													}		
													
										  }
										 else  //״̬��ʾ��ȷ
											{
												 Calculate_FLAG|=0x01<<8;		//����ɹ������־	                  
												clua_x_y[0] = KalmanFilter(clua_x_y[0],FLASH_KALMAN_Q,FLASH_KALMAN_R,0+2*TAG_ID);    //�������˲�
												 clua_x_y[1] = KalmanFilter(clua_x_y[1],FLASH_KALMAN_Q,FLASH_KALMAN_R,1+2*TAG_ID);		//�������˲�
												 Calculate_TAG_X=(int)(clua_x_y[0]); //���赽�Ĵ���
												 Calculate_TAG_Y=(int)(clua_x_y[1]); //���赽�Ĵ���
												 Calculate_TAG_Z=0; //���赽�Ĵ���	
												 Calculate_Last_XYZ[TAG_ID][0]=Calculate_TAG_X;//�ش���ǩ����ֵ
												 Calculate_Last_XYZ[TAG_ID][1]=Calculate_TAG_Y;//�ش���ǩ����ֵ
												 Calculate_Last_XYZ[TAG_ID][2]=Calculate_TAG_Z;//�ش���ǩ����ֵ
												/******************************///����ֵ
												Calculate_Station_TAG[0]=dist[0];												
												 for(o=0;o<7;o++)   //���赽�Ĵ���
												{
													if(Flash_BS_EN_X_Y_Z[o][0]==1) Calculate_Station_TAG[o+1]=dist[o+1];								
													else Calculate_Station_TAG[o+1]=0;							
													}															
												
													OLED_display_data(0, Calculate_Last_XYZ[TAG_ID][0],Calculate_Last_XYZ[TAG_ID][1],2);				//��Ļ��ʾ�Ῠס������ͨѶ					
											}
										

											
																
											/******************************/
												if(Calculate_EN==0x03||Calculate_EN==0x04)//�Զ����
											{
												MODBUS_xyz(Calculate_TAG_ID,Calculate_TAG_X,Calculate_TAG_Y,Calculate_TAG_Z,Calculate_Station_TAG);									
                       
											}
											if(Calculate_EN==0x01||Calculate_EN==0x03)//���μ�������ʹ��
											{												
												Calculate_EN=0;
											}
										
											if(LED_FLAG>6)
											{
													GPIO_Toggle(LED_GPIO,LED);
													LED_FLAG=0;
											}
											else LED_FLAG++;
											
											SYS_MAJOR_BS_FLAG=0;
											
                      if(Calculate_TAG_FLAG<(Flash_TAG_NUM-1))//ID�б�ɨ�� ��ID���
											{
												Calculate_TAG_FLAG++;
											}
											else
											{
												Calculate_TAG_FLAG=0;
											}
													

											
									}																
									//printf("%d\n",SYS_MAJOR_BS_FLAG); 
									/*
									sprintf(USART_STR_BUF,"SYS_MAJOR_BS_FLAG: %d \n", SYS_MAJOR_BS_FLAG);
									Usart3_SendString(USART_STR_BUF,80);									
									*/
									if((ERROR_FLAG>1500)&&(SYS_MAJOR_BS_FLAG!=0))   //��෢������,����������������������ܵ�̫С��������Ŵλ�վ���
									{					
										SYS_MAJOR_BS_FLAG++;
										SYS_Calculate_ACTIVE_FLAG=0;//ϵͳѭ����־λ��������־λ
										SYS_BS_MESSAGE_FLAG=0; //����վ��ϵ�λ�վ�ı�־λ														
										ERROR_FLAG=0;        //�����־��0
									}   
							}													
}
/******************************************************************************
												        ��ʾ����
ע�⣬��ʾ̫��Ӱ������ˢ�� 
*******************************************************************************/
void OLED_display(void)
{
		switch(Flash_Device_Mode)
				{
						case 0:  //��ǩ
						{			
							OLED_ShowString(0,0,"MODE:TAG          ",7);								
					
						}	
						break;
						
						case 1:  //�λ�վ
						{								
								OLED_ShowString(0,0,"MODE:SUB-ANCHOR ",7);
						}						
						break;
						
						case 2:  //����վ
						{
							  OLED_ShowString(0,0,"MODE:MAJ-ANCHOR ",7);							
						}
			}			
			switch(Flash_Device_Mode)
				{					
						case 0:  //��ǩ
						{		
							char a[5]="ID=";	
							char b[5];		  				
							char show_buf[10]="";		
							sprintf(b, "%d", Flash_Device_ID&0x00FF);
							strcat(show_buf,a); 
							strcat(show_buf,b); 							
							OLED_ShowString(0,1,show_buf,7);						
					
						}	
						break;
						
						case 1:  //�λ�վ
						{	
							char a[5]="ID=";	
							char b[5];		  				
							char show_buf[11]="";		
							sprintf(b, "%d", (Flash_Device_ID>>8)&0xFF);
							strcat(show_buf,a); 
							strcat(show_buf,b);							
							OLED_ShowString(0,1,show_buf,7);
						}						
						break;
						
						case 2:  //����վ
						{
							char a[5]="ID=XX";	
							char b[5];		  				
							char show_buf[11]="";				
							sprintf(b, "%d", 0x00);
							strcat(show_buf,a); 						
							OLED_ShowString(0,1,show_buf,7);						
						}
			}		
				
}
/******************************************************************************
												        ��ʾ����
ע�⣬��ʾ̫��Ӱ������ˢ�� 
*******************************************************************************/
void OLED_display_data(int16 dist,int16 X,int16 Y,u16 MODE)
{

				
			if(MODE==0)  //���ģʽ
			{	
				
				char a[20]="               ";		
				char show_buf[18]="";	
				strcat(show_buf,a); 
				OLED_ShowString(0,2,show_buf,7);
			}
			else if(MODE==1)  //���ģʽ
			{
				if(dist!=0)
				{
				char a[5]="DIST=";	
				char b[5];		  				
				char c[8]="cm    ";		
				char show_buf[18]="";	
				sprintf(b, "%d", dist);
				strcat(show_buf,a); 
				strcat(show_buf,b); 
				strcat(show_buf,c); 
				OLED_ShowString(0,2,show_buf,7);	
				}
			//	OLED_ShowString(0,3,"www.gzlwkj.com",7);		
		 }
			else if(MODE==2)//��λģʽ
			{
				if(!((X==0)&&(Y==0)))
				{
				char a[2]="X=";	
				char b[5];		  				
				char c[8]="cm ";					
				char d[2]="Y=";	
				char e[5];		  				
				char f[8]="cm    ";							
			  char show_buf[18]="";				
				sprintf(b, "%d", X);				
				strcat(show_buf,a); 
				strcat(show_buf,b); 
				strcat(show_buf,c); 				
				sprintf(e, "%d", Y);	
				strcat(show_buf,d); 
				strcat(show_buf,e); 
				strcat(show_buf,f); 
				OLED_ShowString(0,2,show_buf,7);
				}
			}
}
/******************************************************************************
												        ������⺯��
*******************************************************************************/
void KEY_SCAN(void)
{
   if(Read_KEY_MODE()==0&&KEY_FLAG==1)//MODE��������
	 {
		 Flash_Device_Mode++;
		 if(Flash_Device_Mode==3)
		 {
			 Flash_Device_Mode=0;
		 }
		 
		 FLASH_write();
		 OLED_Clear(); 
		 OLED_display();
		 KEY_FLAG=0;
	 }
	 if(Read_KEY_SET()==0)//SET��������
	 {
        Calculate_KEY_NUM_FLAG++;
		 		switch(Flash_Device_Mode)
				{
						case 0:  //��ǩ
						{			
							u16 ID=Flash_Device_ID&0x00FF;
							if(KEY_FLAG==1) ID++;  //���ظ�����
							if(Calculate_KEY_NUM_FLAG>10)//���Ϊ����״̬
							{
								ID=0;
							}
							if(ID>99) ID=0;
							Flash_Device_ID=((Flash_Device_ID&0xFF00)|ID);										
						}	
						break;
						
						case 1:  //�λ�վ
						{								
							u16 ID=(Flash_Device_ID>>8)&0x00FF;
							if(KEY_FLAG==1) ID++;   //���ظ�����
							if(Calculate_KEY_NUM_FLAG>10)//���Ϊ����״̬
							{
								ID=0;
							}
							if(ID>6) ID=0;
							Flash_Device_ID=((Flash_Device_ID&0x00FF)|((ID<<8)&0xFF00));	
						}						
						break;
						
						case 2:  //����վ
						{
							  						
						}
			}			
		 
		 FLASH_write();
		 OLED_Clear(); 
		 OLED_display();
		 KEY_FLAG=0;
	 }
	 
	 
	 if(Read_KEY_MODE()==1&&Read_KEY_SET()==1)//����û�����¾ͱ�־λ����
	 {
		 KEY_FLAG=1;
		 Calculate_KEY_NUM_FLAG=0;
	 }
}

/******************************************************************************
												         DWM100��ʼ��
*******************************************************************************/
void DWM1000_init()
{
		config.chan=AIR_Chan[FLASH_Channel];
		config.dataRate=FLASH_Data_rat;	
	  reset_DW1000();//����DW1000 /* Target specific drive of RSTn line into DW1000 low for a period. */
    spi_set_rate_low();//����SPIƵ��
    dwt_initialise(DWT_LOADUCODE);//��ʼ��DW1000
    spi_set_rate_high();//�ظ�SPIƵ��
    dwt_configure(&config);//����DW1000
    dwt_setrxantennadelay(FLASH_RX_ANT_DLY);		//���ý��������ӳ�
    dwt_settxantennadelay(TX_ANT_DLY);		//���÷��������ӳ�
	
}
/******************************************************************************
												         ���DWM1000���ò����Ƿ񱻸ı�
*******************************************************************************/
void DWM1000_check()
{
	u8 DWM1000_SET_EN=0;
	if(DWM1000_SET_DATA[0]!=FLASH_Channel)
	{
		DWM1000_SET_DATA[0]=FLASH_Channel;
		DWM1000_SET_EN=1;
	}
	if(DWM1000_SET_DATA[1]!=FLASH_Data_rat)
	{
		DWM1000_SET_DATA[1]=FLASH_Data_rat;
		DWM1000_SET_EN=1;
	}
	if(DWM1000_SET_DATA[2]!=FLASH_RX_ANT_DLY)
	{
		DWM1000_SET_DATA[2]=FLASH_RX_ANT_DLY;
		DWM1000_SET_EN=1;
	}
  if(DWM1000_SET_EN==1) DWM1000_init();
}

/******************************************************************************
												         ������
*******************************************************************************/
int main(void)
{	
	  peripherals_init();//��ʼ������
		DWM1000_init();     //dwm1000��ʼ��
		OLED_Init();			//��ʼ��OLED  	
		OLED_Clear(); 
		OLED_display();
		while(1)
		{				  			
				MODBUS_event();
			  KEY_SCAN();
			  DWM1000_check();
				switch(Flash_Device_Mode)
				{
						case 0:  //��ǩ
						{			
								MODE_TAG();														
						}	
						break;
						
						case 1:  //�λ�վ
						{								
								MODE_SUB_ANCHOR();
						}						
						break;
						
						case 2:  //����վ
						{
								MODE_MAJOR_ANCHOR();							
						}
			}								
		}
		
		
	
		
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn get_tx_timestamp_u64()
 *
 * @brief Get the TX time-stamp in a 64-bit variable.
 *        /!\ This function assumes that length of time-stamps is 40 bits, for both TX and RX!
 *
 * @param  none
 *
 * @return  64-bit value of the read time-stamp.
 */
static uint64 get_tx_timestamp_u64(void)
{
    uint8 ts_tab[5];
    uint64 ts = 0;
    int i;
    dwt_readtxtimestamp(ts_tab);
    for (i = 4; i >= 0; i--)
    {
        ts <<= 8;
        ts |= ts_tab[i];
    }
    return ts;
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn get_rx_timestamp_u64()
 *
 * @brief Get the RX time-stamp in a 64-bit variable.
 *        /!\ This function assumes that length of time-stamps is 40 bits, for both TX and RX!
 *
 * @param  none
 *
 * @return  64-bit value of the read time-stamp.
 */
static uint64 get_rx_timestamp_u64(void)
{
    uint8 ts_tab[5];
    uint64 ts = 0;
    int i;
    dwt_readrxtimestamp(ts_tab);
    for (i = 4; i >= 0; i--)
    {
        ts <<= 8;
        ts |= ts_tab[i];
    }
    return ts;
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn final_msg_set_ts()
 *
 * @brief Fill a given timestamp field in the final message with the given value. In the timestamp fields of the final
 *        message, the least significant byte is at the lower address.
 *
 * @param  ts_field  pointer on the first byte of the timestamp field to fill
 *         ts  timestamp value
 *
 * @return none
 */
static void final_msg_set_ts(uint8 *ts_field, uint64 ts)
{
    int i;
    for (i = 0; i < FINAL_MSG_TS_LEN; i++)
    {
        ts_field[i] = (uint8) ts;
        ts >>= 8;
    }
}
static void final_msg_set_dist(uint8 *ts_field, uint32 dist)
{
    int i;
    for (i = 0; i < 4; i++)
    {
        ts_field[i] = (uint8) dist;
        dist >>= 8;
    }
}
/*****************************************************************************************************************************************************
 * NOTES:
 *
 * 1. The sum of the values is the TX to RX antenna delay, experimentally determined by a calibration process. Here we use a hard coded typical value
 *    but, in a real application, each device should have its own antenna delay properly calibrated to get the best possible precision when performing
 *    range measurements.
 * 2. The messages here are similar to those used in the DecaRanging ARM application (shipped with EVK1000 kit). They comply with the IEEE
 *    802.15.4 standard MAC data frame encoding and they are following the ISO/IEC:24730-62:2013 standard. The messages used are:
 *     - a poll message sent by the initiator to trigger the ranging exchange.
 *     - a response message sent by the responder allowing the initiator to go on with the process
 *     - a final message sent by the initiator to complete the exchange and provide all information needed by the responder to compute the
 *       time-of-flight (distance) estimate.
 *    The first 10 bytes of those frame are common and are composed of the following fields:
 *     - byte 0/1: frame control (0x8841 to indicate a data frame using 16-bit addressing).
 *     - byte 2: sequence number, incremented for each new frame.
 *     - byte 3/4: PAN TAG_ID (0xDECA).
 *     - byte 5/6: destination address, see NOTE 3 below.
 *     - byte 7/8: source address, see NOTE 3 below.
 *     - byte 9: function code (specific values to indicate which message it is in the ranging process).
 *    The remaining bytes are specific to each message as follows:
 *    Poll message:
 *     - no more data
 *    Response message:
 *     - byte 10: activity code (0x02 to tell the initiator to go on with the ranging exchange).
 *     - byte 11/12: activity parameter, not used here for activity code 0x02.
 *    Final message:
 *     - byte 10 -> 13: poll message transmission timestamp.
 *     - byte 14 -> 17: response message reception timestamp.
 *     - byte 18 -> 21: final message transmission timestamp.
 *    All messages end with a 2-byte checksum automatically set by DW1000.
 * 3. Source and destination addresses are hard coded constants in this example to keep it simple but for a real product every device should have a
 *    unique TAG_ID. Here, 16-bit addressing is used to keep the messages as short as possible but, in an actual application, this should be done only
 *    after an exchange of specific messages used to define those short addresses for each device participating to the ranging exchange.
 * 4. Delays between frames have been chosen here to ensure proper synchronisation of transmission and reception of the frames between the initiator
 *    and the responder and to ensure a correct accuracy of the computed distance. The user is referred to DecaRanging ARM Source Code Guide for more
 *    details about the timings involved in the ranging process.
 * 5. This timeout is for complete reception of a frame, i.e. timeout duration must take into account the length of the expected frame. Here the value
 *    is arbitrary but chosen large enough to make sure that there is enough time to receive the complete response frame sent by the responder at the
 *    110k data rate used (around 3 ms).
 * 6. In a real application, for optimum performance within regulatory limits, it may be necessary to set TX pulse bandwidth and TX power, (using
 *    the dwt_configuretxrf API call) to per device calibrated values saved in the target system or the DW1000 OTP memory.
 * 7. dwt_writetxdata() takes the full size of the message as a parameter but only copies (size - 2) bytes as the check-sum at the end of the frame is
 *    automatically appended by the DW1000. This means that our variable could be two bytes shorter without losing any data (but the sizeof would not
 *    work anymore then as we would still have to indicate the full length of the frame to dwt_writetxdata()). It is also to be noted that, when using
 *    delayed send, the time set for transmission must be far enough in the future so that the DW1000 IC has the time to process and start the
 *    transmission of the frame at the wanted time. If the transmission command is issued too late compared to when the frame is supposed to be sent,
 *    this is indicated by an error code returned by dwt_starttx() API call. Here it is not tested, as the values of the delays between frames have
 *    been carefully defined to avoid this situation.
 * 8. We use polled mode of operation here to keep the example as simple as possible but all status events can be used to generate interrupts. Please
 *    refer to DW1000 User Manual for more details on "interrupts". It is also to be noted that STATUS register is 5 bytes long but, as the event we
 *    use are all in the first bytes of the register, we can use the simple dwt_read32bitreg() API call to access it instead of reading the whole 5
 *    bytes.
 * 9. As we want to send final TX timestamp in the final message, we have to compute it in advance instead of relying on the reading of DW1000
 *    register. Timestamps and delayed transmission time are both expressed in device time units so we just have to add the desired response delay to
 *    response RX timestamp to get final transmission time. The delayed transmission time resolution is 512 device time units which means that the
 *    lower 9 bits of the obtained value must be zeroed. This also allows to encode the 40-bit value in a 32-bit words by shifting the all-zero lower
 *    8 bits.
 * 10. In this operation, the high order byte of each 40-bit timestamps is discarded. This is acceptable as those time-stamps are not separated by
 *     more than 2**32 device time units (which is around 67 ms) which means that the calculation of the round-trip delays (needed in the
 *     time-of-flight computation) can be handled by a 32-bit subtraction.
 * 11. The user is referred to DecaRanging ARM application (distributed with EVK1000 product) for additional practical example of usage, and to the
 *     DW1000 API Guide for more details on the DW1000 driver functions.
1����ֵ���ܺ���ͨ��У׼����ʵ��ȷ����TX��RX�����ӳ١���������ʹ��Ӳ����ĵ���ֵ��
���ǣ���ʵ��Ӧ���У�ÿ���豸��Ӧ�����Լ��������ӳ��ʵ�У׼������ִ��ʱ�����ѿ��ܵľ��ȡ�
���������
2���������Ϣ���ѹ�ARMӦ�ó���EVK1000�׼���һ��ʹ�á���������IEEE��
802.15.4��׼��MAC����֡���룬������ѭISO/IEC:2473062/2013��׼����ʹ�õ���Ϣ�ǣ�
-�����߷��͵���ѯ��Ϣ�Դ�����ཻ����
-��Ӧ�����͵���Ӧ��Ϣ�������𷽼���ִ�н��̡�
-�ɷ��𷽷��͵�������Ϣ����ɽ��������ṩ��Ӧ�������������Ϣ������
����ʱ�䣨���룩���ơ�
��Щ֡��ǰ10���ֽ��ǳ����ģ������������ֶ���ɣ�
-�ֽ�0/1��֡���ƣ�0x88 41����16λѰַָʾ����֡����
-�ֽ�2�����кţ�Ϊÿ����֡���ӡ�
-�ֽ�3/4��PANTAGIGID��0xDECA����
-�ֽ�5/6��Ŀ�ĵ�ַ���������ע��3��
-�ֽ�7/8��Դ��ַ��������ע��3��
-�ֽ�9���������루�ض�ֵ��ָʾ���ڲ������е��ĸ���Ϣ����
������ֽڶ�ÿ����Ϣ�����ض��ģ�
������飺
-����������
��Ӧ��Ϣ��
-�ֽ�10������루0x02���߷����߼������в�ཻ������
-�ֽ�11/12����������˴������ڻ����0x02��
�����Ϣ��
-�ֽ�10��13����ѯ��Ϣ����ʱ�����
-�ֽ�14��17����Ӧ��Ϣ����ʱ�����
-�ֽ�18��21��������Ϣ����ʱ�����
������Ϣ��DW1000�Զ����õ�2�ֽ�У��ͽ�����
��������������У�Դ��ַ��Ŀ�ĵ�ַ��Ӳ���볣�����Ա��ּ򵥣�������ʵ�ʲ�Ʒ��ÿ���豸��Ӧ����һ��
���ص�TAGJID�����16λѰַ������������Ϣ�����̣ܶ����ǣ���ʵ��Ӧ���У���Ӧ��ֻ����
�ڽ����ض���Ϣ֮������Ϊ�����ཻ����ÿ���豸������Щ�̵�ַ��
4������ѡ��֮֡����ӳ٣���ȷ��������֮���֡�ķ��ͺͽ��յ��ʵ�ͬ����
��Ӧ��������֤����������ȷ���ȡ��û�����Ϊ������ѽǱ�Դ����ָ�ϡ�
�йز������еļ�ʱ��ϸ�ڡ�
5���˳�ʱ��Ϊ����ȫ����֡������ʱ����ʱ����뿼��Ԥ��֡�ĳ��ȡ�����ļ�ֵ
������ģ���ѡ�������ȷ�����㹻��ʱ����������Ӧ�߷��͵�������Ӧ֡��
ʹ��1K�������ʣ�Լ3���룩��
6����ʵ��Ӧ���У�Ϊ���ڵ��ڷ�Χ�ڴﵽ������ܣ�������Ҫ����TX��������TX���ʣ���ʹ��
DWTSCOMPUTURTXRF API���õ���Ŀ��ϵͳ��DW1000 OTP�洢���б����ÿ���豸У׼ֵ��
7��DWTJWRIGETXDATA��������Ϣ��������С��Ϊ������������֡��β��У���ʱ���ƣ���СΪ2���ֽڡ�
��DW1000�Զ�׷�ӡ�����ζ�����ǵı��������ڲ���ʧ�κ����ݵ�����¶������ֽڣ�����sieof���ᣩ��
������Ȼ��������Ȼ����ָʾ��ܵ���������ΪDWTTWORGETXDATA������������Ҫע����ǣ�ʹ��ʱ
�ӳٷ��ͣ������ʱ��������δ�������㹻Զ�����DW1000 IC��ʱ�������������
����Ҫ��ʱ���ڴ���֡����������������ʱ��̫����֡������ʱ��ȣ�
������DWTSARSTTTX����API���÷��صĴ��������ָʾ�ġ�����û�в��ԣ���Ϊ֮֡����ӳ�ֵ��
����ϸ�����Ա������������
8������������ʹ����ѯģʽ������ʾ�������ܼ򵥣���������״̬�¼����������������жϡ�����
�йء��жϡ�����ϸ��Ϣ�������DW1000�û��ֲᡣ����Ҫע����ǣ�״̬�Ĵ�����5�ֽڳ�������Ϊ�¼�����
ʹ�ö��ڼĴ����ĵ�һ���ֽ��У����ǿ���ʹ�ü򵥵�DWTSRead 32λRead����API�������������������Ƕ�ȡ����5���ֽڡ�
�ֽڡ�
9����������Ҫ�����յ���Ϣ�з������յ�TXʱ���ʱ�����Ǳ�����ǰ��������������������DW1000�Ķ�ȡ��
�Ĵ�����ʱ������ӳٴ���ʱ�䶼��ʾ���豸ʱ�䵥λ�У���������ֻ�轫��������Ӧ�ӳ���ӵ�
��ӦRXʱ����Ի�����մ���ʱ�䡣�ӳٴ���ʱ��ֱ�����512���豸ʱ�䵥λ������ζ��
����õ�ֵ�ĵ�9λ���뱻���㡣�⻹����ͨ����λȫ����λ����32λ���б���40λֵ��
8λ��
10������������У�����ÿ��40λʱ����ĸ߽��ֽڡ����ǿ��Խ��ܵģ���Ϊ��Щʱ���û�зֿ���
����2��32���豸ʱ�䵥λ����Լ67���룩������ζ�ż��������ӳ٣���Ҫ��
����ʱ��������ͨ��32λ����������
11���û�����Ϊ�ѽ�ARMӦ�ó�����EVK1000��Ʒ�ַ��������ڸ��ӵ�ʵ��ʹ��ʾ�����Լ�
DW1000 APIָ���й���DW1000���������ܵĸ���ϸ�ڡ�
 ****************************************************************************************************************************************************/
