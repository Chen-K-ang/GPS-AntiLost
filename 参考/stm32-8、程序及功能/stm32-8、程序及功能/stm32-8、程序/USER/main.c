#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include <stdio.h>
#include "timer.h"
#include "key.h"
#include "lcd1602.h"
#include <stdio.h>

//ALIENTEK Mini STM32开发板范例代码15
//ADC实验  
//技术支持：www.openedv.com
//广州市星翼电子科技有限公司
#define INIT 		0x00
#define OPEN_01 	0x01
#define SOMEONE     0x09

unsigned char disFlag = 0;	   

unsigned long Mid_Du;       //经纬度处理 度
unsigned long Mid_Fen;      //经纬度处理  分
unsigned long Mid_Vale;     //经纬度处理 中间变量

char Lin0_No[16]="N:000.000000";//存储纬度
char Lin1_Ea[16]="E:000.000000";//存储经度

unsigned long seco_Beijing;//时间转化变量 秒
unsigned long minu_Beijing;//时间转化变量 分
unsigned long hour_Beijing;//时间转化变量  小时
unsigned long days_Beijing;//时间转化变量  天
unsigned long mont_Beijing;//时间转化变量 月
unsigned long year_Beijing;//时间转化变量 年

unsigned char monthrun_table[13]={0,31,29,31,30,31,30,31,31,30,31,30,31};//月份 天数 闰年
//1  2  3  4   5  6  7  8  9 10 11 12    
unsigned char monthmon_table[13]={0,31,28,31,30,31,30,31,31,30,31,30,31};//月份天数
//1  2  3  4   5  6  7  8  9 10 11 12 

char Crtl_Z[1]={0x1a};	   //发送短信标志

unsigned char stepNum  =0;	//运行步骤
unsigned int timeCount =0;//接收短信处理延时
char AT_CMGS[26]="AT+CMGS=\"";
unsigned char rebackMesFlag; //返回短信
unsigned char clearMesFlag =0;//定时清除短信
unsigned char readMesIng = 0;//读取短信中标志

char dis0[16]="2000-00-00   ";
char dis1[16]="00:00:00     ";

//unsigned char relayFlag=1;//
u8 rekey =0,disNum=0;//中间变量
void dealGps(void) ;//gps数据处理

int main(void)
 { 	
	unsigned char i ;	 
	delay_init();	    	 //延时函数初始化	  
	uart_init(115200);	 	//串口初始化为115200
	uart2_init(9600)	;
	 
  TIM3_Int_Init(499,7199);//50ms  	 
	
	LED_Init();		  		//初始化与LED连接的硬件接口 	 
	KEY_Init();
	 
	led_gsm =0 ; ligh =1;//上电初始化
	Lcd_GPIO_init();  //初始化lcd 接口
	Lcd_Init();		 //初始化函数
	delay_ms(200);
	ligh =0;//关闭led
	Lcd_Puts(0,0,(u8 *)Lin0_No);//显示				
	Lcd_Puts(0,1,(u8 *)Lin1_Ea);//显示
	 
	i=60;
	while(i--) delay_ms(100);//延时

	UART_SendStr(USART1 ,"AT+CMGF=1\r\n",11); //设置字符集
	i=7;
	while(i--) delay_ms(100);//延时
	UART_SendStr(USART1 ,"AT+CSCS=\"GSM\"\r\n",15);//设置文本
	i=7;
	while(i--) delay_ms(100);//延时

	UART_SendStr(USART1 , "AT+CMGDA=\"DEL ALL\"\r\n",20); //删除所有短信
	for(i=0;i<5;i++)
	delay_ms(100);          //延时有助于稳定
	
  led_gsm = 1;
	while(1)
	{	
		if((key_c==0)||(key_l==0))//按键按下
		{
			if(rekey == 0)
			{
				delay_ms(10);
				if((key_c==0)||(key_l==0))//确认按键
				{
						if(key_c==0)		//按键显示切换
						{
							rekey =1;
							if(disNum==0){disNum=1;}
							else {disNum=0;}
						}
						else if(key_l==0)		//led灯开关处理
						{ligh=!ligh;rekey=1;}					
				}
			}		
		}
		else
		{rekey =0;}
		
		if(disFlag ==1)	//定时更新显示
		{
			disFlag =0;
	    if(sysmode_GPS==1)		//gps有效
      {
//        sysmode_GPS=0;	
				dealGps();
				if(disNum==0)
				{
					Lcd_Puts(0,0,(u8 *)Lin0_No);//显示				
					Lcd_Puts(0,1,(u8 *)Lin1_Ea);//显示				
				}
				else
				{
					Lcd_Puts(0,0,(u8 *)dis0);//显示				
					Lcd_Puts(0,1,(u8 *)dis1);//显示							
				}	
			}					
		}
		
    if(key_m==0)	//发送按键按下
    {
			delay_ms(10);
			if(key_m==0)	
			{			
				led_gsm =  0; 
				UART_SendStr(USART1 ,"AT+CMGF=1\r\n",11); //设置字符集
				i=3;
				while(i--) delay_ms(100);//延时
				UART_SendStr(USART1 ,"AT+CSCS=\"GSM\"\r\n",15);//设置文本
				i=3;
				while(i--) delay_ms(100);//延时

				UART_SendStr(USART1 ,"AT+CMGS=\"+8618105140357\"\r\n",26); //测试号码        				
//		  UART_SendStr(USART1 ,"AT+CMGS=\"+8615296556420\"\r\n",26); //发送短信号码
				
				i=3;
				while(i--) delay_ms(100);//延时
				if(sysmode_GPS==1)		//gps有效
				{
					sysmode_GPS=0;			
					dealGps();
					UART_SendStr(USART1 ,Lin0_No,12); //发送纬度
					UART_SendStr(USART1 ,Lin1_Ea,12); //发送经度
				}
				else
				{
					UART_SendStr(USART1 ,"gps linking...",14); //gps无信号
				}
				delay_ms(200);
				UART_SendStr(USART1 ,( char *)Crtl_Z, 1);        //发送
				i=30;
				while(i--) delay_ms(100);//延时发送
				led_gsm = 1;
			}
		}
		
		if(readFlag==1)    //读取短信息标志
	  {	     
			readFlag=0;	 //读取标志清零
			readMesIng = 1;//读取短信过程中
			timeCount++ ;
			if((stepNum == 0)&&(timeCount>=4))	//定时读取短信
			{					
				UART_SendStr(USART1 , "AT+CMGF=1\r\n",11);   //设置字符集
				timeCount = 0;//延时设置量	
				stepNum++;//进入操作过程1
			}			
			else if((stepNum == 1)&&(timeCount>=4))	//定时读取短信
			{
				for(i=0;i<SIZEBUF;i++)
				{
					timebuf[i]='0';   //清空timebuf
				}					
				UART_SendStr(USART1 ,"AT+CMGR=1\r\n",11);	
				timeCount = 0;//延时设置量	
				stepNum++;//进入操作过程1
			}
			else if((stepNum == 2)&&(timeCount >= 4))	//定时读取短信
			{	
				rebackMesFlag=INIT;	
				for(i=0;i<(SIZEBUF-3);i++)
				{
					if((timebuf[i]=='G')&&(timebuf[i+1]=='E')&&(timebuf[i+2]=='T'))	//查询是否接收到GET
					{
						rebackMesFlag=OPEN_01;	//状态标识打开
						break ;
					}
					if((timebuf[i]=='+')&&(timebuf[i+1]=='8')&&(timebuf[i+2]=='6'))//查询短信息中的号码
					{
						led_gsm = 0;
						rebackMesFlag = SOMEONE; //有人发送短信 
						AT_CMGS[9]=timebuf[i+0]; //提取号码
						AT_CMGS[10]=timebuf[i+1];
						AT_CMGS[11]=timebuf[i+2];
						AT_CMGS[12]=timebuf[i+3];
						AT_CMGS[13]=timebuf[i+4];
						AT_CMGS[14]=timebuf[i+5];
						AT_CMGS[15]=timebuf[i+6];
						AT_CMGS[16]=timebuf[i+7];
						AT_CMGS[17]=timebuf[i+8];
						AT_CMGS[18]=timebuf[i+9];
						AT_CMGS[19]=timebuf[i+10];
						AT_CMGS[20]=timebuf[i+11];         
						AT_CMGS[21]=timebuf[i+12];
						AT_CMGS[22]=timebuf[i+13];
					}
				}
				AT_CMGS[23]='"';	 //整理字符串 
				AT_CMGS[24]=0x0d;
				AT_CMGS[25]=0x0a;
	
				if(rebackMesFlag != INIT)
				{		
					UART_SendStr(USART1 ,"AT+CMGDA=\"DEL ALL\"\r\n",20);//删除所有短信															
					for(i=0;i<5;i++)
					delay_ms(100);          //延时有助于稳定
							
					UART_SendStr(USART1 ,"AT+CMGF=1\r\n",11);   //设置字符集
					for(i=0;i<3;i++)
					delay_ms(100);          //延时有助于稳定
				
					UART_SendStr(USART1 ,"AT+CSCS=\"GSM\"\r\n",15); //设置干什么、
					for(i=0;i<3;i++)
					delay_ms(100);          //延时有助于稳定
								
			   	UART_SendStr(USART1 , ( char *)AT_CMGS,26);//准备发送短信
					for(i=0;i<2;i++)
					delay_ms(100);          //延时有助于稳定 
	
					if(rebackMesFlag==OPEN_01)
					{
						UART_SendStr(USART1 ,Lin0_No,12); //发送纬度
						UART_SendStr(USART1 ,Lin1_Ea,12); //发送经度											
					}
					else
					{
						UART_SendStr(USART1 ,"CMD_ERR",7); //发送短信息命令错误
					}
					delay_ms(100);  
			    UART_SendStr(USART1 ,(char *)Crtl_Z, 1);        //发送
					for(i=0;i<20;i++)
					delay_ms(100);          //延时有助于稳定	
				}
				timeCount = 0;//延时设置量	
				stepNum++	;//进入操作过程1
			}
			else if((stepNum == 3)&&(timeCount >= 4))	//定时读取短信
			{
				clearMesFlag++;
				if(clearMesFlag>10)
				{
				  clearMesFlag = 0;
					UART_SendStr(USART1 ,"AT+CMGDA=\"DEL ALL\"\r\n",20);//删除所有短信
				}
				timeCount = 0;//延时设置量	
				stepNum = 0	;//进入操作过程1							
				led_gsm=1;	  //关闭指示灯
				readMesIng = 0;//读取短信过程中
			}								 		   
		 }
					
	}											    
}

void dealGps(void)
{		
  seco_Beijing=(gps_infor_time[4]-0x30)*10+(gps_infor_time[5]-0x30);        //提取时间 秒   
  minu_Beijing=(gps_infor_time[2]-0x30)*10+(gps_infor_time[3]-0x30);//提取时间 分
  hour_Beijing=((gps_infor_time[0]-0x30)*10+(gps_infor_time[1]-0x30))+8;//提取时间 小时   
  days_Beijing=(gps_infor_date[0]-0x30)*10+(gps_infor_date[1]-0x30);//提取时间 天  
  mont_Beijing=(gps_infor_date[2]-0x30)*10+(gps_infor_date[3]-0x30);//提取时间 月   
  year_Beijing=(gps_infor_date[4]-0x30)*10+(gps_infor_date[5]-0x30);//提取时间 年   
  
  if(hour_Beijing>=24)	//北京时间转换
  {
    hour_Beijing=hour_Beijing%24;
    days_Beijing=days_Beijing+1;
    if(year_Beijing%4==0)            
    {
      if(days_Beijing>=monthrun_table[mont_Beijing])//天数超过该月份
      {
        days_Beijing=1;
        mont_Beijing++;
        if(mont_Beijing>=12)
        {
          mont_Beijing=1;
          year_Beijing++;   
        }
      }
    }
    if(year_Beijing%4!=0)              
    {
      if(days_Beijing>=monthmon_table[mont_Beijing])//是否夸年
      {
        days_Beijing=1;
        mont_Beijing++;
        if(mont_Beijing>=12)
        {
          mont_Beijing=1;
          year_Beijing++;   
        }
      }
    } 
  }
	sprintf(dis0,"20%02d-%02d-%02d   ",(u16)year_Beijing,(u16)mont_Beijing,(u16)days_Beijing);//打印日期
	sprintf(dis1,"%02d:%02d:%02d      ",(u16)hour_Beijing,(u16)minu_Beijing,(u16)seco_Beijing);//打印时间
	
	Mid_Du=(gps_infor_weijing[0]-0x30)*10000000+(gps_infor_weijing[1]-0x30)*1000000;    //处理经度扩大10000000
	
	Mid_Fen=(gps_infor_weijing[2]-0x30)*10000000+(gps_infor_weijing[3]-0x30)*1000000+
		(gps_infor_weijing[4]-0x30)*100000+(gps_infor_weijing[5]-0x30)*10000+
			(gps_infor_weijing[6]-0x30)*1000+(gps_infor_weijing[7]-0x30)*100;          
	Mid_Fen=Mid_Fen/60;                                                      //分秒换算为小数位
	Mid_Vale=Mid_Du+Mid_Fen;         //最终为度格式000.00000000 非度分秒格式
	Lin0_No[0]='N';                  
	Lin0_No[1]=':';                  
	Lin0_No[2]='0';                  
	Lin0_No[3]=Mid_Vale/10000000+0x30;                  //转化为字符
	Lin0_No[4]=(Mid_Vale/1000000)%10+0x30;
	Lin0_No[5]='.';
	Lin0_No[6]=(Mid_Vale/100000)%10+0x30;
	Lin0_No[7]=(Mid_Vale/10000)%10+0x30;
	Lin0_No[8]=(Mid_Vale/1000)%10+0x30;
	Lin0_No[9]=(Mid_Vale/100)%10+0x30;
	Lin0_No[10]=(Mid_Vale/10)%10+0x30;
	Lin0_No[11]=Mid_Vale%10+0x30;

	Mid_Du=(gps_infor_weijing[8]-0x30)*100000000+(gps_infor_weijing[9]-0x30)*10000000+(gps_infor_weijing[10]-0x30)*1000000; //处理经度扩大10000000     

	Mid_Fen=(gps_infor_weijing[11]-0x30)*10000000+(gps_infor_weijing[12]-0x30)*1000000+
		(gps_infor_weijing[13]-0x30)*100000+(gps_infor_weijing[14]-0x30)*10000+
		(gps_infor_weijing[15]-0x30)*1000+(gps_infor_weijing[16]-0x30)*100; 
	Mid_Fen=Mid_Fen/60;                                                //分秒换算为小数位
	Mid_Vale=Mid_Du+Mid_Fen;                                          //最终为度格式000.00000000 非度分秒格式
	Lin1_Ea[0]='E';                  
	Lin1_Ea[1]=':';     
	Lin1_Ea[2]=Mid_Vale/100000000+0x30;                           //转化为字符
	Lin1_Ea[3]=(Mid_Vale/10000000)%10+0x30;
	Lin1_Ea[4]=(Mid_Vale/1000000)%10+0x30;
	Lin1_Ea[5]='.';
	Lin1_Ea[6]=(Mid_Vale/100000)%10+0x30;
	Lin1_Ea[7]=(Mid_Vale/10000)%10+0x30;
	Lin1_Ea[8]=(Mid_Vale/1000)%10+0x30;
	Lin1_Ea[9]=(Mid_Vale/100)%10+0x30;
	Lin1_Ea[10]=(Mid_Vale/10)%10+0x30;
	Lin1_Ea[11]=Mid_Vale%10+0x30;
}

