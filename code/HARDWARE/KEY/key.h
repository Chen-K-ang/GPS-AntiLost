#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"

#define key1 PAin(4)
#define key2 PAin(5)
#define key3 PAin(6)
#define key4 PAin(7)
#define key5 PBin(14)
#define key6 PBin(15)

void KEY_Init(void);//IO初始化
u8 KEY_Scan(u8 mode);  	//按键扫描函数

#endif
