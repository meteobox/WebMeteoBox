//-----------------------------------------------------------------------------
//AVR Mega128 SPI HAL
#define BIT0							0x01
#define BIT1							0x02
#define BIT2							0x04
#define BIT3							0x08
#define BIT4							0x10
#define BIT5							0x20
#define BIT6							0x40
#define BIT7							0x80

#define Sbi(x,y)					(x |= (y))
#define Cbi(x,y)					(x &=~(y))
#define ChkBit(x,y)					(x  & (y))

#define SPI0_SS_BIT						BIT0
#define SPI0_SS_DDR						DDRB
#define SPI0_SS_PORT					PORTB

#define SPI0_SCLK_BIT					BIT1
#define SPI0_SCLK_DDR					DDRB
#define SPI0_SCLK_PORT					PORTB

#define	SPI0_MOSI_BIT					BIT2
#define SPI0_MOSI_DDR					DDRB
#define SPI0_MOSI_PORT					PORTB

#define	SPI0_MISO_BIT					BIT3
#define SPI0_MISO_DDR					DDRB
#define SPI0_MISO_PORT					PORTB


#define SPI0_WaitForReceive()				
#define SPI0_RxData()	 				(SPDR)

#define SPI0_TxData(Data)				(SPDR = Data)
#define SPI0_WaitForSend()				while( (SPSR & 0x80)==0x00 )

#define SPI0_SendByte(Data)				SPI0_TxData(Data);SPI0_WaitForSend()
#define SPI0_RecvBute()					SPI0_RxData()


#define SPI0_INT_ENABLE()					Sbi(SPCR, SPI0_INT_ENABLE_BIT)
#define SPI0_INT_DISABLE()					Sbi(SPCR, SPI0_INT_ENABLE_BIT)
#define SPI0_ENABLE()						Sbi(SPCR, SPI0_ENABLE_BIT)
#define SPI0_DISABLE()						Cbi(SPCR, SPI0_ENABLE_BIT)
#define SPI0_MASTER_MODE()					Sbi(SPCR, SPI0_MASTER_BIT)
#define SPI0_SLAVE_MODE()					Cbi(SPCR, SPI0_MASTER_BIT)
#define SPI0_CPOL_SET()						Sbi(SPCR, BIT3)
#define SPI0_CPHA_SET()						Sbi(SPCR, BIT2)
#define SPI0_CPOL_CLR()						Cbi(SPCR, BIT3)
#define SPI0_CPHA_CLR()						Cbi(SPCR, BIT2)
#define SPI0_CLK_DIV4()						Cbi(SPCR, BIT1|BIT0)
#define SPI0_CLK_DIV16()					Cbi(SPCR, BIT1);Sbi(SPCR, BIT0)
#define SPI0_CLK_DIV64()					Sbi(SPCR, BIT1);Cbi(SPCR, BIT0)
#define SPI0_CLK_DIV128()					Sbi(SPCR, BIT1|BIT0)
#define SPI0_CLK_DOULBE()					Sbi(SPSR, BIT0)

#define	SPI0_MODE0()						SPI0_CPOL_CLR();SPI0_CPHA_CLR()
#define	SPI0_MODE1()						SPI0_CPOL_CLR();SPI0_CPHA_SET()
#define	SPI0_MODE2()						SPI0_CPOL_SET();SPI0_CPHA_CLR()
#define	SPI0_MODE3()						SPI0_CPOL_SET();SPI0_CPHA_SET()


#define SPI0_SS_WR()						Sbi(SPI0_SS_DDR, SPI0_SS_BIT)
#define SPI0_SS_HIGH()						Sbi(SPI0_SS_PORT, SPI0_SS_BIT)

#define SPI0_SCLK_WR()						Sbi(SPI0_SCLK_DDR, SPI0_SCLK_BIT)
#define SPI0_SCLK_HIGH()					Sbi(SPI0_SCLK_PORT, SPI0_SCLK_BIT)

#define	SPI0_MOSI_WR()						Sbi(SPI0_MOSI_DDR, SPI0_MOSI_BIT)
#define	SPI0_MOSI_H()						Sbi(SPI0_MOSI_PORT, SPI0_MOSI_BIT)

#define SPI0_MISO_RD()						Cbi(SPI0_MISO_DDR, SPI0_MISO_BIT)

#define SPI0_WaitForReceive()				
#define SPI0_RxData()	 					(SPDR)

#define SPI0_TxData(Data)					(SPDR = Data)
#define SPI0_WaitForSend()					while( (SPSR & 0x80)==0x00 )

#define SPI0_INT_ENABLE_BIT					BIT7
#define SPI0_ENABLE_BIT						BIT6
#define SPI0_MASTER_BIT						BIT4

#define SPI0_INT_ENABLE()					Sbi(SPCR, SPI0_INT_ENABLE_BIT)
#define SPI0_INT_DISABLE()					Sbi(SPCR, SPI0_INT_ENABLE_BIT)
#define SPI0_ENABLE()						Sbi(SPCR, SPI0_ENABLE_BIT)
#define SPI0_DISABLE()						Cbi(SPCR, SPI0_ENABLE_BIT)
#define SPI0_MASTER_MODE()					Sbi(SPCR, SPI0_MASTER_BIT)
#define SPI0_SLAVE_MODE()					Cbi(SPCR, SPI0_MASTER_BIT)
#define SPI0_CPOL_SET()						Sbi(SPCR, BIT3)
#define SPI0_CPHA_SET()						Sbi(SPCR, BIT2)
#define SPI0_CPOL_CLR()						Cbi(SPCR, BIT3)
#define SPI0_CPHA_CLR()						Cbi(SPCR, BIT2)
#define SPI0_CLK_DIV4()						Cbi(SPCR, BIT1|BIT0)
#define SPI0_CLK_DIV16()					Cbi(SPCR, BIT1);Sbi(SPCR, BIT0)
#define SPI0_CLK_DIV64()					Sbi(SPCR, BIT1);Cbi(SPCR, BIT0)
#define SPI0_CLK_DIV128()					Sbi(SPCR, BIT1|BIT0)
#define SPI0_CLK_DOULBE()					Sbi(SPSR, BIT0)

#define	SPI0_MODE0()						SPI0_CPOL_CLR();SPI0_CPHA_CLR()
#define	SPI0_MODE1()						SPI0_CPOL_CLR();SPI0_CPHA_SET()
#define	SPI0_MODE2()						SPI0_CPOL_SET();SPI0_CPHA_CLR()
#define	SPI0_MODE3()						SPI0_CPOL_SET();SPI0_CPHA_SET()

#define SPI0_SendByte(Data)					SPI0_TxData(Data);SPI0_WaitForSend()
#define SPI0_RecvByte()						SPI0_RxData()


//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//IInChip SPI HAL
#define IINCHIP_SpiInit					SPI0_Init
#define IINCHIP_SpiSendData				SPI0_SendByte	
#define IINCHIP_SpiRecvData				SPI0_RxData


#define IINCHIP_CS_BIT					0x04
#define IINCHIP_CS_DDR					DDRG
#define IINCHIP_CS_PORT					PORTG

#define IINCHIP_CSInit()				(IINCHIP_CS_DDR |= IINCHIP_CS_BIT)
#define IINCHIP_CSon()					(IINCHIP_CS_PORT |= IINCHIP_CS_BIT)
#define IINCHIP_CSoff()					(IINCHIP_CS_PORT &= ~IINCHIP_CS_BIT)
//-----------------------------------------------------------------------------

void SPI0_Init(void)
{
	SPI0_SS_WR();
	SPI0_SS_HIGH();

	SPI0_SCLK_WR();
	SPI0_SCLK_HIGH();

	SPI0_MOSI_WR();
	SPI0_MISO_RD();

	SPI0_ENABLE();
	SPI0_MASTER_MODE();
	
	SPI0_MODE0();

	SPI0_CLK_DOULBE();
	SPI0_CLK_DIV4();
}
	
//----------------------------------------------------------------
unsigned char SPI0_WriteReadByte(unsigned char Data)
{
	SPI0_TxData(Data);	

	SPI0_WaitForSend();
 
	return SPI0_RxData();
}

void SPI0_WriteByte(unsigned char Data)
{
	SPI0_TxData(Data);	

	SPI0_WaitForSend();	
}
//----------------------------------------------------------------
