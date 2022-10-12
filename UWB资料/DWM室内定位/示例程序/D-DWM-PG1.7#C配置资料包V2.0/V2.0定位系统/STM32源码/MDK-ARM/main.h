#ifndef __MAIN_H
#define __MAIN_H
#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_sleep.h"
#include "port.h"
/* Example application name and version to display on LCD screen. 在业液晶屏上显示的示例应用程序名称和版本*/
#define APP_NAME "DS TWR INIT v1.1"

/* Inter-ranging delay period, in milliseconds. 测距间隔时间，以毫秒为单位。*/
#define RNG_DELAY_MS 5
#define firmware_version 20 //固件版本号 
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
//#define RX_ANT_DLY 32950
	
#define FINAL_MSG_TS_LEN 4  //时间数据长度
extern u32 MODBUS_BaudRate[10];
extern int16_t SYS_dis_buf_t[7]; //主基站获取到的距离数据缓存区
extern u16 ERROR_FLAG;  //测距错误计算次数标志位，达到一定次数跳出

extern  u16   Flash_Usart_BaudRate;   //设备串口通讯波特率 0：4800  1：9600 2：14400 3：19200 4：38400 5：56000 6：57600 7：115200  8：128000 9：256000
extern  u16   Flash_Modbus_ADDR;        //Modbus ID号 
extern  u16   Flash_structure_Mode;     //0:为二维平面 3基站模式
extern  u16   Flash_Ranging_Mode;        //测距模式
extern  u16   Flash_Device_Mode;       //设备模式 0：标签 1：次基站 2：主基站
extern  u16   Flash_Device_ID;         //高8位为次基站ID，范围0~6  低8位为标签ID 0~99    （程序内部 标签ID为0~247  次基站ID为248~245  主基站ID为255）
extern  u16   Flash_MAJOR_BS_X_Y_Z[3];   //主基站的位置，      X,Y,Z各两字节
extern  u16   Flash_BS_EN_X_Y_Z[7][4];   //7次基站的位置，    使能标志：0为关  1为开   X,Y,Z各两字节
extern  u16 	Flash_TAG_NUM;					//测量标签ID
extern  u8 	  Flash_TAG_BUF[100];      //标签ID列表

extern  u16   Flash_FLAG;	 //标志位
extern  u16   FLASH_Channel;              //空中信道
extern  u16   FLASH_Data_rat;             //空中传输速率     
extern  u16   FLASH_RX_ANT_DLY;           //接收延时
extern  u16   FLASH_KALMAN_Q;       	    //卡尔曼滤波-Q
extern  u16   FLASH_KALMAN_R;					    //卡尔曼滤波-R

extern  u16   Calculate_EN;       //测量使能
extern  u16		Calculate_FLAG;			//测量状态标志
extern  u16		Calculate_TAG_ID;						//测量执行的标签ID
extern  u16		Calculate_TAG_X;		//测量标签的X轴
extern  u16		Calculate_TAG_Y;		//测量标签的Y轴
extern  u16		Calculate_TAG_Z;     //测量标签的Z轴
extern  u16		Calculate_Station_TAG[8]; //测量标签的距离

extern  unsigned char USART_STR_BUF[300];   //调试用的字符串输出缓存
 void GPIO_Toggle(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
#endif


