#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_sleep.h"
#include <string.h>
#include <stdio.h>
#include "port.h"
#include "main.h"
#include "usart.h"
#include "delay.h"
#include "dma.h"
unsigned char usart1_rx_buf[USART1_RX_LENGTH_MAX];//����1�Ľ������ݻ���
unsigned int usart1_rx_length=0;
unsigned char usart2_rx_buf[USART2_RX_LENGTH_MAX];//����2�Ľ������ݻ���
unsigned int usart2_rx_length=0;
unsigned char usart3_rx_buf[USART3_RX_LENGTH_MAX];
unsigned int usart3_rx_length=0;
unsigned char time3_usart1,time3_usart2,time3_usart3;//time3���ڸ����ڽ��յı���
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos ʹ��	  
#endif
 

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
	/* Whatever you require here. If the only file you are using is */ 
	/* standard output using printf() for debugging, no file handling */ 
	/* is required. */ 
}; 
/* FILE is typedef�� d in stdio.h. */ 
FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)

{

USART_SendData(USART1, (unsigned char) ch);// USART1 ???? USART2 ?

while (!(USART1->SR & USART_FLAG_TXE));

return (ch);


}
#endif 
//end
//////////////////////////////////////////////////////////////////

#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������

void USART1_IRQHandler(void)
{
	
		if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
		{
				time3_usart1 = 0;
        usart1_rx_buf[usart1_rx_length] = USART_ReceiveData(USART1);//(USART1->DR);	//��ȡ���յ�������
        if(usart1_rx_length < USART1_RX_LENGTH_MAX)
            usart1_rx_length++;
 
	}

} 
#endif			

void USART3_IRQHandler()	//����3�ж�
{
	
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
		{
		time3_usart3 = 0;
        usart3_rx_buf[usart3_rx_length] = USART_ReceiveData(USART3);
        if(usart3_rx_length < USART3_RX_LENGTH_MAX)
            usart3_rx_length++; 
		}
	
}

//��ʼ��IO ����1
//pclk2:PCLK2ʱ��Ƶ��(Mhz)
//bound:������
//CHECK OK
//091209
void uart_init(unsigned long pclk2)
{  	 
USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	/* USARTx configured as follow:
		  - BaudRate = 115200 baud
		  - Word Length = 8 Bits
		  - One Stop Bit
		  - No parity
		  - Hardware flow control disabled (RTS and CTS signals)
		  - Receive and transmit enabled
	*/
	USART_InitStructure.USART_BaudRate = pclk2 ;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	/* Enable GPIO clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

	//For EVB1000 -> USART2_REMAP = 0

	/* Enable the USART2 Pins Software Remapping */

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);


	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART Rx as input floating */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* USART configuration */
	USART_Init(USART1, &USART_InitStructure);
	
   NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
	 // Enable USART1 Receive interrupts ʹ�ܴ��ڽ����ж�  
   USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  
	/* Enable USART */
	USART_Cmd(USART1, ENABLE);
	
	
	
	
	
}


void Usart3_Init(unsigned long pclk2) //����3��ʼ��
{
	USART_InitTypeDef USART_InitStructure;  
  NVIC_InitTypeDef NVIC_InitStructure;   
    GPIO_InitTypeDef GPIO_InitStructure;    //����һ���ṹ�������������ʼ��GPIO  
   //ʹ�ܴ��ڵ�RCCʱ��  
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE); //ʹ��UART3����GPIOB��ʱ��  
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);  
  
   //����ʹ�õ�GPIO������  
   // Configure USART2 Rx (PB.11) as input floating    
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;  
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;  
   GPIO_Init(GPIOB, &GPIO_InitStructure);  
  
   // Configure USART2 Tx (PB.10) as alternate function push-pull  
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;  
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  
   GPIO_Init(GPIOB, &GPIO_InitStructure);  
  
   //���ô���  
   USART_InitStructure.USART_BaudRate =pclk2;  
   USART_InitStructure.USART_WordLength = USART_WordLength_8b;  
   USART_InitStructure.USART_StopBits = USART_StopBits_1;  
   USART_InitStructure.USART_Parity = USART_Parity_No;  
   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  
   USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  
  
  
   // Configure USART3   
   USART_Init(USART3, &USART_InitStructure);//���ô���3  
  
	
	//�����ж�����  
   //Configure the NVIC Preemption Priority Bits     
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);  
  
   // Enable the USART3 Interrupt   
   NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;  
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
   NVIC_Init(&NVIC_InitStructure);  
	
	
  // Enable USART1 Receive interrupts ʹ�ܴ��ڽ����ж�  
   USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);  
   //���ڷ����ж��ڷ�������ʱ����  
   //USART_ITConfig(USART2, USART_IT_TXE, ENABLE);  
	
	
	
   // Enable the USART3   
   USART_Cmd(USART3, ENABLE);//ʹ�ܴ���3  
  
   
      
      
}
void U3SendChar(int ch)   //����3�����ַ�
{
	//while(!(USART2->SR & 0x00000040));
	//USART_SendData(USART2,c);
	/* e.g. write a character to the USART */
	USART_SendData(USART3, ch);

	/* Loop until the end of transmission */
	while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
}

void U1SendChar(int ch) 
{
		//while(!(USART2->SR & 0x00000040));
	//USART_SendData(USART2,c);
	/* e.g. write a character to the USART */
	USART_SendData(USART1, ch);

	/* Loop until the end of transmission */
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
}

void Usart1_SendString(unsigned char *data,unsigned int num)//����1�����ַ���
{
	unsigned int i;
	for(i = 0;i < num;i++)
	{
		USART_STR_BUF[i]=data[i];		
		//U1SendChar(data[i]);
	}
	MYDMA_Enable1(num);
	
}

void Usart3_SendString(unsigned char *data,unsigned int num)//����3�����ַ���
{
	unsigned int i;
	for(i = 0;i < num;i++)
	{
		USART_STR_BUF[i]=data[i];	
		//U3SendChar(data[i]);
	}
	MYDMA_Enable3(num);
}




