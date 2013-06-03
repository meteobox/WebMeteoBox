/**
 * @file		delay.c
 * @brief 		waiting functions
 */
#include <util/delay.h>
#include "types.h"
#include "delay.h"


//---- MODIFY_2005_10_31 : PM-A1 V1.1 --> V1.2 (16MHz --> 8MHz)
void wait_1us(u_int cnt)
{
	/* 16MHz : 16 CLK 1us : 1 + (1 + (1+1)*4 + 1 + (2+1))*cnt + 1  + 1*/ 
	/*	
	asm volatile
	(
		"movw	r24, %A0"		"\n\t"
		"L_US:"				"\n\t"
		"ldi	r26, lo8(4)"	 	"\n\t"
		"L_US0:"			"\n\t"
		"dec	r26"			"\n\t"
		"brne	L_US0"			"\n\t"
		"sbiw	r24, 1"			"\n\t"
		"brne	L_US"			"\n\t"
		"nop"				"\n\t"
		:  :"r" (cnt)
	);	
	*/	
	/* 8MHz : 8 CLK 1us : 1 + (1*5 + (2+1))*cnt + 1  + 1*/ 
	
/*
	asm volatile
	(
		"movw	r24, %A0"		"\n\t"
		"L_US:"				"\n\t"
		"nop"				"\n\t"
		"nop"				"\n\t"
		"nop"				"\n\t"
		"nop"				"\n\t"
		"nop"				"\n\t"
		"sbiw	r24, 1"			"\n\t"
		"brne	L_US"			"\n\t"
		"nop"				"\n\t"
		:  :"r" (cnt)
	);	
	
	//+2008/01/03 - jhpark	winaver 2007 version inline asm option
	asm volatile
	(
		"movw	r24, %A0"		"\n\t"
		"L_US%=:"				"\n\t"
		"nop"				"\n\t"
		"nop"				"\n\t"
		"nop"				"\n\t"
		"nop"				"\n\t"
		"nop"				"\n\t"
		"sbiw	r24, 1"			"\n\t"
		"brne	L_US%="			"\n\t"
		"nop"				"\n\t"
		:  :"r" (cnt)
	);	
*/
	//+2008/11/20 - jhpark	use WinAVR Lib for winaver 2008 version
	_delay_us(cnt);	
}
//---- END_MODIFY

/*
********************************************************************************
*               WAIT FUNCTION
*
* Description : This function waits for 10 milliseconds
* Arguments   : cnt - is the time to wait
* Returns     : None
* Note        : Internal Function
********************************************************************************
*/
void wait_10ms(u_int cnt)
{
	for (; cnt; cnt--) wait_1ms(10);
}


/*
********************************************************************************
*               WAIT FUNCTION
*
* Description : This function waits for 1 milliseconds
* Arguments   : cnt - is the time to wait
* Returns     : None
* Note        : Internal Function
********************************************************************************
*/
void wait_1ms(u_int cnt)
{
	for (; cnt; cnt--) wait_1us(1000);
}
