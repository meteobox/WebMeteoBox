/**********************************************************

Software I2C Library for AVR Devices.

Copyright 2008-2012
eXtreme Electronics, India
www.eXtremeElectronics.co.in
**********************************************************/
 
#include <avr/io.h>
#include <util/delay.h>

#include "i2csoft.h"
#include "term.h"

/*
#define Q_DEL _delay_loop_2(3)
#define H_DEL _delay_loop_2(5)
*/

#define Q_DEL pause()
#define H_DEL pause()

#define SCLPORT	PORTE	//TAKE PORTD as SCL OUTPUT WRITE
#define SCLDDR	DDRE	//TAKE DDRB as SCL INPUT/OUTPUT configure

#define SDAPORT	PORTE	//TAKE PORTD as SDA OUTPUT WRITE
#define SDADDR	DDRE	//TAKE PORTD as SDA INPUT configure

#define SDAPIN	PINE	//TAKE PORTD TO READ DATA
#define SCLPIN	PINE	//TAKE PORTD TO READ DATA

//#define SCL	PE2		//PORTD.0 PIN AS SCL PIN
//#define SDA	PE3		//PORTD.1 PIN AS SDA PIN

#define SCL	2		//PORTD.0 PIN AS SCL PIN
#define SDA	3		//PORTD.1 PIN AS SDA PIN


#define SOFT_I2C_SDA_LOW	SDADDR|=((1<<SDA))
#define SOFT_I2C_SDA_HIGH	SDADDR&=(~(1<<SDA))

#define SOFT_I2C_SCL_LOW	SCLDDR|=((1<<SCL))
#define SOFT_I2C_SCL_HIGH	SCLDDR&=(~(1<<SCL))


void pause(void)        //Фиксированная во времени пауза
{
    static int p;              //Счетчик времени
    for (p=10; p > 0; p--);  //Задержка на TIME-итераций
    return;            //Возврат после выполнения функции
}  


void SoftI2CInit()
{
	SDAPORT&= (1<<SDA);
	SCLPORT&= (1<<SCL);
	/*
	PORTE &= (1<<5);
	PORTE &= (1<<4);
	*/
	
	SOFT_I2C_SDA_HIGH;	
	SOFT_I2C_SCL_HIGH;	
		
}
void SoftI2CStart()
{
	SOFT_I2C_SCL_HIGH;
	H_DEL;
	
	SOFT_I2C_SDA_LOW;	
	H_DEL;  	
}

void SoftI2CStop()
{
	 SOFT_I2C_SDA_LOW;
	 H_DEL;
	 SOFT_I2C_SCL_HIGH;
	 Q_DEL;
	 SOFT_I2C_SDA_HIGH;
	 H_DEL;
}

uint8_t SoftI2CWriteByte(uint8_t data)
{
	 
	 uint8_t i;
	 	
	 for(i=0;i<8;i++)
	 {
		SOFT_I2C_SCL_LOW;
		Q_DEL;
		
		if(data & 0x80)
			SOFT_I2C_SDA_HIGH;
		else
			SOFT_I2C_SDA_LOW;	
		
		H_DEL;
		
		SOFT_I2C_SCL_HIGH;
		H_DEL;
		
		while((SCLPIN & (1<<SCL))==0);
			
		data=data<<1;
	}
	 
	//The 9th clock (ACK Phase)
	SOFT_I2C_SCL_LOW;
	Q_DEL;
		
	SOFT_I2C_SDA_HIGH;		
	H_DEL;
		
	SOFT_I2C_SCL_HIGH;
	H_DEL;	
	
	uint8_t ack=!(SDAPIN & (1<<SDA));
	
	SOFT_I2C_SCL_LOW;
	H_DEL;
	
	return ack;
	 
}
 
 
uint8_t SoftI2CReadByte(uint8_t ack)
{
	uint8_t data=0x00;
	uint8_t i;
			
	for(i=0;i<8;i++)
	{
			
		SOFT_I2C_SCL_LOW;
		H_DEL;
		SOFT_I2C_SCL_HIGH;
		H_DEL;
			
		while((SCLPIN & (1<<SCL))==0);
		
		if(SDAPIN &(1<<SDA))
			data|=(0x80>>i);
			
	}
		
	SOFT_I2C_SCL_LOW;
	Q_DEL;						//Soft_I2C_Put_Ack
	
	if(ack)
	{
		SOFT_I2C_SDA_LOW;	
	}
	else
	{
		SOFT_I2C_SDA_HIGH;
	}	
	H_DEL;
	
	SOFT_I2C_SCL_HIGH;
	H_DEL;
	
	SOFT_I2C_SCL_LOW;
	H_DEL;
			
	SOFT_I2C_SDA_HIGH; // 28.03.2013
			
	return data;
	
}

int read_bmp085_int_register(unsigned char r)
{
unsigned char msb, lsb;

    SoftI2CStart();
	SoftI2CWriteByte(0xEE);
	SoftI2CWriteByte(r);

	SoftI2CStart();
	SoftI2CWriteByte(0xEF);
	
	
	msb=SoftI2CReadByte(1);
	lsb=SoftI2CReadByte(0);
	
	SoftI2CStop();
	
	return(((int)msb<<8) | ((int)lsb));
}


unsigned int bmp085_read_ut() {

 SoftI2CStart();
 SoftI2CWriteByte(0xEE);
 SoftI2CWriteByte(0xf4);
 SoftI2CWriteByte(0x2e);
 SoftI2CStop();

 my_wait_1ms(5);
 
 return read_bmp085_int_register(0xf6);
 }
 
 
void bmp085_read_up(long int * ln) {

 unsigned char msb, lsb, xlsb;

 SoftI2CStart();
 SoftI2CWriteByte(0xEE);
 SoftI2CWriteByte(0xf4);
 SoftI2CWriteByte(0xf4); //f4
 SoftI2CStop();

 my_wait_1ms(26);
 

    SoftI2CStart();
	SoftI2CWriteByte(0xEE);
	SoftI2CWriteByte(0xf6);

	SoftI2CStart();
	SoftI2CWriteByte(0xEF);
	
	
	msb=SoftI2CReadByte(1);
	lsb=SoftI2CReadByte(1);
	/*
	SoftI2CStop();
	
	
	SoftI2CStart();
	SoftI2CWriteByte(0xEE);
	SoftI2CWriteByte(0xf8);

	SoftI2CStart();
	SoftI2CWriteByte(0xEF);
	*/
	
	xlsb=SoftI2CReadByte(0);
	
	SoftI2CStop();


	*ln=(((long int)msb<<16) + ((long int)lsb<<8) + ((long int)xlsb)) >> (8-3);

 }
















