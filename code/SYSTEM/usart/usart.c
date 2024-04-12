#include "sys.h"
#include "usart.h"	

#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos Ê¹ÓÃ	  
#endif

#define STAGE_SOHE  0x01
#define STAGE_TYPE  0x02
#define STAGE_NONE  0x03
#define STAGE_DATA  0x04

unsigned char	devide_flag;		        //GPSÊý¾Ý¶ººÅ·Ö¸ô·û±êÖ¾
unsigned char	speed_end;			//ÊÕµ½ËÙ¶ÈÊý¾Ý½áÊø±êÖ¾
unsigned char	dir_end;			//ÊÕµ½·½ÏòÊý¾Ý½áÊø±êÖ¾
unsigned char  sysmode_GPS=0;                    //gpsÓÐÎÞÐ§±êÖ¾
unsigned char  ew_flag;                        //¶«Î÷±êÖ¾
unsigned char  ns_flag;                        //ÄÏ±±±êÖ¾

unsigned char	gps_infor_weijing[17];    //¾­Î³¶ÈÔÝ´æ ¸ñÊ½ÊÇ¶È·ÖÃëÐÎÊ½
unsigned char	gps_infor_speed[4];       //ÔÝ´æËÙ¶È
unsigned char	gps_infor_time[6];        //ÔÝ´æÊ±¼ä
unsigned char	gps_infor_date[6];        //ÔÝ´æÈÕÆÚ
unsigned char	gps_infor_dir[3];         //ÔÝ´æ·½Ïò

unsigned char recv1_step=STAGE_SOHE;                       //´®¿Ú½ÓÊÕÖ¸Áî²½Öè
unsigned char uart1_r_buf;                       //´®¿Ú»º´æ
unsigned char rev1_buf_busy;                    //´®¿ÚÅÐÃ¦
unsigned char temp1_buf[85];                   //´®¿Ú½ÓÊÕÊý×é
unsigned int record1=0;   
unsigned char rendFlag=0;


//////////////////////////////////////////////////////////////////
//¼ÓÈëÒÔÏÂ´úÂë,Ö§³Öprintfº¯Êý,¶ø²»ÐèÒªÑ¡Ôñuse MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//±ê×¼¿âÐèÒªµÄÖ§³Öº¯Êý                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//¶¨Òå_sys_exit()ÒÔ±ÜÃâÊ¹ÓÃ°ëÖ÷»úÄ£Ê½    
_sys_exit(int x) 
{ 
	x = x; 
} 
//ÖØ¶¨Òåfputcº¯Êý 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//Ñ­»··¢ËÍ,Ö±µ½·¢ËÍÍê±Ï   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

/*Ê¹ÓÃmicroLibµÄ·½·¨*/
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

//³õÊ¼»¯IO ´®¿Ú1 
//bound:²¨ÌØÂÊ
void uart_init(u32 bound){
    //GPIO¶Ë¿ÚÉèÖÃ
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//Ê¹ÄÜUSART1£¬GPIOAÊ±ÖÓ
 	USART_DeInit(USART1);  //¸´Î»´®¿Ú1
	 //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//¸´ÓÃÍÆÍìÊä³ö
    GPIO_Init(GPIOA, &GPIO_InitStructure); //³õÊ¼»¯PA9
   
    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//¸¡¿ÕÊäÈë
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //³õÊ¼»¯PA10

   //Usart1 NVIC ÅäÖÃ

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//ÇÀÕ¼ÓÅÏÈ¼¶3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//×ÓÓÅÏÈ¼¶3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQÍ¨µÀÊ¹ÄÜ
	NVIC_Init(&NVIC_InitStructure);	//¸ù¾ÝÖ¸¶¨µÄ²ÎÊý³õÊ¼»¯VIC¼Ä´æÆ÷
  
   //USART ³õÊ¼»¯ÉèÖÃ

	USART_InitStructure.USART_BaudRate = bound;//Ò»°ãÉèÖÃÎª9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//×Ö³¤Îª8Î»Êý¾Ý¸ñÊ½
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//Ò»¸öÍ£Ö¹Î»
	USART_InitStructure.USART_Parity = USART_Parity_No;//ÎÞÆæÅ¼Ð£ÑéÎ»
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//ÎÞÓ²¼þÊý¾ÝÁ÷¿ØÖÆ
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//ÊÕ·¢Ä£Ê½

	USART_Init(USART1, &USART_InitStructure); //³õÊ¼»¯´®¿Ú
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//¿ªÆôÖÐ¶Ï
	USART_Cmd(USART1, ENABLE);                    //Ê¹ÄÜ´®¿Ú 

}


unsigned char timebuf[110];//ÔÝ´æ´®¿ÚÊý¾Ý
unsigned char clear=0;	 //´®¿ÚbufÇå¿Õ¼ÆÊý
unsigned char count=0; 	 //´®¿Ú½ÓÊÕ¼ÆÊý
unsigned char readFlag=0; //¶ÁÈ¡¶ÌÐÅ±êÖ¾

extern unsigned char modeFlag; //Ä£Ê½

void USART1_IRQHandler(void)                	//´®¿Ú1ÖÐ¶Ï·þÎñ³ÌÐò
{
	u8 Res1;
#ifdef OS_TICKS_PER_SEC	 	//Èç¹ûÊ±ÖÓ½ÚÅÄÊý¶¨ÒåÁË,ËµÃ÷ÒªÊ¹ÓÃucosIIÁË.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //½ÓÊÕÖÐ¶Ï(½ÓÊÕµ½µÄÊý¾Ý±ØÐëÊÇ0x0d 0x0a½áÎ²)
		{
			Res1=USART_ReceiveData(USART1);//(USART1->DR);	//¶ÁÈ¡½ÓÊÕµ½µÄÊý¾Ý
			clear=0;      //¿ÕÏÐ¶¨Ê±¶ÁÈ¡ ÇåÁã
			count++;      //Êý¾Ý¸öÊý¼ÇÂ¼
			timebuf[count]=Res1;
			if(count>=(109))                  //Á¬Ðø½ÓÊÕ16¸ö×Ö·ûÐÅÏ¢
			{
				count=0;
				readFlag=1;               //½ÓÊÕÍê³É±êÖ¾Î»ÖÃ1
			}
			
     } 
#ifdef OS_TICKS_PER_SEC	 	//Èç¹ûÊ±ÖÓ½ÚÅÄÊý¶¨ÒåÁË,ËµÃ÷ÒªÊ¹ÓÃucosIIÁË.
	OSIntExit();  											 
#endif
} 
	

//³õÊ¼»¯IO ´®¿Ú2
//bound:²¨ÌØÂÊ
void uart2_init(u32 bound)
{
    //GPIO¶Ë¿ÚÉèÖÃ
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//Ê¹ÄÜUSART1£¬GPIOAÊ±ÖÓ 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 , ENABLE);	//Ê¹ÄÜUSART1£¬GPIOAÊ±ÖÓ
	
 	USART_DeInit(USART2);  //¸´Î»´®¿Ú1
	 //USART1_TX   PA.2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//¸´ÓÃÍÆÍìÊä³ö
    GPIO_Init(GPIOA, &GPIO_InitStructure); //³õÊ¼»¯PA9
   
    //USART1_RX	  PA.3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//¸¡¿ÕÊäÈë
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //³õÊ¼»¯PA10

   //Usart1 NVIC ÅäÖÃ

  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//ÇÀÕ¼ÓÅÏÈ¼¶3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//×ÓÓÅÏÈ¼¶3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQÍ¨µÀÊ¹ÄÜ
	NVIC_Init(&NVIC_InitStructure);	//¸ù¾ÝÖ¸¶¨µÄ²ÎÊý³õÊ¼»¯VIC¼Ä´æÆ÷
  
   //USART ³õÊ¼»¯ÉèÖÃ

	USART_InitStructure.USART_BaudRate = bound;//Ò»°ãÉèÖÃÎª9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//×Ö³¤Îª8Î»Êý¾Ý¸ñÊ½
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//Ò»¸öÍ£Ö¹Î»
	USART_InitStructure.USART_Parity = USART_Parity_No;//ÎÞÆæÅ¼Ð£ÑéÎ»
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//ÎÞÓ²¼þÊý¾ÝÁ÷¿ØÖÆ
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//ÊÕ·¢Ä£Ê½

	USART_Init(USART2, &USART_InitStructure); //³õÊ¼»¯´®¿Ú
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//¿ªÆôÖÐ¶Ï
	USART_Cmd(USART2, ENABLE);                    //Ê¹ÄÜ´®¿Ú 

}


void USART2_IRQHandler(void)                	//´®¿Ú1ÖÐ¶Ï·þÎñ³ÌÐò
{
	u8 uart2_rbuf;
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //½ÓÊÕÖÐ¶Ï(½ÓÊÕµ½µÄÊý¾Ý±ØÐëÊÇ0x0d 0x0a½áÎ²)
		{
			uart2_rbuf=USART_ReceiveData(USART2);//(USART1->DR);	//¶ÁÈ¡½ÓÊÕµ½µÄÊý¾Ý
		uart1_r_buf=uart2_rbuf;                      //ÌáÈ¡½ÓÊÕbuf
	  rev1_buf_busy=0x00;                         //ÅÐ¶Ï ·ÀÖ¹breakÎÊÌâ
	  switch(recv1_step)
	  {
	  case STAGE_SOHE: if(uart1_r_buf == '$')     //¾ßÌå²Î¿¼GPS±ê×¼Ð­ÒéNMEA0183
	  {
	    rev1_buf_busy=0x01;
	    if(uart1_r_buf == '$')              //²é¿´ÊÕµ½$
	    {
	      recv1_step=STAGE_TYPE;            //Ìø×ªµ½ÏÂÒ»²½
	      record1=0;                        //¼ÆÊýÇåÁã
	    }
	    else
	    {
	      recv1_step=STAGE_SOHE;        //»Ö¸´³õÊ¼»¯
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
	    {                                                 //È·ÈÏ¿ªÍ·$GPRMC && (temp1_buf[1] == 'P') 
	      if((temp1_buf[0] == 'G') && (temp1_buf[2] == 'R') && (temp1_buf[3] == 'M') && (temp1_buf[4] == 'C'))
	      {
	        recv1_step=STAGE_NONE;    //Ìø×ªµ½ÏÂÒ»²½
	        record1=0;
	      } 
	      else
	      {
	        recv1_step=STAGE_SOHE;//»Ö¸´³õÊ¼»¯
	        record1=0;
	      }
	    }
	  }
	  break;
	  case STAGE_NONE: if(rev1_buf_busy == 0x00)//½ÓÊÕÊý¾Ý¸ñÊ½:$GPRMC,054347.00,A,3202.04770,N,11846.23632,E,0.000,0.00,221013,,,A*67
	  {
	    rev1_buf_busy=0x01;
	    record1++;
	    if((record1 > 0x01) && (record1 < 0x08))                                                                                    
	    {
	      gps_infor_time[record1-2]=uart1_r_buf;			//´æ´¢Ê±¼ä					
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
	      if(uart1_r_buf == 'A')  //gpsÊÕµ½Êý¾Ý ÇÒÓÐÐ§
	      { 
	        recv1_step=STAGE_DATA;    //Ìø×ªµ½ÏÂÒ»²½
	      }
	      else
	      {
	        sysmode_GPS=0;
	        recv1_step=STAGE_SOHE;    //ÎÞÐ§»Ö¸´³õÊ¼»¯
	        record1=0;
	      }
	    }
	  }
	  break;
	  case STAGE_DATA:  if(rev1_buf_busy == 0x00)
	  {
	    rev1_buf_busy=0x01;
	    record1++;
	    if(uart1_r_buf == ',')    //ÅÐ¶ÏÎª¶ººÅ
	    { 
	      devide_flag++;      //¶ººÅ ´ÎÊý¼ÇÂ¼
	      record1=0;
	    }
	    if(devide_flag == 3)
	    {
	      if((record1 > 0) && (record1 < 5))
	      {
	        gps_infor_weijing[record1-1]=uart1_r_buf;	    //´æ´¢¾­Î³¶È ´Ë´¦ÎªÎ³¶È					
	      }
	      if((record1 > 5) && (record1 < 10))             //Ìø¹ýÐ¡ÊýµãµÄ´æ´¢
	      {
	        gps_infor_weijing[record1-2]=uart1_r_buf;	   //´æ´¢¾­Î³¶È ´Ë´¦ÎªÎ³¶È														
	      }
	    }
	    if(devide_flag == 4)
	    {
	      if(record1 > 0)
	      {
	        ns_flag=uart1_r_buf;            //½ÓÊÜÎ³¶È NS±êÖ¾
	      }
	    }
	    if(devide_flag == 5)
	    {
	      if((record1 > 0) && (record1 < 6))
	      {
	        gps_infor_weijing[record1+7]=uart1_r_buf;	  //Ìø¹ýÐ¡ÊýµãµÄ´æ´¢										
	      }
	      if((record1 > 6) && (record1 < 11))                //
	      {
	        gps_infor_weijing[record1+6]=uart1_r_buf;       //´æ´¢¾­Î³¶È	 ´Ë´¦Îª¾­¶È																	
	      }
	    }
	    if(devide_flag == 6)
	    {
	      if(record1 > 0)
	      {
	        ew_flag=uart1_r_buf;            //¾­Î³¶È EW±êÖ¾
	      }
	    }
	    if(devide_flag == 7)
	    {
	      if(speed_end == 0x00)
	      {
	        if((record1 > 0) && (uart1_r_buf != '.'))
	        {
	          gps_infor_speed[record1-1]=uart1_r_buf;   //½ÓÊÜËÙÂÊ
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
	          gps_infor_dir[record1-1]=uart1_r_buf;   //´æ´¢·½Ïò
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
	      recv1_step=STAGE_SOHE;    //½ÓÊÜÍê³É ÐÅºÅÈ·¶¨
	      record1=0;                //»Ö¸´³õÊ¼»¯×´Ì¬ ÎªÏÂÒ»´Î×ö×¼±¸
	      devide_flag=0;
	      sysmode_GPS=1;         //±  êÖ¾ GPSÐÅºÅÓÐÐ§
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

