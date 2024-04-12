#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//Mini STM32������
//����1��ʼ��		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.csom
//�޸�����:2011/6/14
//�汾��V1.4
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
//********************************************************************************
//V1.3�޸�˵�� 
//֧����Ӧ��ͬƵ���µĴ��ڲ���������.
//�����˶�printf��֧��
//�����˴��ڽ��������.
//������printf��һ���ַ���ʧ��bug
//V1.4�޸�˵��
//1,�޸Ĵ��ڳ�ʼ��IO��bug
//2,�޸���USART_RX_STA,ʹ�ô����������ֽ���Ϊ2��14�η�
//3,������USART_REC_LEN,���ڶ��崮�����������յ��ֽ���(������2��14�η�)
//4,�޸���EN_USART1_RX��ʹ�ܷ�ʽ


//����봮���жϽ��գ��벻Ҫע�����º궨��
extern unsigned char relayFlag;
void uart_init(u32 bound);
void uart2_init(u32 bound);
void UART_SendStr(USART_TypeDef* USARTx, char* str, u16 legth);

#define	SIZEBUF	(110)  //��������

extern unsigned char timebuf[SIZEBUF];//�ݴ洮������
extern unsigned char clear;	 //����buf��ռ���
extern unsigned char count; 	 //���ڽ��ռ���
extern unsigned char readFlag;	//��ȡ���ű�־

extern unsigned char  sysmode_GPS;
extern unsigned char	gps_infor_weijing[17];
extern unsigned char rendFlag;
extern unsigned char	gps_infor_time[6];        //�ݴ�ʱ��
extern unsigned char	gps_infor_date[6];        //�ݴ�����
	
#endif


