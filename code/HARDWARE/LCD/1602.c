#include "1602.h"	
#include "delay.h"
//1602初始化
//u16 temp;
//void lcd_delay( unsigned char ms) /*LCD1602 延时*/
//{
//	unsigned int j;
//	while(ms--){
//		for(j=0;j<300;j++){;}
//	}   
//}

//void GPIO_InitStructReadtempCmd(void)
//{
//	GPIO_InitTypeDef GPIO_InitStructure;
//	
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
//	
//	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9
//					|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOB, &GPIO_InitStructure);
//}

//void lcd_busy_wait(void) /*LCD1602 忙等待*/
//{
//	u8 sta;
//	GPIOB->ODR=0xFF;
//	RS=0; 
//	do
//	{	
//		EN = 1;
//		lcd_delay(5);	
//		sta = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7);
//	}while(sta & 0x80);
//	EN=0;
//}

//void lcd_command_write( unsigned int command) /*LCD1602 命令字写入*/
//{
//	RS=0;
//	EN=0;
//	lcd_delay(5);
//	temp=(temp&0xf00f)|(command<<4);
//	GPIO_Write(GPIOB,temp);
//	lcd_delay(10);
//	EN=1;
//	lcd_delay(10);
//	EN=0;
//}


//void lcd_system_reset(void) /*LCD1602 初始化*/
//{
//	lcd_command_write(0x38);
//	lcd_command_write(0x38);
//	lcd_command_write(0x38);
//	lcd_command_write(0x0c);
//	lcd_command_write(0x06);
//	lcd_command_write(0x01);
//	lcd_command_write(0x80);
//}

//void lcd_char_write(unsigned int x_pos,unsigned int y_pos,unsigned int lcd_dat) /*LCD1602 字符写入*/
//{
//	x_pos &= 0x0f; 
//	y_pos &= 0x01; 
//	if(y_pos==1) x_pos += 0x40;
//	x_pos += 0x80;
//	lcd_command_write(x_pos);
//	lcd_delay(5);
//	RS=1;
//	EN=0;
//	lcd_delay(5);
//	temp=(temp&0xf00f)|(lcd_dat<<4);
//	GPIO_Write(GPIOB,temp); 
//	lcd_delay(10);
//	EN=1;
//	lcd_delay(10);
//	EN=0; 
//}









/*******************************************************************************
* Function Name  : LCD1602_Init
* Description    : 1602???
* Input          : None
* Output         : None
* Return         : None
* Date           : 2021.4.10
*******************************************************************************/
void LCD1602_Init(void) 
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	                                                     //使能PA,PB时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13;       //LCD1602的8跟数据线	    		
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);//开启SWD，失能JTAG，PB4不是独立引脚
    
    delay_ms(10);
    LCD1602_Write_Cmd(0x38);
    LCD1602_Write_Cmd(0x38);
    LCD1602_Write_Cmd(0x08);
    LCD1602_Write_Cmd(0x01);
    LCD1602_Write_Cmd(0x06);
    LCD1602_Write_Cmd(0x0c);
}

/*******************************************************************************
* Function Name  : LCD1602IO_Output
* Description    : 1602?????????????
* Input          : data:?????
* Output         : None
* Return         : None
* Date           : 2021.4.10
*******************************************************************************/
void LCD1602IO_Output(INT8U data)
{
    //???1????1,???1??1?????
    if((data & 0X80) == 0X80)   
    {
        D7_HIGH();
    }
    else
    {
        D7_LOW();
    }
    //???2????1,???2??1?????
    if((data & 0X40) == 0X40)   
    {
        D6_HIGH();
    }
    else
    {
        D6_LOW();
    }
    //???3????1,???3??1?????
    if((data & 0X20) == 0X20)   
    {
        D5_HIGH();
    }
    else
    {
        D5_LOW();
    }
    //???4????1,???4??1?????
    if((data & 0X10) == 0X10)   
    {
        D4_HIGH();
    }
    else
    {
        D4_LOW();
    }
    //???5????1,???5??1?????
    if((data & 0X08) == 0X08)   
    {
        D3_HIGH();
    }
    else
    {
        D3_LOW();
    }
    //???6????1,???6??1?????
    if((data & 0X04) == 0X04)   
    {
        D2_HIGH();
    }
    else
    {
        D2_LOW();
    }
    //???7????1,???7??1?????
    if((data & 0X02) == 0X02)   
    {
        D1_HIGH();
    }
    else
    {
        D1_LOW();
    }
    //???8????1,???8??1?????
    if((data & 0X01) == 0X01)   
    {
        D0_HIGH();
    }
    else
    {
        D0_LOW();
    }
}

/*******************************************************************************
* Function Name  : LCD1602_Write_Cmd
* Description    : 1602???
* Input          : data:??
* Output         : None
* Return         : None
* Date           : 2021.4.10
*******************************************************************************/
void LCD1602_Write_Cmd(INT8U cmd)	 			  
{
    //???     ??:RS=L,RW=L,E=????? 
    RS_LOW(); 
    RW_LOW();
    EN_LOW();

    LCD1602IO_Output(cmd);
    
    delay_ms(10); 
    EN_HIGH();
    delay_ms(10); 
    EN_LOW(); 
}
 
/*******************************************************************************
* Function Name  : LCD1602_Write_Data
* Description    : 1602???
* Input          : data:??
* Output         : None
* Return         : None
* Date           : 2021.4.10
*******************************************************************************/
void LCD1602_Write_Data(INT8U data)	     	  
{ 
    //???     ??:RS=H,RW=L,E=?????
    RS_HIGH();	
    RW_LOW(); 
    EN_LOW();

    LCD1602IO_Output(data);	
    
    delay_ms(10); 
    EN_HIGH();    
    delay_ms(10); 
    EN_LOW();
}

void LCD_Write_String(unsigned char x,unsigned char y,unsigned char *s) 
{ 
	if (y == 0) 
	{     
		LCD1602_Write_Cmd(0x80 + x);     //?????
	}
	else 
	{      
		LCD1602_Write_Cmd(0xC0 + x);      //?????
	}        
	while (*s) 
	{     
		LCD1602_Write_Data( *s);     
		s ++;     
	}
}























































