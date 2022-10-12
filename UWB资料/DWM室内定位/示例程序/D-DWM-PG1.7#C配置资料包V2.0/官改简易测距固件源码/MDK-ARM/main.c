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
 *这是一个简单的代码示例，它作为SS TWR距离测量交换机中的应答器。此应用程序等待“投票”。
*消息（记录投票的RX时间戳）期望从“SS TWR发起者”示例代码（与此应用程序配套），以及
*然后发送一个“响应”消息来完成交换。响应消息包含该应用程序记录的所有时间戳，
*包括响应消息本身的计算/预测的TX时间戳。同伴“SS TWR发起者”示例应用
*计算出飞行时间在空中，因此，估计的距离之间的两个设备。
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

/* Example application name and version to display on LCD screen. 在业液晶屏上显示的示例应用程序名称和版本*/
#define APP_NAME "DS TWR INIT v1.1"

/* Inter-ranging delay period, in milliseconds. 测距间隔时间，以毫秒为单位。*/
#define RNG_DELAY_MS 100

/* Default communication configuration. We use here EVK1000's default mode (mode 3).默认通信配置。我们在这里使用EVK1000的模式3。*/
static dwt_config_t config = {
    2,               /* Channel number.通道号。*/
    DWT_PRF_64M,     /* Pulse repetition frequency.脉冲重复频率*/
    DWT_PLEN_1024,   /* Preamble length. 前导长度。 */
    DWT_PAC32,       /* Preamble acquisition chunk size. Used in RX only. 前导获取块大小。仅用于RX。 */
    9,               /* TX preamble code. Used in TX only. TX前导码。只在TX使用。 */
    9,               /* RX preamble code. Used in RX only. RX前导码。仅用于RX。*/
    1,               /* Use non-standard SFD (Boolean)  使用非标准的SFD（布尔）*/
    DWT_BR_110K,     /* Data rate. 数据速率。 */
    DWT_PHRMODE_STD, /* PHY header mode. PHY头模式。 */
    (1025 + 64 - 32) /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only.
												SFD超时（前导长度+ 1 +SFD长度-PAC大小）。仅用于RX。*/
};
/* Default antenna delay values for 64 MHz PRF. See NOTE 1 below. 64 MHz PRF的默认天线延迟值。见下面的注释1。*/
#define TX_ANT_DLY 0
#define RX_ANT_DLY 32899
static uint8 rx_poll_msg[] = {0x00, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0x21, 0, 0};
static uint8 tx_resp_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0x10, 0x02, 0, 0, 0, 0};
static uint8 rx_final_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0x23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	
/* Frames used in the ranging process. See NOTE 2 below. 在测距过程中使用的帧。见下面的注释2。*/
static uint8 tx_poll_msg[] = {0x00, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0x21, 0, 0};
static uint8 rx_resp_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0x10, 0x02, 0, 0, 0, 0};
static uint8 tx_final_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0x23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
/* Length of the common part of the message (up to and including the function code, see NOTE 2 below). 
	消息的公共部分的长度（包括函数代码，见下面的注释2）。*/
typedef signed long long int64;
typedef unsigned long long uint64;
static uint64 poll_rx_ts;
static uint64 resp_tx_ts;
static uint64 final_rx_ts;

static double tof;
static double distance,dist2;
int16_t dist[8];
#define ALL_MSG_COMMON_LEN 10
/* Indexes to access some of the fields in the frames defined above. 索引访问进程中涉及的框架中的一些字段。*/
#define ALL_MSG_SN_IDX 2
#define FINAL_MSG_POLL_TX_TS_IDX 10
#define FINAL_MSG_RESP_RX_TS_IDX 14
#define FINAL_MSG_FINAL_TX_TS_IDX 18
#define FINAL_MSG_TS_LEN 4
/* Frame sequence number, incremented after each transmission. 帧序列号，每次传输后递增。 */
static uint32 frame_seq_nb = 0;

/* Buffer to store received response message.缓冲区来存储接收到的消息。
 * Its size is adjusted to longest frame that this example code is supposed to handle.它的大小被调整到这个示例代码应该处理的最长帧。 */
#define RX_BUF_LEN 20
#define RX_BUF_LEN2 24
static uint8 rx_buffer[RX_BUF_LEN+4];

/* Hold copy of status register state here for reference, so reader can examine it at a breakpoint. 
在这里保存状态寄存器状态的副本以供参考，以便读者可以在断点上检查它。*/
static uint32 status_reg = 0;

/* UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) conversion factor.
UWB微秒（UUS）到设备时间单位（DTU，约15.65 ps）的转换系数。
 * 1 uus = 512 / 499.2 祍 and 1 祍 = 499.2 * 128 dtu. 
  1 UUS＝512／499.2，1＝499.2×128 DTU。*/
#define UUS_TO_DWT_TIME 65536

/* Delay between frames, in UWB microseconds. See NOTE 4 below.帧之间的延迟，以UWB微秒为单位。见下面的注释4。*/
/* This is the delay from the end of the frame transmission to the enable of the receiver, as programmed for the DW1000's wait for response feature.
这是从帧传输结束到接收机启用的延迟，如DW1000等待响应特性的编程。*/
#define POLL_TX_TO_RESP_RX_DLY_UUS 150
/* This is the delay from Frame RX timestamp to TX reply timestamp used for calculating/setting the DW1000's delayed TX function. This includes the
 * frame length of approximately 2.66 ms with above configuration.
这是从帧RX时间戳到用于计算/设置DW1000延迟TX功能的TX回复时间戳的延迟。这包括具有上述配置的大约2.66毫秒的帧长度。 */
#define RESP_RX_TO_FINAL_TX_DLY_UUS 3100
/* Receive response timeout. See NOTE 5 below. 接收响应超时。见下面的注释5。*/
#define RESP_RX_TIMEOUT_UUS 4700

#define POLL_RX_TO_RESP_TX_DLY_UUS 2600
/* This is the delay from the end of the frame transmission to the enable of the receiver, as programmed for the DW1000's wait for response feature. 
这是从帧传输结束到接收机启用的延迟，如DW1000等待响应特性的编程。*/
#define RESP_TX_TO_FINAL_RX_DLY_UUS 500
/* Receive final timeout. See NOTE 5 below.接收最终超时。见下面的注释5。 */
#define FINAL_RX_TIMEOUT_UUS 4300
#define SPEED_OF_LIGHT 299702547
/* Time-stamps of frames transmission/reception, expressed in device time units.时间邮票of frames发送/接收装置，表达时间单位。
 * As they are 40-bit wide, we need to define a 64-bit int type to handle them. 由于它们是40位宽的，所以我们需要定义一个64位int类型来处理它们。*/
typedef unsigned long long uint64;
static uint64 poll_tx_ts;
static uint64 resp_rx_ts;
static uint64 final_tx_ts;

/* Declaration of static functions. 静态函数的声明。*/
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

uint8_t Work_Mode=0;  //模式选择， 0为标签 1为基站
uint8_t TAG_ID=0;         //标签ID 
uint8_t ANCHOR_ID=1;      //基站ID
unsigned char USART_STR_BUF[200];   //调试用的字符串输出缓存

int main(void)
{
    /* Start with board specific hardware init. */
    peripherals_init();//初始化外设

    /* Display application name on LCD. */
  //  lcd_display_str(APP_NAME);

    /* Reset and initialise DW1000.
     * For initialisation, DW1000 clocks must be temporarily set to crystal speed. After initialisation SPI rate can be increased for optimum
     * performance. */
    reset_DW1000();//重启DW1000 /* Target specific drive of RSTn line into DW1000 low for a period. */
    spi_set_rate_low();//降低SPI频率
    dwt_initialise(DWT_LOADUCODE);//初始化DW1000
    spi_set_rate_high();//回复SPI频率
	
    /* Configure DW1000. See NOTE 6 below. */
    dwt_configure(&config);//配置DW1000

    /* Apply default antenna delay value. See NOTE 1 below. */
    dwt_setrxantennadelay(RX_ANT_DLY);		//设置接收天线延迟
    dwt_settxantennadelay(TX_ANT_DLY);		//设置发射天线延迟

    /* Set expected response's delay and timeout. See NOTE 4 and 5 below.
     * As this example only handles one incoming frame with always the same delay and timeout, those values can be set here once for all. */
    dwt_setrxaftertxdelay(POLL_TX_TO_RESP_RX_DLY_UUS);//设置发送后开启接收，并设定延迟时间
    dwt_setrxtimeout(RESP_RX_TIMEOUT_UUS);						//设置接收超时时间

		send[0]=0x6D;											//串口数据
		send[1]=0xD6;											//串口数据

		tx_poll_msg[6] = ANCHOR_ID;	//UWB POLL 包数据
		rx_resp_msg[6] = ANCHOR_ID;	//UWB RESPONSE 包数据
		tx_final_msg[6] = ANCHOR_ID;//UWB Fianl 包数据
		
		rx_poll_msg[6] = ANCHOR_ID;
		tx_resp_msg[6] = ANCHOR_ID;
		rx_final_msg[6] = ANCHOR_ID;
		
		tx_poll_msg[5] = TAG_ID;//UWB POLL 包数据
		rx_resp_msg[5] = TAG_ID;//UWB RESPONSE 包数据
		tx_final_msg[5] = TAG_ID;//UWB Fianl 包数据
    /* Loop forever initiating ranging exchanges. */
		//LED_blink();
		if(!Work_Mode)   //选择发送模式（TAG标签）还是接收模式(ANCHOR基站)
		{
    while (1)			//发送模式(TAG标签)
    {

        /* Write frame data to DW1000 and prepare transmission. See NOTE 7 below. */
        tx_poll_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
        dwt_writetxdata(sizeof(tx_poll_msg), tx_poll_msg, 0);//将Poll包数据传给DW1000，将在开启发送时传出去
        dwt_writetxfctrl(sizeof(tx_poll_msg), 0);//设置超宽带发送数据长度

        /* Start transmission, indicating that a response is expected so that reception is enabled automatically after the frame is sent and the delay
         * set by dwt_setrxaftertxdelay() has elapsed. */
        dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);//开启发送，发送完成后等待一段时间开启接收，等待时间在dwt_setrxaftertxdelay中设置

        /* We assume that the transmission is achieved correctly, poll for reception of a frame or error/timeout. See NOTE 8 below. */
        while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR)))//不断查询芯片状态直到成功接收或者发生错误
        { };

        /* Increment frame sequence number after transmission of the poll message (modulo 256). */
        frame_seq_nb++;

        if (status_reg & SYS_STATUS_RXFCG)//如果成功接收
        {					
            uint32 frame_len;

            /* Clear good RX frame event and TX frame sent in the DW1000 status register. */
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);//清楚寄存器标志位

            /* A frame has been received, read it into the local buffer. */
            frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;	//获得接收到的数据长度

                dwt_readrxdata(rx_buffer, frame_len, 0);   //读取接收数据


            /* Check that the frame is the expected response from the companion "DS TWR responder" example.
             * As the sequence number field of the frame is not relevant, it is cleared to simplify the validation of the frame. */
            rx_buffer[ALL_MSG_SN_IDX] = 0;
            if (rx_buffer[9]==0x10)//判断接收到的数据是否是response数据
            {
                uint32 final_tx_time;

							
                /* Retrieve poll transmission and response reception timestamp. */
                poll_tx_ts = get_tx_timestamp_u64();										//获得POLL发送时间T1
                resp_rx_ts = get_rx_timestamp_u64();										//获得RESPONSE接收时间T4

								memcpy(&dist[TAG_ID],&rx_buffer[11],2);
							
                /* Compute final message transmission time. See NOTE 9 below. */
                final_tx_time = (resp_rx_ts + (RESP_RX_TO_FINAL_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;//计算final包发送时间，T5=T4+Treply2
                dwt_setdelayedtrxtime(final_tx_time);//设置final包发送时间T5

                /* Final TX timestamp is the transmission time we programmed plus the TX antenna delay. */
                final_tx_ts = (((uint64)(final_tx_time & 0xFFFFFFFE)) << 8) + TX_ANT_DLY;//final包实际发送时间是计算时间加上发送天线delay

                /* Write all timestamps in the final message. See NOTE 10 below. */
                final_msg_set_ts(&tx_final_msg[FINAL_MSG_POLL_TX_TS_IDX], poll_tx_ts);//将T1，T4，T5写入发送数据
                final_msg_set_ts(&tx_final_msg[FINAL_MSG_RESP_RX_TS_IDX], resp_rx_ts);
                final_msg_set_ts(&tx_final_msg[FINAL_MSG_FINAL_TX_TS_IDX], final_tx_ts);

                /* Write and send final message. See NOTE 7 below. */
                tx_final_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
                dwt_writetxdata(sizeof(tx_final_msg), tx_final_msg, 0);//将发送数据写入DW1000
                dwt_writetxfctrl(sizeof(tx_final_msg), 0);//设定发送数据长度
                dwt_starttx(DWT_START_TX_DELAYED);//设定为延迟发送
						
												dID=TAG_ID;
												printf("TAG_ID: %2.0f		", dID);
												dID=ANCHOR_ID;
												printf("ANCHOR_ID: %2.0f		", dID);
                        printf("Distance: %5.0f cm\n", (double)dist[TAG_ID]);
												sprintf(USART_STR_BUF,"TAG_ID: %d		ANCHOR_ID: %d		Distance: %5.0f cm\n",TAG_ID , ANCHOR_ID , (double)dist[TAG_ID]);
											  Usart3_SendString(USART_STR_BUF,80);
							 GPIO_Toggle(LED_GPIO,LED);
                /* Poll DW1000 until TX frame sent event set. See NOTE 8 below. */
                while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS))//不断查询芯片状态直到发送完成
                { };

                /* Clear TXFRS event. */
                dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);//清楚标志位

                /* Increment frame sequence number after transmission of the final message (modulo 256). */
                frame_seq_nb++;
								time32_reset = 0;
								//GPIO_Toggle(GPIOA,LED_PIN);//LED闪烁
								jumptime = 0;
            }else{
							
							jumptime =5;//如果通讯失败，将间隔时间增加5ms，避开因为多标签同时发送引起的冲突。
						}
        }
        else
        {
						
            /* Clear RX error events in the DW1000 status register. */
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
						jumptime =5;
        }

        /* Execute a delay between ranging exchanges. */
        deca_sleep(RNG_DELAY_MS+jumptime);//休眠固定时间
    }
	}else{
		while (1)//接收模式(ANCHOR基站)
    {
			GPIO_Toggle(LED_GPIO,LED);
        /* Clear reception timeout to start next ranging process. */
        dwt_setrxtimeout(0);//设定接收超时时间，0位没有超时时间

        /* Activate reception immediately. */
        dwt_rxenable(0);//打开接收

        /* Poll for reception of a frame or error/timeout. See NOTE 7 below. */
        while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR)))//不断查询芯片状态直到接收成功或者出现错误
        { };

        if (status_reg & SYS_STATUS_RXFCG)//成功接收
        {
           

            /* Clear good RX frame event in the DW1000 status register. */
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG);//清楚标志位

            /* A frame has been received, read it into the local buffer. */
            frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;//获得接收数据长度

                dwt_readrxdata(rx_buffer, frame_len, 0);//读取接收数据
            

            /* Check that the frame is a poll sent by "DS TWR initiator" example.
             * As the sequence number field of the frame is not relevant, it is cleared to simplify the validation of the frame. */
            rx_buffer[ALL_MSG_SN_IDX] = 0;
							TAG_ID = rx_buffer[5];
							rx_poll_msg[5] = TAG_ID;//为多标签通讯服务，防止一次通讯中接收到不同ID标签的数据
							tx_resp_msg[5] = TAG_ID;
							rx_final_msg[5] = TAG_ID;
            if (rx_buffer[9]==0x21)//判断是否是poll包数据
            {
                uint32 resp_tx_time;

                /* Retrieve poll reception timestamp. */
                poll_rx_ts = get_rx_timestamp_u64();//获得Poll包接收时间T2

                /* Set send time for response. See NOTE 8 below. */
                resp_tx_time = (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;//计算Response发送时间T3。
                dwt_setdelayedtrxtime(resp_tx_time);//设置Response发送时间T3

                /* Set expected delay and timeout for final message reception. */
                dwt_setrxaftertxdelay(RESP_TX_TO_FINAL_RX_DLY_UUS);//设置发送完成后开启接收延迟时间
                dwt_setrxtimeout(FINAL_RX_TIMEOUT_UUS);//接收超时时间

                /* Write and send the response message. See NOTE 9 below.*/
								memcpy(&tx_resp_msg[11],&dist[TAG_ID],2);
                tx_resp_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
                dwt_writetxdata(sizeof(tx_resp_msg), tx_resp_msg, 0);//写入发送数据
                dwt_writetxfctrl(sizeof(tx_resp_msg), 0);//设定发送长度
                dwt_starttx(DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED);//延迟发送，等待接收

                /* We assume that the transmission is achieved correctly, now poll for reception of expected "final" frame or error/timeout.
                 * See NOTE 7 below. */
                while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR)))///不断查询芯片状态直到接收成功或者出现错误
                { };

                /* Increment frame sequence number after transmission of the response message (modulo 256). */
                frame_seq_nb++;

                if (status_reg & SYS_STATUS_RXFCG)//接收成功
                {
                    /* Clear good RX frame event and TX frame sent in the DW1000 status register. */
                    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);//清楚标志位

                    /* A frame has been received, read it into the local buffer. */
                    frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;//数据长度

                        dwt_readrxdata(rx_buffer, frame_len, 0);//读取接收数据
                    

                    /* Check that the frame is a final message sent by "DS TWR initiator" example.
                     * As the sequence number field of the frame is not used in this example, it can be zeroed to ease the validation of the frame. */
                    rx_buffer[ALL_MSG_SN_IDX] = 0;
                    if (rx_buffer[9]==0x23)//判断是否为Fianl包
                    {
                        uint32 poll_tx_ts, resp_rx_ts, final_tx_ts;
                        uint32 poll_rx_ts_32, resp_tx_ts_32, final_rx_ts_32;
                        double Ra, Rb, Da, Db;
                        int64 tof_dtu;
												
                        /* Retrieve response transmission and final reception timestamps. */
                        resp_tx_ts = get_tx_timestamp_u64();//获得response发送时间T3
                        final_rx_ts = get_rx_timestamp_u64();//获得final接收时间T6

                        /* Get timestamps embedded in the final message. */
                        final_msg_get_ts(&rx_buffer[FINAL_MSG_POLL_TX_TS_IDX], &poll_tx_ts);//从接收数据中读取T1，T4，T5
                        final_msg_get_ts(&rx_buffer[FINAL_MSG_RESP_RX_TS_IDX], &resp_rx_ts);
                        final_msg_get_ts(&rx_buffer[FINAL_MSG_FINAL_TX_TS_IDX], &final_tx_ts);

                        /* Compute time of flight. 32-bit subtractions give correct answers even if clock has wrapped. See NOTE 10 below. */
                        poll_rx_ts_32 = (uint32)poll_rx_ts;//使用32位数据计算
                        resp_tx_ts_32 = (uint32)resp_tx_ts;
                        final_rx_ts_32 = (uint32)final_rx_ts;
                        Ra = (double)(resp_rx_ts - poll_tx_ts);//Tround1 = T4 - T1  
                        Rb = (double)(final_rx_ts_32 - resp_tx_ts_32);//Tround2 = T6 - T3 
                        Da = (double)(final_tx_ts - resp_rx_ts);//Treply2 = T5 - T4  
                        Db = (double)(resp_tx_ts_32 - poll_rx_ts_32);//Treply1 = T3 - T2  
                        tof_dtu = (int64)((Ra * Rb - Da * Db) / (Ra + Rb + Da + Db));//计算公式

                        tof = tof_dtu * DWT_TIME_UNITS;
                        distance = tof * SPEED_OF_LIGHT;//距离=光速*飞行时间
												dist2 = distance - dwt_getrangebias(config.chan,(float)distance, config.prf);//距离减去矫正系数
												
												dis = dist2*100;//dis 为单位为cm的距离
												dist[TAG_ID] = LP(dis,TAG_ID);//LP 为低通滤波器，让数据更稳定
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
1。该值的总和是通过校准过程实验确定的TX到RX天线延迟。这里我们使用硬编码的典型值。
但是，在实际应用中，每个设备都应该有自己的天线延迟适当校准，以在执行时获得最佳可能的精度。
距离测量。
2。这里的消息与脱钩ARM应用程序（EVK1000套件）一起使用。他们遵守IEEE。
802.15.4标准的MAC数据帧编码，它们遵循ISO/IEC:2473062/2013标准。所使用的消息是：
-发起者发送的轮询消息以触发测距交换。
-响应方发送的响应消息，允许发起方继续执行进程。
-由发起方发送的最终消息以完成交换，并提供响应者所需的所有信息来计算
飞行时间（距离）估计。
这些帧的前10个字节是常见的，并且由以下字段组成：
-字节0/1：帧控制（0x88 41，用16位寻址指示数据帧）。
-字节2：序列号，为每个新帧增加。
-字节3/4：PANTAGIGID（0xDECA）。
-字节5/6：目的地址，见下面的注释3。
-字节7/8：源地址，见下文注释3。
-字节9：函数代码（特定值来指示它在测距过程中的哪个消息）。
其余的字节对每个消息都是特定的：
民意测验：
-不再有数据
响应消息：
-字节10：活动代码（0x02告诉发起者继续进行测距交换）。
-字节11/12：活动参数，此处不用于活动代码0x02。
最后消息：
-字节10＞13：轮询消息传输时间戳。
-字节14＞17：响应消息接收时间戳。
-字节18＞21：最后的消息传输时间戳。
所有消息以DW1000自动设置的2字节校验和结束。
三。在这个例子中，源地址和目的地址是硬编码常量，以保持简单，但对于实际产品，每个设备都应该有一个
独特的TAGJID。这里，16位寻址被用来保持消息尽可能短，但是，在实际应用中，这应该只做。
在交换特定消息之后，用于为参与测距交换的每个设备定义那些短地址。
4。这里选择帧之间的延迟，以确保发送器之间的帧的发送和接收的适当同步。
和应答器，保证计算距离的正确精度。用户被称为更多的脱角臂源代码指南。
有关测距过程中的计时的细节。
5。此超时是为了完全接收帧，即超时持续时间必须考虑预期帧的长度。这里的价值
是任意的，但选择大到足以确保有足够的时间来接收响应者发送的完整响应帧。
使用1K数据速率（约3毫秒）。
6。在实际应用中，为了在调节范围内达到最佳性能，可能需要设置TX脉冲带宽和TX功率，（使用
DWTSCOMPUTURTXRF API调用到在目标系统或DW1000 OTP存储器中保存的每个设备校准值。
7。DWTJWRIGETXDATA（）将消息的完整大小作为参数，但仅在帧结尾的校验和时复制（大小为2）字节。
由DW1000自动追加。这意味着我们的变量可以在不丢失任何数据的情况下短两个字节（但是sieof不会）。
工作，然后我们仍然必须指示框架的完整长度为DWTTWORGETXDATA（））。还需要注意的是，使用时
延迟发送，传输的时间设置在未来必须足够远，因此DW1000 IC有时间来处理和启动
在需要的时间内传输帧。如果传输命令发出的时间太晚，与帧被发送时相比，
这是由DWTSARSTTTX（）API调用返回的错误代码所指示的。这里没有测试，因为帧之间的延迟值有
被仔细定义以避免这种情况。
8。我们在这里使用轮询模式来保持示例尽可能简单，但是所有状态事件都可以用来产生中断。拜托
有关“中断”的详细信息，请参阅DW1000用户手册。还需要注意的是，状态寄存器是5字节长，但作为事件我们
使用都在寄存器的第一个字节中，我们可以使用简单的DWTSRead 32位Read（）API调用来访问它，而不是读取整个5个字节。
字节。
9。当我们想要在最终的消息中发送最终的TX时间戳时，我们必须提前计算它，而不是依赖于DW1000的读取。
寄存器。时间戳和延迟传输时间都表示在设备时间单位中，所以我们只需将期望的响应延迟添加到
响应RX时间戳以获得最终传输时间。延迟传输时间分辨率是512个设备时间单位，这意味着
所获得的值的低9位必须被归零。这还允许通过移位全零下位来在32位字中编码40位值。
8位。
10。在这个操作中，丢弃每个40位时间戳的高阶字节。这是可以接受的，因为那些时间戳没有分开。
超过2×32的设备时间单位（大约67毫秒），这意味着计算往返延迟（需要在
飞行时间计算可以通过32位减法来处理。
11。用户被称为脱角ARM应用程序（用EVK1000产品分发），用于附加的实际使用示例，以及
DW1000 API指南中关于DW1000驱动程序功能的更多细节。
 ****************************************************************************************************************************************************/
