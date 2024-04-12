#include <string.h>
#include "delay.h"
#include "sys.h"
#include "timer.h"
#include "key.h"
#include "usart.h"
#include "deal.h"
#include "1602.h"

#define INIT 	 0x00
#define OPEN_01  0x01
#define SOMEONE  0x09


u8 portA4_Array[]="OUT LIM";

unsigned char BufTab[10];     //wifi数据暂存
unsigned char Count;          //串口数据计数
unsigned char UartBusy=0;     //串口忙
unsigned char sendDataFlag=0; //发送短信标志
unsigned char i;
unsigned char diplayFlag=0;   //读取显示标志

unsigned char time_count = 3;
unsigned char begin = 0; //开始定位
unsigned char dealgps;  //定时处理围栏限制数据
unsigned char ErrorNum=0;   //记录错误次数
unsigned char CheckNum=0;   //检测次数
unsigned long ReportLater=0;//上报延时
unsigned long KeyLater=3;   //按键延时计数
unsigned long Mid_Du;       //经纬度处理 度
unsigned long Mid_Fen;      //经纬度处理 分
unsigned long Mid_Vale;     //经纬度处理 中间变量

double lat1 = 0.0f, lat2 = 0.0f;  //1储存初始 lat经度
double lng1 = 0.0f, lng2 = 0.0f;  //2储存当前 lng维度
double distance = 0, dis_limit = 0; 

u8 Lin0_No[16]="N:000.000000";//存储纬度
u8 Lin1_Ea[16]="E:000.000000";//存储经度
u8 Lin2_No[16]="N:000.000000";//存储纬度
u8 Lin3_Ea[16]="E:000.000000";//存储经度

char Crtl_Z[1]={0x1a};	//发送短信标志

u8 number_Array[]="0123456789.V";
u8 wan,qian,bai,shi,ge;

unsigned char stepNum  =0;	//运行步骤
unsigned int timeCount =0;//接收短信处理延时
char AT_CMGS[26]="AT+CMGS=\"";
unsigned char rebackMesFlag; //返回短信
unsigned char clearMesFlag =0;//定时清除短信
unsigned char readMesIng = 0;//读取短信中标志

void dealGps(double lat, double lng);//gps数据处理
void key_deal(void);
u8 display(void);
void read_gsm_note(void); //读取短信

void Split(u16 count)//将各位数字拆开函数
{ 
	wan=count/10000;//求万位数字
	qian=count%10000/1000;//求千位数字
	bai=count%1000/100;//求百位数字
	shi=count%100/10;//求十位数字
	ge=count%10;//求各位数字  
}

int main(void)
{	
	delay_init();          //延时函数初始化
	NVIC_Configuration();  //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	KEY_Init();
//	GPIO_InitStructReadtempCmd(); //lcd
//	lcd_system_reset();
	
	LCD1602_Init();

	uart_init(9600);
	uart2_init(9600);

	TIM3_Int_Init(71,1000); //10Khz的计数频率，计数到500为50ms
	
	Timer_Init();
	
	
	 
	dealGps(lat2, lng2);
	lat1 = lat2; lng1 = lng2;
	i=5;
	while(i--) delay_ms(100);//延时

	UART_SendStr(USART1 ,"AT+CMGF=1\r\n",11); //设置字符集
	i=10;
	while(i--) delay_ms(100);//延时
	
	UART_SendStr(USART1 ,"AT+CSCS=\"GSM\"\r\n",15);//设置文本
	i=10;
	while(i--) delay_ms(100);//延时

	while(1) {	
		
		if (dealgps == 1 && begin) {   //定时处理数据
			dealgps = 0;
			distance = get_distance(lat1, lng1, lat2, lng2);    //检测距离函数
			CheckNum++;
			if(distance > dis_limit){            //判断 查看正常次数

				LCD_Write_String(0,0, portA4_Array);
				
				ErrorNum++;
				if (CheckNum>=5)                      //进行5次处理
					if (KeyLater>=3) {             //非按键下
						if (ErrorNum>=1){       //五次处理有一次出
							UART_SendStr(USART1 ,"AT+CMGDA=\"DEL ALL\"\r\n",20);//删除所有短信															
							for(i=0;i<5;i++)
								delay_ms(100);//延时有助于稳定

							UART_SendStr(USART1 ,"AT+CMGF=1\r\n",11);   //设置字符集
							for(i=0;i<3;i++)
								delay_ms(100);//延时有助于稳定

							UART_SendStr(USART1 ,"AT+CSCS=\"GSM\"\r\n",15); //设置干什么
							for(i=0;i<3;i++)
								delay_ms(100);//延时有助于稳定

							UART_SendStr(USART1 ,"AT+CMGSM=\"+8615633691336\"\r\n",26); //测试号15633691336
							for(i=0;i<2;i++)
								delay_ms(100);//延时有助于稳定 

							UART_SendStr(USART1 ,"LOST",7); //发送超过围墙消息“LOST”
							
							delay_ms(100);
							UART_SendStr(USART1 ,(char *)Crtl_Z, 1);//发送
							for(i=0;i<20;i++)
								delay_ms(100);//延时有助于稳定	
						}
						else {
							ReportLater=0; //上报延时计数
						}
						ErrorNum=0;            //清空滤波计数
						CheckNum=0;
					}
			}

			if(ReportLater >= time_count) {	//默认是3s刷新
				UART_SendStr(USART1 ,"AT+CMGF=1\r\n",11); //设置字符集
				i=3;
				while(i--) delay_ms(100);//延时

				UART_SendStr(USART1 ,"AT+CSCS=\"GSM\"\r\n",15);//设置文本
				i=3;
				while(i--) delay_ms(100);//延时
				//测试号//**All notes can be deleted and modified**//
				UART_SendStr(USART1 ,"AT+CMGSM=\"+8615633691336\"\r\n",26); 

				i=2;
				while(i--) delay_ms(100);//延时

				UART_SendStr(USART1 ,"The device is out!",8); //发送出范围信息		

				if(sysmode_GPS==1){		//gps有效
					sysmode_GPS=0;
					dealGps(lat2, lng2);	
				} else
					UART_SendStr(USART1 ,"gps linking...",14); //gps无信号
				delay_ms(200);//延时

				UART_SendStr(USART1 ,( char *)Crtl_Z, 1); //发送
				i=40;
				while(i--) delay_ms(100);//延时

				ReportLater=0;  //上报标志清空
				KeyLater=0;     //按键清零延时处理
			}
			read_gsm_note(); //读取短信操作
			display();  //lcd显示
		}
	}
}

void dealGps(double lat, double lng)
{
	u8 j;
	
	Mid_Du=(gps_infor_weijing[0]-0x30)*10000000+(gps_infor_weijing[1]-0x30)*1000000;    //处理经度扩大10000000
	
	Mid_Fen=(gps_infor_weijing[2]-0x30)*10000000+(gps_infor_weijing[3]-0x30)*1000000+
		(gps_infor_weijing[4]-0x30)*100000+(gps_infor_weijing[5]-0x30)*10000+
			(gps_infor_weijing[6]-0x30)*1000+(gps_infor_weijing[7]-0x30)*100;          
	Mid_Fen=Mid_Fen/60;              //分秒换算为小数位
	Mid_Vale=Mid_Du+Mid_Fen;         //最终为度格式000.00000000 非度分秒格式
	lat = Mid_Vale;
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
	Mid_Vale=Mid_Du+Mid_Fen;  //最终为度格式000.00000000 非度分秒格式
	lng = Mid_Vale;
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
	
	for(j=0;j<16;j++)
	{
		Lin2_No[j]=Lin0_No[j];
		Lin3_Ea[j]=Lin1_Ea[j];
	}
}

void key_deal(void)
{
	if(key1 == 0){ //定位开始
		delay_ms(3);
		if(key1 ==0){
			dealGps(lat1, lng1);
			begin = 1;
		}
	} else if(key2==0){ //11米
		delay_ms(3);
		if(key2 ==0){
			if (dis_limit > 11 && dis_limit < 111)
				dis_limit=dis_limit+11;
			else
				dis_limit = 11;
			diplayFlag = 1;
			Split(dis_limit);//调用数值拆分函数
			
		}
	} else if(key3==0){ //111米
		delay_ms(3);
		if(key3 ==0){
			if (dis_limit >= 111 && dis_limit < 800)
				dis_limit=dis_limit+111;
			else
				dis_limit = 111;
			diplayFlag = 1;
			Split(dis_limit);//调用数值拆分函数
			
		}
	} else if(key4==0){ //默认800
		delay_ms(3);
		if(key4 ==0){
			dis_limit = 800;
			diplayFlag = 1;
			Split(dis_limit);//调用数值拆分函数
		}
	} else if(key5==0){  //减小刷新时间
		delay_ms(3);
		if(key5 ==0){
			time_count++; 
		}
	} else if(key6==0){ //加快刷新时间
		delay_ms(3);
		if(key6 ==0){
			time_count--;
			if (time_count == 0)
				time_count = 1;
		}
	}
}

u8 display(void)
{
	if (diplayFlag) {
		diplayFlag = 0;
		LCD1602_Write_Cmd(0x01);
		delay_ms(20);
		LCD1602_Write_Cmd(0x80);//设置显示地址为LCD第二行，一旦首地址确定，显示完第一个数字后，光标会自动加1右移  
		LCD1602_Write_Data(number_Array[qian]);//第一位显示千位
		LCD1602_Write_Data(number_Array[bai]);
		LCD1602_Write_Data(number_Array[shi]);
		LCD1602_Write_Data(number_Array[ge]);
		delay_ms(1000);
		return 0;
	}
	LCD_Write_String(0, 0, Lin0_No);
	LCD_Write_String(0, 1, Lin1_Ea);
	
	return 1;
}

void read_gsm_note(void)
{
	if(readFlag==1){    //读取短信息标志
		     
		readFlag=0;	 //读取标志清零
		readMesIng = 1;//读取短信过程中
		timeCount++ ;
		if((stepNum == 0)&&(timeCount>=4)){	//定时读取短信
							
			UART_SendStr(USART1 , "AT+CMGF=1\r\n",11);   //设置字符集
			timeCount = 0;//延时设置量	
			stepNum++;//进入操作过程1
		} else if((stepNum == 1)&&(timeCount>=4)){	//定时读取短信
	
			for(i=0;i<SIZEBUF;i++)
				timebuf[i]='0';   //清空timebuf					
			UART_SendStr(USART1 ,"AT+CMGR=1\r\n",11);	
			timeCount = 0;//延时设置量	
			stepNum++;//进入操作过程1
		}else if((stepNum == 2)&&(timeCount >= 4)){	//定时读取短信
		
			rebackMesFlag=INIT;	
			for(i=0;i<(SIZEBUF-3);i++){
				if((timebuf[i]=='G')&&(timebuf[i+1]=='E')&&(timebuf[i+2]=='T')){//查询是否接收到GET
					rebackMesFlag=OPEN_01;	//状态标识打开
					break ;
				}
				if((timebuf[i]=='+')&&(timebuf[i+1]=='8')&&(timebuf[i+2]=='6')){//查询短信息中的号码	
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

			if(rebackMesFlag != INIT){
				UART_SendStr(USART1 ,"AT+CMGDA=\"DEL ALL\"\r\n",20);//删除所有短信															
				for(i=0;i<5;i++)
					delay_ms(100);//延时有助于稳定

				UART_SendStr(USART1 ,"AT+CMGF=1\r\n",11);   //设置字符集
				for(i=0;i<3;i++)
					delay_ms(100);//延时有助于稳定

				UART_SendStr(USART1 ,"AT+CSCS=\"GSM\"\r\n",15); //设置干什么
				for(i=0;i<3;i++)
					delay_ms(100);//延时有助于稳定

				UART_SendStr(USART1 , ( char *)AT_CMGS,26);//准备发送短信
				for(i=0;i<2;i++)
					delay_ms(100);//延时有助于稳定 

				if(rebackMesFlag==OPEN_01){
					UART_SendStr(USART1, (char*)Lin0_No,12); //发送纬度
					UART_SendStr(USART1, (char*)Lin1_Ea,12); //发送经度											
				}else{
					UART_SendStr(USART1 ,"CMD_ERR",7); //发送短信息命令错误
				}
				delay_ms(100);
				UART_SendStr(USART1 ,(char *)Crtl_Z, 1);//发送
				for(i=0;i<20;i++)
					delay_ms(100);//延时有助于稳定	
			}
			timeCount = 0;//延时设置量	
			stepNum++;//进入操作过程1
		}else if((stepNum == 3)&&(timeCount >= 4)){	//定时读取短信
	
			clearMesFlag++;
			if(clearMesFlag>10){
				clearMesFlag = 0;
				UART_SendStr(USART1 ,"AT+CMGDA=\"DEL ALL\"\r\n",20);//删除所有短信
			}
			timeCount = 0;//延时设置量	
			stepNum = 0	;//进入操作过程1							
			readMesIng = 0;//读取短信过程中
		}							 		   
	}
}

void TIM1_UP_IRQHandler(void)
{
	static u32 time50ms = 0;
	
	if(TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
	{
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);  //清除TIMx更新中断标志 
		
		time50ms++;
		if(time50ms%5==0)
		{
			dealgps =1;
			if(time50ms%10==0)
				sendDataFlag =1;
			if(time50ms%20==0)  //1s处理一次
			{
				ReportLater++;
				KeyLater++;
			}
		}
		key_deal(); //按键处理
		if(time50ms%40==0)  //2s处理一次
		{
//			if(diplayFlag)
//				diplayFlag = 0;
//			else
//				diplayFlag = 1;
		}
		if (time50ms > 10000)
			time50ms = 0;
	}
}
