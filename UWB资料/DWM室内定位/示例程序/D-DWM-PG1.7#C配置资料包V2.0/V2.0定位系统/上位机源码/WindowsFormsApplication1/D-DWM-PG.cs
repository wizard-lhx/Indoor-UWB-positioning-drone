using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;
using System.Threading;
using System.Reflection;
using Excel = Microsoft.Office.Interop.Excel;
using System.IO;
using System.Reflection;
 
using OpenTK;
using OpenTK.Graphics;
using AxlesGL;

namespace WindowsFormsApplication1
{
    public partial class Form1 : Form
    {
        Axles3D formx = new Axles3D();
        private int state_Flag = 0;
        private int GETDATA_NEW = 0;  //是否为新数据标志位
        private Int16 time_SCAN_NUM = 0;//扫描MODBUS—ID标志计数用
        private static volatile byte[] rBuff = null;                  //正确接收缓冲器
        private int[] Flag_BaudRate_Delay = new int[] {9,7,4,3,2,1,1,1,1,1};//波特率决定接收延时
        private int connect_Flag = 0;//0：未搜索 1：未连接串口  2：未扫描设备  3：未连接设备  4：通讯成功中 5：通讯失败中
        private int Flag_BaudRate= 0;//波特率决定接收延时
        private Byte Click_Flag = 0;   //按钮执行事件标志  0：读取配置   1：写入配置  2：设置测距 3：访问测距信息4：停止测距
        private Int16 isSCAN = 0;   //是否扫描设备开启标志
        private Int16 CJ_EN = 0;   //测距发送协议使能
        private Byte NOW_ID = 0;    //连接设备的ID号
        private string[] ID_buf = new String[] { }; //设备搜索的缓存
        private int CRCL, CRCH, CRCBL, CRCBH;
        private Int16 isOpen = 0;   //串口是否开启标志
      //  private Int16[] dis_buff = new Int16[7];  //给画图距离缓存区
        private bool isGJ = false;   //是否开启轨迹
        private float map_multiple = 1; //地图缩小倍数
        private Int16 map_size_x = 800, map_size_y = 450;  //地图大小
        private Int16 BX_size_x = 450, BX_size_y = 115;  //波形图大小
        private Int16 map_origin_x = 50, map_origin_y = 50;  //原点坐标
        private Byte CJ_NG_Flag = 0;   //测距标志位，每一位对应着基站的使能，第一行标签
        private int KALMAN_Q = 1;      	  //卡尔曼-Q
        private int KALMAN_R = 500;					//卡尔曼-R
        private static double[] x_last = new double[512];//卡尔曼
        private double[] p_last = new double[512];//卡尔曼     
        private string pathname = string.Empty;     		//定义路径名变量
        private static Int16[,] sys_data_EN_x_y_z = new Int16[8, 4];
        private static Int16[,] sys_data_TAG_x_y_z = new Int16[100, 3];
        private byte[] aucCRCHi = new byte[]  {  
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
};


        private byte[] aucCRCLo = new byte[] {  
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
};
        private Bitmap bMap;
        private Graphics gph;
        private Bitmap bMap_BX1;
        private Graphics gph_BX1;
        private Bitmap bMap_BX2;
        private Graphics gph_BX2;
        public Form1()
        {
            InitializeComponent();
            CheckForIllegalCrossThreadCalls = false;//为了让其他类能调用本类组件
       
        }
        void SCAN_ID_Click(Int16 i)
        {

            Array.Resize<string>(ref ID_buf, ID_buf.Length + 1);
            ID_buf[ID_buf.Length - 1] = i.ToString();

        }
        void printf_data(byte[] Frame, int Length,int T_R)//打印串口数据
        {
            
            Int16 i_len; 
            Int16 i_han;
             i_han = (Int16)this.textBox_com_data.Lines.Count();
            if (i_han > 10)
            {
                textBox_com_data.Clear();
            }

            if(T_R==0) textBox_com_data.Text += "发送：";
            else textBox_com_data.Text += "接收：";
            for (i_len = 0; i_len < Length; i_len++)   //打印字符串
            {
                textBox_com_data.Text += Frame[i_len].ToString("X2");
                textBox_com_data.Text += " ";
            }
            textBox_com_data.Text += "\r\n"; 
            /*
            textBox_com_data.Focus(); //获取焦点
            textBox_com_data.Select(textBox_com_data.TextLength, 0);//光标
            textBox_com_data.ScrollToCaret(); //滚动条*/
        }
        void senddata(byte[] Frame, int Length)     // 传入要校验的数组名及其长度    
        {
            Int32 crc;            
            crc = CRC16(Frame, Length - 2);
            Frame[Length - 2] = (byte)(crc / 256);
            Frame[Length - 1] = (byte)(crc % 256);

            printf_data(Frame, Length, 0);

            try
            {
                serialPort1.Write(Frame, 0, Length);
            }
            catch
            {
                MessageBox.Show("串口通讯错误", "提示");
                return;
            }
        }
        //通过逆序CRC16表, 逆序CRC计算  
        int CRC16(byte[] Frame, int Length)     // 传入要校验的数组名及其长度    
        {
            byte CRCHi = 0xFF;
            byte CRCLo = 0xFF;
            int iIndex = 0;


            for (int i = 0; i < Length; i++)
            {
                iIndex = CRCHi ^ (Frame[i]);
                CRCHi = (byte)(CRCLo ^ aucCRCHi[iIndex]);
                CRCLo = aucCRCLo[iIndex];
            }


            return (int)(CRCHi << 8 | CRCLo);// CRC校验返回值   // CRCHI 向左移动，就是逆序计算的代表              
        }
         void Drawing_init()  //画图初始化函数
        {
              //画图初始化    
            bMap = new Bitmap(map_size_x, map_size_y);
            gph = Graphics.FromImage(bMap);
            gph.Clear(Color.White);

            //画图初始化    
            bMap_BX1 = new Bitmap(BX_size_x, BX_size_y);
            gph_BX1 = Graphics.FromImage(bMap_BX1);
            gph_BX1.Clear(Color.White);

            //画图初始化    
            bMap_BX2 = new Bitmap(BX_size_x, BX_size_y);
            gph_BX2 = Graphics.FromImage(bMap_BX2);
            gph_BX2.Clear(Color.White);

            //显示在pictureBox1控件中
            this.pictureBox1.Image = bMap;
            this.pictureBox_pass1.Image = bMap_BX1;
            this.pictureBox_pass2.Image = bMap_BX2;
            
            //保存输出图片    
           // bMap.Save("c://test.bmp");
        }

         void Drawing_draw_bs(Int16 x, Int16 y)  //画基站
         {
             Color color = Color.FromArgb( 34,  139, 34);
             Pen WW = new Pen( color,3);

             gph.DrawLine(WW, x, y, x-7, y+18);  //画x轴
             gph.DrawLine(WW, x, y, x + 7, y + 18);  //画x轴
             gph.DrawLine(WW, x-4, y+10, x+ 4, y+10 );  //画x轴
             gph.DrawEllipse(WW, x-4, y-4, 8, 8);


         }
         void Drawing_BOXING1_update(Int16 num)  //画图更新
         {
             Int16 ff,gg;
             gph_BX1.Clear(Color.White);

             //Color color = Color.FromArgb(34, 139, 34);
             Pen[] WW = new Pen[8];
             WW[0] = new Pen(Color.Green, 2);
             WW[1] = new Pen(Color.Pink, 2);
             WW[2] = new Pen(Color.Red, 2);
             WW[3] = new Pen(Color.Blue, 2);
             WW[4] = new Pen(Color.Gold, 2);
             WW[5] = new Pen(Color.Orange, 2);
             WW[6] = new Pen(Color.Tomato, 2);
             WW[7] = new Pen(Color.Aqua, 2);
             for (ff = 0; ff <(BX_size_x-1); ff++)
             {
                 for (gg = 0; gg < 8; gg++)
                 {
                     Int16 GGA, GGB, BL;

                     if (numericUpDown_Y1_F.Value == 0)
                     {
                         BL = 10;
                     }
                     else
                     {
                         BL = (Int16)numericUpDown_Y1_F.Value;
                     }

                     if (dataGridView_GJ1.Rows[ff].Cells[gg+3].Value == null)
                     {
                         GGA = (Int16)(BX_size_y - 0);
                     }
                     else
                     {
                         GGA = (Int16)(BX_size_y - (Convert.ToInt16(dataGridView_GJ1.Rows[ff].Cells[gg+3].Value.ToString()) / BL));
                     }

                     if (dataGridView_GJ1.Rows[ff + 1].Cells[gg+3].Value == null)
                     {
                         GGB = (Int16)(BX_size_y - 0);
                     }
                     else
                     {
                         GGB = (Int16)(BX_size_y - (Convert.ToInt16(dataGridView_GJ1.Rows[ff + 1].Cells[gg+3].Value.ToString()) / BL));
                     }
                     gph_BX1.DrawLine(WW[gg], ff, GGA, ff + 1, GGB);    //画线

                 }
             }
             this.pictureBox_pass1.Image = bMap_BX1;

         }
         void Drawing_BOXING2_update(Int16 num)  //画图更新
         {
             Int16 ff, gg;
             gph_BX2.Clear(Color.White);

             //Color color = Color.FromArgb(34, 139, 34);
             Pen[] WW = new Pen[8];
             WW[0] = new Pen(Color.Green, 2);
             WW[1] = new Pen(Color.Pink, 2);
             WW[2] = new Pen(Color.Red, 2);
             WW[3] = new Pen(Color.Blue, 2);
             WW[4] = new Pen(Color.Gold, 2);
             WW[5] = new Pen(Color.Orange, 2);
             WW[6] = new Pen(Color.Tomato, 2);
             WW[7] = new Pen(Color.Aqua, 2);
             for (ff = 0; ff < (BX_size_x - 1); ff++)
             {
                 for (gg = 0; gg < 8; gg++)
                 {
                     Int16 GGA, GGB, BL;

                     if (numericUpDown_Y2_F.Value == 0)
                     {
                         BL = 10;
                     }
                     else
                     {
                         BL = (Int16)numericUpDown_Y2_F.Value;
                     }

                     if (dataGridView_GJ2.Rows[ff].Cells[gg+3].Value == null)
                     {
                         GGA = (Int16)(BX_size_y - 0);
                     }
                     else
                     {
                         GGA = (Int16)(BX_size_y - (Convert.ToInt16(dataGridView_GJ2.Rows[ff].Cells[gg+3].Value.ToString()) / BL));
                     }

                     if (dataGridView_GJ2.Rows[ff + 1].Cells[gg+3].Value == null)
                     {
                         GGB = (Int16)(BX_size_y - 0);
                     }
                     else
                     {
                         GGB = (Int16)(BX_size_y - (Convert.ToInt16(dataGridView_GJ2.Rows[ff + 1].Cells[gg+3].Value.ToString()) / BL));
                     }
                     gph_BX2.DrawLine(WW[gg], ff, GGA, ff + 1, GGB);  //画线

                 }
             }
             this.pictureBox_pass2.Image = bMap_BX2;

         }
         void Drawing_update_one_to_one(Int16 num)  //画图更新
         {
             //画图初始化   
             Int16 Px, Py, xx;
             gph.Clear(Color.White);
             Int16 x, y, z;
             Int16 BQ_num, ff;
             try
             {
                 System.Drawing.Image img = System.Drawing.Image.FromFile(pathname);
                 gph.DrawImage(img, 0, 0, map_size_x, map_size_y);
             }
             catch
             {

             }
             map_multiple = (float)numericUpDown_map_multiple.Value;             
             if (checkBox_axis.Checked == true) //画坐标轴
             {
                 PointF cPt;
                 PointF[] xPt = new PointF[3]{
                 new   PointF(map_origin_x,0),
                 new   PointF(map_origin_x-8,15),
                 new   PointF(map_origin_x+8,15)};//X轴三角形    
                 PointF[] yPt = new PointF[3]{
                 new   PointF(map_size_x,map_size_y - map_origin_y),
                 new   PointF(map_size_x-15,map_size_y - (map_origin_y-8)),
                 new   PointF(map_size_x-15,map_size_y - (map_origin_y+8))};//Y轴三角形    
                 gph.DrawPolygon(Pens.Black, xPt);//X轴三角形 
                 gph.FillPolygon(new SolidBrush(Color.Black), xPt);
                 gph.DrawPolygon(Pens.Black, yPt);//Y轴三角形   
                 gph.FillPolygon(new SolidBrush(Color.Black), yPt);

                 gph.DrawLine(Pens.Black, map_origin_x, map_size_y - (map_origin_y - 20), map_origin_x, 0);  //画y轴
                 gph.DrawLine(Pens.Black, map_origin_x - 20, map_size_y - map_origin_y, map_size_x, map_size_y - map_origin_y);  //画X轴
                 {
                     cPt = new PointF(map_origin_x - 20, map_size_y - (map_origin_y - 5));
                     gph.DrawString("0m", new Font("宋体", 10), Brushes.Black, cPt);
                 }
                 {  //画X轴刻点
                     Int16 zb_num;
                     Int16 i;
                     zb_num = (Int16)((map_size_x - map_origin_x) / 100);
                     if (((map_size_x - map_origin_x) % 100) == 0) zb_num--;
                     for (i = 0; i < zb_num; i++)
                     {
                         float mm;
                         cPt = new PointF(map_origin_x + (i + 1) * 100 - 10, map_size_y - (map_origin_y - 10));//中心点 
                         gph.DrawLine(Pens.Black, map_origin_x + (i + 1) * 100, map_size_y - map_origin_y, map_origin_x + (i + 1) * 100, map_size_y - (map_origin_y + 10));  //画x轴
                         mm = ((i + 1) * (map_multiple));
                         gph.DrawString(mm.ToString("f2") + "m", new Font("宋体", 10), Brushes.Black, cPt);
                     }
                 }
                 {  //画X轴刻点
                     Int16 zb_num;
                     Int16 i;
                     zb_num = (Int16)((map_size_y - map_origin_y) / 100);
                     if (((map_size_y - map_origin_y) % 100) == 0) zb_num--;
                     for (i = 0; i < zb_num; i++)
                     {
                         float mm;
                         cPt = new PointF(map_origin_x - 40, map_size_y - (map_origin_y + (i + 1) * 100 + 7));//中心点 
                         gph.DrawLine(Pens.Black, map_origin_x, map_size_y - (map_origin_y + (i + 1) * 100), map_origin_x + 10, map_size_y - (map_origin_y + (i + 1) * 100));  //画y轴
                         mm = ((i + 1) * (map_multiple));
                         gph.DrawString(mm.ToString("f2") + "m", new Font("宋体", 10), Brushes.Black, cPt);
                     }
                 }
             }
             {
                 Int16 sx, sy;
                 sx = 0;
                 sy = 0;
                 Drawing_draw_bs((Int16)(sx / map_multiple + map_origin_x), (Int16)(map_size_y - (sy / map_multiple + map_origin_y)));
                 if (checkBox_name.Checked == true)
                 {
                     gph.DrawString((char)(0x41 + 0) + "基站", new Font("宋体", 10), Brushes.Black, new PointF((sx / map_multiple + map_origin_x - 12), map_size_y - (sy / map_multiple - 18 + map_origin_y)));
                 }
                 if (checkBox_draw_round.Checked == true)
                 {
                     Int16 dis_buff = (Int16)(Convert.ToInt32(dataGridView_TAG.Rows[num].Cells[0 + 4].Value.ToString()));
                     gph.DrawEllipse(Pens.Red, (((sx - dis_buff) / map_multiple) + map_origin_x), map_size_y - (((sy + dis_buff) / map_multiple) + map_origin_y), dis_buff * 2 / map_multiple, dis_buff * 2 / map_multiple);
                 }

             }

             BQ_num=(Int16)(this.dataGridView_TAG.Rows.Count);
             //画标签
             for (ff = 0; ff < BQ_num;ff++ )
             {

                 x = (Int16)(Convert.ToInt16(dataGridView_TAG.Rows[ff].Cells[4].Value.ToString()));
                 y = 0;
                 z = 0;
         
                 gph.DrawEllipse(Pens.Blue, (x / map_multiple - 4 + map_origin_x), map_size_y - (y / map_multiple + 4 + map_origin_y), 8, 8);
                 gph.FillEllipse(new SolidBrush(Color.Black), (x / map_multiple - 4 + map_origin_x), map_size_y - (y / map_multiple + 4 + map_origin_y), 8, 8);
                 
                 if (checkBox_coordinate.Checked == true) //画标签坐标
                 {
                     Int16 cc_x = (Int16)(x), cc_y = (Int16)(y);

                     PointF cPt = new PointF((x / map_multiple + 10 + map_origin_x), map_size_y - (y / map_multiple + 5 + map_origin_y));//中心点 
                     gph.DrawString("(" + cc_x.ToString() + "cm," + cc_y.ToString() + "cm)", new Font("宋体", 10), Brushes.Black, cPt);

                     if (dataGridView_TAG.Rows[ff].Cells[0].Value == null)
                     {
                         gph.DrawString((Int16)(Convert.ToInt16(0)) + "标签", new Font("宋体", 10), Brushes.Black, new PointF((x / map_multiple + map_origin_x), map_size_y - (y / map_multiple - 10 + map_origin_y)));
                     }
                     else
                     {
                         gph.DrawString((Int16)(Convert.ToInt16(dataGridView_TAG.Rows[ff].Cells[0].Value.ToString())) + "标签", new Font("宋体", 10), Brushes.Black, new PointF((x / map_multiple + map_origin_x), map_size_y - (y / map_multiple - 10 + map_origin_y)));
                     }
                 }

             }
             //显示在pictureBox1控件中
             this.pictureBox1.Image = bMap;
             //保存输出图片    
             //bMap.Save("c://test.bmp");
         }

         void Drawing_update(Int16 num)  //画图更新
         {
             //画图初始化   
             Int16 Px, Py, xx;
             if (isGJ == false)//没有打开轨迹显示就清空
             {
                 gph.Clear(Color.White);
             }
             Int16 x, y, z;
             Int16 BQ_num,ff;

             try//载入地图
             {
                 System.Drawing.Image img = System.Drawing.Image.FromFile(pathname);
                 gph.DrawImage(img, 0, 0, map_size_x, map_size_y);                
             }
             catch
             {

             }            

             map_multiple = (float)numericUpDown_map_multiple.Value;
             /*
             for (Px = 0; Px <= map_size_x; Px++)  //画距离点
             {
                 for (Py = 0; Py <= map_size_y; Py++)
                 {
                     if ((Px % 50 == 0) && (Py % 50 == 0))
                     {
                         //PointF cPt = new PointF(40, 420);//中心点 
                         gph.DrawEllipse(Pens.Black, Px, Py, 2, 2);
                         gph.FillEllipse(new SolidBrush(Color.Black), Px, Py, 2, 2);
                     }
                 }
             }
             */
   
           
             if (checkBox_axis.Checked == true) //画坐标轴
             {
                 PointF cPt;
                 PointF[] xPt = new PointF[3]{
                 new   PointF(map_origin_x,0),
                 new   PointF(map_origin_x-8,15),
                 new   PointF(map_origin_x+8,15)};//X轴三角形    
                 PointF[] yPt = new PointF[3]{
                 new   PointF(map_size_x,map_size_y - map_origin_y),
                 new   PointF(map_size_x-15,map_size_y - (map_origin_y-8)),
                 new   PointF(map_size_x-15,map_size_y - (map_origin_y+8))};//Y轴三角形    
                 gph.DrawPolygon(Pens.Black, xPt);//X轴三角形 
                 gph.FillPolygon(new SolidBrush(Color.Black), xPt);
                 gph.DrawPolygon(Pens.Black, yPt);//Y轴三角形   
                 gph.FillPolygon(new SolidBrush(Color.Black), yPt);

                 gph.DrawLine(Pens.Black, map_origin_x, map_size_y - (map_origin_y - 20), map_origin_x, 0);  //画y轴
                 gph.DrawLine(Pens.Black, map_origin_x - 20, map_size_y - map_origin_y, map_size_x, map_size_y - map_origin_y);  //画X轴
                 {
                     cPt = new PointF(map_origin_x - 20, map_size_y - (map_origin_y - 5));
                     gph.DrawString("0m", new Font("宋体", 10), Brushes.Black, cPt);
                 }
                 {  //画X轴刻点
                     Int16 zb_num;
                     Int16 i;
                     zb_num = (Int16)((map_size_x - map_origin_x) / 100);
                     if (((map_size_x - map_origin_x) % 100) == 0) zb_num--;
                     for (i = 0; i < zb_num; i++)
                     {
                         float mm;
                         cPt = new PointF(map_origin_x + (i + 1) * 100 - 10, map_size_y - (map_origin_y - 10));//中心点 
                         gph.DrawLine(Pens.Black, map_origin_x + (i + 1) * 100, map_size_y - map_origin_y, map_origin_x + (i + 1) * 100, map_size_y - (map_origin_y + 10));  //画x轴
                         mm = ((i + 1) * (map_multiple));
                         gph.DrawString(mm.ToString("f2") + "m", new Font("宋体", 10), Brushes.Black, cPt);
                     }
                 }
                 {  //画X轴刻点
                     Int16 zb_num;
                     Int16 i;
                     zb_num = (Int16)((map_size_y - map_origin_y) / 100);
                     if (((map_size_y - map_origin_y) % 100) == 0) zb_num--;
                     for (i = 0; i < zb_num; i++)
                     {
                         float mm;
                         cPt = new PointF(map_origin_x - 40, map_size_y - (map_origin_y + (i + 1) * 100 + 7));//中心点 
                         gph.DrawLine(Pens.Black, map_origin_x, map_size_y - (map_origin_y + (i + 1) * 100), map_origin_x + 10, map_size_y - (map_origin_y + (i + 1) * 100));  //画y轴
                         mm = ((i + 1) *(map_multiple));
                         gph.DrawString(mm.ToString("f2") + "m", new Font("宋体", 10), Brushes.Black, cPt);
                     }
                 }

             }
             
         
             for (xx = 0; xx <8; xx++)  //画基站圈
             {
                 Int16 sx, sy;

                 if (((CJ_NG_Flag>>xx)&0x01) == 1)
                 {
                     if (dataGridView_BS_SET.Rows[xx].Cells[1].Value == null) sx = 0;                   
                     else sx = (Int16)(Convert.ToInt16(dataGridView_BS_SET.Rows[xx].Cells[1].Value.ToString()));

                     if (dataGridView_BS_SET.Rows[xx].Cells[2].Value == null) sy = 0;
                     else sy = (Int16)(Convert.ToInt16(dataGridView_BS_SET.Rows[xx].Cells[2].Value.ToString()));
                 
             
                     //画基站点
                     /*
                      gph.DrawEllipse(Pens.Blue, (sx / map_multiple - 4 + map_origin_x), map_size_y - (sy / map_multiple + 4 + map_origin_y), 8, 8);
                      gph.FillEllipse(new SolidBrush(Color.Black), (sx / map_multiple - 4 + map_origin_x), map_size_y - (sy / map_multiple + 4 + map_origin_y), 8, 8);
                      */
                     
                     Drawing_draw_bs((Int16)(sx / map_multiple + map_origin_x), (Int16)(map_size_y - (sy / map_multiple + map_origin_y)));
                     if (checkBox_name.Checked == true)
                     {
                         gph.DrawString( (char)(0x41+xx)+"基站", new Font("宋体", 10), Brushes.Black, new PointF((sx / map_multiple + map_origin_x - 12), map_size_y - (sy / map_multiple - 18 + map_origin_y)));
                     }
                     if (checkBox_draw_round.Checked == true)
                     {
                         Int16 dis_buff;
                         if (dataGridView_TAG.Rows[num].Cells[xx + 4].Value == null)
                         {
                             dis_buff = 0;
                         }
                         else
                         {
                             dis_buff = (Int16)(Convert.ToInt32(dataGridView_TAG.Rows[num].Cells[xx + 4].Value.ToString()));
                         }
                         gph.DrawEllipse(Pens.Red, (((sx - dis_buff) / map_multiple) + map_origin_x), map_size_y - (((sy + dis_buff) / map_multiple) + map_origin_y), dis_buff * 2 / map_multiple, dis_buff * 2 / map_multiple);
                     }
                 }
             }


             BQ_num=(Int16)(this.dataGridView_TAG.Rows.Count);

             //画标签
             for (ff = 0; ff < BQ_num;ff++ )
             {
                 Int16 i;

                 x = (Int16)(Convert.ToInt16(dataGridView_TAG.Rows[ff].Cells[1].Value.ToString()));
                 y = (Int16)(Convert.ToInt16(dataGridView_TAG.Rows[ff].Cells[2].Value.ToString()));
                 z = (Int16)(Convert.ToInt16(dataGridView_TAG.Rows[ff].Cells[3].Value.ToString()));
                

                 gph.DrawEllipse(Pens.Blue, (x / map_multiple - 4 + map_origin_x), map_size_y - (y / map_multiple + 4 + map_origin_y), 8, 8);
                 gph.FillEllipse(new SolidBrush(Color.Black), (x / map_multiple - 4 + map_origin_x), map_size_y - (y / map_multiple + 4 + map_origin_y), 8, 8);

                 
                 if (checkBox_coordinate.Checked == true) //画标签坐标
                 {
                     Int16 cc_x = (Int16)(x), cc_y = (Int16)(y);
                     PointF cPt = new PointF((x / map_multiple + 10 + map_origin_x), map_size_y - (y / map_multiple + 5 + map_origin_y));//中心点 
                     gph.DrawString("(" + cc_x.ToString() + "cm," + cc_y.ToString() + "cm)", new Font("宋体", 10), Brushes.Black, cPt);

                     if (dataGridView_TAG.Rows[ff].Cells[0].Value == null)
                     {
                         gph.DrawString((Int16)(Convert.ToInt16(0)) + "标签", new Font("宋体", 10), Brushes.Black, new PointF((x / map_multiple + map_origin_x), map_size_y - (y / map_multiple - 10 + map_origin_y)));
                     }
                     else
                     {
                         gph.DrawString((Int16)(Convert.ToInt16(dataGridView_TAG.Rows[ff].Cells[0].Value.ToString())) + "标签", new Font("宋体", 10), Brushes.Black, new PointF((x / map_multiple + map_origin_x), map_size_y - (y / map_multiple - 10 + map_origin_y)));
                     }
                 }

             }
             //显示在pictureBox1控件中
             this.pictureBox1.Image = bMap;
             //保存输出图片    
             //bMap.Save("c://test.bmp");
         }
        

        private void Form1_Load(object sender, EventArgs e)
         {

             Drawing_init();//画图初始化
             Int16 i, j;
             for (i = 0; i < BX_size_x; i++)
             {
                 this.dataGridView_GJ1.Rows.Add(); //添加行
                 for (j = 0; j < 11; j++)
                 {
                     this.dataGridView_GJ1.Rows[i].Cells[j].Value = 0;
                 }
             }
             for (i = 0; i < BX_size_x; i++)
             {
                 this.dataGridView_GJ2.Rows.Add(); //添加行
                 for (j = 0; j < 11; j++)
                 {
                     this.dataGridView_GJ2.Rows[i].Cells[j].Value = 0;
                 }
             }


             this.dataGridView_TAG.Rows.Add(); //添加行
             for (j = 0; j < 11; j++)
             {
                 this.dataGridView_TAG.Rows[0].Cells[j].Value = 0;
             }

             for (i = 0; i <8; i++)
             {
                 this.dataGridView_BS_SET.Rows.Add();  //添加行
             }

             this.dataGridView_BS_SET.Rows[0].Cells[0].Value = "    A基站";
             this.dataGridView_BS_SET.Rows[1].Cells[0].Value = "    B基站";
             this.dataGridView_BS_SET.Rows[2].Cells[0].Value = "    C基站";
             this.dataGridView_BS_SET.Rows[3].Cells[0].Value = "    D基站";
             this.dataGridView_BS_SET.Rows[4].Cells[0].Value = "    E基站";
             this.dataGridView_BS_SET.Rows[5].Cells[0].Value = "    F基站";
             this.dataGridView_BS_SET.Rows[6].Cells[0].Value = "    G基站";
             this.dataGridView_BS_SET.Rows[7].Cells[0].Value = "    H基站";
             //设置为只读
             dataGridView_BS_SET.Columns[0].ReadOnly = true;
             map_multiple = (Int16)numericUpDown_map_multiple.Value;

             {  //搜索串口
                 string[] str = SerialPort.GetPortNames();
                 toolStripComboBox_com.Items.Clear();
                 toolStripComboBox_com.Items.AddRange(str);
                 if (str.Length != 0) connect_Flag = 1; //调到搜索到串口
                 toolStripComboBox_com.SelectedIndex = 0;
                 toolStripComboBox_Rate.SelectedIndex = 7;
             }



             /*
             String[,] values = new String[7,4]   {{ "主基站A","0","0","0"},
                                                     { "主基站A","0","0","0" },
                                                     { "主基站A","0","0","0" },
                                                     { "主基站A","0","0","0" },
                                                     { "主基站A","0","0","0" },
                                                     { "主基站A","0","0","0" },
                                                     { "主基站A","0","0","0" }};
            
             */
             //dataGridView1.Rows.cel

         }



 
   
         private void SerDataReceive(object sender, SerialDataReceivedEventArgs e)
        {
            int Text_Len = 0; //串口数据长度
            if (serialPort1.IsOpen == false)
            {
                return;
            }

            if (Click_Flag == 0)
            {
                Thread.Sleep((Int32)(Flag_BaudRate_Delay[Flag_BaudRate])*30);
            }
            else
            {
                Thread.Sleep((Int32)(Flag_BaudRate_Delay[Flag_BaudRate])*4);
            }
            byte[] byteReceive;
            try
            {
                byteReceive = new byte[serialPort1.BytesToRead];
            }
            catch (InvalidOperationException ex)
            {
                return;
            }


            serialPort1.Read(byteReceive, 0, byteReceive.Length);

            serialPort1.DiscardInBuffer();
            if (byteReceive.Length < 5)
            {
                return;
            }
            Text_Len = byteReceive.Length;
            {
                Int32 crc;
                crc = CRC16(byteReceive, Text_Len - 2);
                CRCH = crc / 256;
                CRCL = crc % 256;
                CRCBH = byteReceive[Text_Len - 2];
                CRCBL = byteReceive[Text_Len - 1];

                if (CRCBH != CRCH) return;
                if (CRCBL != CRCL) return;
            }
            GETDATA_NEW = 1;
            rBuff = new byte[Text_Len];
            Array.Copy(byteReceive, rBuff, Text_Len);

        }





         private void timer_MODBUS_connet_Tick(object sender, EventArgs e)
         {
             byte[] send_buf = new byte[300];
             Int32 send_length = 0;
             timer_MODBUS_connet.Enabled = false;
             //01 03 00 20 00 17
             // send_buf[0] = (byte)Convert.ToInt16(comboBox_Rate.SelectedItem.ToString());
             if (Click_Flag == 0) //读取配置
             {
                 send_buf[0] = NOW_ID;
                 send_buf[1] = 0x03;
                 send_buf[2] = 0x00;
                 send_buf[3] = 0x00;
                 send_buf[4] = 0x00;
                 send_buf[5] = 0x6A;
                 send_buf[6] = 0x00;
                 send_buf[7] = 0x00;
                 send_length = 8;
                 senddata(send_buf, send_length);
             }
             if (Click_Flag == 1)//写入配置
             {
                 int i;
                 int buff_t;
                 send_buf[0] = NOW_ID;
                 send_buf[1] = 0x10;
                 send_buf[2] = 0x00;
                 send_buf[3] = 0x00;
                 send_buf[4] = 0x00;
                 send_buf[5] = 0x6A;
                 send_buf[6] = 0xD4;

                 send_buf[7] = 0x00;
                 send_buf[8] = (byte)(comboBox_MODBUS_RATE.SelectedIndex);

                 send_buf[9] = 0x00;
                 send_buf[10] = (byte)(numericUpDown_ID.Value);

                 send_buf[11] = (byte)(comboBox_RANGING.SelectedIndex);
                 send_buf[12] = (byte)(comboBox_DW_MODE.SelectedIndex);

                 send_buf[13] = 0x00;
                 send_buf[14] = (byte)(comboBox_TAG_or_BS.SelectedIndex);


                 if (comboBox_TAG_or_BS.SelectedIndex == 0) { send_buf[15] = 0x00; send_buf[16] = (byte)(numericUpDown_TAG_or_BS_ID.Value); }
                 if (comboBox_TAG_or_BS.SelectedIndex == 1) { send_buf[15] = (byte)(numericUpDown_TAG_or_BS_ID.Value); send_buf[16] = 0x00; }
                 if (comboBox_TAG_or_BS.SelectedIndex == 2) { send_buf[15] = 0x00; send_buf[16] = 0x00; }



                 send_buf[17] = (byte)(comboBox_AIR_CHAN.SelectedIndex);//空中信道
                 send_buf[18] = (byte)(comboBox_AIR_RAT.SelectedIndex);//空中速率

                 send_buf[19] = (byte)(numericUpDown_KAM_Q.Value / 256);
                 send_buf[20] = (byte)(numericUpDown_KAM_Q.Value % 256);

                 send_buf[21] = (byte)(numericUpDown_KAM_R.Value / 256);
                 send_buf[22] = (byte)(numericUpDown_KAM_R.Value % 256);

                 send_buf[23] = (byte)(numericUpDown_RX_DELAY.Value / 256);
                 send_buf[24] = (byte)(numericUpDown_RX_DELAY.Value % 256);

                 if (dataGridView_BS_SET.Rows[0].Cells[1].Value == null) buff_t = 0;
                 else buff_t = Convert.ToInt16(dataGridView_BS_SET.Rows[0].Cells[1].Value.ToString());
                 send_buf[25] = (byte)((buff_t >> 8) & 0xFF);
                 send_buf[26] = (byte)((buff_t & 0xFF));


                 if (dataGridView_BS_SET.Rows[0].Cells[2].Value == null) buff_t = 0;
                 else buff_t = Convert.ToInt16(dataGridView_BS_SET.Rows[0].Cells[2].Value.ToString());
                 send_buf[27] = (byte)((buff_t >> 8) & 0xFF);
                 send_buf[28] = (byte)(buff_t & 0xFF);

                 if (dataGridView_BS_SET.Rows[0].Cells[3].Value == null) buff_t = 0;
                 else buff_t = Convert.ToInt16(dataGridView_BS_SET.Rows[0].Cells[3].Value.ToString());
                 send_buf[29] = (byte)((buff_t >> 8) & 0xFF);
                 send_buf[30] = (byte)(buff_t & 0xFF);


                 if (checkBox_B.Checked == true) { send_buf[31] = 0x00; send_buf[32] = 0x01; }
                 else { send_buf[31] = 0x00; send_buf[32] = 0x00; }

                 if (checkBox_C.Checked == true) { send_buf[39] = 0x00; send_buf[40] = 0x01; }
                 else { send_buf[39] = 0x00; send_buf[40] = 0x00; }

                 if (checkBox_D.Checked == true) { send_buf[47] = 0x00; send_buf[48] = 0x01; }
                 else { send_buf[47] = 0x00; send_buf[48] = 0x00; }

                 if (checkBox_E.Checked == true) { send_buf[55] = 0x00; send_buf[56] = 0x01; }
                 else { send_buf[55] = 0x00; send_buf[56] = 0x00; }

                 if (checkBox_F.Checked == true) { send_buf[63] = 0x00; send_buf[64] = 0x01; }
                 else { send_buf[63] = 0x00; send_buf[64] = 0x00; }

                 if (checkBox_G.Checked == true) { send_buf[71] = 0x00; send_buf[72] = 0x01; }
                 else { send_buf[71] = 0x00; send_buf[72] = 0x00; }

                 if (checkBox_H.Checked == true) { send_buf[79] = 0x00; send_buf[80] = 0x01; }
                 else { send_buf[79] = 0x00; send_buf[80] = 0x00; }

                 for (i = 0; i < 7; i++)
                 {

                     if (dataGridView_BS_SET.Rows[i+1].Cells[1].Value == null) buff_t = 0;
                     else buff_t = Convert.ToInt16(dataGridView_BS_SET.Rows[i + 1].Cells[1].Value.ToString());
                     send_buf[30 + 8 * i + 3] = (byte)((buff_t >> 8) & 0xFF);
                     send_buf[30 + 8 * i + 4] = (byte)(buff_t & 0xFF);

                     if (dataGridView_BS_SET.Rows[i+1].Cells[2].Value == null) buff_t = 0;
                     else buff_t = Convert.ToInt16(dataGridView_BS_SET.Rows[i + 1].Cells[2].Value.ToString());
                     send_buf[30 + 8 * i + 5] = (byte)((buff_t >> 8) & 0xFF);
                     send_buf[30 + 8 * i + 6] = (byte)(buff_t & 0xFF);

                     if (dataGridView_BS_SET.Rows[i+1].Cells[3].Value == null) buff_t = 0;
                     else buff_t = Convert.ToInt16(dataGridView_BS_SET.Rows[i + 1].Cells[3].Value.ToString());
                     send_buf[30 + 8 * i + 7] = (byte)((buff_t >> 8) & 0xFF);
                     send_buf[30 + 8 * i + 8] = (byte)(buff_t & 0xFF);

                 }

                 send_buf[90] = (byte)numericUpDown_TAG_num.Value;

                 for (i = 0; i < (byte)numericUpDown_TAG_num.Value; i++)
                 {
                     if (dataGridView_TAG.Rows[i].Cells[0].Value == null)
                     {
                         if ((i % 2) == 1)//如果是奇数
                         {
                             send_buf[116 + i] = 0;
                         }
                         else  //偶数
                         {
                             send_buf[116 + i + 2] = 0;
                         }
                     }
                     else  
                     {
                         if ((i % 2) == 1)//如果是奇数
                         {
                             send_buf[116 + i] = (byte)Convert.ToInt16(dataGridView_TAG.Rows[i].Cells[0].Value.ToString());
                         }
                         else  //偶数
                         {
                             send_buf[116 + i+ 2] = (byte)Convert.ToInt16(dataGridView_TAG.Rows[i].Cells[0].Value.ToString());
                         }
                        
                     }

                     
                 }


                 send_length =221;
                 senddata(send_buf, send_length);
             }

             if (Click_Flag == 3) //读取版本
             {
                 /*
                 send_buf[0] = NOW_ID;
                 send_buf[1] = 0x03;
                 send_buf[2] = 0x00;
                 send_buf[3] = 0x31;
                 send_buf[4] = 0x00;
                 send_buf[5] = 0x01;
                 send_buf[6] = 0x00;
                 send_buf[7] = 0x00;
                 send_length = 8;
                 senddata(send_buf, send_length);
                  */
             }
             if (Click_Flag == 4) //停止测距
             {
                 int buff_t;
                 send_buf[0] = NOW_ID;
                 send_buf[1] = 0x10;
                 send_buf[2] = 0x00;
                 send_buf[3] = 0x20;
                 send_buf[4] = 0x00;
                 send_buf[5] = 0x02;
                 send_buf[6] = 0x04;

                 send_buf[7] = 0x00;
                 send_buf[8] = 0x00;

                 buff_t = Convert.ToInt16(dataGridView_TAG.Rows[0].Cells[0].Value.ToString());
                 send_buf[9] = 0x00;
                 send_buf[10] = (byte)buff_t;

                 send_length = 13;
                 senddata(send_buf, send_length);
             }
             timer_MODBUS_connet.Enabled = true;
         }

         private void timer_SCAN_ID_Tick(object sender, EventArgs e)
         {
           
             if (time_SCAN_NUM < 256)
             {

     
                 toolStripProgressBar__SCAN.Value = time_SCAN_NUM;
                 byte[] send_buf = new byte[8];
                 send_buf[0] = (byte)time_SCAN_NUM;
                 send_buf[1] = 0x03;
                 send_buf[2] = 0x00;
                 send_buf[3] = 0x00;
                 send_buf[4] = 0x00;
                 send_buf[5] = 0x00;
                 senddata(send_buf, 8);

                 time_SCAN_NUM++;
             }
             else
             {

                 time_SCAN_NUM = 0;
                 timer_SCAN_ID.Enabled = false;
                 isSCAN = 0;
                 toolStripComboBox_ID.Items.Clear();
                 if (ID_buf.Length > 0 && ID_buf != null)
                 {
                     // Array.Resize<string>(ref ID_buf, ID_buf.Length + 1);
                     toolStripComboBox_ID.Items.AddRange(ID_buf);
                     toolStripComboBox_ID.SelectedIndex = 0;
                     connect_Flag = 3;
                 }

             }
         }

         private void timer_connect_Flag_Tick(object sender, EventArgs e)
         {             
             if (GETDATA_NEW == 1)//接收到新数据
             {
                 timer_connect_Flag.Enabled = false; //关闭自己时钟使能
                 if (isSCAN == 1)//正在扫描状态
                 {
                     if (rBuff[1] == 0x03)
                     {
                         SCAN_ID_Click(rBuff[0]);
                     }

                 }
                 else
                 {

                     printf_data(rBuff, rBuff.Length, 1);

                     if (Click_Flag == 0)
                     {
                         if (rBuff[0] == NOW_ID)
                         {
                             if (rBuff[1] == 0x03 && rBuff.Length == 217)
                             {
                                        int i;
                                        this.comboBox_MODBUS_RATE.SelectedIndex = rBuff[4];   //设备波特率
                                        this.numericUpDown_ID.Value = rBuff[6];               //MODBUS_ID
                                        this.comboBox_RANGING.SelectedIndex = rBuff[7];      //测距方式
                                        this.comboBox_DW_MODE.SelectedIndex = rBuff[8];      //定位方式
                                        this.comboBox_TAG_or_BS.SelectedIndex = rBuff[10];   //设备模式
                                        if (rBuff[10] == 0) this.numericUpDown_TAG_or_BS_ID.Value = rBuff[12];     //标签ID
                                        if (rBuff[10] == 1) this.numericUpDown_TAG_or_BS_ID.Value = rBuff[11];    //次基站ID
                                        if (rBuff[10] == 2) this.numericUpDown_TAG_or_BS_ID.Value = 0;            //主基站ID

                                        this.comboBox_AIR_CHAN.SelectedIndex = rBuff[13];  //空中信道
                                        this.comboBox_AIR_RAT.SelectedIndex = rBuff[14];   //空中波特率

                                        this.numericUpDown_KAM_Q.Value = ((rBuff[15] << 8)&0xFF00) | rBuff[16]; //卡尔曼滤波—Q
                                        this.numericUpDown_KAM_R.Value = ((rBuff[17] << 8)&0xFF00) | rBuff[18]; //卡尔曼滤波—R
                                        this.numericUpDown_RX_DELAY.Value = ((rBuff[19] << 8)&0xFF00) | rBuff[20]; //接收天线延时

                                        this.dataGridView_BS_SET.Rows[0].Cells[1].Value = Convert.ToInt16((Int16)(((rBuff[21] << 8) & 0xFF00) | rBuff[22]));
                                        this.dataGridView_BS_SET.Rows[0].Cells[2].Value = Convert.ToInt16((Int16)(((rBuff[23] << 8) & 0xFF00) | rBuff[24]));
                                        this.dataGridView_BS_SET.Rows[0].Cells[3].Value = Convert.ToInt16((Int16)(((rBuff[25] << 8) & 0xFF00) | rBuff[26]));

                                        if (rBuff[28] == 0x01) checkBox_B.Checked = true;
                                        else checkBox_B.Checked = false;
                                         if (rBuff[36] == 0x01) checkBox_C.Checked = true;
                                        else checkBox_C.Checked = false;
                                        if (rBuff[44] == 0x01) checkBox_D.Checked = true;
                                        else checkBox_D.Checked = false;
                                        if (rBuff[52] == 0x01) checkBox_E.Checked = true;
                                        else checkBox_E.Checked = false;
                                        if (rBuff[60] == 0x01) checkBox_F.Checked = true;
                                        else checkBox_F.Checked = false;
                                        if (rBuff[68] == 0x01) checkBox_G.Checked = true;
                                        else checkBox_G.Checked = false;
                                        if (rBuff[76] == 0x01) checkBox_H.Checked = true;
                                        else checkBox_H.Checked = false;
                                        for (i = 0; i < 7; i++)
                                        {
                                            this.dataGridView_BS_SET.Rows[i + 1].Cells[1].Value = Convert.ToInt16((Int16)(((rBuff[26 + 8 * i + 3] << 8) & 0xFF00) | rBuff[26 + 8 * i + 4]));
                                            this.dataGridView_BS_SET.Rows[i + 1].Cells[2].Value = Convert.ToInt16((Int16)(((rBuff[26 + 8 * i + 5] << 8) & 0xFF00) | rBuff[26 + 8 * i + 6]));
                                            this.dataGridView_BS_SET.Rows[i + 1].Cells[3].Value = Convert.ToInt16((Int16)(((rBuff[26 + 8 * i + 7] << 8) & 0xFF00) | rBuff[26 + 8 * i + 8]));
                                        }

                                        numericUpDown_TAG_num.Value = rBuff[86];


                                        for (i = 0; i < rBuff[86]; i++)
                                        {
                                            if ((i % 2) == 1)//如果为奇数
                                            {
                                                dataGridView_TAG.Rows[i].Cells[0].Value = rBuff[113 + i - 1];
                                            }
                                            else
                                            {
                                                dataGridView_TAG.Rows[i].Cells[0].Value = rBuff[113 + i + 1];
                                            }
                                        }
                                        if (rBuff[214] < 20 || rBuff[214] >29)//若为版本以外的
                                        {
                                        state_Flag=10;
                                        Click_Flag = 100;
                                        MessageBox.Show("不支持此设备固件版本，请使用V2.0~2.9固件版本的设备，请关闭软件谢谢！", "提示");
                                        return;
                                        }
                                 
                                        this.toolStripStatusLabel__firmware_version.Text = "设备固件版本：V" + (rBuff[214] / 10).ToString() + "." + (rBuff[214] % 10).ToString();


                                        state_Flag = 3;
                                        Click_Flag = 3;
                                        MessageBox.Show("读取配置成功", "提示");  
                             }
                         }
                     }
                     if (Click_Flag == 1)
                     {
                         if (rBuff[0] == NOW_ID)
                         {
                             if (rBuff[1] == 0x10 && rBuff.Length == 8)
                             {
                                 Click_Flag = 100;
                                 MessageBox.Show("写入配置成功", "提示");  

                             }
                         }
                     }
                     if (Click_Flag == 2)
                     {
                         if (rBuff[0] == NOW_ID)
                         {
                             if (rBuff[1] == 0x10  && rBuff.Length == 8)  //收到协议应答
                             {
                                 CJ_EN = 0;
                             }
                             if (rBuff[1] == 0x03 && rBuff[2] == 0x18 && rBuff.Length == 31)//收到定位数据返回
                             {
                                 int i;
                                 CJ_EN = 0;     
                                 for (i = 0; i < dataGridView_TAG.RowCount; i++)
                                 {
                                     Int16 TAG_ID;

                                     if (dataGridView_TAG.Rows[i].Cells[0].Value == null)//取标签测量表格的ID
                                     {
                                         TAG_ID = 0;
                                     }
                                     else
                                     {
                                         TAG_ID = Convert.ToInt16(dataGridView_TAG.Rows[i].Cells[0].Value.ToString());
                                     }

                                     if (rBuff[4] == TAG_ID)
                                     {
                                         int f;
                                         Color OK_color = Color.FromArgb(144, 238, 144);
                                         Color NG_color = Color.FromArgb(255, 48, 48);
                                         this.dataGridView_TAG.Rows[i].Cells[0].Value = ((rBuff[3] << 8) & 0xFF00) | rBuff[4];
                                         CJ_NG_Flag = rBuff[6];//赋予值到标志位，方便地图显示的基站画圈调用

                                         if (rBuff[6] == 0x00)//全部基站都没有找到标签   
                                         {
                                             dataGridView_TAG.Rows[i].Cells[0].Style.BackColor = NG_color;
                                         }
                                         else
                                         {
                                             dataGridView_TAG.Rows[i].Cells[0].Style.BackColor = OK_color;
                                         }

                                         if (((rBuff[5] >> 0) & 0x01) == 0x00)//没有计算成功
                                         {
                                             dataGridView_TAG.Rows[i].Cells[1].Style.BackColor = NG_color;
                                             dataGridView_TAG.Rows[i].Cells[2].Style.BackColor = NG_color;
                                             dataGridView_TAG.Rows[i].Cells[3].Style.BackColor = NG_color;
                                         }
                                         else
                                         {
                                             dataGridView_TAG.Rows[i].Cells[1].Style.BackColor = OK_color;
                                             dataGridView_TAG.Rows[i].Cells[2].Style.BackColor = OK_color;
                                             dataGridView_TAG.Rows[i].Cells[3].Style.BackColor = OK_color;
                                         }
                                         for (f = 0; f < 8; f++) //输出测距错误的基站
                                         {

                                              if (((rBuff[6] >> f) & 0x01) == 0x01)
                                              {
                                                  dataGridView_TAG.Rows[i].Cells[4 + f].Style.BackColor = OK_color;
                                              }
                                              else
                                              {
                                                  dataGridView_TAG.Rows[i].Cells[4 + f].Style.BackColor = NG_color;
                                              }
                                         }
                                         {
                                             if (checkBox_JS.Checked == true) //软件解算标签坐标
                                             {
                                                 this.dataGridView_TAG.Rows[i].Cells[1].Value = Convert.ToInt16((Int16)(((rBuff[7] << 8) & 0xFF00) | rBuff[8]));
                                                 this.dataGridView_TAG.Rows[i].Cells[2].Value = Convert.ToInt16((Int16)(((rBuff[9] << 8) & 0xFF00) | rBuff[10]));
                                                 this.dataGridView_TAG.Rows[i].Cells[3].Value = Convert.ToInt16((Int16)(((rBuff[11] << 8) & 0xFF00) | rBuff[12]));
                                             }
                                             this.dataGridView_TAG.Rows[i].Cells[4].Value = Convert.ToInt16((Int16)(((rBuff[13] << 8) & 0xFF00) | rBuff[14]));
                                             this.dataGridView_TAG.Rows[i].Cells[5].Value = Convert.ToInt16((Int16)(((rBuff[15] << 8) & 0xFF00) | rBuff[16]));
                                             this.dataGridView_TAG.Rows[i].Cells[6].Value = Convert.ToInt16((Int16)(((rBuff[17] << 8) & 0xFF00) | rBuff[18]));
                                             this.dataGridView_TAG.Rows[i].Cells[7].Value = Convert.ToInt16((Int16)(((rBuff[19] << 8) & 0xFF00) | rBuff[20]));
                                             this.dataGridView_TAG.Rows[i].Cells[8].Value = Convert.ToInt16((Int16)(((rBuff[21] << 8) & 0xFF00) | rBuff[22]));
                                             this.dataGridView_TAG.Rows[i].Cells[9].Value = Convert.ToInt16((Int16)(((rBuff[23] << 8) & 0xFF00) | rBuff[24]));
                                             this.dataGridView_TAG.Rows[i].Cells[10].Value = Convert.ToInt16((Int16)(((rBuff[25] << 8) & 0xFF00) | rBuff[26]));
                                             this.dataGridView_TAG.Rows[i].Cells[11].Value = Convert.ToInt16((Int16)(((rBuff[27] << 8) & 0xFF00) | rBuff[28]));
                                         }

                                         {  //把对应标签ID的数据写入缓存列表
                                             Int16 LB_TAG_ID;
                                             if (numericUpDown_pass1_ID.Value == null)//取pass1标签测量表格的ID
                                             {
                                                 LB_TAG_ID = 0;
                                             }
                                             else
                                             {
                                                 LB_TAG_ID = Convert.ToInt16(numericUpDown_pass1_ID.Value);
                                             }
                                             if (LB_TAG_ID == rBuff[4])
                                             {
                                                 this.dataGridView_GJ1.Rows.Add();
                                                 this.dataGridView_GJ1.Rows[BX_size_x].Cells[0].Value = Convert.ToInt16((Int16)(((rBuff[7] << 8) & 0xFF00) | rBuff[8]));
                                                 this.dataGridView_GJ1.Rows[BX_size_x].Cells[1].Value = Convert.ToInt16((Int16)(((rBuff[9] << 8) & 0xFF00) | rBuff[10]));
                                                 this.dataGridView_GJ1.Rows[BX_size_x].Cells[2].Value = Convert.ToInt16((Int16)(((rBuff[11] << 8) & 0xFF00) | rBuff[12]));

                                                 this.dataGridView_GJ1.Rows[BX_size_x].Cells[3].Value = Convert.ToInt16((Int16)(((rBuff[13] << 8) & 0xFF00) | rBuff[14]));
                                                 this.dataGridView_GJ1.Rows[BX_size_x].Cells[4].Value = Convert.ToInt16((Int16)(((rBuff[15] << 8) & 0xFF00) | rBuff[16]));
                                                 this.dataGridView_GJ1.Rows[BX_size_x].Cells[5].Value = Convert.ToInt16((Int16)(((rBuff[17] << 8) & 0xFF00) | rBuff[18]));
                                                 this.dataGridView_GJ1.Rows[BX_size_x].Cells[6].Value = Convert.ToInt16((Int16)(((rBuff[19] << 8) & 0xFF00) | rBuff[20]));
                                                 this.dataGridView_GJ1.Rows[BX_size_x].Cells[7].Value = Convert.ToInt16((Int16)(((rBuff[21] << 8) & 0xFF00) | rBuff[22]));
                                                 this.dataGridView_GJ1.Rows[BX_size_x].Cells[8].Value = Convert.ToInt16((Int16)(((rBuff[23] << 8) & 0xFF00) | rBuff[24]));
                                                 this.dataGridView_GJ1.Rows[BX_size_x].Cells[9].Value = Convert.ToInt16((Int16)(((rBuff[25] << 8) & 0xFF00) | rBuff[26]));
                                                 this.dataGridView_GJ1.Rows[BX_size_x].Cells[10].Value = Convert.ToInt16((Int16)(((rBuff[27] << 8) & 0xFF00) | rBuff[28]));
                                                 this.dataGridView_GJ1.Rows.RemoveAt(0);
                                             }

                                             if (numericUpDown_pass2_ID.Value == null)//取pass2标签测量表格的ID
                                             {
                                                 LB_TAG_ID = 0;
                                             }
                                             else
                                             {
                                                 LB_TAG_ID = Convert.ToInt16(numericUpDown_pass2_ID.Value);
                                             }
                                             if (LB_TAG_ID == rBuff[4])
                                             {
                                                 this.dataGridView_GJ2.Rows.Add();
                                                 this.dataGridView_GJ2.Rows[BX_size_x].Cells[0].Value = Convert.ToInt16((Int16)(((rBuff[17] << 8) & 0xFF00) | rBuff[8]));
                                                 this.dataGridView_GJ2.Rows[BX_size_x].Cells[1].Value = Convert.ToInt16((Int16)(((rBuff[9] << 8) & 0xFF00) | rBuff[10]));
                                                 this.dataGridView_GJ2.Rows[BX_size_x].Cells[2].Value = Convert.ToInt16((Int16)(((rBuff[11] << 8) & 0xFF00) | rBuff[12]));

                                                 this.dataGridView_GJ2.Rows[BX_size_x].Cells[3].Value = Convert.ToInt16((Int16)(((rBuff[13] << 8) & 0xFF00) | rBuff[14]));
                                                 this.dataGridView_GJ2.Rows[BX_size_x].Cells[4].Value = Convert.ToInt16((Int16)(((rBuff[15] << 8) & 0xFF00) | rBuff[16]));
                                                 this.dataGridView_GJ2.Rows[BX_size_x].Cells[5].Value = Convert.ToInt16((Int16)(((rBuff[17] << 8) & 0xFF00) | rBuff[18]));
                                                 this.dataGridView_GJ2.Rows[BX_size_x].Cells[6].Value = Convert.ToInt16((Int16)(((rBuff[19] << 8) & 0xFF00) | rBuff[20]));
                                                 this.dataGridView_GJ2.Rows[BX_size_x].Cells[7].Value = Convert.ToInt16((Int16)(((rBuff[21] << 8) & 0xFF00) | rBuff[22]));
                                                 this.dataGridView_GJ2.Rows[BX_size_x].Cells[8].Value = Convert.ToInt16((Int16)(((rBuff[23] << 8) & 0xFF00) | rBuff[24]));
                                                 this.dataGridView_GJ2.Rows[BX_size_x].Cells[9].Value = Convert.ToInt16((Int16)(((rBuff[25] << 8) & 0xFF00) | rBuff[26]));
                                                 this.dataGridView_GJ2.Rows[BX_size_x].Cells[10].Value = Convert.ToInt16((Int16)(((rBuff[27] << 8) & 0xFF00) | rBuff[28]));
                                                 this.dataGridView_GJ2.Rows.RemoveAt(0);
                                             }
                                         }

                                     }

                                 }                             
                             }
                         }
                     }
                     if (Click_Flag == 3)
                     {
                         
                         if (rBuff[0] == NOW_ID)
                         {
                             if (rBuff[1] == 0x03 && rBuff.Length == 7)
                             {
                    

                             }
                         }
                     }
                     if (Click_Flag == 4)
                     {
                         if (rBuff[0] == NOW_ID)
                         {
                             if (rBuff[1] == 0x10 && rBuff.Length == 8)
                             {
                                 Click_Flag = 100;
                                 MessageBox.Show("已停止测距", "提示");

                             }
                         }
                     }




                  }
                        
             GETDATA_NEW = 0;
             timer_connect_Flag.Enabled = true; //打开自己时钟使能
             }
         }




     

     


         private void button_redata_Click(object sender, EventArgs e)
         {
             Click_Flag = 0;
         }

         private void button_wrdaata_Click(object sender, EventArgs e)
         {
             Click_Flag = 1;
         }

         private void button_CJ_STOP_Click(object sender, EventArgs e)
         {
             state_Flag = 3;
             Click_Flag = 3;             
             timer_display.Enabled =true;
             timer_DW.Enabled = true;
         }

         private void numericUpDown_map_multiple_ValueChanged(object sender, EventArgs e)
         {
             if (numericUpDown_map_multiple.Value <= 0) numericUpDown_map_multiple.Value =(decimal)0.01;

         }

         private void numericUpDown_origin_X_ValueChanged(object sender, EventArgs e)
         {
             map_origin_x = (Int16)(numericUpDown_origin_X.Value);
         }

         private void numericUpDown_origin_Y_ValueChanged(object sender, EventArgs e)
         {
             map_origin_y = (Int16)(numericUpDown_origin_Y.Value);
         }

         private void numericUpDown_TAG_num_ValueChanged(object sender, EventArgs e)
         {
             Int16 num;
             if (numericUpDown_TAG_num.Value == 0) numericUpDown_TAG_num.Value = 1;

             num = (Int16)(this.dataGridView_TAG.Rows.Count);

             if (num < numericUpDown_TAG_num.Value)
             {
                 Int16 i;
                 for(i=0;i<numericUpDown_TAG_num.Value-num;i++)
                 {
                     this.dataGridView_TAG.Rows.Add();
                     this.dataGridView_TAG.Rows[num + i].Cells[0].Value = 0;
                     this.dataGridView_TAG.Rows[num + i].Cells[1].Value = 0;
                     this.dataGridView_TAG.Rows[num + i].Cells[2].Value = 0;
                     this.dataGridView_TAG.Rows[num + i].Cells[3].Value = 0;
                     this.dataGridView_TAG.Rows[num + i].Cells[4].Value = 0;
                     this.dataGridView_TAG.Rows[num + i].Cells[5].Value = 0;
                     this.dataGridView_TAG.Rows[num + i].Cells[6].Value = 0;
                     this.dataGridView_TAG.Rows[num + i].Cells[7].Value = 0;
                     this.dataGridView_TAG.Rows[num + i].Cells[8].Value = 0;
                     this.dataGridView_TAG.Rows[num + i].Cells[9].Value = 0;
                     this.dataGridView_TAG.Rows[num + i].Cells[10].Value = 0;
                     this.dataGridView_TAG.Rows[num + i].Cells[11].Value = 0;
                 }

             }
             if (num > numericUpDown_TAG_num.Value)
             {
                 Int16 i;
                 for (i = 0; i < num - numericUpDown_TAG_num.Value; i++)
                 {
                     this.dataGridView_TAG.Rows.RemoveAt(this.dataGridView_TAG.Rows.Count - 1);
                 }
             }
             if (comboBox_DW_MODE.SelectedIndex == 2)
             {
                 Int16 i;
                 for (i = 0; i < numericUpDown_TAG_num.Value; i++)
                 {
                     this.dataGridView_TAG.Rows[i].Cells[0].Value = i;
                 }
             }

             
         }
      

         private void numericUpDown_ID_ValueChanged(object sender, EventArgs e)
         {
             int ff;
             textBox_MODBUS_ID.Text = "0x";
             ff = (int)numericUpDown_ID.Value % 256 / 16;
             if (ff > 9)
             {
                 switch (ff)
                 {
                     case 10: textBox_MODBUS_ID.Text += "A"; break;
                     case 11: textBox_MODBUS_ID.Text += "B"; break;
                     case 12: textBox_MODBUS_ID.Text += "C"; break;
                     case 13: textBox_MODBUS_ID.Text += "D"; break;
                     case 14: textBox_MODBUS_ID.Text += "E"; break;
                     case 15: textBox_MODBUS_ID.Text += "F"; break;
                     default: break;
                 }
             }
             else textBox_MODBUS_ID.Text += ff.ToString();
             ff = (int)numericUpDown_ID.Value % 16;
             if (ff > 9)
             {
                 switch (ff)
                 {
                     case 10: textBox_MODBUS_ID.Text += "A"; break;
                     case 11: textBox_MODBUS_ID.Text += "B"; break;
                     case 12: textBox_MODBUS_ID.Text += "C"; break;
                     case 13: textBox_MODBUS_ID.Text += "D"; break;
                     case 14: textBox_MODBUS_ID.Text += "E"; break;
                     case 15: textBox_MODBUS_ID.Text += "F"; break;
                     default: break;
                 }
             }
             else textBox_MODBUS_ID.Text += ff.ToString(); 
         }

         private void timer_display_Tick(object sender, EventArgs e)
         {
                 if (comboBox_DW_MODE.SelectedIndex == 1)   //二维定位模式
                 {
                     if (checkBox_JS.Checked == false) //软件解算标签坐标
                     {
                         Int16 CLU_num, ff;
                         CLU_num = (Int16)(this.dataGridView_TAG.Rows.Count);
                         //画标签
                         for (ff = 0; ff < CLU_num; ff++)
                         {
                             PersonPosition(ff);
                         }
                     }
                     Drawing_update(0);
                 }
                 else if (comboBox_DW_MODE.SelectedIndex == 0) //一对一测距模式
                 {
                     Drawing_update_one_to_one(0);
                 }
                 Drawing_BOXING1_update(0);
                 Drawing_BOXING2_update(0);

                 display_3D();//3D传值显示
             
         }

         private void timer_DW_Tick(object sender, EventArgs e)
         {
             byte[] send_buf = new byte[100];
             Int32 send_length = 0;
             if (comboBox_DW_MODE.SelectedIndex <= 1)
             {
                   //单标签模式
                
                     if (Click_Flag == 2) //开始测距
                     {
                         if (CJ_EN == 1)
                         {
                             send_buf[0] = NOW_ID;
                             send_buf[1] = 0x10;
                             send_buf[2] = 0x00;
                             send_buf[3] = 0x28;
                             send_buf[4] = 0x00;
                             send_buf[5] = 0x01;
                             send_buf[6] = 0x02;
                             send_buf[7] = 0x00;
                             send_buf[8] = 0x04; //持续检测自动发送

                             send_length = 11;

                             senddata(send_buf, send_length);

                     
                             
                         }
                     }
                 
             }
         
             if (Click_Flag == 3) //停止测距
             {                 
                     int buff_t;
                     send_buf[0] = NOW_ID;
                     send_buf[1] = 0x10;
                     send_buf[2] = 0x00;
                     send_buf[3] = 0x28;
                     send_buf[4] = 0x00;
                     send_buf[5] = 0x01;
                     send_buf[6] = 0x02;
                     send_buf[7] = 0x00;                
                     send_buf[8] = 0x00; //持续检测自动发送                               
                     send_length = 11;
                     senddata(send_buf, send_length);
                     timer_DW.Enabled = false;
             }
         }

         private void timer_state_Tick(object sender, EventArgs e)
         {

             {  //检测ID状态值变化
                 int ff;

                 ff = (int)numericUpDown_TAG_or_BS_ID.Value % 256;

                 if (comboBox_TAG_or_BS.SelectedIndex == 0) //标签的时候
                 {
                     textBox_TAG_or_BS_ID.Text = ff.ToString();
                     textBox_TAG_or_BS_ID.Text += "标签";
                 }
                 if (comboBox_TAG_or_BS.SelectedIndex == 1) //次基站的时候
                 {
                     switch (ff)
                     {
                         case 0: textBox_TAG_or_BS_ID.Text = "B基站"; break;
                         case 1: textBox_TAG_or_BS_ID.Text = "C基站"; break;
                         case 2: textBox_TAG_or_BS_ID.Text = "D基站"; break;
                         case 3: textBox_TAG_or_BS_ID.Text = "E基站"; break;
                         case 4: textBox_TAG_or_BS_ID.Text = "F基站"; break;
                         case 5: textBox_TAG_or_BS_ID.Text = "G基站"; break;
                         case 6: textBox_TAG_or_BS_ID.Text = "H基站"; break;
                     }
                 }
                 if (comboBox_TAG_or_BS.SelectedIndex == 2) //主基站的时候
                 {
                     textBox_TAG_or_BS_ID.Text = "A基站";
                 }
             }

             //textBox_state.Text = state_Flag.ToString();
             label13.Text = "地图大小：  H" + map_size_x + "    W" + map_size_y;
             if (isGJ == true)
             {
                 button_GJ.Text = "关闭轨迹显示";
             }
             else
             {
                 button_GJ.Text = "打开轨迹显示";
             }
             if (state_Flag == 0)//未打开串口
             {
                 
                 toolStripStatusLabel_state.Text = "软件状态：未打开串口";
                 ToolStripMenuItem1_OPEN_CLOSE.Text = "打开串口";
                 ToolStripMenuItem_SCAN.Enabled = true;  //打开扫描串口按钮

                 ToolStripMenuItem_Connect.Text = "连接设备";
                 toolStripMenuItem_LJ.Enabled = false;//连接设备按钮
                 ToolStripMenuItem_SCAN_ID.Enabled = false;//扫描设备按钮
                 toolStripComboBox_ID.Enabled = false;//设备ID编辑框

                 comboBox_MODBUS_RATE.Enabled = false;//设置波特率
                 numericUpDown_ID.Enabled = false; //设置modbusID
                 comboBox_TAG_or_BS.Enabled = false; //设备模式
                 comboBox_DW_MODE.Enabled = false; //定位模式
                 numericUpDown_TAG_or_BS_ID.Enabled = false;//设备ID
                 button_redata.Enabled = false; //读取配置按钮
                 button_wrdaata.Enabled = false;//载入配置按钮
                 toolStripComboBox_com.Enabled = true;  //串口选择
                 toolStripComboBox_Rate.Enabled = true; //串口波特率选择
                 comboBox_RANGING.Enabled = false;    //测距方式选择
                 comboBox_AIR_CHAN.Enabled = false;   //空中信道选择
                 comboBox_AIR_RAT.Enabled = false;   //空中速率选择
                 numericUpDown_KAM_Q.Enabled = false;  //卡尔曼Q
                 numericUpDown_KAM_R.Enabled = false;  //卡尔曼R
                 numericUpDown_RX_DELAY.Enabled = false;  //接收延时

                 numericUpDown_origin_X.Enabled = false; //地图X坐标
                 numericUpDown_origin_Y.Enabled = false; //地图Y坐标
                 numericUpDown_map_multiple.Enabled = false; //地图比例
                 checkBox_draw_round.Enabled = false; //地图设置
                 checkBox_axis.Enabled = false;       //地图设置
                 checkBox_coordinate.Enabled = false;  //地图设置
                 checkBox_name.Enabled = false;       //地图设置
                 checkBox_JS.Enabled = false;//地图设置

                 numericUpDown_TAG_num.Enabled = false;  //标签数量框
                 button_CJ_OPEN.Enabled = false;         //开始定位按钮
                 button_CJ_STOP.Enabled = false;       //取消定位按钮
          

                 dataGridView_TAG.Enabled = false;    //标签列表
                 dataGridView_BS_SET.Enabled = false; //基站列表
                 checkBox_B.Enabled = false;    //基站使能
                 checkBox_C.Enabled = false;    //基站使能
                 checkBox_D.Enabled = false;    //基站使能
                 checkBox_E.Enabled = false;    //基站使能
                 checkBox_F.Enabled = false;    //基站使能
                 checkBox_G.Enabled = false;    //基站使能
                 checkBox_H.Enabled = false;    //基站使能
                 timer_DW.Enabled = false;  //定位关
             }
             if (state_Flag == 1)//打开串口
             {
                 toolStripStatusLabel_state.Text = "软件状态：未连接设备";
                 ToolStripMenuItem1_OPEN_CLOSE.Text = "关闭串口";
                 ToolStripMenuItem_SCAN.Enabled = false;  //关闭扫描串口按钮

                 ToolStripMenuItem_Connect.Text = "连接设备";
                 toolStripMenuItem_LJ.Enabled = true; //连接设备按钮
                 ToolStripMenuItem_SCAN_ID.Enabled = true;//扫描设备按钮
                 toolStripComboBox_ID.Enabled = true;//设备ID编辑框

                 comboBox_MODBUS_RATE.Enabled = false;//设置波特率
                 numericUpDown_ID.Enabled = false; //设置modbusID
                 comboBox_TAG_or_BS.Enabled = false; //设备模式
                 comboBox_DW_MODE.Enabled = false; //定位模式
                 numericUpDown_TAG_or_BS_ID.Enabled = false;//设备ID
                 button_redata.Enabled = false; //读取配置按钮
                 button_wrdaata.Enabled = false;//载入配置按钮
                 toolStripComboBox_com.Enabled = false;  //串口选择
                 toolStripComboBox_Rate.Enabled = false; //串口波特率选择
                 comboBox_RANGING.Enabled = false;    //测距方式选择
                 comboBox_AIR_CHAN.Enabled = false;   //空中信道选择
                 comboBox_AIR_RAT.Enabled = false;   //空中速率选择
                 numericUpDown_KAM_Q.Enabled = false;  //卡尔曼Q
                 numericUpDown_KAM_R.Enabled = false;  //卡尔曼R
                 numericUpDown_RX_DELAY.Enabled = false;  //接收延时

                 numericUpDown_origin_X.Enabled = false; //地图X坐标
                 numericUpDown_origin_Y.Enabled = false; //地图Y坐标
                 numericUpDown_map_multiple.Enabled = false; //地图比例
                 checkBox_draw_round.Enabled = false; //地图设置
                 checkBox_axis.Enabled = false;       //地图设置
                 checkBox_coordinate.Enabled = false;  //地图设置
                 checkBox_name.Enabled = false;       //地图设置

                 checkBox_JS.Enabled = false;//地图设置

                 numericUpDown_TAG_num.Enabled = false;  //标签数量框
                 button_CJ_OPEN.Enabled = false;         //开始定位按钮
                 button_CJ_STOP.Enabled = false;       //取消定位按钮
        

                 dataGridView_TAG.Enabled = false;    //标签列表
                 dataGridView_BS_SET.Enabled = false; //基站列表
                 checkBox_B.Enabled = false;    //基站使能
                 checkBox_C.Enabled = false;    //基站使能
                 checkBox_D.Enabled = false;    //基站使能
                 checkBox_E.Enabled = false;    //基站使能
                 checkBox_F.Enabled = false;    //基站使能
                 checkBox_G.Enabled = false;    //基站使能
                 checkBox_H.Enabled = false;    //基站使能
                 timer_DW.Enabled = false;  //定位关
             }
             if (state_Flag == 2)//连接设备，但还没成功读取配置
             {
                 toolStripStatusLabel_state.Text = "软件状态：读取设备失败";
                 ToolStripMenuItem1_OPEN_CLOSE.Text = "关闭串口";
                 ToolStripMenuItem_SCAN.Enabled = false;  //关闭扫描串口按钮

                 ToolStripMenuItem_Connect.Text = "断开连接";
                 toolStripMenuItem_LJ.Enabled = true; //连接设备按钮
                 ToolStripMenuItem_SCAN_ID.Enabled = false;//扫描设备按钮
                 toolStripComboBox_ID.Enabled = false;//设备ID编辑框

                 comboBox_MODBUS_RATE.Enabled = false;//设置波特率
                 numericUpDown_ID.Enabled = false; //设置modbusID
                 comboBox_TAG_or_BS.Enabled = false; //设备模式
                 comboBox_DW_MODE.Enabled = false; //定位模式
                 numericUpDown_TAG_or_BS_ID.Enabled = false;//设备ID
                 button_redata.Enabled = false; //读取配置按钮
                 button_wrdaata.Enabled = false;//载入配置按钮
                 toolStripComboBox_com.Enabled = false;  //串口选择
                 toolStripComboBox_Rate.Enabled = false; //串口波特率选择
                 comboBox_RANGING.Enabled = false;    //测距方式选择
                 comboBox_AIR_CHAN.Enabled = false;   //空中信道选择
                 comboBox_AIR_RAT.Enabled = false;   //空中速率选择
                 numericUpDown_KAM_Q.Enabled = false;  //卡尔曼Q
                 numericUpDown_KAM_R.Enabled = false;  //卡尔曼R
                 numericUpDown_RX_DELAY.Enabled = false;  //接收延时

                 numericUpDown_origin_X.Enabled = false; //地图X坐标
                 numericUpDown_origin_Y.Enabled = false; //地图Y坐标
                 numericUpDown_map_multiple.Enabled = false; //地图比例
                 checkBox_draw_round.Enabled = false; //地图设置
                 checkBox_axis.Enabled = false;       //地图设置
                 checkBox_coordinate.Enabled = false;  //地图设置
                 checkBox_name.Enabled = false;       //地图设置
                 checkBox_JS.Enabled = false;//地图设置

                 numericUpDown_TAG_num.Enabled = false;  //标签数量框
                 button_CJ_OPEN.Enabled = false;         //开始定位按钮
                 button_CJ_STOP.Enabled = false;       //取消定位按钮
                 

                 dataGridView_TAG.Enabled = false;    //标签列表
                 dataGridView_BS_SET.Enabled = false; //基站列表
                 checkBox_B.Enabled = false;    //基站使能
                 checkBox_C.Enabled = false;    //基站使能
                 checkBox_D.Enabled = false;    //基站使能
                 checkBox_E.Enabled = false;    //基站使能
                 checkBox_F.Enabled = false;    //基站使能
                 checkBox_G.Enabled = false;    //基站使能
                 checkBox_H.Enabled = false;    //基站使能
                 timer_DW.Enabled = false;  //定位关
             }
             if (state_Flag == 3)//连接设备，成功读取配置
             {
                 toolStripStatusLabel_state.Text = "软件状态：读取设备成功";
                 ToolStripMenuItem1_OPEN_CLOSE.Text = "关闭串口";
                 ToolStripMenuItem_SCAN.Enabled = false;  //关闭扫描串口按钮

                 ToolStripMenuItem_Connect.Text = "断开连接";
                 toolStripMenuItem_LJ.Enabled = true; //连接设备按钮
                 ToolStripMenuItem_SCAN_ID.Enabled = false;//扫描设备按钮
                 toolStripComboBox_ID.Enabled = false;//设备ID编辑框

                 comboBox_MODBUS_RATE.Enabled = true;//设置波特率
                 numericUpDown_ID.Enabled = true; //设置modbusID
                 comboBox_TAG_or_BS.Enabled = true; //设备模式


                 if (comboBox_TAG_or_BS.SelectedIndex == 0) //标签
                 {
                     comboBox_DW_MODE.Enabled = false; //定位模式
                     numericUpDown_TAG_or_BS_ID.Enabled = true;//设备ID
                     button_redata.Enabled = true; //读取配置按钮
                     button_wrdaata.Enabled = true;//载入配置按钮
                     toolStripComboBox_com.Enabled = false;  //串口选择
                     toolStripComboBox_Rate.Enabled = false; //串口波特率选择
                     comboBox_RANGING.Enabled = false;    //测距方式选择
                     comboBox_AIR_CHAN.Enabled = true;   //空中信道选择
                     comboBox_AIR_RAT.Enabled = true;   //空中速率选择
                     numericUpDown_KAM_Q.Enabled = false;  //卡尔曼Q
                     numericUpDown_KAM_R.Enabled = false;  //卡尔曼R
                     numericUpDown_RX_DELAY.Enabled = true;  //接收延时

                     numericUpDown_origin_X.Enabled = false; //地图X坐标
                     numericUpDown_origin_Y.Enabled = false; //地图Y坐标
                     numericUpDown_map_multiple.Enabled = false; //地图比例
                     checkBox_draw_round.Enabled = false; //地图设置
                     checkBox_axis.Enabled = false;       //地图设置
                     checkBox_coordinate.Enabled = false;  //地图设置
                     checkBox_name.Enabled = false;       //地图设置
 
                     checkBox_JS.Enabled = false;//地图设置

                     numericUpDown_TAG_num.Enabled = false;  //标签数量框
                     button_CJ_OPEN.Enabled = false;         //开始定位按钮
                     button_CJ_STOP.Enabled = false;       //取消定位按钮                  

                     dataGridView_TAG.Enabled = false;    //标签列表
                     dataGridView_BS_SET.Enabled = false; //基站列表
                     checkBox_B.Enabled = false;    //基站使能
                     checkBox_C.Enabled = false;    //基站使能
                     checkBox_D.Enabled = false;    //基站使能
                     checkBox_E.Enabled = false;    //基站使能
                     checkBox_F.Enabled = false;    //基站使能
                     checkBox_G.Enabled = false;    //基站使能
                     checkBox_H.Enabled = false;    //基站使能
                 }
                 if (comboBox_TAG_or_BS.SelectedIndex == 1) //次基站
                 {
                     comboBox_DW_MODE.Enabled = false; //定位模式
                     numericUpDown_TAG_or_BS_ID.Enabled = true;//设备ID
                     button_redata.Enabled = true; //读取配置按钮
                     button_wrdaata.Enabled = true;//载入配置按钮
                     toolStripComboBox_com.Enabled = false;  //串口选择
                     toolStripComboBox_Rate.Enabled = false; //串口波特率选择
                     comboBox_RANGING.Enabled = false;    //测距方式选择
                     comboBox_AIR_CHAN.Enabled = true;   //空中信道选择
                     comboBox_AIR_RAT.Enabled = true;   //空中速率选择
                     numericUpDown_KAM_Q.Enabled = false;  //卡尔曼Q
                     numericUpDown_KAM_R.Enabled = false;  //卡尔曼R
                     numericUpDown_RX_DELAY.Enabled = true;  //接收延时

                     numericUpDown_origin_X.Enabled = false; //地图X坐标
                     numericUpDown_origin_Y.Enabled = false; //地图Y坐标
                     numericUpDown_map_multiple.Enabled = false; //地图比例
                     checkBox_draw_round.Enabled = false; //地图设置
                     checkBox_axis.Enabled = false;       //地图设置
                     checkBox_coordinate.Enabled = false;  //地图设置
                     checkBox_name.Enabled = false;       //地图设置
                     checkBox_JS.Enabled = false;//地图设置
                     numericUpDown_TAG_num.Enabled = false;  //标签数量框
                     button_CJ_OPEN.Enabled = false;         //开始定位按钮
                     button_CJ_STOP.Enabled = false;       //取消定位按钮
    

                     dataGridView_TAG.Enabled = false;    //标签列表
                     dataGridView_BS_SET.Enabled = false; //基站列表
                     checkBox_B.Enabled = false;    //基站使能
                     checkBox_C.Enabled = false;    //基站使能
                     checkBox_D.Enabled = false;    //基站使能
                     checkBox_E.Enabled = false;    //基站使能
                     checkBox_F.Enabled = false;    //基站使能
                     checkBox_G.Enabled = false;    //基站使能
                     checkBox_H.Enabled = false;    //基站使能
                 }
                 if (comboBox_TAG_or_BS.SelectedIndex == 2) //主基站
                 {
                     comboBox_DW_MODE.Enabled = true; //定位模式
                     numericUpDown_TAG_or_BS_ID.Enabled = false;//设备ID
                     button_redata.Enabled = true; //读取配置按钮
                     button_wrdaata.Enabled = true;//载入配置按钮
                     toolStripComboBox_com.Enabled = false;  //串口选择
                     toolStripComboBox_Rate.Enabled = false; //串口波特率选择
                     comboBox_RANGING.Enabled = true;    //测距方式选择
                     comboBox_AIR_CHAN.Enabled = true;   //空中信道选择
                     comboBox_AIR_RAT.Enabled = true;   //空中速率选择
                     numericUpDown_KAM_Q.Enabled = true;  //卡尔曼Q
                     numericUpDown_KAM_R.Enabled = true;  //卡尔曼R
                     numericUpDown_RX_DELAY.Enabled = true;  //接收延时

                     numericUpDown_origin_X.Enabled = true; //地图X坐标
                     numericUpDown_origin_Y.Enabled = true; //地图Y坐标
                     numericUpDown_map_multiple.Enabled = true; //地图比例
                     checkBox_draw_round.Enabled = true; //地图设置
                     checkBox_axis.Enabled = true;       //地图设置
                     checkBox_coordinate.Enabled = true;  //地图设置
                     checkBox_name.Enabled = true;       //地图设置
                     checkBox_JS.Enabled = true;//地图设置

                     numericUpDown_TAG_num.Enabled = true;  //标签数量框
                     button_CJ_OPEN.Enabled = true;         //开始定位按钮
                     button_CJ_STOP.Enabled = false;       //取消定位按钮
   

                     dataGridView_TAG.Enabled = true;    //标签列表
                     dataGridView_BS_SET.Enabled = true; //基站列表
                     checkBox_B.Enabled = true;    //基站使能
                     checkBox_C.Enabled = true;    //基站使能
                     checkBox_D.Enabled = true;    //基站使能
                     checkBox_E.Enabled = true;    //基站使能
                     checkBox_F.Enabled = true;    //基站使能
                     checkBox_G.Enabled = true;    //基站使能
                     checkBox_H.Enabled = true;    //基站使能
                 }

             }
             if (state_Flag == 4)//开始定位
             {
                 toolStripStatusLabel_state.Text = "软件状态：正在定位扫描";
                 ToolStripMenuItem1_OPEN_CLOSE.Text = "关闭串口";
                 ToolStripMenuItem_SCAN.Enabled = false;  //关闭扫描串口按钮

                 ToolStripMenuItem_Connect.Text = "断开连接";
                 toolStripMenuItem_LJ.Enabled = true; //连接设备按钮
                 ToolStripMenuItem_SCAN_ID.Enabled = false;//扫描设备按钮
                 toolStripComboBox_ID.Enabled = false;//设备ID编辑框

                 comboBox_MODBUS_RATE.Enabled = false;//设置波特率
                 numericUpDown_ID.Enabled = false; //设置modbusID
                 comboBox_TAG_or_BS.Enabled = false; //设备模式
                 comboBox_DW_MODE.Enabled = false; //定位模式
                 numericUpDown_TAG_or_BS_ID.Enabled = false;//设备ID
                 button_redata.Enabled = false; //读取配置按钮
                 button_wrdaata.Enabled = false;//载入配置按钮
                 toolStripComboBox_com.Enabled = false;  //串口选择
                 toolStripComboBox_Rate.Enabled = false; //串口波特率选择
                 comboBox_RANGING.Enabled = false;    //测距方式选择
                 comboBox_AIR_CHAN.Enabled = false;   //空中信道选择
                 comboBox_AIR_RAT.Enabled = false;   //空中速率选择
                 numericUpDown_KAM_Q.Enabled = false;  //卡尔曼Q
                 numericUpDown_KAM_R.Enabled = false;  //卡尔曼R
                 numericUpDown_RX_DELAY.Enabled = false;  //接收延时

                 numericUpDown_origin_X.Enabled = true; //地图X坐标
                 numericUpDown_origin_Y.Enabled = true; //地图Y坐标
                 numericUpDown_map_multiple.Enabled = true; //地图比例
                 checkBox_draw_round.Enabled = true; //地图设置
                 checkBox_axis.Enabled = true;       //地图设置
                 checkBox_coordinate.Enabled = true;  //地图设置
                 checkBox_name.Enabled = true;       //地图设置
                 checkBox_JS.Enabled = true;//地图设置

                 numericUpDown_TAG_num.Enabled = false;  //标签数量框
                 button_CJ_OPEN.Enabled = false;         //开始定位按钮
                 button_CJ_STOP.Enabled = true;       //取消定位按钮
   

                 dataGridView_TAG.Enabled = false;    //标签列表
                 dataGridView_BS_SET.Enabled = false; //基站列表
                 checkBox_B.Enabled = false;    //基站使能
                 checkBox_C.Enabled = false;    //基站使能
                 checkBox_D.Enabled = false;    //基站使能
                 checkBox_E.Enabled = false;    //基站使能
                 checkBox_F.Enabled = false;    //基站使能
                 checkBox_G.Enabled = false;    //基站使能
                 checkBox_H.Enabled = false;    //基站使能
             }

             if (state_Flag == 10)//错误版本，静止使用
             {
                 toolStripStatusLabel_state.Text = "软件状态：未打开串口";
                 ToolStripMenuItem1_OPEN_CLOSE.Text = "打开串口";
                 ToolStripMenuItem_SCAN.Enabled = true;  //打开扫描串口按钮

                 ToolStripMenuItem_Connect.Text = "连接设备";
                 toolStripMenuItem_LJ.Enabled = false; //连接设备按钮
                 ToolStripMenuItem_SCAN_ID.Enabled = false;//扫描设备按钮
                 toolStripComboBox_ID.Enabled = false;//设备ID编辑框

                 comboBox_MODBUS_RATE.Enabled = false;//设置波特率
                 numericUpDown_ID.Enabled = false; //设置modbusID
                 comboBox_TAG_or_BS.Enabled = false; //设备模式
                 comboBox_DW_MODE.Enabled = false; //定位模式
                 numericUpDown_TAG_or_BS_ID.Enabled = false;//设备ID
                 button_redata.Enabled = false; //读取配置按钮
                 button_wrdaata.Enabled = false;//载入配置按钮
                 toolStripComboBox_com.Enabled = false;  //串口选择
                 toolStripComboBox_Rate.Enabled = false; //串口波特率选择
                 comboBox_RANGING.Enabled = false;    //测距方式选择
                 comboBox_AIR_CHAN.Enabled = false;   //空中信道选择
                 comboBox_AIR_RAT.Enabled = false;   //空中速率选择
                 numericUpDown_KAM_Q.Enabled = false;  //卡尔曼Q
                 numericUpDown_KAM_R.Enabled = false;  //卡尔曼R
                 numericUpDown_RX_DELAY.Enabled = false;  //接收延时

                 numericUpDown_origin_X.Enabled = false; //地图X坐标
                 numericUpDown_origin_Y.Enabled = false; //地图Y坐标
                 numericUpDown_map_multiple.Enabled = false; //地图比例
                 checkBox_draw_round.Enabled = false; //地图设置
                 checkBox_axis.Enabled = false;       //地图设置
                 checkBox_coordinate.Enabled = false;  //地图设置
                 checkBox_name.Enabled = false;       //地图设置
                 checkBox_JS.Enabled = false;//地图设置

                 numericUpDown_TAG_num.Enabled = false;  //标签数量框
                 button_CJ_OPEN.Enabled = false;         //开始定位按钮
                 button_CJ_STOP.Enabled = false;       //取消定位按钮


                 dataGridView_TAG.Enabled = false;    //标签列表
                 dataGridView_BS_SET.Enabled = false; //基站列表
                 checkBox_B.Enabled = false;    //基站使能
                 checkBox_C.Enabled = false;    //基站使能
                 checkBox_D.Enabled = false;    //基站使能
                 checkBox_E.Enabled = false;    //基站使能
                 checkBox_F.Enabled = false;    //基站使能
                 checkBox_G.Enabled = false;    //基站使能
                 checkBox_H.Enabled = false;    //基站使能
             }
         }

         

         private void ToolStripMenuItem_SCAN_Click(object sender, EventArgs e)
         {
             string[] str = SerialPort.GetPortNames();
             toolStripComboBox_com.Items.Clear();
             toolStripComboBox_com.Items.AddRange(str);
             if (str.Length != 0) connect_Flag = 1; //调到搜索到串口
             toolStripComboBox_com.SelectedIndex = 0;
             toolStripComboBox_Rate.SelectedIndex = 7;
         }

         private void ToolStripMenuItem1_OPEN_CLOSE_Click(object sender, EventArgs e)
         {
             if (state_Flag == 0)
             {
                 if (toolStripComboBox_com.SelectedItem == null)
                 {
                     MessageBox.Show("请选择正确的串口", "提示");
                     return;
                 }
                 serialPort1.PortName = toolStripComboBox_com.SelectedItem.ToString();
                 serialPort1.BaudRate = Convert.ToInt32(toolStripComboBox_Rate.SelectedItem.ToString());
                 Flag_BaudRate = toolStripComboBox_Rate.SelectedIndex;
                 timer_MODBUS_connet.Interval =1000;
                 serialPort1.DataBits = 8;
                 serialPort1.StopBits = StopBits.One;
                 serialPort1.Parity = Parity.None;
                 serialPort1.ReadTimeout = 1000;
                 serialPort1.WriteTimeout = 1000;
                 try
                 {
                     serialPort1.Open();
                 }
                 catch (System.UnauthorizedAccessException)
                 {
                     MessageBox.Show("串口打开失败", "提示");
                     return;
                 }
                 state_Flag = 1;
                 serialPort1.DataReceived += new SerialDataReceivedEventHandler(SerDataReceive);
                 connect_Flag = 2;

             }
             else
             {
                 state_Flag = 0;
                 connect_Flag = 1;
                 timer_MODBUS_connet.Enabled = false;
                 timer_SCAN_ID.Enabled = false;
                 Thread.Sleep(50);
                 serialPort1.Close();
             }
         }

         private void ToolStripMenuItem_SCAN_ID_Click(object sender, EventArgs e)
         {
             //    Array.Clear(ID_buf,0 , ID_buf.Length);
             ID_buf = new String[] { };
             timer_SCAN_ID.Enabled = true;
             isSCAN = 1;
             toolStripProgressBar__SCAN.Value = 0;
         }

         private void ToolStripMenuItem_Connect_Click(object sender, EventArgs e)
         {
             { //清楚测距模式
                 byte[] send_buf = new byte[300];
                 Int32 send_length = 0;
                 send_buf[0] = NOW_ID;
                 send_buf[1] = 0x06;
                 send_buf[2] = 0x00;
                 send_buf[3] = 0x28;
                 send_buf[4] = 0x00;
                 send_buf[5] = 0x00;
                 send_buf[6] = 0x00;
                 send_buf[7] = 0x00;
                 send_length = 8;
                 senddata(send_buf, send_length);
                 Thread.Sleep(10);
             }
             if (connect_Flag == 2 || connect_Flag == 3)
             {

                 try
                 {
                     NOW_ID = (byte)Convert.ToInt16(toolStripComboBox_ID.Text.ToString());
                 }
                 catch
                 {
                     MessageBox.Show("请输入正确ID", "提示");
                     return;
                 }
                 connect_Flag = 5;
                 state_Flag = 2;
                 timer_MODBUS_connet.Enabled = true;
                 Click_Flag = 0;

                 return;
             }

             if (connect_Flag == 4 || connect_Flag == 5)
             {
                 Thread.Sleep(50);

                 state_Flag = 1;
                 timer_MODBUS_connet.Enabled = false;
                 Thread.Sleep(50);
                 connect_Flag = 3;


                 return;
             }
         }

         private void ToolStripMenuItem_map_in_Click(object sender, EventArgs e)
         {
             OpenFileDialog file = new OpenFileDialog();
             file.InitialDirectory = ".";
             file.Filter = "所有文件(*.*)|*.*";
             file.ShowDialog();
             if (file.FileName != string.Empty)
             {
                 pathname = file.FileName;   //获得文件的绝对路径
                 try
                 {

                     this.pictureBox1.Load(pathname);
                 }
                 catch (Exception ex)
                 {
                     MessageBox.Show(ex.Message);
                 }
             }
         }

         private void ToolStripMenuItem1_map_clear_Click(object sender, EventArgs e)
         {
             pathname = null;
             gph.Clear(Color.White);
             this.pictureBox1.Image = bMap;
         }

         private void textBox_com_data_DoubleClick(object sender, EventArgs e)
         {
             textBox_com_data.Clear();
         }

         private void textBox_com_data_TextChanged(object sender, EventArgs e)
         {
             //textBox_com_data.SelectionStart = textBox_com_data.Text.Length;
             //textBox_com_data.Select(textBox_com_data.Text.Length, 0);
             //textBox_com_data.ScrollToCaret();
         }

         private void button_CJ_OPEN_Click(object sender, EventArgs e)
         {
             state_Flag = 4;
             Click_Flag = 2;
             timer_DW.Enabled = true;
             timer_display.Enabled = true;
             CJ_EN = 1;
         }


         private void toolStripStatusLabel2_Click(object sender, EventArgs e)
         {
             System.Diagnostics.Process.Start("www.gzlwkj.com");
         }

         private void ToolStripMenuItem_SCLJ_Click(object sender, EventArgs e)
         {
             FolderBrowserDialog path = new FolderBrowserDialog();
             path.ShowDialog();
             string saveFileName =Path.Combine(path.SelectedPath,"Channel_1_data.xls");
             //SaveFileDialog saveDialog = new SaveFileDialog();
             //saveDialog.DefaultExt = "xls";
             //saveDialog.Filter = "Excel文件|*.xls";
             //saveDialog.FileName = fileName;
             //saveDialog.ShowDialog();
             //saveFileName = saveDialog.FileName;
             if (saveFileName.IndexOf(":") < 0) return; //被点了取消
             Microsoft.Office.Interop.Excel.Application xlApp = new Microsoft.Office.Interop.Excel.Application();
             if (xlApp == null)
             {
                 MessageBox.Show("无法创建Excel对象，可能您的机子未安装Excel");
                 return;
             }
             Microsoft.Office.Interop.Excel.Workbooks workbooks = xlApp.Workbooks;
             Microsoft.Office.Interop.Excel.Workbook workbook = workbooks.Add(Microsoft.Office.Interop.Excel.XlWBATemplate.xlWBATWorksheet);
             Microsoft.Office.Interop.Excel.Worksheet worksheet = (Microsoft.Office.Interop.Excel.Worksheet)workbook.Worksheets[1];//取得sheet1
             //写入标题
             for (int i = 0; i < dataGridView_GJ1.ColumnCount; i++)
             {
                 worksheet.Cells[1, i + 1] = dataGridView_GJ1.Columns[i].HeaderText;
             }
             //写入数值
             for (int r = 0; r < dataGridView_GJ1.Rows.Count; r++)
             {
                 for (int i = 0; i < dataGridView_GJ1.ColumnCount; i++)
                 {
                     worksheet.Cells[r + 2, i + 1] = dataGridView_GJ1.Rows[r].Cells[i].Value;
                 }
                 System.Windows.Forms.Application.DoEvents();
             }
             worksheet.Columns.EntireColumn.AutoFit();//列宽自适应
             if (saveFileName != "")
             {
                 try
                 {
                     workbook.Saved = true;
                     workbook.SaveCopyAs(saveFileName);
                 }
                 catch (Exception ex)
                 {
                     MessageBox.Show("导出文件时出错,文件可能正被打开！\n" + ex.Message);
                 }
             }
             xlApp.Quit();
             GC.Collect();//强行销毁
             MessageBox.Show("文件： " + saveFileName + "保存成功", "信息提示", MessageBoxButtons.OK, MessageBoxIcon.Information);
         }

         private void ToolStripMenuItem_SCLJ2_Click(object sender, EventArgs e)
         {
             FolderBrowserDialog path = new FolderBrowserDialog();
             path.ShowDialog();
             string saveFileName = Path.Combine(path.SelectedPath, "Channel_2_data.xls");
             //SaveFileDialog saveDialog = new SaveFileDialog();
             //saveDialog.DefaultExt = "xls";
             //saveDialog.Filter = "Excel文件|*.xls";
             //saveDialog.FileName = fileName;
             //saveDialog.ShowDialog();
             //saveFileName = saveDialog.FileName;
             if (saveFileName.IndexOf(":") < 0) return; //被点了取消
             Microsoft.Office.Interop.Excel.Application xlApp = new Microsoft.Office.Interop.Excel.Application();
             if (xlApp == null)
             {
                 MessageBox.Show("无法创建Excel对象，可能您的机子未安装Excel");
                 return;
             }
             Microsoft.Office.Interop.Excel.Workbooks workbooks = xlApp.Workbooks;
             Microsoft.Office.Interop.Excel.Workbook workbook = workbooks.Add(Microsoft.Office.Interop.Excel.XlWBATemplate.xlWBATWorksheet);
             Microsoft.Office.Interop.Excel.Worksheet worksheet = (Microsoft.Office.Interop.Excel.Worksheet)workbook.Worksheets[1];//取得sheet1
             //写入标题
             for (int i = 0; i < dataGridView_GJ2.ColumnCount; i++)
             {
                 worksheet.Cells[1, i + 1] = dataGridView_GJ2.Columns[i].HeaderText;
             }
             //写入数值
             for (int r = 0; r < dataGridView_GJ2.Rows.Count; r++)
             {
                 for (int i = 0; i < dataGridView_GJ2.ColumnCount; i++)
                 {
                     worksheet.Cells[r + 2, i + 1] = dataGridView_GJ2.Rows[r].Cells[i].Value;
                 }
                 System.Windows.Forms.Application.DoEvents();
             }
             worksheet.Columns.EntireColumn.AutoFit();//列宽自适应
             if (saveFileName != "")
             {
                 try
                 {
                     workbook.Saved = true;
                     workbook.SaveCopyAs(saveFileName);
                 }
                 catch (Exception ex)
                 {
                     MessageBox.Show("导出文件时出错,文件可能正被打开！\n" + ex.Message);
                 }
             }
             xlApp.Quit();
             GC.Collect();//强行销毁
             MessageBox.Show("文件： " + saveFileName + "保存成功", "信息提示", MessageBoxButtons.OK, MessageBoxIcon.Information);
         }

         private void button2_Click(object sender, EventArgs e)
         {
             isGJ=!isGJ;
         }
         double[,] CircleInsect(double x1, double y1, double r1, double x2, double y2, double r2)//两个圆计算一条交点直线
         {
             //X轴相同情况，Y轴相同情况是有必要列出的，否则会除以0，进入数据非法
             double[,] points = new double[2, 2];
             double Dis = Math.Sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
             if ((x1 == x2) && (y1 == y2) && (r1 == r2)) // 两圆重合的时候（在这里用不到，因为在基站筛选的时候就会被筛选掉）
             {
                 points[0, 0] = 0;
                 points[0, 1] = 0;
                 points[1, 0] = 0;
                 points[1, 1] = 0;
                 return points;
             }
             if (Dis > (r1 + r2))     //分离 
             {

                 double op = Dis / (r1 + r2) + 0.01;
                 double rr1 = r1 * op, rr2 = r2 * op;

                 if ((x1 == x2) && (!(y1 == y2)))  // Ô²AºÍÔ²B ºá×ø±êÏàµÈ
                 {
                     double x0 = x1 = x2;
                     double y0 = y1 + (y2 - y1) * (rr1 * rr1 - rr2 * rr2 + Dis * Dis) / (2.0 * Dis * Dis);
                     double Dis1 = Math.Sqrt(rr1 * rr1 - (x0 - x1) * (x0 - x1) - (y0 - y1) * (y0 - y1));
                     points[0, 0] = x0 - Dis1;
                     points[0, 1] = y0;
                     points[1, 0] = x0 + Dis1;
                     points[1, 1] = y0;
                     //printf("C");
                 }
                 else if ((!(x1 == x2)) && (y1 == y2)) // Ô²AºÍÔ²B ×Ý×ø±êÏàµÈ
                 {
                     double y0 = y1 = y2;
                     double x0 = x1 + (x2 - x1) * (rr1 * rr1 - rr2 * rr2 + Dis * Dis) / (2.0 * Dis * Dis);
                     double Dis1 = Math.Sqrt(rr1 * rr1 - (x0 - x1) * (x0 - x1) - (y0 - y1) * (y0 - y1));
                     points[0, 0] = x0;
                     points[0, 1] = y0 - Dis1;
                     points[1, 0] = x0;
                     points[1, 1] = y0 + Dis1;
                     //printf("D");
                 }
                 else if ((!(x1 == x2)) && (!(y1 == y2))) // ºá×Ý×ø±ê¶¼²»µÈ 
                 {
                     double k1 = (y2 - y1) / (x2 - x1);//ABµÄÐ±ÂÊ
                     double k2 = -1.0 / k1;             // CDµÄÐ±ÂÊ
                     double x0 = x1 + (x2 - x1) * (rr1 * rr1 - rr2 * rr2 + Dis * Dis) / (2 * Dis * Dis);
                     double y0 = y1 + k1 * (x0 - x1);
                     double Dis1 = rr1 * rr1 - (x0 - x1) * (x0 - x1) - (y0 - y1) * (y0 - y1); //CEµÄÆ½·½
                     double Dis2 = Math.Sqrt(Dis1 / (1.0 + k2 * k2));//EFµÄ³¤£¬¹ýC×÷¹ýEµãË®Æ½Ö±ÏßµÄ´¹Ïß£¬½»ÓÚFµã
                     points[0, 0] = x0 - Dis2;
                     points[0, 1] = y0 + k2 * (points[0, 0] - x0);
                     points[1, 0] = x0 + Dis2;
                     points[1, 1] = y0 + k2 * (points[1, 0] - x0);
                     //printf("fuck");
                 }



             }
             if (Dis < Math.Abs(r1 - r2))   // 包含情况
             {
                 points[0, 0] = 0;
                 points[0, 1] = 0;
                 points[1, 0] = 0;
                 points[1, 1] = 0;
                 return points;
             }
             if ((Dis == (r1 + r2)) || (Dis == Math.Abs(r1 - r2))) //相交一个点
             {
                 //	 printf("A");
                 if (Dis == (r1 + r2))// ÍâÇÐ
                 {

                     double op = Dis / (r1 + r2) + 0.01;//增大比例
                     double rr1 = r1 * op, rr2 = r2 * op;

                     if ((x1 == x2) && (!(y1 == y2)))  // Ô²AºÍÔ²B ºá×ø±êÏàµÈ
                     {
                         double x0 = x1 = x2;
                         double y0 = y1 + (y2 - y1) * (rr1 * rr1 - rr2 * rr2 + Dis * Dis) / (2.0 * Dis * Dis);
                         double Dis1 = Math.Sqrt(rr1 * rr1 - (x0 - x1) * (x0 - x1) - (y0 - y1) * (y0 - y1));
                         points[0, 0] = x0 - Dis1;
                         points[0, 1] = y0;
                         points[1, 0] = x0 + Dis1;
                         points[1, 1] = y0;
                     }
                     else if ((!(x1 == x2)) && (y1 == y2)) // Ô²AºÍÔ²B ×Ý×ø±êÏàµÈ
                     {
                         double y0 = y1 = y2;
                         double x0 = x1 + (x2 - x1) * (rr1 * rr1 - rr2 * rr2 + Dis * Dis) / (2.0 * Dis * Dis);
                         double Dis1 = Math.Sqrt(rr1 * rr1 - (x0 - x1) * (x0 - x1) - (y0 - y1) * (y0 - y1));
                         points[0, 0] = x0;
                         points[0, 1] = y0 - Dis1;
                         points[1, 0] = x0;
                         points[1, 1] = y0 + Dis1;
                     }
                     else if ((!(x1 == x2)) && (!(y1 == y2))) // ºá×Ý×ø±ê¶¼²»µÈ 
                     {
                         double k1 = (y2 - y1) / (x2 - x1);//ABµÄÐ±ÂÊ
                         double k2 = -1.0 / k1;             // CDµÄÐ±ÂÊ
                         double x0 = x1 + (x2 - x1) * (rr1 * rr1 - rr2 * rr2 + Dis * Dis) / (2 * Dis * Dis);
                         double y0 = y1 + k1 * (x0 - x1);
                         double Dis1 = rr1 * rr1 - (x0 - x1) * (x0 - x1) - (y0 - y1) * (y0 - y1); //CEµÄÆ½·½
                         double Dis2 = Math.Sqrt(Dis1 / (1.0 + k2 * k2));//EFµÄ³¤£¬¹ýC×÷¹ýEµãË®Æ½Ö±ÏßµÄ´¹Ïß£¬½»ÓÚFµã
                         points[0, 0] = x0 - Dis2;
                         points[0, 1] = y0 + k2 * (points[0, 0] - x0);
                         points[1, 0] = x0 + Dis2;
                         points[1, 1] = y0 + k2 * (points[1, 0] - x0);
                         //printf("fuck");
                     }
                 }

                 if (Dis == Math.Abs(r1 - r2)) //被包含相交一个点
                 {
                     double op = Dis / (r1 + r2) - 0.01;//缩小比例
                     double rr1 = r1 * op, rr2 = r2 * op;

                     if ((x1 == x2) && (!(y1 == y2)))
                     {
                         if (r1 > r2) // AÄÚº¬B
                         {
                             if (y1 > y2)
                             {
                                 points[0, 0] = x1 = x2;
                                 points[0, 1] = y1 - r1;
                             }
                             else
                             {
                                 points[0, 0] = x1 = x2;
                                 points[0, 1] = y1 + r1;
                             }
                         }
                         else // B ÄÚº¬A
                         {
                             if (y1 > y2)
                             {
                                 points[0, 0] = x1 = x2;
                                 points[0, 1] = y1 + r1;
                             }
                             else
                             {
                                 points[0, 0] = x1 = x2;
                                 points[0, 1] = y1 - r1;
                             }
                         }
                     }
                     else if ((!(x1 == x2)) && (y1 == y2))
                     {
                         if (r1 > r2)
                         {
                             if (x1 > x2)
                             {
                                 points[0, 0] = x1 - r1;
                                 points[0, 1] = y1 = y2;
                             }
                             else
                             {
                                 points[0, 0] = x1 + r1;
                                 points[0, 1] = y1 = y2;
                             }
                         }
                         else
                         {
                             if (x1 > x2)
                             {
                                 points[0, 0] = x1 + r1;
                                 points[0, 1] = y1 = y2;
                             }
                             else
                             {
                                 points[0, 0] = x1 - r1;
                                 points[0, 1] = y1 = y2;
                             }
                         }
                     }
                     else if ((!(x1 == x2)) && (!(y1 == y2))) // ÊÇ·ñÒª¿¼ÂÇÄÚº¬¹ØÏµ(Çó×ø±êÊ±ÊÇ·ñÓÐÓ°Ïì)
                     {
                         // ÄÚÇÐÇé¿ö£¬½»µãÔÚABÁ¬ÏßµÄÑÓ³¤ÏßÉÏ£¬Òª¿¼ÂÇÇÐµãµÄÎ»ÖÃ 
                         double k1 = (y2 - y1) / (x2 - x1);
                         double k2 = -1.0 / k1;
                         points[0, 0] = x1 + (x1 - x2) * r1 / Dis;
                         //points[0].Y = y1 + k2 * (points[0].X - x1);
                         points[0, 1] = y1 + (y1 - y2) * r1 / Dis;
                     }
                 }

             }
             if ((Dis < (r1 + r2)) && (Dis > Math.Abs(r1 - r2)))    //相交
             {
                 if ((x1 == x2) && (!(y1 == y2)))  // Ô²AºÍÔ²B ºá×ø±êÏàµÈ
                 {
                     double x0 = x1 = x2;
                     double y0 = y1 + (y2 - y1) * (r1 * r1 - r2 * r2 + Dis * Dis) / (2.0 * Dis * Dis);
                     double Dis1 = Math.Sqrt(r1 * r1 - (x0 - x1) * (x0 - x1) - (y0 - y1) * (y0 - y1));
                     points[0, 0] = x0 - Dis1;
                     points[0, 1] = y0;
                     points[1, 0] = x0 + Dis1;
                     points[1, 1] = y0;
                     //printf("C");
                 }
                 else if ((!(x1 == x2)) && (y1 == y2)) // Ô²AºÍÔ²B ×Ý×ø±êÏàµÈ
                 {
                     double y0 = y1 = y2;
                     double x0 = x1 + (x2 - x1) * (r1 * r1 - r2 * r2 + Dis * Dis) / (2.0 * Dis * Dis);
                     double Dis1 = Math.Sqrt(r1 * r1 - (x0 - x1) * (x0 - x1) - (y0 - y1) * (y0 - y1));
                     points[0, 0] = x0;
                     points[0, 1] = y0 - Dis1;
                     points[1, 0] = x0;
                     points[1, 1] = y0 + Dis1;
                     //printf("D");
                 }
                 else if ((!(x1 == x2)) && (!(y1 == y2))) // ºá×Ý×ø±ê¶¼²»µÈ 
                 {
                     double k1 = (y2 - y1) / (x2 - x1);//ABµÄÐ±ÂÊ
                     double k2 = -1.0 / k1;             // CDµÄÐ±ÂÊ
                     double x0 = x1 + (x2 - x1) * (r1 * r1 - r2 * r2 + Dis * Dis) / (2 * Dis * Dis);
                     double y0 = y1 + k1 * (x0 - x1);
                     double Dis1 = r1 * r1 - (x0 - x1) * (x0 - x1) - (y0 - y1) * (y0 - y1); //CEµÄÆ½·½
                     double Dis2 = Math.Sqrt(Dis1 / (1.0 + k2 * k2));//EFµÄ³¤£¬¹ýC×÷¹ýEµãË®Æ½Ö±ÏßµÄ´¹Ïß£¬½»ÓÚFµã
                     points[0, 0] = x0 - Dis2;
                     points[0, 1] = y0 + k2 * (points[0, 0] - x0);
                     points[1, 0] = x0 + Dis2;
                     points[1, 1] = y0 + k2 * (points[1, 0] - x0);
                 }

             }
             return points;
         }

         double[] GetCrossPoint(double pA_x, double pA_y, double pB_x, double pB_y, double pC_x, double pC_y, double pD_x, double pD_y)//两条直线计算交点
         {

             double[] xy = new double[2];
             double k1, b1;
             double k2, b2;


             xy[1] = (pB_x * pC_y * pA_y - pA_x * pB_y * pC_y - pD_y * pB_x * pA_y + pA_x * pB_y * pD_y + pC_x * pD_y * pA_y - pD_x * pC_y * pA_y - pC_x * pB_y * pD_y + pD_x * pB_y * pC_y) /
                 (pA_y * pC_x - pD_x * pA_y - pB_y * pC_x + pD_x * pB_y + pB_x * pC_y - pA_x * pC_y - pD_y * pB_x + pD_y * pA_x);

             xy[0] = (pA_y * (pC_x - pD_x) * (pB_x - pA_x) - pD_y * (pC_x - pD_x) * (pB_x - pA_x) + pD_x * (pC_y - pD_y) * (pB_x - pA_x) + pA_x * (pA_y - pB_y) * (pC_x - pD_x)) /
                 ((pC_y - pD_y) * (pB_x - pA_x) + (pA_y - pB_y) * (pC_x - pD_x));

             return xy;
         }

         double[] Get_three_BS_Out_XYZ(double A_x, double A_y, double A_r,  //三个圆计算一个交点
                                   double B_x, double B_y, double B_r,
                                   double C_x, double C_y, double C_r)
         {
             double[,] PP_points_A = new double[2, 2]; //¼ÆËãµÄ±ßÏß»º´æ
             double[,] PP_points_B = new double[2, 2]; //¼ÆËãµÄ±ßÏß»º´æ
             double[,] PP_points_C = new double[2, 2]; //¼ÆËãµÄ±ßÏß»º´æ
             double[] PP_point_A = new double[2];   //¼ÆËãµãµÄ»º´æ
             double[] PP_point_B = new double[2];   //¼ÆËãµãµÄ»º´æ
             double[] PP_point_C = new double[2];   //¼ÆËãµãµÄ»º´æ

             double[] PP_point_out = new double[2];
             PP_points_A = CircleInsect(A_x, A_y, A_r, B_x, B_y, B_r);
             PP_points_B = CircleInsect(A_x, A_y, A_r, C_x, C_y, C_r);
             PP_points_C = CircleInsect(B_x, B_y, B_r, C_x, C_y, C_r);
             /*
             gph.DrawLine(Pens.Black, map_origin_x + (float)PP_points_A[0, 0] / map_multiple, map_size_y - ((float)PP_points_A[0, 1] / map_multiple + map_origin_y), map_origin_x + (float)PP_points_A[1, 0] / map_multiple, map_size_y - ((float)PP_points_A[1, 1] / map_multiple + map_origin_y));  //线
             gph.DrawLine(Pens.Black, map_origin_x + (float)PP_points_B[0, 0] / map_multiple, map_size_y - ((float)PP_points_B[0, 1] / map_multiple + map_origin_y), map_origin_x + (float)PP_points_B[1, 0] / map_multiple, map_size_y - ((float)PP_points_B[1, 1] / map_multiple + map_origin_y));  //线
             gph.DrawLine(Pens.Black, map_origin_x + (float)PP_points_C[0, 0] / map_multiple, map_size_y - ((float)PP_points_C[0, 1] / map_multiple + map_origin_y), map_origin_x + (float)PP_points_C[1, 0] / map_multiple, map_size_y - ((float)PP_points_C[1, 1] / map_multiple + map_origin_y));  //线
             */
             PP_point_A = GetCrossPoint(PP_points_A[0, 0], PP_points_A[0, 1], PP_points_A[1, 0], PP_points_A[1, 1], PP_points_B[0, 0], PP_points_B[0, 1], PP_points_B[1, 0], PP_points_B[1, 1]);
             PP_point_B = GetCrossPoint(PP_points_A[0, 0], PP_points_A[0, 1], PP_points_A[1, 0], PP_points_A[1, 1], PP_points_C[0, 0], PP_points_C[0, 1], PP_points_C[1, 0], PP_points_C[1, 1]);
             PP_point_C = GetCrossPoint(PP_points_B[0, 0], PP_points_B[0, 1], PP_points_B[1, 0], PP_points_B[1, 1], PP_points_C[0, 0], PP_points_C[0, 1], PP_points_C[1, 0], PP_points_C[1, 1]);


             PP_point_out[0] = PP_point_A[0] + PP_point_B[0] + PP_point_C[0];
             PP_point_out[1] = PP_point_A[1] + PP_point_B[1] + PP_point_C[1];

             PP_point_out[0] = PP_point_out[0] / 3;
             PP_point_out[1] = PP_point_out[1] / 3;
             return PP_point_out;

         }




         double[] PersonPosition(int NUM) //  //用四个圆计算一个坐标                       
         {
             double[,] point = new double[56, 2];
             Int16[,] BS_buf = new Int16[8, 3];
             double[] point_out = new double[2];
             double[] BS_EN = new double[8];
             int i = 0, num = 0;
             int E = 0, R = 0, T = 0;
             for (i=0; i < 8; i++)
             {
                 if (dataGridView_BS_SET.Rows[i].Cells[0].Value == null)
                 {
                     BS_buf[i, 0] = 0;
                 }
                 else BS_buf[i, 0] = (Int16)(Convert.ToInt16(dataGridView_BS_SET.Rows[i].Cells[1].Value.ToString()));

                 if (dataGridView_BS_SET.Rows[i].Cells[1].Value == null)
                 {
                     BS_buf[i, 1] = 0;
                 }
                 else BS_buf[i, 1] = (Int16)(Convert.ToInt16(dataGridView_BS_SET.Rows[i].Cells[2].Value.ToString()));

                 if (dataGridView_BS_SET.Rows[i].Cells[2].Value == null)
                 {
                     BS_buf[i, 2] = 0;
                 }
                 else BS_buf[i, 2] = (Int16)(Convert.ToInt16(dataGridView_TAG.Rows[NUM].Cells[4 + i].Value.ToString()));
             }
             BS_EN[0] = 1;
             if (checkBox_B.Checked == true) BS_EN[1] = 1;
             else BS_EN[1] = 0;
             if (checkBox_C.Checked == true) BS_EN[2] = 1;
             else BS_EN[2] = 0;
             if (checkBox_D.Checked == true) BS_EN[3] = 1;
             else BS_EN[3] = 0;
             if (checkBox_E.Checked == true) BS_EN[4] = 1;
             else BS_EN[4] = 0;
             if (checkBox_F.Checked == true) BS_EN[5] = 1;
             else BS_EN[5] = 0;
             if (checkBox_G.Checked == true) BS_EN[6] = 1;
             else BS_EN[6] = 0;
             if (checkBox_H.Checked == true) BS_EN[7] = 1;
             else BS_EN[7] = 0;


             for (E = 0; E < 6; E++)  //两两圆形
             {
                 if (BS_EN[E] == 1)
                 {
                     for (R = E + 1; R < 7; R++)
                     {
                         if (BS_EN[R] == 1)
                         {
                             for (T = R + 1; T < 8; T++)
                             {
                                 if (BS_EN[T] == 1)
                                 {
                                     double[] point_xy = new double[2];
                                     //用每三个圆求出一个质心
                                     Int16 sure_flag = 0;
                                     double Dist_BS_BS;

                                     Dist_BS_BS = Math.Sqrt(Math.Pow((BS_buf[E,0] - BS_buf[R,0]), 2) + Math.Pow((BS_buf[E,1] - BS_buf[R,1]), 2));
                                     if (((Dist_BS_BS + BS_buf[R,2]) > BS_buf[E,2]) && ((Dist_BS_BS + BS_buf[E,2]) > BS_buf[R,2]))
                                     {
                                         sure_flag++;
                                     }

                                     Dist_BS_BS = Math.Sqrt(Math.Pow((BS_buf[E,0] - BS_buf[T,0]), 2) + Math.Pow((BS_buf[E,1] - BS_buf[T,1]), 2));
                                     if (((Dist_BS_BS + BS_buf[T,2]) > BS_buf[E,2]) && ((Dist_BS_BS + BS_buf[E,2]) > BS_buf[T,2]))
                                     {
                                         sure_flag++;
                                     }

                                     Dist_BS_BS = Math.Sqrt(Math.Pow((BS_buf[R,0] - BS_buf[T,0]), 2) + Math.Pow((BS_buf[R,1] - BS_buf[T,1]), 2));
                                     if (((Dist_BS_BS + BS_buf[T,2]) > BS_buf[R,2]) && ((Dist_BS_BS + BS_buf[R,2]) > BS_buf[T,2]))
                                     {
                                         sure_flag++;
                                     }
                                     if (sure_flag == 3)
                                     {
                                         point_xy = Get_three_BS_Out_XYZ(BS_buf[E, 0], BS_buf[E, 1], BS_buf[E, 2], BS_buf[R, 0], BS_buf[R, 1], BS_buf[R, 2], BS_buf[T, 0], BS_buf[T, 1], BS_buf[T, 2]);
                                         //gph.DrawEllipse(Pens.Pink, ((float)point_xy[0] / map_multiple - 4 + map_origin_x), map_size_y - ((float)point_xy[1] / map_multiple + 4 + map_origin_y), 8, 8);
                                         //gph.FillEllipse(new SolidBrush(Color.Pink), ((float)point_xy[0] / map_multiple - 4 + map_origin_x), map_size_y - ((float)point_xy[1] / map_multiple + 4 + map_origin_y), 8, 8);
                                         point[num, 0] = point_xy[0];
                                         point[num, 1] = point_xy[1];
                                         num++;
                                     }
                                 }
                             }
                         }
                     }
                 }
             }
             //求一个初略的质心
             point_out[0] = 0.0;
             point_out[1] = 0.0;
             for (i = 0; i < num; i++)//
             {
                 point_out[0] += point[i, 0];
                 point_out[1] += point[i, 1];
             }
             if (num != 0)
             {
                 point_out[0] = point_out[0] / num;
                 point_out[1] = point_out[1] / num;
             }
             if (numericUpDown_KAM_Q.Value == null)
             {
                 KALMAN_Q = 1;
             }
             else KALMAN_Q = (Int16)(Convert.ToInt16(numericUpDown_KAM_Q.Value.ToString()));
             if (numericUpDown_KAM_R.Value == null)
             {
                 KALMAN_R = 1;
             }
             else KALMAN_R = (Int16)(Convert.ToInt16(numericUpDown_KAM_R.Value.ToString()));

             //if ((point_out[0] != 0) && (point_out[1] != 0))
             {
                 point_out[0] = KalmanFilter(point_out[0], KALMAN_Q, KALMAN_R, (Int16)(0 + 2 * NUM));
                 point_out[1] = KalmanFilter(point_out[1], KALMAN_Q, KALMAN_R, (Int16)(1 + 2 * NUM));

                 dataGridView_TAG.Rows[NUM].Cells[1].Value = (Int16)point_out[0];
                 dataGridView_TAG.Rows[NUM].Cells[2].Value = (Int16)point_out[1];
             }
             return point_out;
         }

         double KalmanFilter(double ResrcData, double ProcessNiose_Q, double MeasureNoise_R, Int16 db)
         {

             double R = MeasureNoise_R;
             double Q = ProcessNiose_Q;
             double x_mid = x_last[db];
             double x_now;
             double p_mid;
             double p_now;
             double kg;

             x_mid = x_last[db];                       //x_last=x(k-1|k-1),x_mid=x(k|k-1)
             p_mid = p_last[db] + Q;                     //p_mid=p(k|k-1),p_last=p(k-1|k-1),Q=??
             kg = p_mid / (p_mid + R);                 //kg?kalman filter,R ???
             x_now = x_mid + kg * (ResrcData - x_mid);   //???????
             p_now = (1 - kg) * p_mid;                 //??????covariance
             p_last[db] = p_now;                     //??covariance ?
             x_last[db] = x_now;                     //???????	
             return x_now;
         }
       void display_3D()
       {
           Int16 i, o;
           for (i = 0; i < 8; i++)
           {
               for (o = 0; o < 3; o++)
               {
                   if (dataGridView_BS_SET.Rows[i].Cells[1 + o].Value == null)
                   {
                       sys_data_EN_x_y_z[i, 1 + o] = 0;
                   }
                   else
                   {
                       sys_data_EN_x_y_z[i, 1 + o] = (Int16)(Convert.ToInt16(dataGridView_BS_SET.Rows[i].Cells[1 + o].Value.ToString()));
                   }
               }
           }
           sys_data_EN_x_y_z[0, 0] = 1;
           if (checkBox_B.Checked == true) { sys_data_EN_x_y_z[1, 0] = 1; }
           else { sys_data_EN_x_y_z[1, 0] = 0; }
           if (checkBox_C.Checked == true) { sys_data_EN_x_y_z[2, 0] = 1; }
           else { sys_data_EN_x_y_z[2, 0] = 0; }
           if (checkBox_D.Checked == true) { sys_data_EN_x_y_z[3, 0] = 1; }
           else { sys_data_EN_x_y_z[3, 0] = 0; }
           if (checkBox_E.Checked == true) { sys_data_EN_x_y_z[4, 0] = 1; }
           else { sys_data_EN_x_y_z[4, 0] = 0; }
           if (checkBox_F.Checked == true) { sys_data_EN_x_y_z[5, 0] = 1; }
           else { sys_data_EN_x_y_z[5, 0] = 0; }
           if (checkBox_G.Checked == true) { sys_data_EN_x_y_z[6, 0] = 1; }
           else { sys_data_EN_x_y_z[6, 0] = 0; }
           if (checkBox_H.Checked == true) { sys_data_EN_x_y_z[7, 0] = 1; }
           else { sys_data_EN_x_y_z[7, 0] = 0; }
           
           for (o = 0; o < 3; o++)
            {
                sys_data_TAG_x_y_z[0, o] = (Int16)(Convert.ToInt16(dataGridView_TAG.Rows[0].Cells[1+o].Value.ToString()));
           }
            formx.data_IN(sys_data_EN_x_y_z);
            formx.data_TAG_IN(sys_data_TAG_x_y_z);
            formx.data_MAP((Int16)(Convert.ToInt16(trackBar1.Value.ToString())));
       }

      

   
    }
}
