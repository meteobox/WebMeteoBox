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
 
#ifndef _SMB6PROTOCOL_
#define _SMB6PROTOCOL_

#define eeprom_login        0x40        /* добавлено 18.10.2010 */
#define eeprom_password     0x50		/* добавлено 18.10.2010 */

#define	MAX_BUF_SIZE	2048	        /* maximum size of Rx buffer. 2048 */

#define eeprom_server_config  0x0A0 // 0a1
#define eeprom_sensors_config 0x1A0 // Адрес для сохранения параметров датчиков
#define eeprom_relay_config   0x1A0+MAX_SENSORS*SIZE_SENSOR_STRUCT // Адрес для сохранения параметров датчиков

typedef	unsigned char	BYTE;		/* 8-bit value */
typedef	unsigned char	UCHAR;		/* 8-bit value */
typedef	unsigned int	INT;		/* 16-bit value */
typedef	unsigned int	UINT;		/* 16-bit value */
typedef	unsigned short	USHORT;		/* 16-bit value */
typedef	unsigned short	WORD;		/* 16-bit value */
typedef	unsigned long	ULONG;		/* 32-bit value */
typedef	unsigned long	DWORD;		/* 32-bit value */

char MLogin[15];  // Login
char MPasswd[15]; // Password

extern unsigned char count_ds18b2;
extern u_long remote_server_ip;

extern u_char d[9];
extern void data_get(char *x, char base);

extern void init (void);
extern void recive_rs0(BYTE socket_i);
extern void recive_rs1(BYTE socket_i);
extern void exe_raw0(BYTE socket_i, BYTE *cmd, char lengs);
extern void exe_raw1(BYTE socket_i, BYTE *cmd, char lengs);
extern void proc_http2(SOCKET s, u_char * buf, int len);
extern void Print(char *str);
extern void exe_18b20();
extern void exe_DHT();
extern void exe_web_client();
extern void exe_sensors();
extern void exe_relay();
extern float myatof(char *s);
extern float powi(int x, int y);
extern void LED_ON_OFF( char ckl );
extern void PB_WRITE( char pin, char val );
extern char PB_READ(char pin);


#endif
