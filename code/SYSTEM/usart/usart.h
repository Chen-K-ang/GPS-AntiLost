#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//Mini STM32开发板
//串口1初始化		   
//正点原子@ALIENTEK
//技术论坛:www.openedv.csom
//修改日期:2011/6/14
//版本：V1.4
//版权所有，盗版必究。
//Copyright(C) 正点原子 2009-2019
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
////////////////////////////////////////////////////////////////////////////////// 	

#define SIZEBUF 110

extern unsigned char BufTab[10];
extern unsigned char Count;	   
extern unsigned char UartBusy; 

extern unsigned char timebuf[SIZEBUF];//暂存串口数据
extern unsigned char clear;	 //串口buf清空计数
extern unsigned char count; 	 //串口接收计数
extern unsigned char readFlag;	//读取短信标志

void uart_init(u32 bound);
void uart2_init(u32 bound);
void UART_SendStr(USART_TypeDef* USARTx, char* str, u16 legth);

extern unsigned char sysmode_GPS;
extern unsigned char gps_infor_weijing[17];
extern unsigned char rendFlag;
extern unsigned char	gps_infor_time[6];        //暂存时间
extern unsigned char	gps_infor_date[6];        //暂存日期

//如果想串口中断接收，请不要注释以下宏定义
void uart_init(u32 bound);
#endif


