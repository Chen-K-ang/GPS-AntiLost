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

unsigned char BufTab[10];     //wifi�����ݴ�
unsigned char Count;          //�������ݼ���
unsigned char UartBusy=0;     //����æ
unsigned char sendDataFlag=0; //���Ͷ��ű�־
unsigned char i;
unsigned char diplayFlag=0;   //��ȡ��ʾ��־

unsigned char time_count = 3;
unsigned char begin = 0; //��ʼ��λ
unsigned char dealgps;  //��ʱ����Χ����������
unsigned char ErrorNum=0;   //��¼�������
unsigned char CheckNum=0;   //������
unsigned long ReportLater=0;//�ϱ���ʱ
unsigned long KeyLater=3;   //������ʱ����
unsigned long Mid_Du;       //��γ�ȴ��� ��
unsigned long Mid_Fen;      //��γ�ȴ��� ��
unsigned long Mid_Vale;     //��γ�ȴ��� �м����

double lat1 = 0.0f, lat2 = 0.0f;  //1�����ʼ lat����
double lng1 = 0.0f, lng2 = 0.0f;  //2���浱ǰ lngά��
double distance = 0, dis_limit = 0; 

u8 Lin0_No[16]="N:000.000000";//�洢γ��
u8 Lin1_Ea[16]="E:000.000000";//�洢����
u8 Lin2_No[16]="N:000.000000";//�洢γ��
u8 Lin3_Ea[16]="E:000.000000";//�洢����

char Crtl_Z[1]={0x1a};	//���Ͷ��ű�־

u8 number_Array[]="0123456789.V";
u8 wan,qian,bai,shi,ge;

unsigned char stepNum  =0;	//���в���
unsigned int timeCount =0;//���ն��Ŵ�����ʱ
char AT_CMGS[26]="AT+CMGS=\"";
unsigned char rebackMesFlag; //���ض���
unsigned char clearMesFlag =0;//��ʱ�������
unsigned char readMesIng = 0;//��ȡ�����б�־

void dealGps(double lat, double lng);//gps���ݴ���
void key_deal(void);
u8 display(void);
void read_gsm_note(void); //��ȡ����

void Split(u16 count)//����λ���ֲ𿪺���
{ 
	wan=count/10000;//����λ����
	qian=count%10000/1000;//��ǧλ����
	bai=count%1000/100;//���λ����
	shi=count%100/10;//��ʮλ����
	ge=count%10;//���λ����  
}

int main(void)
{	
	delay_init();          //��ʱ������ʼ��
	NVIC_Configuration();  //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	KEY_Init();
//	GPIO_InitStructReadtempCmd(); //lcd
//	lcd_system_reset();
	
	LCD1602_Init();

	uart_init(9600);
	uart2_init(9600);

	TIM3_Int_Init(71,1000); //10Khz�ļ���Ƶ�ʣ�������500Ϊ50ms
	
	Timer_Init();
	
	
	 
	dealGps(lat2, lng2);
	lat1 = lat2; lng1 = lng2;
	i=5;
	while(i--) delay_ms(100);//��ʱ

	UART_SendStr(USART1 ,"AT+CMGF=1\r\n",11); //�����ַ���
	i=10;
	while(i--) delay_ms(100);//��ʱ
	
	UART_SendStr(USART1 ,"AT+CSCS=\"GSM\"\r\n",15);//�����ı�
	i=10;
	while(i--) delay_ms(100);//��ʱ

	while(1) {	
		
		if (dealgps == 1 && begin) {   //��ʱ��������
			dealgps = 0;
			distance = get_distance(lat1, lng1, lat2, lng2);    //�����뺯��
			CheckNum++;
			if(distance > dis_limit){            //�ж� �鿴��������

				LCD_Write_String(0,0, portA4_Array);
				
				ErrorNum++;
				if (CheckNum>=5)                      //����5�δ���
					if (KeyLater>=3) {             //�ǰ�����
						if (ErrorNum>=1){       //��δ�����һ�γ�
							UART_SendStr(USART1 ,"AT+CMGDA=\"DEL ALL\"\r\n",20);//ɾ�����ж���															
							for(i=0;i<5;i++)
								delay_ms(100);//��ʱ�������ȶ�

							UART_SendStr(USART1 ,"AT+CMGF=1\r\n",11);   //�����ַ���
							for(i=0;i<3;i++)
								delay_ms(100);//��ʱ�������ȶ�

							UART_SendStr(USART1 ,"AT+CSCS=\"GSM\"\r\n",15); //���ø�ʲô
							for(i=0;i<3;i++)
								delay_ms(100);//��ʱ�������ȶ�

							UART_SendStr(USART1 ,"AT+CMGSM=\"+8615633691336\"\r\n",26); //���Ժ�15633691336
							for(i=0;i<2;i++)
								delay_ms(100);//��ʱ�������ȶ� 

							UART_SendStr(USART1 ,"LOST",7); //���ͳ���Χǽ��Ϣ��LOST��
							
							delay_ms(100);
							UART_SendStr(USART1 ,(char *)Crtl_Z, 1);//����
							for(i=0;i<20;i++)
								delay_ms(100);//��ʱ�������ȶ�	
						}
						else {
							ReportLater=0; //�ϱ���ʱ����
						}
						ErrorNum=0;            //����˲�����
						CheckNum=0;
					}
			}

			if(ReportLater >= time_count) {	//Ĭ����3sˢ��
				UART_SendStr(USART1 ,"AT+CMGF=1\r\n",11); //�����ַ���
				i=3;
				while(i--) delay_ms(100);//��ʱ

				UART_SendStr(USART1 ,"AT+CSCS=\"GSM\"\r\n",15);//�����ı�
				i=3;
				while(i--) delay_ms(100);//��ʱ
				//���Ժ�//**All notes can be deleted and modified**//
				UART_SendStr(USART1 ,"AT+CMGSM=\"+8615633691336\"\r\n",26); 

				i=2;
				while(i--) delay_ms(100);//��ʱ

				UART_SendStr(USART1 ,"The device is out!",8); //���ͳ���Χ��Ϣ		

				if(sysmode_GPS==1){		//gps��Ч
					sysmode_GPS=0;
					dealGps(lat2, lng2);	
				} else
					UART_SendStr(USART1 ,"gps linking...",14); //gps���ź�
				delay_ms(200);//��ʱ

				UART_SendStr(USART1 ,( char *)Crtl_Z, 1); //����
				i=40;
				while(i--) delay_ms(100);//��ʱ

				ReportLater=0;  //�ϱ���־���
				KeyLater=0;     //����������ʱ����
			}
			read_gsm_note(); //��ȡ���Ų���
			display();  //lcd��ʾ
		}
	}
}

void dealGps(double lat, double lng)
{
	u8 j;
	
	Mid_Du=(gps_infor_weijing[0]-0x30)*10000000+(gps_infor_weijing[1]-0x30)*1000000;    //����������10000000
	
	Mid_Fen=(gps_infor_weijing[2]-0x30)*10000000+(gps_infor_weijing[3]-0x30)*1000000+
		(gps_infor_weijing[4]-0x30)*100000+(gps_infor_weijing[5]-0x30)*10000+
			(gps_infor_weijing[6]-0x30)*1000+(gps_infor_weijing[7]-0x30)*100;          
	Mid_Fen=Mid_Fen/60;              //���뻻��ΪС��λ
	Mid_Vale=Mid_Du+Mid_Fen;         //����Ϊ�ȸ�ʽ000.00000000 �Ƕȷ����ʽ
	lat = Mid_Vale;
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
	Mid_Vale=Mid_Du+Mid_Fen;  //����Ϊ�ȸ�ʽ000.00000000 �Ƕȷ����ʽ
	lng = Mid_Vale;
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
	
	for(j=0;j<16;j++)
	{
		Lin2_No[j]=Lin0_No[j];
		Lin3_Ea[j]=Lin1_Ea[j];
	}
}

void key_deal(void)
{
	if(key1 == 0){ //��λ��ʼ
		delay_ms(3);
		if(key1 ==0){
			dealGps(lat1, lng1);
			begin = 1;
		}
	} else if(key2==0){ //11��
		delay_ms(3);
		if(key2 ==0){
			if (dis_limit > 11 && dis_limit < 111)
				dis_limit=dis_limit+11;
			else
				dis_limit = 11;
			diplayFlag = 1;
			Split(dis_limit);//������ֵ��ֺ���
			
		}
	} else if(key3==0){ //111��
		delay_ms(3);
		if(key3 ==0){
			if (dis_limit >= 111 && dis_limit < 800)
				dis_limit=dis_limit+111;
			else
				dis_limit = 111;
			diplayFlag = 1;
			Split(dis_limit);//������ֵ��ֺ���
			
		}
	} else if(key4==0){ //Ĭ��800
		delay_ms(3);
		if(key4 ==0){
			dis_limit = 800;
			diplayFlag = 1;
			Split(dis_limit);//������ֵ��ֺ���
		}
	} else if(key5==0){  //��Сˢ��ʱ��
		delay_ms(3);
		if(key5 ==0){
			time_count++; 
		}
	} else if(key6==0){ //�ӿ�ˢ��ʱ��
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
		LCD1602_Write_Cmd(0x80);//������ʾ��ַΪLCD�ڶ��У�һ���׵�ַȷ������ʾ���һ�����ֺ󣬹����Զ���1����  
		LCD1602_Write_Data(number_Array[qian]);//��һλ��ʾǧλ
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
	if(readFlag==1){    //��ȡ����Ϣ��־
		     
		readFlag=0;	 //��ȡ��־����
		readMesIng = 1;//��ȡ���Ź�����
		timeCount++ ;
		if((stepNum == 0)&&(timeCount>=4)){	//��ʱ��ȡ����
							
			UART_SendStr(USART1 , "AT+CMGF=1\r\n",11);   //�����ַ���
			timeCount = 0;//��ʱ������	
			stepNum++;//�����������1
		} else if((stepNum == 1)&&(timeCount>=4)){	//��ʱ��ȡ����
	
			for(i=0;i<SIZEBUF;i++)
				timebuf[i]='0';   //���timebuf					
			UART_SendStr(USART1 ,"AT+CMGR=1\r\n",11);	
			timeCount = 0;//��ʱ������	
			stepNum++;//�����������1
		}else if((stepNum == 2)&&(timeCount >= 4)){	//��ʱ��ȡ����
		
			rebackMesFlag=INIT;	
			for(i=0;i<(SIZEBUF-3);i++){
				if((timebuf[i]=='G')&&(timebuf[i+1]=='E')&&(timebuf[i+2]=='T')){//��ѯ�Ƿ���յ�GET
					rebackMesFlag=OPEN_01;	//״̬��ʶ��
					break ;
				}
				if((timebuf[i]=='+')&&(timebuf[i+1]=='8')&&(timebuf[i+2]=='6')){//��ѯ����Ϣ�еĺ���	
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

			if(rebackMesFlag != INIT){
				UART_SendStr(USART1 ,"AT+CMGDA=\"DEL ALL\"\r\n",20);//ɾ�����ж���															
				for(i=0;i<5;i++)
					delay_ms(100);//��ʱ�������ȶ�

				UART_SendStr(USART1 ,"AT+CMGF=1\r\n",11);   //�����ַ���
				for(i=0;i<3;i++)
					delay_ms(100);//��ʱ�������ȶ�

				UART_SendStr(USART1 ,"AT+CSCS=\"GSM\"\r\n",15); //���ø�ʲô
				for(i=0;i<3;i++)
					delay_ms(100);//��ʱ�������ȶ�

				UART_SendStr(USART1 , ( char *)AT_CMGS,26);//׼�����Ͷ���
				for(i=0;i<2;i++)
					delay_ms(100);//��ʱ�������ȶ� 

				if(rebackMesFlag==OPEN_01){
					UART_SendStr(USART1, (char*)Lin0_No,12); //����γ��
					UART_SendStr(USART1, (char*)Lin1_Ea,12); //���;���											
				}else{
					UART_SendStr(USART1 ,"CMD_ERR",7); //���Ͷ���Ϣ�������
				}
				delay_ms(100);
				UART_SendStr(USART1 ,(char *)Crtl_Z, 1);//����
				for(i=0;i<20;i++)
					delay_ms(100);//��ʱ�������ȶ�	
			}
			timeCount = 0;//��ʱ������	
			stepNum++;//�����������1
		}else if((stepNum == 3)&&(timeCount >= 4)){	//��ʱ��ȡ����
	
			clearMesFlag++;
			if(clearMesFlag>10){
				clearMesFlag = 0;
				UART_SendStr(USART1 ,"AT+CMGDA=\"DEL ALL\"\r\n",20);//ɾ�����ж���
			}
			timeCount = 0;//��ʱ������	
			stepNum = 0	;//�����������1							
			readMesIng = 0;//��ȡ���Ź�����
		}							 		   
	}
}

void TIM1_UP_IRQHandler(void)
{
	static u32 time50ms = 0;
	
	if(TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)  //���TIM3�����жϷ������
	{
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);  //���TIMx�����жϱ�־ 
		
		time50ms++;
		if(time50ms%5==0)
		{
			dealgps =1;
			if(time50ms%10==0)
				sendDataFlag =1;
			if(time50ms%20==0)  //1s����һ��
			{
				ReportLater++;
				KeyLater++;
			}
		}
		key_deal(); //��������
		if(time50ms%40==0)  //2s����һ��
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
