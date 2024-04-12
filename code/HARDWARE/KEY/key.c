#include "key.h"
#include "delay.h"

//������ʼ������ 
//PA0���ó�����
void KEY_Init(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;

	//ʹ����Ӧ�ӿڵ�ʱ�ӣ��Լ�RCC_APB2Periph_AFIO
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOF, ENABLE);
//	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);//��ȫ����SWD��JTAG 
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);     //����JTAG
	
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);//ʹ��PORTAʱ��
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;//PA0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA0

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_14 | GPIO_Pin_15;//PA0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOA0	
} 


