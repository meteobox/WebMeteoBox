// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
*
* Atmel Corporation
*
* \li File:               OWIHighLevelFunctions.h
* \li Compiler:           IAR EWAAVR 3.20a
* \li Support mail:       avr@atmel.com
*
* \li Supported devices:  All AVRs.
*
* \li Application Note:   AVR318 - Dallas 1-Wire(R) master.
*                         
*
* \li Description:        Header file for OWIHighLevelFunctions.c
*
*                         $Revision: 1.7 $
*                         $Date: Thursday, August 19, 2004 14:27:18 UTC $
*
*    � ������� �������� ���� ����. ������� ������� ������ ������� 1Wire
*    ���������, ��������� �� ����, � ������� ������ ����������� ����������
*    ����� ������������. ������� � ���� �� ����������������� ������� AVR318
*    � ��������� ��.
*                                    Pashgan  http://ChipEnable.Ru
*
****************************************************************************/

#ifndef _OWI_ROM_FUNCTIONS_H_
#define _OWI_ROM_FUNCTIONS_H_

#include <string.h> // Used for memcpy.


typedef struct
{
    unsigned char id[8];    //!< The 64 bit identifier.
	unsigned char scratchpad[9];
	unsigned int temperature;	
} OWI_device;


typedef struct  // ����� ������� ��� � EEPROM! 28.12.2012
{
    float         value;           // ������� ��������
	unsigned char id[8];           //!< The 64 bit identifier.
	unsigned char name[15];        // ��� ����������
	unsigned char type;	           // ���: ��������,�����������,���������
	unsigned char flag;			   // ���������� �������� � ��������?
	float         offset;	       // �������� ��� ����������
	//float         min;             // min
	//float         max;             // max
	//char          pio;	        	// ����� ����� atmega
} sensor_structure;


typedef struct  // ����� ������� ��� � EEPROM! 18.02.2013
{
	unsigned char id[8];           //!< The 64 bit identifier.
	unsigned char flag;			   // ��������� �������������?	0=off, 
    float         min;             // min
	float         max;             // max
	unsigned char pio;            // ����� ����� atmega
	unsigned char opearnd;	       // ???
} io_structure;


#define SEARCH_SUCCESSFUL     0x00
#define SEARCH_CRC_ERROR      0x01
#define SEARCH_ERROR          0xff
#define AT_FIRST              0xff

void OWI_SendByte(unsigned char data, unsigned char pin);
unsigned char OWI_ReceiveByte(unsigned char pin);
void OWI_SkipRom(unsigned char pin);
void OWI_ReadRom(unsigned char * romValue, unsigned char pin);
void OWI_MatchRom(unsigned char * romValue, unsigned char pin);
unsigned char OWI_SearchRom(unsigned char * bitPattern, unsigned char lastDeviation, unsigned char pins);
unsigned char OWI_SearchDevices(OWI_device * devices, unsigned char numDevices, unsigned char pin, unsigned char *num);
unsigned char FindFamily(unsigned char familyID, OWI_device * devices, unsigned char numDevices, unsigned char lastNum);

#endif
