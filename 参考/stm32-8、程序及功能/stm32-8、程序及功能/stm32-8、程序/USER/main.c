#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include <stdio.h>
#include "timer.h"
#include "key.h"
#include "lcd1602.h"
#include <stdio.h>

//ALIENTEK Mini STM32�����巶������15
//ADCʵ��  
//����֧�֣�www.openedv.com
//������������ӿƼ����޹�˾
#define INIT 		0x00
#define OPEN_01 	0x01
#define SOMEONE     0x09

unsigned char disFlag = 0;	   

unsigned long Mid_Du;       //��γ�ȴ��� ��
unsigned long Mid_Fen;      //��γ�ȴ���  ��
unsigned long Mid_Vale;     //��γ�ȴ��� �м����

char Lin0_No[16]="N:000.000000";//�洢γ��
char Lin1_Ea[16]="E:000.000000";//�洢����

unsigned long seco_Beijing;//ʱ��ת������ ��
unsigned long minu_Beijing;//ʱ��ת������ ��
unsigned long hour_Beijing;//ʱ��ת������  Сʱ
unsigned long days_Beijing;//ʱ��ת������  ��
unsigned long mont_Beijing;//ʱ��ת������ ��
unsigned long year_Beijing;//ʱ��ת������ ��

unsigned char monthrun_table[13]={0,31,29,31,30,31,30,31,31,30,31,30,31};//�·� ���� ����
//1  2  3  4   5  6  7  8  9 10 11 12    
unsigned char monthmon_table[13]={0,31,28,31,30,31,30,31,31,30,31,30,31};//�·�����
//1  2  3  4   5  6  7  8  9 10 11 12 

char Crtl_Z[1]={0x1a};	   //���Ͷ��ű�־

unsigned char stepNum  =0;	//���в���
unsigned int timeCount =0;//���ն��Ŵ�����ʱ
char AT_CMGS[26]="AT+CMGS=\"";
unsigned char rebackMesFlag; //���ض���
unsigned char clearMesFlag =0;//��ʱ�������
unsigned char readMesIng = 0;//��ȡ�����б�־

char dis0[16]="2000-00-00   ";
char dis1[16]="00:00:00     ";

//unsigned char relayFlag=1;//
u8 rekey =0,disNum=0;//�м����
void dealGps(void) ;//gps���ݴ���

int main(void)
 { 	
	unsigned char i ;	 
	delay_init();	    	 //��ʱ������ʼ��	  
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200
	uart2_init(9600)	;
	 
  TIM3_Int_Init(499,7199);//50ms  	 
	
	LED_Init();		  		//��ʼ����LED���ӵ�Ӳ���ӿ� 	 
	KEY_Init();
	 
	led_gsm =0 ; ligh =1;//�ϵ��ʼ��
	Lcd_GPIO_init();  //��ʼ��lcd �ӿ�
	Lcd_Init();		 //��ʼ������
	delay_ms(200);
	ligh =0;//�ر�led
	Lcd_Puts(0,0,(u8 *)Lin0_No);//��ʾ				
	Lcd_Puts(0,1,(u8 *)Lin1_Ea);//��ʾ
	 
	i=60;
	while(i--) delay_ms(100);//��ʱ

	UART_SendStr(USART1 ,"AT+CMGF=1\r\n",11); //�����ַ���
	i=7;
	while(i--) delay_ms(100);//��ʱ
	UART_SendStr(USART1 ,"AT+CSCS=\"GSM\"\r\n",15);//�����ı�
	i=7;
	while(i--) delay_ms(100);//��ʱ

	UART_SendStr(USART1 , "AT+CMGDA=\"DEL ALL\"\r\n",20); //ɾ�����ж���
	for(i=0;i<5;i++)
	delay_ms(100);          //��ʱ�������ȶ�
	
  led_gsm = 1;
	while(1)
	{	
		if((key_c==0)||(key_l==0))//��������
		{
			if(rekey == 0)
			{
				delay_ms(10);
				if((key_c==0)||(key_l==0))//ȷ�ϰ���
				{
						if(key_c==0)		//������ʾ�л�
						{
							rekey =1;
							if(disNum==0){disNum=1;}
							else {disNum=0;}
						}
						else if(key_l==0)		//led�ƿ��ش���
						{ligh=!ligh;rekey=1;}					
				}
			}		
		}
		else
		{rekey =0;}
		
		if(disFlag ==1)	//��ʱ������ʾ
		{
			disFlag =0;
	    if(sysmode_GPS==1)		//gps��Ч
      {
//        sysmode_GPS=0;	
				dealGps();
				if(disNum==0)
				{
					Lcd_Puts(0,0,(u8 *)Lin0_No);//��ʾ				
					Lcd_Puts(0,1,(u8 *)Lin1_Ea);//��ʾ				
				}
				else
				{
					Lcd_Puts(0,0,(u8 *)dis0);//��ʾ				
					Lcd_Puts(0,1,(u8 *)dis1);//��ʾ							
				}	
			}					
		}
		
    if(key_m==0)	//���Ͱ�������
    {
			delay_ms(10);
			if(key_m==0)	
			{			
				led_gsm =  0; 
				UART_SendStr(USART1 ,"AT+CMGF=1\r\n",11); //�����ַ���
				i=3;
				while(i--) delay_ms(100);//��ʱ
				UART_SendStr(USART1 ,"AT+CSCS=\"GSM\"\r\n",15);//�����ı�
				i=3;
				while(i--) delay_ms(100);//��ʱ

				UART_SendStr(USART1 ,"AT+CMGS=\"+8618105140357\"\r\n",26); //���Ժ���        				
//		  UART_SendStr(USART1 ,"AT+CMGS=\"+8615296556420\"\r\n",26); //���Ͷ��ź���
				
				i=3;
				while(i--) delay_ms(100);//��ʱ
				if(sysmode_GPS==1)		//gps��Ч
				{
					sysmode_GPS=0;			
					dealGps();
					UART_SendStr(USART1 ,Lin0_No,12); //����γ��
					UART_SendStr(USART1 ,Lin1_Ea,12); //���;���
				}
				else
				{
					UART_SendStr(USART1 ,"gps linking...",14); //gps���ź�
				}
				delay_ms(200);
				UART_SendStr(USART1 ,( char *)Crtl_Z, 1);        //����
				i=30;
				while(i--) delay_ms(100);//��ʱ����
				led_gsm = 1;
			}
		}
		
		if(readFlag==1)    //��ȡ����Ϣ��־
	  {	     
			readFlag=0;	 //��ȡ��־����
			readMesIng = 1;//��ȡ���Ź�����
			timeCount++ ;
			if((stepNum == 0)&&(timeCount>=4))	//��ʱ��ȡ����
			{					
				UART_SendStr(USART1 , "AT+CMGF=1\r\n",11);   //�����ַ���
				timeCount = 0;//��ʱ������	
				stepNum++;//�����������1
			}			
			else if((stepNum == 1)&&(timeCount>=4))	//��ʱ��ȡ����
			{
				for(i=0;i<SIZEBUF;i++)
				{
					timebuf[i]='0';   //���timebuf
				}					
				UART_SendStr(USART1 ,"AT+CMGR=1\r\n",11);	
				timeCount = 0;//��ʱ������	
				stepNum++;//�����������1
			}
			else if((stepNum == 2)&&(timeCount >= 4))	//��ʱ��ȡ����
			{	
				rebackMesFlag=INIT;	
				for(i=0;i<(SIZEBUF-3);i++)
				{
					if((timebuf[i]=='G')&&(timebuf[i+1]=='E')&&(timebuf[i+2]=='T'))	//��ѯ�Ƿ���յ�GET
					{
						rebackMesFlag=OPEN_01;	//״̬��ʶ��
						break ;
					}
					if((timebuf[i]=='+')&&(timebuf[i+1]=='8')&&(timebuf[i+2]=='6'))//��ѯ����Ϣ�еĺ���
					{
						led_gsm = 0;
						rebackMesFlag = SOMEONE; //���˷��Ͷ��� 
						AT_CMGS[9]=timebuf[i+0]; //��ȡ����
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
				AT_CMGS[23]='"';	 //�����ַ��� 
				AT_CMGS[24]=0x0d;
				AT_CMGS[25]=0x0a;
	
				if(rebackMesFlag != INIT)
				{		
					UART_SendStr(USART1 ,"AT+CMGDA=\"DEL ALL\"\r\n",20);//ɾ�����ж���															
					for(i=0;i<5;i++)
					delay_ms(100);          //��ʱ�������ȶ�
							
					UART_SendStr(USART1 ,"AT+CMGF=1\r\n",11);   //�����ַ���
					for(i=0;i<3;i++)
					delay_ms(100);          //��ʱ�������ȶ�
				
					UART_SendStr(USART1 ,"AT+CSCS=\"GSM\"\r\n",15); //���ø�ʲô��
					for(i=0;i<3;i++)
					delay_ms(100);          //��ʱ�������ȶ�
								
			   	UART_SendStr(USART1 , ( char *)AT_CMGS,26);//׼�����Ͷ���
					for(i=0;i<2;i++)
					delay_ms(100);          //��ʱ�������ȶ� 
	
					if(rebackMesFlag==OPEN_01)
					{
						UART_SendStr(USART1 ,Lin0_No,12); //����γ��
						UART_SendStr(USART1 ,Lin1_Ea,12); //���;���											
					}
					else
					{
						UART_SendStr(USART1 ,"CMD_ERR",7); //���Ͷ���Ϣ�������
					}
					delay_ms(100);  
			    UART_SendStr(USART1 ,(char *)Crtl_Z, 1);        //����
					for(i=0;i<20;i++)
					delay_ms(100);          //��ʱ�������ȶ�	
				}
				timeCount = 0;//��ʱ������	
				stepNum++	;//�����������1
			}
			else if((stepNum == 3)&&(timeCount >= 4))	//��ʱ��ȡ����
			{
				clearMesFlag++;
				if(clearMesFlag>10)
				{
				  clearMesFlag = 0;
					UART_SendStr(USART1 ,"AT+CMGDA=\"DEL ALL\"\r\n",20);//ɾ�����ж���
				}
				timeCount = 0;//��ʱ������	
				stepNum = 0	;//�����������1							
				led_gsm=1;	  //�ر�ָʾ��
				readMesIng = 0;//��ȡ���Ź�����
			}								 		   
		 }
					
	}											    
}

void dealGps(void)
{		
  seco_Beijing=(gps_infor_time[4]-0x30)*10+(gps_infor_time[5]-0x30);        //��ȡʱ�� ��   
  minu_Beijing=(gps_infor_time[2]-0x30)*10+(gps_infor_time[3]-0x30);//��ȡʱ�� ��
  hour_Beijing=((gps_infor_time[0]-0x30)*10+(gps_infor_time[1]-0x30))+8;//��ȡʱ�� Сʱ   
  days_Beijing=(gps_infor_date[0]-0x30)*10+(gps_infor_date[1]-0x30);//��ȡʱ�� ��  
  mont_Beijing=(gps_infor_date[2]-0x30)*10+(gps_infor_date[3]-0x30);//��ȡʱ�� ��   
  year_Beijing=(gps_infor_date[4]-0x30)*10+(gps_infor_date[5]-0x30);//��ȡʱ�� ��   
  
  if(hour_Beijing>=24)	//����ʱ��ת��
  {
    hour_Beijing=hour_Beijing%24;
    days_Beijing=days_Beijing+1;
    if(year_Beijing%4==0)            
    {
      if(days_Beijing>=monthrun_table[mont_Beijing])//�����������·�
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
      if(days_Beijing>=monthmon_table[mont_Beijing])//�Ƿ����
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
	sprintf(dis0,"20%02d-%02d-%02d   ",(u16)year_Beijing,(u16)mont_Beijing,(u16)days_Beijing);//��ӡ����
	sprintf(dis1,"%02d:%02d:%02d      ",(u16)hour_Beijing,(u16)minu_Beijing,(u16)seco_Beijing);//��ӡʱ��
	
	Mid_Du=(gps_infor_weijing[0]-0x30)*10000000+(gps_infor_weijing[1]-0x30)*1000000;    //����������10000000
	
	Mid_Fen=(gps_infor_weijing[2]-0x30)*10000000+(gps_infor_weijing[3]-0x30)*1000000+
		(gps_infor_weijing[4]-0x30)*100000+(gps_infor_weijing[5]-0x30)*10000+
			(gps_infor_weijing[6]-0x30)*1000+(gps_infor_weijing[7]-0x30)*100;          
	Mid_Fen=Mid_Fen/60;                                                      //���뻻��ΪС��λ
	Mid_Vale=Mid_Du+Mid_Fen;         //����Ϊ�ȸ�ʽ000.00000000 �Ƕȷ����ʽ
	Lin0_No[0]='N';                  
	Lin0_No[1]=':';                  
	Lin0_No[2]='0';                  
	Lin0_No[3]=Mid_Vale/10000000+0x30;                  //ת��Ϊ�ַ�
	Lin0_No[4]=(Mid_Vale/1000000)%10+0x30;
	Lin0_No[5]='.';
	Lin0_No[6]=(Mid_Vale/100000)%10+0x30;
	Lin0_No[7]=(Mid_Vale/10000)%10+0x30;
	Lin0_No[8]=(Mid_Vale/1000)%10+0x30;
	Lin0_No[9]=(Mid_Vale/100)%10+0x30;
	Lin0_No[10]=(Mid_Vale/10)%10+0x30;
	Lin0_No[11]=Mid_Vale%10+0x30;

	Mid_Du=(gps_infor_weijing[8]-0x30)*100000000+(gps_infor_weijing[9]-0x30)*10000000+(gps_infor_weijing[10]-0x30)*1000000; //����������10000000     

	Mid_Fen=(gps_infor_weijing[11]-0x30)*10000000+(gps_infor_weijing[12]-0x30)*1000000+
		(gps_infor_weijing[13]-0x30)*100000+(gps_infor_weijing[14]-0x30)*10000+
		(gps_infor_weijing[15]-0x30)*1000+(gps_infor_weijing[16]-0x30)*100; 
	Mid_Fen=Mid_Fen/60;                                                //���뻻��ΪС��λ
	Mid_Vale=Mid_Du+Mid_Fen;                                          //����Ϊ�ȸ�ʽ000.00000000 �Ƕȷ����ʽ
	Lin1_Ea[0]='E';                  
	Lin1_Ea[1]=':';     
	Lin1_Ea[2]=Mid_Vale/100000000+0x30;                           //ת��Ϊ�ַ�
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

