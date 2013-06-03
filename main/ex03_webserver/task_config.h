#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#define EEPOP						0
#define SPECIALOP					0x82

#define VER_H						1
#define VER_L						1
#define SOCK_CONFIG					3
#define EEPADDR_VER					4

#define SBUF_LENGTH					253

// Special char values
#define CR                 			0x0D
#define LF                 			0x0A

#define CONFIG_CLIENT_PORT			1444 // old = 1460  
#define CONFIG_SERVER_PORT			8444 // old 8001   
//#define CONFIG_UP_PORT				48715

/*
typedef struct
{
	unsigned char op[4];
	unsigned char ver[2];
	unsigned char mac[6];
	unsigned char ip[4];
	unsigned char subnet[4];
	unsigned char gw[4];
	unsigned char dhcp;	
} StrConfigParam;
*/


typedef struct 
{
	unsigned char op[4];
	unsigned char ver[2];
	unsigned char mac[6];
	unsigned char ip[4];
	unsigned char subnet[4];
	unsigned char gw[4];
	unsigned char dns[4]; // 17.12.2012
	unsigned char dhcp;	
} StrConfigParam;

typedef struct 
{
	unsigned char op[4];
	unsigned char Gw[4];
	unsigned char Subnet[4];
	unsigned char Lip[4];
	unsigned char Mac[6];
	unsigned char dns[4]; // 17.12.2012
} CONFIG_MSG;

typedef struct 
{
	unsigned char enable;
	unsigned char refresh_time;
	unsigned char server_name[30];
	unsigned char server_ip[4];
	unsigned char script_path[55];
	unsigned int  port;
} Server_config;



// на ip 159.93.87.211
//#define DefConfigParam			"\x82\x82\x82\x82\x00\x00\x00\x08\xDC\x00\x02\x05\x9f\x5d\x57\xd3\xFF\xFF\xFF\x00\x9f\x5d\x57\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
//																												  192 168  1   1 	
//#define DefConfigParam			"\x82\x82\x82\x82\x00\x00\x00\x08\xDC\x00\x02\x05\x9f\x5d\x57\xd2\xFF\xFF\xFF\x00\x9f\x5d\x57\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
//#define DefConfigParam		"\x82\x82\x82\x82\x00\x00\x00\x08\xDC\x00\x00\x00\xC0\xA8\x01\x5C\xFF\xFF\xFF\x00\xC0\xA8\x01\x5E\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"


#define GoBoot()	do{								\
						MCUCR |= _BV(IVCE);			\
						MCUCR |= _BV(IVSEL);		\
						((void (*)())0x1e000)();	\
					}while(0)

#define GoApp()		do{								\
						MCUCR |= _BV(IVCE);		\
						MCUCR &= ~_BV(IVSEL);		\
						((void (*)())0x0000)();		\
					}while(0)					
//-----------------------------------------------------------------------------
void default_network(void);
void SetConfig(void);
void ProcessConfig(unsigned char Sock);
void write_end_page(void);
