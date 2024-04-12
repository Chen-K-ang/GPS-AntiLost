#ifndef _1602_H
#define _1602_H

#include "sys.h"
#include "delay.h"

//#define RS PBout(12)
//#define EN PBout(13)

//void lcd_char_write(unsigned int x_pos,unsigned int y_pos,unsigned int lcd_dat);
//void lcd_system_reset(void);
//void lcd_command_write( unsigned int command);
//void lcd_busy_wait(void);
//void lcd_delay( unsigned char ms);
//void GPIO_InitStructReadtempCmd(void);


#define RS_HIGH() 	  			GPIOB->BSRR = GPIO_Pin_13
#define RS_LOW() 				GPIOB->BRR = GPIO_Pin_13

#define RW_HIGH() 	  			GPIOB->BSRR = GPIO_Pin_12
#define RW_LOW() 				GPIOB->BRR = GPIO_Pin_12

#define EN_HIGH() 	  			GPIOB->BSRR = GPIO_Pin_11
#define EN_LOW() 				GPIOB->BRR = GPIO_Pin_11

#define D0_HIGH() 	  			GPIOB->BSRR = GPIO_Pin_10
#define D0_LOW() 				GPIOB->BRR = GPIO_Pin_10

#define D1_HIGH() 	  			GPIOB->BSRR = GPIO_Pin_9
#define D1_LOW() 				GPIOB->BRR = GPIO_Pin_9

#define D2_HIGH() 	  			GPIOB->BSRR = GPIO_Pin_8
#define D2_LOW() 				GPIOB->BRR = GPIO_Pin_8

#define D3_HIGH() 	  			GPIOB->BSRR = GPIO_Pin_7
#define D3_LOW() 				GPIOB->BRR = GPIO_Pin_7

#define D4_HIGH() 	  			GPIOB->BSRR = GPIO_Pin_6
#define D4_LOW() 				GPIOB->BRR = GPIO_Pin_6

#define D5_HIGH() 	  			GPIOB->BSRR = GPIO_Pin_5
#define D5_LOW() 				GPIOB->BRR = GPIO_Pin_5

#define D6_HIGH() 	  			GPIOB->BSRR = GPIO_Pin_4
#define D6_LOW() 				GPIOB->BRR = GPIO_Pin_4

#define D7_HIGH() 	  			GPIOB->BSRR = GPIO_Pin_3
#define D7_LOW() 				GPIOB->BRR = GPIO_Pin_3




void LCD1602IO_Output(INT8U data);
void LCD1602_Init(void);  
void LCD1602_Write_Cmd(INT8U com); 
void LCD1602_Write_Data(INT8U data); 
void LCD_Write_String(unsigned char x,unsigned char y,unsigned char *s) ; 


























#endif
