/*! ----------------------------------------------------------------------------
 *  @file    main.c
 *  @brief   Double-sided two-way ranging (DS TWR) initiator example code
 *
 *           This is a simple code example which acts as the initiator in a DS TWR distance measurement exchange. This application sends a "poll"
 *           frame (recording the TX time-stamp of the poll), and then waits for a "response" message expected from the "DS TWR responder" example
 *           code (companion to this application). When the response is received its RX time-stamp is recorded and we send a "final" message to
 *           complete the exchange. The final message contains all the time-stamps recorded by this application, including the calculated/predicted TX
 *           time-stamp for the final message itself. The companion "DS TWR responder" example application works out the time-of-flight over-the-air
 *           and, thus, the estimated distance between the two devices.
 *����һ���򵥵Ĵ���ʾ��������ΪSS TWR��������������е�Ӧ��������Ӧ�ó���ȴ���ͶƱ����
*��Ϣ����¼ͶƱ��RXʱ����������ӡ�SS TWR�����ߡ�ʾ�����루���Ӧ�ó������ף����Լ�
*Ȼ����һ������Ӧ����Ϣ����ɽ�������Ӧ��Ϣ������Ӧ�ó����¼������ʱ�����
*������Ӧ��Ϣ����ļ���/Ԥ���TXʱ�����ͬ�顰SS TWR�����ߡ�ʾ��Ӧ��
*���������ʱ���ڿ��У���ˣ����Ƶľ���֮��������豸��
 * @attention
 *
 * Copyright 2015 (c) Decawave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author Decawave
 */
#include <string.h>
#include <stdio.h>
#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_sleep.h"
#include "lcd.h"
#include "port.h"

/* Example application name and version to display on LCD screen. ��ҵҺ��������ʾ��ʾ��Ӧ�ó������ƺͰ汾*/
#define APP_NAME "DS TWR INIT v1.1"

/* Inter-ranging delay period, in milliseconds. �����ʱ�䣬�Ժ���Ϊ��λ��*/
#define RNG_DELAY_MS 100

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
#define RX_ANT_DLY 32899
static uint8 rx_poll_msg[] = {0x00, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0x21, 0, 0};
static uint8 tx_resp_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0x10, 0x02, 0, 0, 0, 0};
static uint8 rx_final_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0x23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	
/* Frames used in the ranging process. See NOTE 2 below. �ڲ�������ʹ�õ�֡���������ע��2��*/
static uint8 tx_poll_msg[] = {0x00, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0x21, 0, 0};
static uint8 rx_resp_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0x10, 0x02, 0, 0, 0, 0};
static uint8 tx_final_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0x23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
/* Length of the common part of the message (up to and including the function code, see NOTE 2 below). 
	��Ϣ�Ĺ������ֵĳ��ȣ������������룬�������ע��2����*/
typedef signed long long int64;
typedef unsigned long long uint64;
static uint64 poll_rx_ts;
static uint64 resp_tx_ts;
static uint64 final_rx_ts;

static double tof;
static double distance,dist2;
int16_t dist[8];
#define ALL_MSG_COMMON_LEN 10
/* Indexes to access some of the fields in the frames defined above. �������ʽ������漰�Ŀ���е�һЩ�ֶΡ�*/
#define ALL_MSG_SN_IDX 2
#define FINAL_MSG_POLL_TX_TS_IDX 10
#define FINAL_MSG_RESP_RX_TS_IDX 14
#define FINAL_MSG_FINAL_TX_TS_IDX 18
#define FINAL_MSG_TS_LEN 4
/* Frame sequence number, incremented after each transmission. ֡���кţ�ÿ�δ��������� */
static uint32 frame_seq_nb = 0;

/* Buffer to store received response message.���������洢���յ�����Ϣ��
 * Its size is adjusted to longest frame that this example code is supposed to handle.���Ĵ�С�����������ʾ������Ӧ�ô�����֡�� */
#define RX_BUF_LEN 20
#define RX_BUF_LEN2 24
static uint8 rx_buffer[RX_BUF_LEN+4];

/* Hold copy of status register state here for reference, so reader can examine it at a breakpoint. 
�����ﱣ��״̬�Ĵ���״̬�ĸ����Թ��ο����Ա���߿����ڶϵ��ϼ������*/
static uint32 status_reg = 0;

/* UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) conversion factor.
UWB΢�루UUS�����豸ʱ�䵥λ��DTU��Լ15.65 ps����ת��ϵ����
 * 1 uus = 512 / 499.2 �s and 1 �s = 499.2 * 128 dtu. 
  1 UUS��512��499.2��1��499.2��128 DTU��*/
#define UUS_TO_DWT_TIME 65536

/* Delay between frames, in UWB microseconds. See NOTE 4 below.֮֡����ӳ٣���UWB΢��Ϊ��λ���������ע��4��*/
/* This is the delay from the end of the frame transmission to the enable of the receiver, as programmed for the DW1000's wait for response feature.
���Ǵ�֡������������ջ����õ��ӳ٣���DW1000�ȴ���Ӧ���Եı�̡�*/
#define POLL_TX_TO_RESP_RX_DLY_UUS 150
/* This is the delay from Frame RX timestamp to TX reply timestamp used for calculating/setting the DW1000's delayed TX function. This includes the
 * frame length of approximately 2.66 ms with above configuration.
���Ǵ�֡RXʱ��������ڼ���/����DW1000�ӳ�TX���ܵ�TX�ظ�ʱ������ӳ١�����������������õĴ�Լ2.66�����֡���ȡ� */
#define RESP_RX_TO_FINAL_TX_DLY_UUS 3100
/* Receive response timeout. See NOTE 5 below. ������Ӧ��ʱ���������ע��5��*/
#define RESP_RX_TIMEOUT_UUS 4700

#define POLL_RX_TO_RESP_TX_DLY_UUS 2600
/* This is the delay from the end of the frame transmission to the enable of the receiver, as programmed for the DW1000's wait for response feature. 
���Ǵ�֡������������ջ����õ��ӳ٣���DW1000�ȴ���Ӧ���Եı�̡�*/
#define RESP_TX_TO_FINAL_RX_DLY_UUS 500
/* Receive final timeout. See NOTE 5 below.�������ճ�ʱ���������ע��5�� */
#define FINAL_RX_TIMEOUT_UUS 4300
#define SPEED_OF_LIGHT 299702547
/* Time-stamps of frames transmission/reception, expressed in device time units.ʱ����Ʊof frames����/����װ�ã����ʱ�䵥λ��
 * As they are 40-bit wide, we need to define a 64-bit int type to handle them. ����������40λ��ģ�����������Ҫ����һ��64λint�������������ǡ�*/
typedef unsigned long long uint64;
static uint64 poll_tx_ts;
static uint64 resp_rx_ts;
static uint64 final_tx_ts;

/* Declaration of static functions. ��̬������������*/
static uint64 get_tx_timestamp_u64(void);
static uint64 get_rx_timestamp_u64(void);
static void final_msg_set_ts(uint8 *ts_field, uint64 ts);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn main()
 *
 * @brief Application entry point.
 *
 * @param  none
 *
 * @return none
 */
 static void final_msg_get_ts(const uint8 *ts_field, uint32 *ts)
{
    int i;
    *ts = 0;
    for (i = 0; i < FINAL_MSG_TS_LEN; i++)
    {
        *ts += ts_field[i] << (i * 8);
    }
}
 void GPIO_Toggle(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	GPIO_WriteBit(GPIOx, GPIO_Pin, (BitAction)!GPIO_ReadOutputDataBit(GPIOx, GPIO_Pin));
}
int fputc(int ch, FILE *f)

{

USART_SendData(USART1, (unsigned char) ch);// USART1 ???? USART2 ?

while (!(USART1->SR & USART_FLAG_TXE));

return (ch);


}

void USART_putc(char c)
{
	//while(!(USART2->SR & 0x00000040));
	//USART_SendData(USART2,c);
	/* e.g. write a character to the USART */
	USART_SendData(USART1, c);

	/* Loop until the end of transmission */
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)	;
}

void USART_puts(uint8_t *s,uint8_t len)
{
	int i;
	for(i=0; i<len; i++)
	{
		USART_putc(s[i]);
	}
}
int ld[100];
int LP(int tmp,uint8_t channel)
{
int data;
	data = 0.7*ld[channel]+0.3*tmp;
	ld[channel]=data;
	return data;
}
uint16_t Checksum_u16(uint8_t* pdata, uint32_t len) 
{
    uint16_t sum = 0;
    uint32_t i;
    for(i=0; i<len; i++)
        sum += pdata[i];
    sum = ~sum;
    return sum;
}


/*
void LED_blink(void)
{
	uint8_t ii;
	for (ii=0;ii<10;ii++)
{
	GPIO_Toggle(GPIOA,LED_PIN);
	deca_sleep(100);
}
}*/
extern volatile unsigned long time32_reset;
extern uint8_t Work_Mode;
uint32 frame_len;
uint8_t send[9];
char dist_str[16] = {0};
int32_t dis;
double dID;
uint8_t jumptime=0;
uint32_t rec_dist,hex_dist;
uint16_t check;

uint8_t Work_Mode=0;  //ģʽѡ�� 0Ϊ��ǩ 1Ϊ��վ
uint8_t TAG_ID=0;         //��ǩID 
uint8_t ANCHOR_ID=1;      //��վID
unsigned char USART_STR_BUF[200];   //�����õ��ַ����������

int main(void)
{
    /* Start with board specific hardware init. */
    peripherals_init();//��ʼ������

    /* Display application name on LCD. */
  //  lcd_display_str(APP_NAME);

    /* Reset and initialise DW1000.
     * For initialisation, DW1000 clocks must be temporarily set to crystal speed. After initialisation SPI rate can be increased for optimum
     * performance. */
    reset_DW1000();//����DW1000 /* Target specific drive of RSTn line into DW1000 low for a period. */
    spi_set_rate_low();//����SPIƵ��
    dwt_initialise(DWT_LOADUCODE);//��ʼ��DW1000
    spi_set_rate_high();//�ظ�SPIƵ��
	
    /* Configure DW1000. See NOTE 6 below. */
    dwt_configure(&config);//����DW1000

    /* Apply default antenna delay value. See NOTE 1 below. */
    dwt_setrxantennadelay(RX_ANT_DLY);		//���ý��������ӳ�
    dwt_settxantennadelay(TX_ANT_DLY);		//���÷��������ӳ�

    /* Set expected response's delay and timeout. See NOTE 4 and 5 below.
     * As this example only handles one incoming frame with always the same delay and timeout, those values can be set here once for all. */
    dwt_setrxaftertxdelay(POLL_TX_TO_RESP_RX_DLY_UUS);//���÷��ͺ������գ����趨�ӳ�ʱ��
    dwt_setrxtimeout(RESP_RX_TIMEOUT_UUS);						//���ý��ճ�ʱʱ��

		send[0]=0x6D;											//��������
		send[1]=0xD6;											//��������

		tx_poll_msg[6] = ANCHOR_ID;	//UWB POLL ������
		rx_resp_msg[6] = ANCHOR_ID;	//UWB RESPONSE ������
		tx_final_msg[6] = ANCHOR_ID;//UWB Fianl ������
		
		rx_poll_msg[6] = ANCHOR_ID;
		tx_resp_msg[6] = ANCHOR_ID;
		rx_final_msg[6] = ANCHOR_ID;
		
		tx_poll_msg[5] = TAG_ID;//UWB POLL ������
		rx_resp_msg[5] = TAG_ID;//UWB RESPONSE ������
		tx_final_msg[5] = TAG_ID;//UWB Fianl ������
    /* Loop forever initiating ranging exchanges. */
		//LED_blink();
		if(!Work_Mode)   //ѡ����ģʽ��TAG��ǩ�����ǽ���ģʽ(ANCHOR��վ)
		{
    while (1)			//����ģʽ(TAG��ǩ)
    {

        /* Write frame data to DW1000 and prepare transmission. See NOTE 7 below. */
        tx_poll_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
        dwt_writetxdata(sizeof(tx_poll_msg), tx_poll_msg, 0);//��Poll�����ݴ���DW1000�����ڿ�������ʱ����ȥ
        dwt_writetxfctrl(sizeof(tx_poll_msg), 0);//���ó�����������ݳ���

        /* Start transmission, indicating that a response is expected so that reception is enabled automatically after the frame is sent and the delay
         * set by dwt_setrxaftertxdelay() has elapsed. */
        dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);//�������ͣ�������ɺ�ȴ�һ��ʱ�俪�����գ��ȴ�ʱ����dwt_setrxaftertxdelay������

        /* We assume that the transmission is achieved correctly, poll for reception of a frame or error/timeout. See NOTE 8 below. */
        while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR)))//���ϲ�ѯоƬ״ֱ̬���ɹ����ջ��߷�������
        { };

        /* Increment frame sequence number after transmission of the poll message (modulo 256). */
        frame_seq_nb++;

        if (status_reg & SYS_STATUS_RXFCG)//����ɹ�����
        {					
            uint32 frame_len;

            /* Clear good RX frame event and TX frame sent in the DW1000 status register. */
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);//����Ĵ�����־λ

            /* A frame has been received, read it into the local buffer. */
            frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;	//��ý��յ������ݳ���

                dwt_readrxdata(rx_buffer, frame_len, 0);   //��ȡ��������


            /* Check that the frame is the expected response from the companion "DS TWR responder" example.
             * As the sequence number field of the frame is not relevant, it is cleared to simplify the validation of the frame. */
            rx_buffer[ALL_MSG_SN_IDX] = 0;
            if (rx_buffer[9]==0x10)//�жϽ��յ��������Ƿ���response����
            {
                uint32 final_tx_time;

							
                /* Retrieve poll transmission and response reception timestamp. */
                poll_tx_ts = get_tx_timestamp_u64();										//���POLL����ʱ��T1
                resp_rx_ts = get_rx_timestamp_u64();										//���RESPONSE����ʱ��T4

								memcpy(&dist[TAG_ID],&rx_buffer[11],2);
							
                /* Compute final message transmission time. See NOTE 9 below. */
                final_tx_time = (resp_rx_ts + (RESP_RX_TO_FINAL_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;//����final������ʱ�䣬T5=T4+Treply2
                dwt_setdelayedtrxtime(final_tx_time);//����final������ʱ��T5

                /* Final TX timestamp is the transmission time we programmed plus the TX antenna delay. */
                final_tx_ts = (((uint64)(final_tx_time & 0xFFFFFFFE)) << 8) + TX_ANT_DLY;//final��ʵ�ʷ���ʱ���Ǽ���ʱ����Ϸ�������delay

                /* Write all timestamps in the final message. See NOTE 10 below. */
                final_msg_set_ts(&tx_final_msg[FINAL_MSG_POLL_TX_TS_IDX], poll_tx_ts);//��T1��T4��T5д�뷢������
                final_msg_set_ts(&tx_final_msg[FINAL_MSG_RESP_RX_TS_IDX], resp_rx_ts);
                final_msg_set_ts(&tx_final_msg[FINAL_MSG_FINAL_TX_TS_IDX], final_tx_ts);

                /* Write and send final message. See NOTE 7 below. */
                tx_final_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
                dwt_writetxdata(sizeof(tx_final_msg), tx_final_msg, 0);//����������д��DW1000
                dwt_writetxfctrl(sizeof(tx_final_msg), 0);//�趨�������ݳ���
                dwt_starttx(DWT_START_TX_DELAYED);//�趨Ϊ�ӳٷ���
						
												dID=TAG_ID;
												printf("TAG_ID: %2.0f		", dID);
												dID=ANCHOR_ID;
												printf("ANCHOR_ID: %2.0f		", dID);
                        printf("Distance: %5.0f cm\n", (double)dist[TAG_ID]);
												sprintf(USART_STR_BUF,"TAG_ID: %d		ANCHOR_ID: %d		Distance: %5.0f cm\n",TAG_ID , ANCHOR_ID , (double)dist[TAG_ID]);
											  Usart3_SendString(USART_STR_BUF,80);
							 GPIO_Toggle(LED_GPIO,LED);
                /* Poll DW1000 until TX frame sent event set. See NOTE 8 below. */
                while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS))//���ϲ�ѯоƬ״ֱ̬���������
                { };

                /* Clear TXFRS event. */
                dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);//�����־λ

                /* Increment frame sequence number after transmission of the final message (modulo 256). */
                frame_seq_nb++;
								time32_reset = 0;
								//GPIO_Toggle(GPIOA,LED_PIN);//LED��˸
								jumptime = 0;
            }else{
							
							jumptime =5;//���ͨѶʧ�ܣ������ʱ������5ms���ܿ���Ϊ���ǩͬʱ��������ĳ�ͻ��
						}
        }
        else
        {
						
            /* Clear RX error events in the DW1000 status register. */
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
						jumptime =5;
        }

        /* Execute a delay between ranging exchanges. */
        deca_sleep(RNG_DELAY_MS+jumptime);//���߹̶�ʱ��
    }
	}else{
		while (1)//����ģʽ(ANCHOR��վ)
    {
			GPIO_Toggle(LED_GPIO,LED);
        /* Clear reception timeout to start next ranging process. */
        dwt_setrxtimeout(0);//�趨���ճ�ʱʱ�䣬0λû�г�ʱʱ��

        /* Activate reception immediately. */
        dwt_rxenable(0);//�򿪽���

        /* Poll for reception of a frame or error/timeout. See NOTE 7 below. */
        while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR)))//���ϲ�ѯоƬ״ֱ̬�����ճɹ����߳��ִ���
        { };

        if (status_reg & SYS_STATUS_RXFCG)//�ɹ�����
        {
           

            /* Clear good RX frame event in the DW1000 status register. */
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG);//�����־λ

            /* A frame has been received, read it into the local buffer. */
            frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;//��ý������ݳ���

                dwt_readrxdata(rx_buffer, frame_len, 0);//��ȡ��������
            

            /* Check that the frame is a poll sent by "DS TWR initiator" example.
             * As the sequence number field of the frame is not relevant, it is cleared to simplify the validation of the frame. */
            rx_buffer[ALL_MSG_SN_IDX] = 0;
							TAG_ID = rx_buffer[5];
							rx_poll_msg[5] = TAG_ID;//Ϊ���ǩͨѶ���񣬷�ֹһ��ͨѶ�н��յ���ͬID��ǩ������
							tx_resp_msg[5] = TAG_ID;
							rx_final_msg[5] = TAG_ID;
            if (rx_buffer[9]==0x21)//�ж��Ƿ���poll������
            {
                uint32 resp_tx_time;

                /* Retrieve poll reception timestamp. */
                poll_rx_ts = get_rx_timestamp_u64();//���Poll������ʱ��T2

                /* Set send time for response. See NOTE 8 below. */
                resp_tx_time = (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;//����Response����ʱ��T3��
                dwt_setdelayedtrxtime(resp_tx_time);//����Response����ʱ��T3

                /* Set expected delay and timeout for final message reception. */
                dwt_setrxaftertxdelay(RESP_TX_TO_FINAL_RX_DLY_UUS);//���÷�����ɺ��������ӳ�ʱ��
                dwt_setrxtimeout(FINAL_RX_TIMEOUT_UUS);//���ճ�ʱʱ��

                /* Write and send the response message. See NOTE 9 below.*/
								memcpy(&tx_resp_msg[11],&dist[TAG_ID],2);
                tx_resp_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
                dwt_writetxdata(sizeof(tx_resp_msg), tx_resp_msg, 0);//д�뷢������
                dwt_writetxfctrl(sizeof(tx_resp_msg), 0);//�趨���ͳ���
                dwt_starttx(DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED);//�ӳٷ��ͣ��ȴ�����

                /* We assume that the transmission is achieved correctly, now poll for reception of expected "final" frame or error/timeout.
                 * See NOTE 7 below. */
                while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR)))///���ϲ�ѯоƬ״ֱ̬�����ճɹ����߳��ִ���
                { };

                /* Increment frame sequence number after transmission of the response message (modulo 256). */
                frame_seq_nb++;

                if (status_reg & SYS_STATUS_RXFCG)//���ճɹ�
                {
                    /* Clear good RX frame event and TX frame sent in the DW1000 status register. */
                    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);//�����־λ

                    /* A frame has been received, read it into the local buffer. */
                    frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;//���ݳ���

                        dwt_readrxdata(rx_buffer, frame_len, 0);//��ȡ��������
                    

                    /* Check that the frame is a final message sent by "DS TWR initiator" example.
                     * As the sequence number field of the frame is not used in this example, it can be zeroed to ease the validation of the frame. */
                    rx_buffer[ALL_MSG_SN_IDX] = 0;
                    if (rx_buffer[9]==0x23)//�ж��Ƿ�ΪFianl��
                    {
                        uint32 poll_tx_ts, resp_rx_ts, final_tx_ts;
                        uint32 poll_rx_ts_32, resp_tx_ts_32, final_rx_ts_32;
                        double Ra, Rb, Da, Db;
                        int64 tof_dtu;
												
                        /* Retrieve response transmission and final reception timestamps. */
                        resp_tx_ts = get_tx_timestamp_u64();//���response����ʱ��T3
                        final_rx_ts = get_rx_timestamp_u64();//���final����ʱ��T6

                        /* Get timestamps embedded in the final message. */
                        final_msg_get_ts(&rx_buffer[FINAL_MSG_POLL_TX_TS_IDX], &poll_tx_ts);//�ӽ��������ж�ȡT1��T4��T5
                        final_msg_get_ts(&rx_buffer[FINAL_MSG_RESP_RX_TS_IDX], &resp_rx_ts);
                        final_msg_get_ts(&rx_buffer[FINAL_MSG_FINAL_TX_TS_IDX], &final_tx_ts);

                        /* Compute time of flight. 32-bit subtractions give correct answers even if clock has wrapped. See NOTE 10 below. */
                        poll_rx_ts_32 = (uint32)poll_rx_ts;//ʹ��32λ���ݼ���
                        resp_tx_ts_32 = (uint32)resp_tx_ts;
                        final_rx_ts_32 = (uint32)final_rx_ts;
                        Ra = (double)(resp_rx_ts - poll_tx_ts);//Tround1 = T4 - T1  
                        Rb = (double)(final_rx_ts_32 - resp_tx_ts_32);//Tround2 = T6 - T3 
                        Da = (double)(final_tx_ts - resp_rx_ts);//Treply2 = T5 - T4  
                        Db = (double)(resp_tx_ts_32 - poll_rx_ts_32);//Treply1 = T3 - T2  
                        tof_dtu = (int64)((Ra * Rb - Da * Db) / (Ra + Rb + Da + Db));//���㹫ʽ

                        tof = tof_dtu * DWT_TIME_UNITS;
                        distance = tof * SPEED_OF_LIGHT;//����=����*����ʱ��
												dist2 = distance - dwt_getrangebias(config.chan,(float)distance, config.prf);//�����ȥ����ϵ��
												
												dis = dist2*100;//dis Ϊ��λΪcm�ľ���
												dist[TAG_ID] = LP(dis,TAG_ID);//LP Ϊ��ͨ�˲����������ݸ��ȶ�
												time32_reset = 0;
											
//												GPIO_Toggle(GPIOA,LED_PIN);
										
												dID=TAG_ID;
												printf("TAG_ID: %2.0f		", dID);
												dID=ANCHOR_ID;
												printf("ANCHOR_ID: %2.0f		", dID);
                        printf("Distance: %5.0f cm\n", (double)dist[TAG_ID]);
												
												sprintf(USART_STR_BUF,"TAG_ID: %d		ANCHOR_ID: %d		Distance: %5.0f cm\n",TAG_ID,ANCHOR_ID,(double)dist[TAG_ID]);
											  Usart3_SendString(USART_STR_BUF,80);
                       
                    }
                }
                else
                {
                    /* Clear RX error events in the DW1000 status register. */
                    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
                }
            }
        }
        else
        {
            /* Clear RX error events in the DW1000 status register. */
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
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
