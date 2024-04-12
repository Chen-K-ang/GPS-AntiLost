#include "sys.h"
#include "usart.h"	

#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos ʹ��	  
#endif

#define STAGE_SOHE  0x01
#define STAGE_TYPE  0x02
#define STAGE_NONE  0x03
#define STAGE_DATA  0x04

unsigned char	devide_flag;		        //GPS���ݶ��ŷָ�����־
unsigned char	speed_end;			//�յ��ٶ����ݽ�����־
unsigned char	dir_end;			//�յ��������ݽ�����־
unsigned char  sysmode_GPS=0;                    //gps����Ч��־
unsigned char  ew_flag;                        //������־
unsigned char  ns_flag;                        //�ϱ���־

unsigned char	gps_infor_weijing[17];    //��γ���ݴ� ��ʽ�Ƕȷ�����ʽ
unsigned char	gps_infor_speed[4];       //�ݴ��ٶ�
unsigned char	gps_infor_time[6];        //�ݴ�ʱ��
unsigned char	gps_infor_date[6];        //�ݴ�����
unsigned char	gps_infor_dir[3];         //�ݴ淽��

unsigned char recv1_step=STAGE_SOHE;                       //���ڽ���ָ���
unsigned char uart1_r_buf;                       //���ڻ���
unsigned char rev1_buf_busy;                    //������æ
unsigned char temp1_buf[85];                   //���ڽ�������
unsigned int record1=0;   
unsigned char rendFlag=0;


//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

/*ʹ��microLib�ķ���*/
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
  
u8 uartFlag = 0;

//��ʼ��IO ����1 
//bound:������
void uart_init(u32 bound){
    //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
 	USART_DeInit(USART1);  //��λ����1
	 //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA9
   
    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA10

   //Usart1 NVIC ����

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

	USART_Init(USART1, &USART_InitStructure); //��ʼ������
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�����ж�
	USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ��� 

}


unsigned char timebuf[110];//�ݴ洮������
unsigned char clear=0;	 //����buf��ռ���
unsigned char count=0; 	 //���ڽ��ռ���
unsigned char readFlag=0; //��ȡ���ű�־

extern unsigned char modeFlag; //ģʽ

void USART1_IRQHandler(void)                	//����1�жϷ������
{
	u8 Res1;
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
		{
			Res1=USART_ReceiveData(USART1);//(USART1->DR);	//��ȡ���յ�������
			clear=0;      //���ж�ʱ��ȡ ����
			count++;      //���ݸ�����¼
			timebuf[count]=Res1;
			if(count>=(109))                  //��������16���ַ���Ϣ
			{
				count=0;
				readFlag=1;               //������ɱ�־λ��1
			}
			
     } 
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
	OSIntExit();  											 
#endif
} 
	

//��ʼ��IO ����2
//bound:������
void uart2_init(u32 bound)
{
    //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ�� 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 , ENABLE);	//ʹ��USART1��GPIOAʱ��
	
 	USART_DeInit(USART2);  //��λ����1
	 //USART1_TX   PA.2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA9
   
    //USART1_RX	  PA.3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA10

   //Usart1 NVIC ����

  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

	USART_Init(USART2, &USART_InitStructure); //��ʼ������
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�����ж�
	USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ��� 

}


void USART2_IRQHandler(void)                	//����1�жϷ������
{
	u8 uart2_rbuf;
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
		{
			uart2_rbuf=USART_ReceiveData(USART2);//(USART1->DR);	//��ȡ���յ�������
		uart1_r_buf=uart2_rbuf;                      //��ȡ����buf
	  rev1_buf_busy=0x00;                         //�ж� ��ֹbreak����
	  switch(recv1_step)
	  {
	  case STAGE_SOHE: if(uart1_r_buf == '$')     //����ο�GPS��׼Э��NMEA0183
	  {
	    rev1_buf_busy=0x01;
	    if(uart1_r_buf == '$')              //�鿴�յ�$
	    {
	      recv1_step=STAGE_TYPE;            //��ת����һ��
	      record1=0;                        //��������
	    }
	    else
	    {
	      recv1_step=STAGE_SOHE;        //�ָ���ʼ��
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
	    {                                                 //ȷ�Ͽ�ͷ$GPRMC && (temp1_buf[1] == 'P') 
	      if((temp1_buf[0] == 'G') && (temp1_buf[2] == 'R') && (temp1_buf[3] == 'M') && (temp1_buf[4] == 'C'))
	      {
	        recv1_step=STAGE_NONE;    //��ת����һ��
	        record1=0;
	      } 
	      else
	      {
	        recv1_step=STAGE_SOHE;//�ָ���ʼ��
	        record1=0;
	      }
	    }
	  }
	  break;
	  case STAGE_NONE: if(rev1_buf_busy == 0x00)//�������ݸ�ʽ:$GPRMC,054347.00,A,3202.04770,N,11846.23632,E,0.000,0.00,221013,,,A*67
	  {
	    rev1_buf_busy=0x01;
	    record1++;
	    if((record1 > 0x01) && (record1 < 0x08))                                                                                    
	    {
	      gps_infor_time[record1-2]=uart1_r_buf;			//�洢ʱ��					
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
	      if(uart1_r_buf == 'A')  //gps�յ����� ����Ч
	      { 
	        recv1_step=STAGE_DATA;    //��ת����һ��
	      }
	      else
	      {
	        sysmode_GPS=0;
	        recv1_step=STAGE_SOHE;    //��Ч�ָ���ʼ��
	        record1=0;
	      }
	    }
	  }
	  break;
	  case STAGE_DATA:  if(rev1_buf_busy == 0x00)
	  {
	    rev1_buf_busy=0x01;
	    record1++;
	    if(uart1_r_buf == ',')    //�ж�Ϊ����
	    { 
	      devide_flag++;      //���� ������¼
	      record1=0;
	    }
	    if(devide_flag == 3)
	    {
	      if((record1 > 0) && (record1 < 5))
	      {
	        gps_infor_weijing[record1-1]=uart1_r_buf;	    //�洢��γ�� �˴�Ϊγ��					
	      }
	      if((record1 > 5) && (record1 < 10))             //����С����Ĵ洢
	      {
	        gps_infor_weijing[record1-2]=uart1_r_buf;	   //�洢��γ�� �˴�Ϊγ��														
	      }
	    }
	    if(devide_flag == 4)
	    {
	      if(record1 > 0)
	      {
	        ns_flag=uart1_r_buf;            //����γ�� NS��־
	      }
	    }
	    if(devide_flag == 5)
	    {
	      if((record1 > 0) && (record1 < 6))
	      {
	        gps_infor_weijing[record1+7]=uart1_r_buf;	  //����С����Ĵ洢										
	      }
	      if((record1 > 6) && (record1 < 11))                //
	      {
	        gps_infor_weijing[record1+6]=uart1_r_buf;       //�洢��γ��	 �˴�Ϊ����																	
	      }
	    }
	    if(devide_flag == 6)
	    {
	      if(record1 > 0)
	      {
	        ew_flag=uart1_r_buf;            //��γ�� EW��־
	      }
	    }
	    if(devide_flag == 7)
	    {
	      if(speed_end == 0x00)
	      {
	        if((record1 > 0) && (uart1_r_buf != '.'))
	        {
	          gps_infor_speed[record1-1]=uart1_r_buf;   //��������
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
	          gps_infor_dir[record1-1]=uart1_r_buf;   //�洢����
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
	      recv1_step=STAGE_SOHE;    //������� �ź�ȷ��
	      record1=0;                //�ָ���ʼ��״̬ Ϊ��һ����׼��
	      devide_flag=0;
	      sysmode_GPS=1;         //�  �־ GPS�ź���Ч
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
		USART_SendData(USARTx, *str);
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
		str++;
	}
}

