#include "sys.h"
#include "usart.h"	
#include "led.h"  
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos 使用	  
#endif
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//串口1初始化		   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/8/18
//版本：V1.5
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved
//********************************************************************************
//V1.3修改说明 
//支持适应不同频率下的串口波特率设置.
//加入了对printf的支持
//增加了串口接收命令功能.
//修正了printf第一个字符丢失的bug
//V1.4修改说明
//1,修改串口初始化IO的bug
//2,修改了USART_RX_STA,使得串口最大接收字节数为2的14次方
//3,增加了USART_REC_LEN,用于定义串口最大允许接收的字节数(不大于2的14次方)
//4,修改了EN_USART1_RX的使能方式
//V1.5修改说明
//1,增加了对UCOSII的支持
////////////////////////////////////////////////////////////////////////////////// 	  
 
#define STAGE_SOHE  0x01
#define STAGE_TYPE  0x02
#define STAGE_NONE  0x03
#define STAGE_DATA  0x04

unsigned char	devide_flag;		        //GPS数据逗号分隔符标志
unsigned char	speed_end;			//收到速度数据结束标志
unsigned char	dir_end;			//收到方向数据结束标志
unsigned char  sysmode_GPS=0;                    //gps有无效标志
unsigned char  ew_flag;                        //东西标志
unsigned char  ns_flag;                        //南北标志

unsigned char	gps_infor_weijing[17];    //经纬度暂存 格式是度分秒形式
unsigned char	gps_infor_speed[4];       //暂存速度
unsigned char	gps_infor_time[6];        //暂存时间
unsigned char	gps_infor_date[6];        //暂存日期
unsigned char	gps_infor_dir[3];         //暂存方向

unsigned char recv1_step=STAGE_SOHE;                       //串口接收指令步骤
unsigned char uart1_r_buf;                       //串口缓存
unsigned char rev1_buf_busy;                    //串口判忙
unsigned char temp1_buf[85];                   //串口接收数组
unsigned int record1=0;   
unsigned char rendFlag=0;

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

/*使用microLib的方法*/
 /* 
int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, (uint8_t) ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}	
   
    return ch;
}
int GetKey (void)  { 

    while (!(USART1->SR & USART_FLAG_RXNE));

    return ((int)(USART1->DR & 0x1FF));
}
*/

//初始化IO 串口1 
//bound:波特率
void uart_init(u32 bound){
    //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
 	USART_DeInit(USART1);  //复位串口1
	 //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA9
   
    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA10

   //Usart1 NVIC 配置

  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

	USART_Init(USART1, &USART_InitStructure); //初始化串口
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启中断
	USART_Cmd(USART1, ENABLE);                    //使能串口 

}

unsigned char timebuf[SIZEBUF];//暂存串口数据
unsigned char clear=0;	 //串口buf清空计数
unsigned char count=0; 	 //串口接收计数
unsigned char readFlag=0;	//读取短信标志

extern unsigned char modeFlag; //模式

void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	u8 Res1;
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
		{
			Res1=USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据
			clear=0;      //空闲定时读取 清零
			count++;      //数据个数记录
			timebuf[count]=Res1;
			if(count>=(SIZEBUF-1))                  //连续接收16个字符信息
			{
				count=0;
				readFlag=1;               //接收完成标志位置1
			}
			
     } 
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntExit();  											 
#endif
} 


//初始化IO 串口2
//bound:波特率
void uart2_init(u32 bound)
{
    //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 , ENABLE);	//使能USART1，GPIOA时钟
	
 	USART_DeInit(USART2);  //复位串口1
	 //USART1_TX   PA.2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA9
   
    //USART1_RX	  PA.3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA10

   //Usart1 NVIC 配置

  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

	USART_Init(USART2, &USART_InitStructure); //初始化串口
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断
	USART_Cmd(USART2, ENABLE);                    //使能串口 

}


void USART2_IRQHandler(void)                	//串口1中断服务程序
{
	u8 uart2_rbuf;
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
		{
			uart2_rbuf=USART_ReceiveData(USART2);//(USART1->DR);	//读取接收到的数据

			
		uart1_r_buf=uart2_rbuf;                      //提取接收buf
	  rev1_buf_busy=0x00;                         //判断 防止break问题
	  switch(recv1_step)
	  {
	  case STAGE_SOHE: if(uart1_r_buf == '$')     //具体参考GPS标准协议NMEA0183
	  {
	    rev1_buf_busy=0x01;
	    if(uart1_r_buf == '$')              //查看收到$
	    {
	      recv1_step=STAGE_TYPE;            //跳转到下一步
	      record1=0;                        //计数清零
	    }
	    else
	    {
	      recv1_step=STAGE_SOHE;        //恢复初始化
	      record1=0;
	    }
	  }
	  break;
	  case STAGE_TYPE: if(rev1_buf_busy == 0x00)
	  {

	    rev1_buf_busy=0x01;
	    temp1_buf[record1]=uart1_r_buf;
	    record1++;
	    if(record1 == 0x05)
	    {                                                 //确认开头$GPRMC
	      if((temp1_buf[0] == 'G') && (temp1_buf[1] == 'P') && (temp1_buf[2] == 'R') && (temp1_buf[3] == 'M') && (temp1_buf[4] == 'C'))
	      {
	        recv1_step=STAGE_NONE;    //跳转到下一步
	        record1=0;
	      } 
	      else
	      {
	        recv1_step=STAGE_SOHE;//恢复初始化
	        record1=0;
	      }
	    }
	  }
	  break;
	  case STAGE_NONE: if(rev1_buf_busy == 0x00)//接收数据格式:$GPRMC,054347.00,A,3202.04770,N,11846.23632,E,0.000,0.00,221013,,,A*67
	  {
	    rev1_buf_busy=0x01;
	    record1++;
	    if((record1 > 0x01) && (record1 < 0x08))                                                                                    
	    {
	      gps_infor_time[record1-2]=uart1_r_buf;			//存储时间					
	    }
	    if((uart1_r_buf == ',') && (record1 > 0x07) && (record1 < 0x010))   //||((uart1_r_buf == ',') && (record1==0x02))
	    {
	      record1=0xcc;
	    }
	    if(record1 ==  0xcd)
	    {
	      record1=0;
	      devide_flag=2;
	      speed_end=0x00;
	      dir_end=0x00;
	      if(uart1_r_buf == 'A')  //gps收到数据 且有效
	      { 
	        recv1_step=STAGE_DATA;    //跳转到下一步
	      }
	      else
	      {
	        sysmode_GPS=0;
	        recv1_step=STAGE_SOHE;    //无效恢复初始化
	        record1=0;
	      }
	    }
	  }
	  break;
	  case STAGE_DATA:  if(rev1_buf_busy == 0x00)
	  {
	    rev1_buf_busy=0x01;
	    record1++;
	    if(uart1_r_buf == ',')    //判断为逗号
	    { 
	      devide_flag++;      //逗号 次数记录
	      record1=0;
	    }
	    if(devide_flag == 3)
	    {
	      if((record1 > 0) && (record1 < 5))
	      {
	        gps_infor_weijing[record1-1]=uart1_r_buf;	    //存储经纬度 此处为纬度					
	      }
	      if((record1 > 5) && (record1 < 10))             //跳过小数点的存储
	      {
	        gps_infor_weijing[record1-2]=uart1_r_buf;	   //存储经纬度 此处为纬度														
	      }
	    }
	    if(devide_flag == 4)
	    {
	      if(record1 > 0)
	      {
	        ns_flag=uart1_r_buf;            //接受纬度 NS标志
	      }
	    }
	    if(devide_flag == 5)
	    {
	      if((record1 > 0) && (record1 < 6))
	      {
	        gps_infor_weijing[record1+7]=uart1_r_buf;	  //跳过小数点的存储										
	      }
	      if((record1 > 6) && (record1 < 11))                //
	      {
	        gps_infor_weijing[record1+6]=uart1_r_buf;       //存储经纬度	 此处为经度																	
	      }
	    }
	    if(devide_flag == 6)
	    {
	      if(record1 > 0)
	      {
	        ew_flag=uart1_r_buf;            //经纬度 EW标志
	      }
	    }
	    if(devide_flag == 7)
	    {
	      if(speed_end == 0x00)
	      {
	        if((record1 > 0) && (uart1_r_buf != '.'))
	        {
	          gps_infor_speed[record1-1]=uart1_r_buf;   //接受速率
	        }
	        else if(uart1_r_buf == '.')
	        {
	          record1--;
	          speed_end=0xff;
	        }
	      }
	      else if(speed_end == 0xff)
	      {
	        speed_end=0xfe;
	        gps_infor_speed[record1-1]=uart1_r_buf;
	        gps_infor_speed[3]=gps_infor_speed[record1-1];
	        gps_infor_speed[2]=gps_infor_speed[record1-2];
	        if(record1 > 2)
	        {
	          gps_infor_speed[1]=gps_infor_speed[record1-3];
	        }
	        else
	        {
	          gps_infor_speed[1]=0x30;
	        }
	        if(record1 > 3)
	        {
	          gps_infor_speed[0]=gps_infor_speed[record1-4];
	        }
	        else
	        {
	          gps_infor_speed[0]=0x30;
	        }
	      }
	    }
	    if(devide_flag == 8)
	    {
	      if(dir_end == 0x00)
	      {
	        if((record1 > 0) && (uart1_r_buf != '.'))
	        {
	          gps_infor_dir[record1-1]=uart1_r_buf;   //存储方向
	        }
	        else if(uart1_r_buf == '.')
	        {
	          record1--;
	          dir_end=0xff;
	        }
	      }
	      else if(dir_end == 0xff)
	      {
	        dir_end=0xfe;
	        if(record1 == 2)
	        {
	          gps_infor_dir[2]=gps_infor_dir[record1-2];
	          gps_infor_dir[1]=0x30;
	          gps_infor_dir[0]=0x30;
	        }
	        if(record1 == 3)
	        {
	          gps_infor_dir[2]=gps_infor_dir[record1-2];
	          gps_infor_dir[1]=gps_infor_dir[record1-3];
	          gps_infor_dir[0]=0x30;
	        }
	      }
	    }
	    if(devide_flag == 9)
	    {
	      if((record1 > 0) && (record1 < 7))
	      {
	        gps_infor_date[record1-1]=uart1_r_buf;
	      }
	    }
	    if(uart1_r_buf == 0x0d)
	    {
	      recv1_step=STAGE_SOHE;    //接受完成 信号确定
	      record1=0;                //恢复初始化状态 为下一次做准备
	      devide_flag=0;
	      sysmode_GPS=1;         //标志 GPS信号有效
	    }
	  }
	  break;
	  }			

		
    } 
} 


void UART_SendStr(USART_TypeDef* USARTx, char* str, u16 legth)
{	
	u16 im;
	for(im = 0; im < legth; im++ )
	{		
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
		USART_SendData(USARTx, *str);
		str++;
	}
}




