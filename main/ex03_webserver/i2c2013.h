#ifndef I2C_H_INCLUDED
#define I2C_H_INCLUDED
//===============================================================

//// подключение I2C
# define PORT_I2C PORTE/*имя порта к которому подключена шина  I2С*/
# define DDR_I2C  DDRE
# define PIN_I2C  PINE
# define SCL  2
# define SDA  3

#define RELEASE_I2C_BUS() { SCL_1(); SDA_1(); }
// ============================================================================
#define TIME	   F_CPU/800000 // константа  задержки
#define NO_I2C_ACK 1
#define OK_I2C_ACK 0

#define ClearBit(reg, bit)        reg &= (~(1<<(bit)))
#define SetBit(reg, bit)          reg |= (1<<(bit))
#define BitIsSet(reg, bit) ((reg & (1<<(bit))) != 0)

//---------------Функция задержки времени---------------
void pause(void)        //Фиксированная во времени пауза
{
    static char p;              //Счетчик времени
    for (p=10; p > 0; p--);  //Задержка на TIME-итераций
    return;            //Возврат после выполнения функции
}                           //Окончание функции "pause"

void SCL_1  (void)
{
    ClearBit(DDR_I2C,SCL);
    pause();
}
void SCL_0 (void)
{
    SetBit(DDR_I2C,SCL);
    pause();
}
void SDA_1 (void)
{
    ClearBit(DDR_I2C,SDA);
    pause();
}
void  SDA_0  (void)
{
    SetBit(DDR_I2C,SDA);
    pause();
}

void i2c_Init (void)
{
ClearBit(PORT_I2C,SDA);
ClearBit(PORT_I2C,SCL);
SCL_1 () ;
SDA_0();
SDA_1();
}
void i2c_Start (void)
{
    SCL_1();
    SDA_1();
    SCL_1();
    SDA_0 ();
    SCL_0 ();
    SDA_0();

}
void i2c_Stop (void)
{
    SCL_0();
    SDA_0();
    SCL_1();
    SDA_0();
    SCL_1();
    SDA_1();
}

char i2c_Write(char a)
{
	char i;
	char return_ack;

	for (i = 0; i < 8; i++)
    {
		SCL_0();
		if (a & 0x80)
			SDA_1();
		else
			SDA_0();

		SCL_1();
		a <<= 1;
	}
	SCL_0();

	/* ack Read */
	SDA_1();
	SCL_1();

	if (BitIsSet(PIN_I2C,SDA))
		return_ack = NO_I2C_ACK;
	else
		return_ack = OK_I2C_ACK;

    SCL_0();

	return (return_ack);
}
#endif // I2C_H_INCLUDED
