/*
 * WebMeteoBox source code. ver 1.0.0.0
 * Created by Zager V.B. and Krylov A.I. 2012-2013 Dubna (c)
 * 
 * Project home page: http://meteobox.tk
 * Email: valery@jinr.ru
 *
 *
 * 03.06.2013 
 *
 */
 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

#include "../mcu/types.h"
//#include "../mcu/serial.h"
#include "../evb/evb.h"
#include "../evb/config.h"
//#include "../evb/lcd.h"
#include "../mcu/delay.h"
#include "../main/ex03_webserver/protocol.h"

#define	ATMEGA128_0WAIT		0
#define	ATMEGA128_1WAIT		1
#define	ATMEGA128_2WAIT		2
#define	ATMEGA128_3WAIT		3
#define 	ATMEGA128_NUM_WAIT	ATMEGA128_0WAIT


extern SYSINFO	SysInfo;

void evb_init(void)
{
mcu_init();	

init();
//    uart_init(0, 7);
	
// Serial Port Initialize
//_______________RS-485______________________________  

//  UCSR0C =((1<<UPM10)|(1<<UCSZ10)|(1<<UCSZ00));
/* Set baud rate */
//  UBRR0H = 0x00;//(unsigned char) (baud>>8); 
//  UBRR0L = 0x00;//(unsigned char) baud; 921,600 kbod
//  UBRR0L = 0x07;//(unsigned char) baud; 115,200 kbod
//  UBRR0L = 0x0F;//(unsigned char) baud; 57,600 kbod
//  UBRR0L = 0x2F;//(unsigned char) baud; 19,200 kbod
//  UBRR0L = 0x80;//(unsigned char) baud; 7,200 kbod
/* Enable receiver and transmitter */
//  UCSR0B = ((1<<RXCIE0)|(1<<TXCIE0)|(1<<RXEN0)|(1<<TXEN0));
/* Set frame format: 8data,1 none parity,1stop bit */
//  UCSR0C =0x0E;

//fdevopen((void *)uart0_putchar, (void *)uart0_getchar);

}



void mcu_init(void) 
{
	cli();

	EICRA = 0x00;			// External Interrupt Control Register A clear
	EICRB = 0x02;			// External Interrupt Control Register B clear // edge 
	EIMSK = (1 << INT0);		// External Interrupt Mask Register : 0x10
	EIFR = 0xFF;			// External Interrupt Flag Register all clear

	PORTE=0xFF; //0xfb
	DDRE=0xF6; //0x76
	
	PORTB=0xFF; // rj45
	DDRB=0xFF;  // rj45
	
	PORTD |= (1<<7); // XP3 - Hardware Reset 13.05.2013
	DDRD &= ~(1<<7); // XP3 - Hardware Reset 13.05.2013
	
	PORTF=0x00; // 17.01.2013 для датчика давления
	DDRF=0x00;  // 17.01.2013 для датчика давления
	
// ADC initialization
// ADC Clock frequency: 115,200 kHz
// ADC Voltage Reference: AVCC pin
// ADMUX=ADC_REF_AVCC & 0xff;
	ADMUX=ADC_REF_VREF & 0xff;
	ADCSRA=0x87; 


//_______________таймер-0 кварц 8 мгц ________________________  
	ASSR  = 0x00;
	TCCR0 = 0x0F;// TIMER1 clock is xtal/1024
	OCR0  = 0x4E;// при xtal/1024 -- 10mc


//_______________таймер-0 кварц 14.7456________________________  
//	ASSR  = 0x00;
//	TCCR0 = 0x0F;// TIMER1 clock is xtal/1024
//	OCR0  = 0x90;// при xtal/1024 -- 10mc
//	OCR0  = 0x48;// при xtal/1024 -- 5mc

//_______________enable_Timer_Interrapt's_____
	TIFR   = 0x00;// clear TIMER1 interrupts flags
	TIMSK  = 0x02;//enable TIMER0 compare interrupt //0x04;// enable TIMER1 overflow interrupt
	
	MCUCR = 0x80;		// MCU control regiseter : enable external ram
	XMCRA = 0x00;		// External Memory Control Register A : 
						// Low sector   : 0x1100 ~ 0x7FFF
						// Upper sector : 0x8000 ~ 0xFFFF

	sei();				// enable interrupts
}


void evb_soft_reset(void)
{
//	set_reset_flag(SYSTEM_AUTO_RESET);
	asm volatile("jmp 0x0000");	
}
