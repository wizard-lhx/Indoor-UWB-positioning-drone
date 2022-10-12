#ifndef __USART_H
#define __USART_H
#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_sleep.h"
#include "stdio.h"	

#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����
//�������ݽ��ջ��泤��
#define USART1_RX_LENGTH_MAX    300
#define USART2_RX_LENGTH_MAX    300
#define USART3_RX_LENGTH_MAX    300

extern unsigned char USART2_OV_FLAG;
extern unsigned char USART1_OV_FLAG;
extern unsigned char usart1_rx_buf[USART1_RX_LENGTH_MAX];//����1�Ľ������ݻ���
extern unsigned int usart1_rx_length;
extern unsigned char usart2_rx_buf[USART2_RX_LENGTH_MAX];
extern unsigned int usart2_rx_length;
extern unsigned char usart3_rx_buf[USART3_RX_LENGTH_MAX];
extern unsigned int usart3_rx_length;
extern unsigned char time3_usart1,time3_usart2,time3_usart3;

//����봮���жϽ��գ��벻Ҫע�����º궨��
void uart_init(unsigned long pclk2);
void U1SendChar(int ch);
void Usart1_SendString(unsigned char *data,unsigned int num);
void U3SendChar(int ch);   //����3�����ַ�
void Usart3_Init(unsigned long pclk2); //����3��ʼ��
void Usart3_SendString(unsigned char *data,unsigned int num);//����3�����ַ���


#endif	


