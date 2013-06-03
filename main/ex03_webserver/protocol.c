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

/*
20.05.2013 Добавлена поддержка токовых клещей SCT-013-030 (Max 30A)

13.12.2012 Начало работы над прошивкой метеостанции "Дубна", буду портировать поддержку DNS имён!
		   Если Ваше устр-во допускает только ввод IP адреса, то используйте IP 91.122.49.168 или 94.19.113.221 до которого у Вас наименьший ping

05.09.2011 Прошивка для работы с 1-Wire протоколом по шине через PORTF

*/

/*

CREATE TABLE `mcsi`.`room514` (
`id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY ,
`date` DATETIME NOT NULL ,
`serial_code` VARCHAR( 16 ) NOT NULL ,
`value` DOUBLE NOT NULL ,
INDEX ( `serial_code` ) 
) ENGINE = InnoDB COMMENT = '18b20 table'


*/

#include "../../iinchip/socket.h"
#include "../../inet/httpd.h"
#include "protocol.h"
#include "task_config.h"
#include "../../iinchip/w5300.h"
#include "../../evb/evb.h"
#include "term.h"
#include "DHT.h"

#include <stdio.h>
#include <avr/eeprom.h>
#include <string.h>
#include <ctype.h>   
#include <math.h>   


#define DEBUG_METEOBOX /* При отладке включаем */

#define SCT-013-030 /* Включаем есть есть токовые клещи SCT-013-030 */ 


#define eeprom_config_s		0x81		/* адрес в памяти для CONFIG_S[32] */
//___________________________________________________________________________

#define VREF         3.30

#define RS_READY     0 
#define RS_CFG       1
#define RS_SEND      2
#define RS_REPLY     3 

#define FIRMWARE_VERSION "1.0.0"
#define FIRMWARE_BUILD_D "27/05/2013 15:47"

#define my_style  "<html><head><title>Internet Climate Monitoring. (c) ver " FIRMWARE_VERSION " " FIRMWARE_BUILD_D " </title>\n<META HTTP-EQUIV=\"no-cache\">\n<META HTTP-EQUIV=\"Expires\" CONTENT=\"Wed, 8 Mar 2007 00:01:05 GMT\">\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1251\">\n</head>\n<style>input {border:solid 1px #000;} A {COLOR: blue; TEXT-DECORATION: underline} A:hover {COLOR: blue; TEXT-DECORATION: underline}  A:active {COLOR: blue; TEXT-DECORATION: underline} A:visited {COLOR: blue; TEXT-DECORATION: underline} </style><body>\n<SCRIPT LANGUAGE=\"javascript\">function confdel(){\nvar question = confirm(\"Are you sure to reboot?\");\n if (question == false)  return false; else  return true; }</SCRIPT>\n"
#define my_title  "<center><p><font color=black size=5><u><b>Internet Climate Monitoring</b></u></font></p></center>\n"


#define COUNT_AUTOUPDATE 7

//#pragma warning(disable:4996)  

typedef struct mode_block {
    u_char           *name;
    unsigned char   id;
}modes;  

modes update_time[COUNT_AUTOUPDATE] =
  {		 
   "10 seconds", 1,	 // Умножать на 1000. 
   "30 seconds", 3,		 
   "1 minute",   6,  		 
   "3 minute",   18,     
   "5 minute",   30,
   "10 minutes", 60,  		 
   "15 minutes", 90     

  }; 

  



typedef struct smart_block {
    u_char                 *name;
    short int             id;
}sm; 


//typedef struct _HTTP_REQ HTTP_REQ;

#define	TMP_BUF_SIZE	300 // 500 Основная память тратится тут!!! число множим на 2!!!

typedef struct _HTTP_REQ
{
	u_char	METHOD;				/* request method(METHOD_GET...). */
	u_char	TYPE;				/* request type(PTYPE_HTML...).   */
	u_char	URI[TMP_BUF_SIZE];			/* request file name.             */
}HTTP_REQ;


//u_char head[80]; //TMP_BUF_SIZE


void (*appptr)( void ) = 0x0000; // Set up application
void (*bootloaderptr)( void ) = 0xFC00; // Set up bootloader 

u_char *str = (u_char *)(0x1100);
u_char *tx_buf = (u_char *)(0x3600); 

extern StrConfigParam NetworkParam;
extern Server_config server_data[2];
extern unsigned char count_ds18b2=0;

char auth=0; // Признак аутентификации 19.10.2010

//just taken from the BMP085 datasheet
extern int ac1;
extern int ac2;
extern int ac3;
extern unsigned int ac4;
extern unsigned int ac5;
extern unsigned int ac6;
extern int b1;
extern int b2;
extern int mb;
extern int mc;
extern int md;
extern int temperature;
extern long int pressure;



long int	Time_ms = 0L; //86400000L; // 8640000 сутки

char TCP_SEND_DATA(char num);

void proc_http2(SOCKET s, u_char * buf, int len);
void make_head(u_int len);
unsigned char parse_request(HTTP_REQ * request, u_char * buf);

char* itoa_long(
	unsigned long value,	/**< is a integer value to be converted */
	char* str,	/**< is a pointer to string to be returned */
	u_int base	/**< is a base value (must be in the range 2 - 16) */
	);


char sessionid[11]; // Сессия соединения. устанавливается при правильном вводе логина и пароля. 27.12.2012
char usersession[11]="\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"; // Сессия передаваемая пользователем 27.12.2012


int i;
unsigned long int DS18B20refreshTime = 500; // *10ms

unsigned long int WebrefreshTime0 = 6000; // 1min
unsigned long int WebrefreshTime1 = 6000; // 1min

volatile unsigned long int TimeTs0=0L;
volatile unsigned long int TimeWt0=0L;
volatile unsigned long int TimeWt1=0L;
volatile char Tr0  = 0;
volatile char Wt0  = 0; // Logger 1 refresh timer
volatile char Wt1  = 0; // Logger 2 refresh timer
//_______________________________________________________________________
//_______________________________________________________________________
//--------------------Timer's-------------------------------------------------
void SIG_OUTPUT_COMPARE0( void ) __attribute__ ((signal)); 
void SIG_OUTPUT_COMPARE0( void )
{
  Time_ms += 10; 

  if (Wt0){
    if( TimeWt0 > 0) --TimeWt0; 
    else { Wt0 = 0; }
  }
  
  if (Wt1){
    if( TimeWt1 > 0) --TimeWt1; 
    else { Wt1 = 0; }
  }
  
  
  if (Tr0){
    if( TimeTs0 > 0) --TimeTs0; 
    else { Tr0 = 0; }
  }
  
}
//____________________End Timer's_____________________________________________

float powi(int x, int y)   
// Determines x-raised-to-the-power-of-y and returns the   
// result as a float   
 {   
 int d;   
 float p = 1;   
    
 for (d = 0; d < y; d++)   
  p *= x;   
     
 return p;   
 }  
   
float myatof(char *s)   
 {   
 float v = 0.0,   
       scale = 0.1;   
 char  mneg = ' ',   
       eneg = ' ';   
 int   e = 0;   
          
 while (isspace(*s))   
  s++;   
   
 if (*s == '-')   
  mneg = *s++;   
 else   
  if (*s == '+')   
   s++;   
     
 while (isdigit(*s))   
  v = 10.0 * v + *s++ - '0';   
     
 if (*s == '.')    
  s++;   
      
 while(isdigit(*s))    
  {   
  v += (*s++ - '0') * scale;   
  scale /= 10.0;   
  }   
     
 if (toupper(*s) == 'E')    
  {   
  s++;   
  if (*s == '-')   
   eneg = *s++;   
  else    
   if (*s == '+')   
    s++;   
  while (isdigit(*s))   
   e = 10 * e + *s++ - '0';   
  if (eneg == '-')   
   v = v / powi(10,e);   
  else   
   v = v * powi(10,e);   
  }   
     
 if (mneg == '-')   
  v = -v;   
      
 return v;   
 }   
   


float read_adc(unsigned char adc_input)
{  
float adc,bar;
ADMUX=adc_input | (ADC_REF_AVCC & 0xff);
// Start the AD conversion
ADCSRA|=0x40;
// Wait for the AD conversion to complete
while ((ADCSRA & 0x10)==0);
ADCSRA|=0x10;

 adc=((float)(ADCW/1024.0));
 
 
 bar=(adc*VREF); 
  
return bar;
}


unsigned int read_adc_int(unsigned char adc_input)
{  
float adc,bar;
ADMUX=adc_input | (ADC_REF_AVCC & 0xff);
// Start the AD conversion
ADCSRA|=0x40;
// Wait for the AD conversion to complete
while ((ADCSRA & 0x10)==0);
ADCSRA|=0x10;

return ADCW;
}



float read_current()
{
   unsigned int i,a;
   float k=0,l=0;
   
   a=read_adc_int(6);
   
  // for(i=0; read_ADC(1)<10 && i<100; i++)
  // continue;

   for (i=0; i<400; i++)
   {
     k+= abs(read_adc_int(0)-a);
	 _delay_us(25);
   }
   //return k/256;
   k/= 400;
   l=k/1024.0*VREF*30;
   //l*= .10;	// LSB for 30A range
   return 220*l;   // return WATT 220Vrms x Irms
   //return l;
}





/*
typedef struct 
{
    signed   int  value;           // Текущее значение
	unsigned char id[8];           //!< The 64 bit identifier.
	unsigned char name[15];        // Имя устройства
	unsigned char type;	           // Тип: давление,температура,влажность
	unsigned char flag;			   // Отправлять логгером в интернет?
	char          offset;	       // Смещение для калибровки
} sensor_structure;
*/


char LED_READ()
{
  return ((PINE&(1<<6))>0);
}

void LED_ON_OFF( char ckl )
{
  if (ckl == 1) PORTE |= (1<<6);
  else PORTE &= ~(1<<6);
}


char PB_READ(char pin)
{
  return ((PINB&(1<<pin))>0);
}

void PB_WRITE( char pin, char val )
{
  if (val == 1) PORTB |= (1<<pin);
  else PORTB &= ~(1<<pin);
}


void exe_relay() // Работа с Реле
{
char a,b;
	for (a=0; a<MAX_RELAY; a++)
		if (all_relay[a].flag != 0)
		{
			for (b=0; b<count_sensors; b++)
			{
				if (memcmp(all_sensors[b].id,all_relay[a].id,8)==0)				
				{	 			
					//LED_ON_OFF(!LED_READ()); my_wait_1ms(100);
					if ((all_sensors[b].value+all_sensors[b].offset)<all_relay[a].min) PB_WRITE(all_relay[a].pio,0);
					if ((all_sensors[b].value+all_sensors[b].offset)>all_relay[a].max) PB_WRITE(all_relay[a].pio,1);
				}
			}
		}
}


void exe_sensors() // Опрашиваем все сенсоры и обновляем массив all_sensors 28.12.2012
{
char a;
char cnt=0; // счетчик датчиков
float tmp;

	if (!Tr0) { 
		
		LED_ON_OFF(!LED_READ());
		
		
		
		//PB_WRITE(6,PB_READ(2));
		//PB_WRITE(2,!PB_READ(2));
		
		
		TimeTs0=DS18B20refreshTime;
		ReadDHT(DHTPIN);
		if (bGlobalErr==0) {// Есть данные с AM3202 - датчик  влажности и температуры
			// 1 датчик влажности
			all_sensors[0].type=2; // 2 - влажность
			all_sensors[0].value=(dht_hum/10.0);
			
			all_sensors[0].id[0]=0xAA; // Создаём уникальный ID на базе mac адреса
			all_sensors[0].id[1]=0x0B;
			memcpy(&all_sensors[0].id[2],NetworkParam.mac,6);
			
			// 2 датчик темературы
			all_sensors[1].type=1; // 1 - температура
			all_sensors[1].value=(dht_term/10.0);
			all_sensors[1].id[0]=0xAA; // Создаём уникальный ID на базе mac адреса
			all_sensors[1].id[1]=0x0C;
			memcpy(&all_sensors[1].id[2],NetworkParam.mac,6);
			cnt=2;
		}
		/*
		Если есть датчик давления то его значение будет добавлятся тут!
		*/

		asm("WDR");
		if (ac1 != -1 && ac2 != -1 && ac3 != -1)
		{		
			bmp085_read_temperature_and_pressure(&temperature,&pressure);
			all_sensors[cnt].type=3; // 3 - Давление
			all_sensors[cnt].value=(pressure/133.32); // Преобразуем Паскали в Мм
			all_sensors[cnt].id[0]=0xAA; // Создаём уникальный ID на базе mac адреса
			all_sensors[cnt].id[1]=0x0D;
			memcpy(&all_sensors[cnt].id[2],NetworkParam.mac,6);
			cnt++;		
		}
		
		
#ifdef SCT-013-030
		asm("WDR");
		all_sensors[cnt].type=4; // 4 - Мощность Ватт
		all_sensors[cnt].value=read_current();
		all_sensors[cnt].id[0]=0xAA; // Создаём уникальный ID на базе mac адреса
		all_sensors[cnt].id[1]=0x0E;
		memcpy(&all_sensors[cnt].id[2],NetworkParam.mac,6);
		cnt++;		
#endif		
		
		asm("WDR");
		if (count_ds18b2>0) // Если есть датчики температуры DS18B20
		{
			Start_18b20_Convert(OWI_PIN_2);			
			for (a=0; a<count_ds18b2; a++) { 
				//DS18B20_ReadTemperature_Fast(OWI_PIN_2, allDevices[a].id, &allDevices[a].temperature);
				asm("WDR");
				if (DS18B20_ReadTemperature_Fast_Float(OWI_PIN_2, allDevices[a].id,&all_sensors[cnt].value) == READ_SUCCESSFUL)
				{
					//my_wait_1ms(50);
					all_sensors[cnt].type=1; // 1 - температура
					memcpy(&all_sensors[cnt].id,allDevices[a].id,8);
					cnt++;
				}
			}
		}

		count_sensors=cnt;
		Tr0=1;
	}

}



//_____________________инициализация_____________________________________
void init (void)
{
	
	Tr0				= 0;
	TimeTs0         = DS18B20refreshTime;
}



void session_init() // Initialization session array
{
	itoa_long(Time_ms,sessionid,10);
}


void get_title(char *l)
{
	strcat_P(l,PSTR(my_title));
}

void get_style(char *l)
{
	strcat_P(l,PSTR(my_style));
}



void return_menu(char *sid, char *userid)
{
char flag=0;

    if (strcmp(sid,userid)==0) flag=1;	

    if (flag==0) 
	{ 
		strcat_P(str,PSTR("<font color=red><b>You have not entered a correct user login and/or password. <br> Please click BACK, and try again.<b></font>"));
		//sid[0]=0;userid[0]=0;
		return;
	}


	strcat_P(str,PSTR("<a href=?act=View&sid="));
	strcat(str,userid);
	strcat_P(str,PSTR("&>Home</a><br>"));
	
	if (flag==1)	
	{
		strcat_P(str,PSTR("<a href=?act=Setup&sid="));
		strcat(str,userid);
		strcat_P(str,PSTR("&>TCP/IP settings</a><br>"));
		
		strcat_P(str,PSTR("<a href=?act=Change&sid="));
		strcat(str,userid);
		strcat_P(str,PSTR("&>Set password</a><br>"));
		
		strcat_P(str,PSTR("<a href=?act=sens&sid="));
		strcat(str,userid);
		strcat_P(str,PSTR("&>Sensors</a><br>"));
	
		strcat_P(str,PSTR("<a href=?act=logger1&sid="));
		strcat(str,userid);
		strcat_P(str,PSTR("&>Logger 1</a><br>"));
		
		strcat_P(str,PSTR("<a href=?act=logger2&sid="));
		strcat(str,userid);
		strcat_P(str,PSTR("&>Logger 2</a><br>"));		
		
		strcat_P(str,PSTR("<a href=?act=relay&sid="));
		strcat(str,userid);
		strcat_P(str,PSTR("&>Relay</a><br>"));	
		
		strcat_P(str,PSTR("<a href=?act=FW&sid="));
		strcat(str,userid);
		strcat_P(str,PSTR("&>Firmware</a><br>"));		
		
		strcat_P(str,PSTR("<a href=?act=reboot&sid="));
		strcat(str,userid);
		strcat_P(str,PSTR("& onClick=\"return confdel();\">Reboot</a><br>"));
	}

	strcat_P(str,PSTR("<br><a href=?act=Exit&sid="));
	strcat(str,userid);
	strcat_P(str,PSTR("&>Exit</a><br>"));
}



char x[TMP_BUF_SIZE+1]; // Для разбора строки http запроса!
unsigned char * findstr(char * s, char * find, char * stop)
{
int len,len2;
char *x2;
	
	len = strlen(find);

	x2=strstr(s,find);
	
	
	strcpy(x,&x2[len]);
	len = strlen(x);
	
	x2=strstr(x,stop);
	len2 = strlen(x2);
	len = len-len2;
	
	x[len] = '\0';
	s = x;
	
return s; //s
}



u_char d[9];
void data_get(char *x, char base)
{
u_char a,b,n=0,j=0;
u_char buf[10];

	b = strlen(x);	
	for (a=0; a<b; a++)
	 {
	 if (*(x+a) == '.')
	 { 
		buf[j++] = '\0';
		d[n++] = ATOI(buf,base);
		j=0;
	 }
	 else buf[j++] = *(x+a);
	 }
	
	buf[j++] = '\0';
	d[n++] = ATOI(buf,base); 
} // data_get





char* itoa_long(
	unsigned long value,	/**< is a integer value to be converted */
	char* str,	/**< is a pointer to string to be returned */
	u_int base	/**< is a base value (must be in the range 2 - 16) */
	)
{
	char c;
	char* tstr = str;
	char* ret = str;
	if(value == 0) *str++='0';
	while(value > 0)
	{
		*str++ =(char)D2C((char)(value%base));
		value /= base;
	}
	*str-- ='\0';
	while(tstr < str )
	{
		c = *tstr;
		*tstr++ = *str;	
		*str-- = c;
	}
	return ret;
}


unsigned char h2int(char c)
{
    if (c >= '0' && c <='9'){
        return((unsigned char)c - '0');
    }
    if (c >= 'a' && c <='f'){
        return((unsigned char)c - 'a' + 10);
    }
    if (c >= 'A' && c <='F'){
        return((unsigned char)c - 'A' + 10);
    }
    return(0);
}

void urldecode(char *urlbuf)
{
    char c;
    char *dst;
    dst=urlbuf;
    while ((c = *urlbuf)) {
        if (c == '+') c = ' ';
        if (c == '%') {
            urlbuf++;
            c = *urlbuf;
            urlbuf++;
            c = (h2int(c) << 4) | h2int(*urlbuf);
        }
        *dst = c;
        dst++;
        urlbuf++;
    }
    *dst = '\0';
}



// =============================== ДЛЯ ОТЛАДКИ!!!! 08.06.2009
//my_struct_t * pdata = (my_struct_t *) 0x1100; // abs address in external memory
char str2[25];
char strmac[25];
HTTP_REQ request;
	u_char * content;
	u_char * name;
	u_int fl_len, tx_len;	    	
	char flag=0;
	u_char a,b;
	unsigned char c;
	unsigned short int *xy;
	unsigned int pt;
	unsigned long diff,day,hr,min,sec;
	float offset;
// =============================== ДЛЯ ОТЛАДКИ!!!! 08.06.2009
void proc_http2(SOCKET s, u_char * buf, int len)
{
	parse_request(&request, buf);

	switch (request.METHOD)
	{
	case METHOD_ERR :
		//printf("METHOD_ERR : %s\r\n", request.URI);
		break;
	case METHOD_GET :
		name = (u_char *)&(request.URI[1]);
			
			
			memcpy(strmac,findstr(name,"l=","&"),sizeof(strmac)); // Вытаскиваем логин и пароль
			memcpy(str2,findstr(name,"p=","&"),sizeof(str2));   // Вытаскиваем логин и пароль
			memcpy(usersession,findstr(name,"sid=","&"),sizeof(usersession)); 

			//if (strlen(sessionid)==0 && strstr(strmac,MLogin) && strstr(str2,MPasswd)) 
			if (strstr(strmac,MLogin) && strstr(str2,MPasswd)) 
			{
				itoa_long(Time_ms,sessionid,10); // Проверяем и создаём сессию.
				strcpy(usersession,sessionid);
			}
			
			if (strlen(sessionid)==0) session_init();
				
				if (strcmp(usersession,sessionid)==0) // Проверяем сессию
				{					
					
					strcpy(strmac,findstr(name,"fwd=","&")); // 19.12.2012
					if (strstr(strmac,"YES")) // Ввели YES значит переходим в bootloader!
					{
						flag=12;						
					}
					
					strcpy(strmac,findstr(name,"submitpass=","&")); // 18.10.2010
					if (strstr(strmac,"Set")) // ВВели новый логин пароль и нажали кнопку изменить!
					{
						flag=14;						
					}

					strcpy(strmac,findstr(name,"wiresave0=","&"));
					if (strstr(strmac,"Save")) // if Save Logger1 Setup
					{
						flag=47;						
					}
					
					strcpy(strmac,findstr(name,"wiresave1=","&"));
					if (strstr(strmac,"Save")) // if Save Logger2 Setup
					{
						flag=48;						
					}
					
					strcpy(strmac,findstr(name,"ssave=","&"));
					if (strstr(strmac,"Save")) // if Save Sensors configuration
					{
						flag=49;						
					}
					
					strcpy(strmac,findstr(name,"saveto=","&"));
					if (strstr(strmac,"Save")) // if Save block config
					{
						flag=5;						
					}		
					
					
/*--------------   действия над act   ---------------------------*/					
					
					strcpy(strmac,findstr(name,"act=","&"));
					if (strstr(strmac,"Setup")) // if Setup
					{
						flag=1;
					} 
					
					if (strstr(strmac,"Save")) // if Save
					{
						flag=2;
					} 					
					
					if (strstr(strmac,"FlashEdit")) // if FlashEdit
					{
						flag=4;
					}
					
					if (strstr(strmac,"View")) // if View
					{
						flag=3;
					}
					
					if (strstr(strmac,"Login")) // if Login
					{
						flag=3;
					}
					
					if (strstr(strmac,"reboot")) // reboot CPU1
					{
						flag=10;						
					}
					
					if (strstr(strmac,"FW")) // FirmWare Upgrade
					{
						flag=11;						
					}
					
					if (strstr(strmac,"Change")) // Change Password
					{
						flag=13;						
					}
					
		
					if (strstr(strmac,"sens")) 
					{
						flag=22;
					}
					
					if (strstr(strmac,"logger1")) // Logger1 setup
					{
						flag=23;
					}					
					
					if (strstr(strmac,"logger2")) // Logger2 setup
					{
						flag=24;
					}					
					
					if (strstr(strmac,"relay")) // Logger2 setup
					{
						flag=25;
					}
					
					if (strstr(strmac,"Exit")) // Exit
					{
						flag=15;						
					}					
				} else flag=0;

			
			if (flag==0)
			{
				strcpy(strmac,findstr(name,"act=","&"));
				if (strstr(strmac,"View")) // if View
				{
					flag=3;
				}
				
				if (strstr(strmac,"Login")) // if Login
				{
					flag=3;
				}
				
				if (strstr(strmac,"getdata")) // Упрощённый вывод данных 25.02.2013
				{
					flag=4;
				}
				
			}
			
			
			if (flag == 1)  // Для вывода страницы с редактированием настроек.
			{
				str[0]=0;
				get_style(str);
				get_title(str);
				strcat_P(str,PSTR("<form method=\"get\" action=\"\">"));
//---------------- Код основной таблицы с пунктами меню! begin--------
				strcat_P(str,PSTR("<center><table border=1 cellspacing=0 cellpadding=10><tr><td width=180 valign=top>"));
				return_menu(sessionid,usersession);
				strcat_P(str,PSTR("</td><td width=200 align=center><br>"));
//---------------- Код основной таблицы с пунктами меню! end--------				
				strcat_P(str,PSTR("<table border=0 cellspacing=0 cellpadding=10><tr><td align=center width=170>"));				
				strcat_P(str,PSTR("<B>IP<br><input type=text name=i maxlength=25 value=\"")); // IP
				strmac[0]=0;
				for (a = 0; a < 4; a++) // делаем строку с IP
				{				  
				  itoa(IINCHIP_READ(SIPR0+a),str2,10);				  
				  strcat(strmac,str2);
				  if (a != 3)  strcat(strmac,".");
				}
				strcat(str,strmac);
				strcat_P(str,PSTR("\"><br><br>")); 
				
				
				strcat_P(str,PSTR("MAC address<br><input type=text name=m maxlength=25 value=\"")); // MAC
				strmac[0]=0;
				for (a = 0; a < 6; a++)  // делаем строку с MAC
				{				
				  itoa(IINCHIP_READ(SHAR0+a),str2,16);
				  if (strlen(str2)<2) strcat(strmac,"0");
				  strcat(strmac,str2);
				  if (a != 5)  strcat(strmac,".");
				} 
				strcat(str,strmac);
				strcat_P(str,PSTR("\"><br><br>"));
				strcat_P(str,PSTR("Net mask<br><input type=text name=s maxlength=25 value=\"")); // Subnet mask
				strmac[0]=0;
				for (a = 0; a < 4; a++) // делаем строку с Subnet
				{				  
				  itoa(IINCHIP_READ(SUBR0+a),str2,10);				  
				  strcat(strmac,str2);
				  if (a != 3)  strcat(strmac,".");
				}
				strcat(str,strmac);
				strcat_P(str,PSTR("\"><br><br>"));
				strcat_P(str,PSTR("Gateway<br><input type=text name=g maxlength=25 value=\"")); // Gateway
				strmac[0]=0;
				for (a = 0; a < 4; a++) // делаем строку с GATEWAY
				{				  
				  itoa(IINCHIP_READ(GAR0+a),str2,10);				  
				  strcat(strmac,str2);
				  if (a != 3)  strcat(strmac,".");
				}
				strcat(str,strmac);
				strcat_P(str,PSTR("\"><br><br>"));
				
				strcat_P(str,PSTR("DNS server<br><input type=text name=d maxlength=25 value=\"")); // DNS
				strmac[0]=0;
				for (a = 0; a < 4; a++) // делаем строку с DNS
				{				  
				  itoa(NetworkParam.dns[a],str2,10);				  
				  strcat(strmac,str2);
				  if (a != 3)  strcat(strmac,".");
				}
				strcat(str,strmac);
				strcat_P(str,PSTR("\"><br><br>"));				
				strcat_P(str,PSTR("<center><input type=\"submit\" name=\"act\" value=\"Save\"><br><br></center>"));
				strcat_P(str,PSTR("<input type=hidden name=sid value=\""));
				strcat(str,usersession);
				strcat_P(str,PSTR("\">"));
				strcat_P(str,PSTR("</td></tr></table></form></td></tr></table></center></body></html>"));
			}
			
			if (flag==0)  // страница ввода логина и пароля
			{	
				str[0]=0;
				get_style(str);
				get_title(str);
				strcat_P(str,PSTR("<center><fieldset style=\"display: inline-block; width: 380px;\"><table border=0 cellspacing=5 cellpadding=5>"));
				for (a=0; a<count_sensors; a++) 
				{
					strcat_P(str,PSTR("<tr><td><nobr><b style=\"font-size:25px;\">"));
					strcat(str,all_sensors[a].name);
					strcat_P(str,PSTR("</b></nobr></td><td><b style=\"font-size:25px;\">"));
					
					if (all_sensors[a].type==1) { offset=((all_sensors[a].value)+all_sensors[a].offset); sprintf(strmac,"%2.2f",offset); strcat(str,strmac); strcat(str,"&deg;C"); }
					if (all_sensors[a].type==2) { offset=((all_sensors[a].value)+all_sensors[a].offset); sprintf(strmac,"%2.1f",offset); strcat(str,strmac); strcat(str,"%"); }
					if (all_sensors[a].type==3) { offset=(all_sensors[a].value+all_sensors[a].offset); sprintf(strmac,"%3.1f",offset); strcat(str,strmac); strcat(str,"mm"); }					
					if (all_sensors[a].type==4) { offset=(all_sensors[a].value+all_sensors[a].offset); sprintf(strmac,"%3.1f",offset); strcat(str,strmac); strcat(str,"W"); }					
					
					strcat_P(str,PSTR("</b></td></tr>"));
				}
				strcat_P(str,PSTR("</table></fieldset></center>"));
				strcat_P(str,PSTR("<center><a href=# onclick=\"javascript: document.getElementById('tbl').style.display='block'\">Setup</a><form method=\"get\" action=\"\"><table border=0 cellspacing=0 cellpadding=2 id=tbl style=\"display: none; width:250px;\"><tr><td>")); // display: none;
				
#ifdef DEBUG_METEOBOX
				strcat_P(str,PSTR("Login:</td><td><input type=password name=l maxlength=25 value=\"admin\">"));				
#else				
				strcat_P(str,PSTR("Login:</td><td><input type=password name=l maxlength=25 value=\"\">"));
#endif
				
				strcat_P(str,PSTR("<input type=hidden name=sid value=\"0\"></td></tr><tr><td>"));

#ifdef DEBUG_METEOBOX				
				strcat_P(str,PSTR("Password:&nbsp;&nbsp;</td><td><input type=password name=p maxlength=25 value=\"admin\"></td></tr><tr><td align=center colspan=2><br>"));				
#else				
				strcat_P(str,PSTR("Password:&nbsp;&nbsp;</td><td><input type=password name=p maxlength=25 value=\"\"></td></tr><tr><td align=center colspan=2><br>"));
#endif
				strcat_P(str,PSTR("<input type=\"submit\" name=\"act\" value=\"Login\"></center>"));
				strcat_P(str,PSTR("</td></tr></table></form><p><font color=gray size=2>Uptime: "));
				itoa_long(Time_ms,str2,10);
				
				diff=floor(Time_ms/1000);
				day=floor(diff/60/60/24);
				diff-=day*60*60*24;
				hr =floor(diff/60/60);
				diff-=hr*60*60;
				min=floor(diff/60);
				diff -= min*60;
				sec=diff;
				itoa_long(day,str2,10);
				strcat(str,str2);
				strcat(str," days ");				
				itoa_long(hr,str2,10);
				strcat(str,str2);
				strcat(str,":");
				itoa_long(min,str2,10);
				strcat(str,str2);
				strcat(str,":");
				itoa_long(sec,str2,10);
				strcat(str,str2);
				
				if (ac1 != -1 && ac2 != -1 && ac3 != -1) // Есть датчик давления?
				{
					strcat_P(str,PSTR("<br>On board "));
					sprintf(str2,"%2.1f&deg;C",temperature/10.0);
					strcat(str,str2);
				}
				
				
				strcat_P(str,PSTR("</font></p><br><a href=http://narodmon.ru>\"Народный мониторинг\"</a><br>Powered By <a href=http://meteobox.tk>MeteoBox</a>"));
				
				//itoa_long(SIZE_SENSOR_STRUCT,str2,10);
				//strcat(str,str2);
				
				/*
				strcat(str,"<br>");	

				sprintf(str2,"%f ",read_adc(0));
				strcat(str,str2);
				
		
				sprintf(str2,"%f ",read_current());
				strcat(str,str2);
				
				sprintf(str2,"%f",read_adc(6));
				strcat(str,str2);
				*/
				
				//sprintf(str2,"%d %d %d %d %d<br>",b1,b2,mb,mc,md);
				//strcat(str,str2);

				//sprintf(str2,"t=%2.2f b=%d up=%d<br>",temperature/10.0,pressure,bmp085_read_up());
				//a=bmp085_read_up();
				//offset=(bmp085_read_up());				
				//sprintf(str2,"up=%3.3f<br>",offset);
				
				
				//temperature=bmp085_read_ut();
				
				
				//bmp085_read_up(&pressure);
				//itoa_long(pressure,str2,10);
				//strcat(str,str2);
				
				//strcat(str,inet_ntoa( htonl(remote_server_ip)));				
				//strcat(str,"<br>");
				
				
				/*
				sprintf(str2,"%2.2f | %2.2f E:%d<br>",((float)dht_term/10.0),((float)dht_hum)/10.0,bGlobalErr);
				
				strcat(str,str2);
				
				sprintf(str2,"%02x,%02x",dht_dat[2],dht_dat[3]);
				
				strcat(str,str2);
				
				strcat(str,"<br>[usersession=");
				strcat(str,usersession);
				strcat(str,"]|[sessionid=");
				strcat(str,sessionid);
				strcat(str,"]");
				*/
				
				strcat_P(str,PSTR("</center></body></html>"));				
			}
			
			if (flag==2)  // эта страница грузится пр нажатии кнопки Save
			{				    
				strcpy(strmac,findstr(name,"i=","&"));
				strcpy_P(str,PSTR("<html><head><META HTTP-EQUIV=\"Refresh\" Content=\"5; URL=http://"));
				strcat(str,strmac);
				strcat_P(str,PSTR("\"></head><body>"));
				get_title(str);
				strcat_P(str,PSTR("<center>Reboot.... Please wait 5 sec...."));				
				strcat_P(str,PSTR("</center></body></html>"));	
			}
			
			if (flag==3) // Ввели Логин и Пароль правильно....
			{
				str[0]=0;
				get_style(str);
				get_title(str);
//---------------- Код основной таблицы с пунктами меню! begin--------
				strcat_P(str,PSTR("<center><table border=1 cellspacing=0 cellpadding=10><tr><td width=180 valign=top>"));
				return_menu(sessionid,usersession);
				strcat_P(str,PSTR("</td><td width=250 valign=top>"));
				strcat_P(str,PSTR("<u><b><center>Device info</center></b></u><br><pre>"));
				strcat_P(str,PSTR("Project home page: <a href=http://meteobox.tk>meteobox.tk</a><br></pre>"));
				//strcat_P(str,PSTR("E-mail           : vzager@gmail.com<br>"));
				//strcat_P(str,PSTR("ICQ              : 164825449<br></pre>"));
//---------------- Код основной таблицы с пунктами меню! end--------	
//------------------------------------------------------------------			
				strcat_P(str,PSTR("<br></td></tr></table></body></html>"));
			}	
			
			if (flag==4)  // эта страница грузится при URL запросе ?act=getdata
			{				    
				str[0]=0;
				for (a=0; a<count_sensors; a++) 
				{
					strcat(str,all_sensors[a].name);
					strcat(str,"=");	
					offset=all_sensors[a].value+all_sensors[a].offset;
					sprintf(strmac,"%2.2f",offset);
					strcat(str,strmac);	
					strcat(str,"\r\n");	
				}
			}
			
			
			if (flag==10) // Reboot Device
			{
				strmac[0]=0;
				for (a = 0; a < 4; a++) // делаем строку с IP
				{				  
				  itoa(IINCHIP_READ(SIPR0+a),str2,10);				  
				  strcat(strmac,str2);
				  if (a != 3)  strcat(strmac,".");
				}
				
				strcpy_P(str,PSTR("<html><head><META HTTP-EQUIV=\"Refresh\" Content=\"5; URL=http://"));
				strcat(str,strmac);
		        strcat_P(str,PSTR("\"></head><body>"));				
				
				get_title(str);
				strcat_P(str,PSTR("<center>Reboot.... Please wait 5 sec...."));				
				strcat_P(str,PSTR("</center></body></html>"));
			}
			
			if (flag==11) // Firmware upgrade dialog 19.12.2012
			{
				str[0]=0;
				get_style(str);
				get_title(str);
//---------------- Код основной таблицы с пунктами меню! begin--------
				strcat_P(str,PSTR("<center><table border=1 cellspacing=0 cellpadding=10><tr><td width=180 valign=top>"));
				return_menu(sessionid,usersession);
				strcat_P(str,PSTR("</td><td width=250 align=center>"));
				strcat_P(str,PSTR("<u>Upgrade Firmware</u><br><pre><br><br>Current version:<b>"));
				
				strcat(str,FIRMWARE_VERSION);
				
				strcat_P(str,PSTR("</B><br><br><Form method=\"get\" action=\"\">"));
				strcat_P(str,PSTR("<input type=hidden name=sid value=\""));
				strcat(str,usersession);
				strcat_P(str,PSTR("\">"));
//---------------- Код основной таблицы с пунктами меню! end--------	
				strcat_P(str,PSTR("Type <font color=red><b>YES</b></font> for enter firmware upgrade mode.<br>"));
				strcat_P(str,PSTR("<input type=text name=fwd maxlength=5><br><br>"));				
				strcat_P(str,PSTR("<center><input type=submit name=fwu value=\"Enter upgrade mode\"></center>"));								
				strcat_P(str,PSTR("</form></pre></td></tr></table></body></html>"));
			}
			
			if (flag==12) // Firmware upgrade type YES!!!! 19.12.2012
			{
				strmac[0]=0;
				for (a = 0; a < 4; a++) // делаем строку с IP
				{				  
				  itoa(IINCHIP_READ(SIPR0+a),str2,10);				  
				  strcat(strmac,str2);
				  if (a != 3)  strcat(strmac,".");
				}
				
				strcpy_P(str,PSTR("<html><head><body>"));								
				get_title(str);
				strcat_P(str,PSTR("<br><br><br><center>For upgrade firmware run Upgrade Tool<br>"));				
				strcat_P(str,PSTR("Enter IP in dialog:<b>"));				
				strcat(str,strmac);
				//strcat_P(str,PSTR("<br><br><br>"));
				strcat_P(str,PSTR("</b></center></body></html>"));
			}
			

			if (flag==13) // Change password
			{
				str[0]=0;
				get_style(str);
				get_title(str);
//---------------- Код основной таблицы с пунктами меню! begin--------
				strcat_P(str,PSTR("<center><table border=1 cellspacing=0 cellpadding=10><tr><td width=180 valign=top>"));
				return_menu(sessionid,usersession);
				strcat_P(str,PSTR("</td><td width=250 align=left>"));
				strcat_P(str,PSTR("<u>Change Login and Password</u><br><pre>"));
				strcat_P(str,PSTR("<Form method=\"get\" action=\"\">"));
//---------------- Код основной таблицы с пунктами меню! end--------	
				strcat_P(str,PSTR("Login    <input type=text name=newl maxlength=15><br>"));
				strcat_P(str,PSTR("Password <input type=text name=newp maxlength=15><br><br>"));
				strcat_P(str,PSTR("<input type=hidden name=sid value=\""));
				strcat(str,usersession);
				strcat_P(str,PSTR("\">"));				
				strcat_P(str,PSTR("<center><input type=submit name=submitpass value=\"Set\"></center>"));								
				strcat_P(str,PSTR("</form></pre></td></tr></table></body></html>"));			
			}

			if (flag==14) // Save new  login and password in eeprom
			{
				a=0;
				strcpy(strmac,findstr(name,"newp=","&"));
				if (strlen(strmac)<3) 
				{
				
					strcpy_P(str,PSTR("<html><body>Password lenght <3! </body></html>"));
					a=1;
					
				}
				
				strcpy(strmac,findstr(name,"newl=","&"));
				if (strlen(strmac)<3) 
				{
				
					strcpy_P(str,PSTR("<html><body>Login lenght <3! </body></html>"));
					a=1;
					
				}
				
				if (a==0)
				{				
					eeprom_write_block("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",(unsigned char*)eeprom_password, 15);
					eeprom_write_block("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",(unsigned char*)eeprom_login, 15);
					
					strcpy(strmac,findstr(name,"newp=","&"));
					eeprom_write_block(strmac,(unsigned char*)eeprom_password, strlen(strmac)+1);
					
					strcpy(strmac,findstr(name,"newl=","&"));
					eeprom_write_block(strmac, (unsigned char*)eeprom_login, strlen(strmac)+1);
					
					strmac[0]=0;
					for (a = 0; a < 4; a++) // делаем строку с IP
					{				  
					  itoa(IINCHIP_READ(SIPR0+a),str2,10);				  
					  strcat(strmac,str2);
					  if (a != 3)  strcat(strmac,".");
					}
					
					strcpy_P(str,PSTR("<html><head><META HTTP-EQUIV=\"Refresh\" Content=\"7; URL=http://"));
					strcat(str,strmac);				
					strcat_P(str,PSTR("\"></head><body>"));				
					
					get_title(str);
					strcat_P(str,PSTR("<center>Reboot.... Please wait 7 sec...."));				
					strcat_P(str,PSTR("</center></body></html>"));	
					flag=10; // Reboot MCSI
				}
			}
			
			if (flag==15) // Exit
			{			
				strmac[0]=0;
				for (a = 0; a < 4; a++) // делаем строку с IP
				{				  
				  itoa(IINCHIP_READ(SIPR0+a),str2,10);				  
				  strcat(strmac,str2);
				  if (a != 3)  strcat(strmac,".");
				}

				flag=0;
				strcpy(usersession,"");
				strcpy(sessionid,"");
				strcpy_P(str,PSTR("<html><head><META HTTP-EQUIV=\"Refresh\" Content=\"1; URL=http://"));
				strcat(str,strmac);				
				strcat_P(str,PSTR("?sid=0&\"></head><body>"));
				get_title(str);
				strcat_P(str,PSTR("<center>Exit<br><br>Please wait 1 sec...."));				
				strcat_P(str,PSTR("</center></body></html>"));
			}
			

			if (flag==22) // Show ALL Sensors information 09/01/2013
			{
				str[0]=0;
				get_style(str);
				get_title(str);
//---------------- Код основной таблицы с пунктами меню! begin--------
				strcat_P(str,PSTR("<center><table border=1 cellspacing=0 cellpadding=5><tr><td width=180 valign=top>"));
				return_menu(sessionid,usersession);
				strcat_P(str,PSTR("</td><td width=250 align=left>"));
				strcat_P(str,PSTR("<u>Sensors configuration</u><br>"));
				strcat_P(str,PSTR("<table border=1 cellspacing=0 cellpadding=10>"));
				strcat_P(str,PSTR("<tr><td align=center>Alias</td>"));
				strcat_P(str,PSTR("<td align=center>Offset</td>"));
				strcat_P(str,PSTR("<td align=center>Serial number</td>"));
				strcat_P(str,PSTR("<td align=center>Value</td>"));
				strcat_P(str,PSTR("<td align=center>Calculated</td>"));
				strcat_P(str,PSTR("<td>&nbsp;</td></tr>"));
				
				for (a=0; a<count_sensors; a++) 
				{				  
				  asm("WDR");
				  eeprom_read_block(&all_sensors[a].name,   (unsigned char*)(eeprom_sensors_config+12)+(a*sizeof(all_sensors[0])), sizeof(all_sensors[0].name));
				  eeprom_read_block(&all_sensors[a].offset, (unsigned char*)(eeprom_sensors_config+29)+(a*sizeof(all_sensors[0])), 4);
				  strcat_P(str,PSTR("<tr><td><form method=\"get\" action=\"\"><input type=text maxlength=14 size=14 name=\"s")); // Строка имя датчика
				  itoa(a,str2,10);
				  strcat(str,str2);
				  strcat_P(str,PSTR("\" value=\""));
				  strcat(str,all_sensors[a].name);
				  strcat_P(str,PSTR("\"></td>"));
				  
				  strcat_P(str,PSTR("<td><input type=text maxlength=5 size=5 name=\"o")); // Строка смещение
				  strcat(str,str2);
				  strcat_P(str,PSTR("\" value=\""));
				  sprintf(str2,"%2.2f",all_sensors[a].offset);
				  strcat(str,str2);
				  strcat_P(str,PSTR("\"></td><td>"));
				  
				  for(b=0; b<8; b++)
				   {
						itoa(all_sensors[a].id[b],str2,16);						
						if (strlen(str2)<2) strcat(str,"0");
						strcat(str,strupr(str2));
				   }				    
					strcat(str,"</td><td><nobr>");					
					
					if (all_sensors[a].type==1) { offset=((all_sensors[a].value)); sprintf(strmac,"%2.2f",offset); strcat(str,strmac); strcat(str,"&deg;C"); }
					if (all_sensors[a].type==2) { offset=((all_sensors[a].value)); sprintf(strmac,"%2.1f",offset); strcat(str,strmac); strcat(str,"%"); }
					if (all_sensors[a].type==3) { offset=(all_sensors[a].value); sprintf(strmac,"%3.1f",offset); strcat(str,strmac); strcat(str,"mm"); }
					if (all_sensors[a].type==4) { offset=(all_sensors[a].value); sprintf(strmac,"%3.1f",offset); strcat(str,strmac); strcat(str,"W"); }
					
					strcat(str,"</td><td><nobr>");	
					
					if (all_sensors[a].type==1) { offset=((all_sensors[a].value)+all_sensors[a].offset); sprintf(strmac,"%2.2f",offset); strcat(str,strmac); strcat(str,"&deg;C"); }
					if (all_sensors[a].type==2) { offset=((all_sensors[a].value)+all_sensors[a].offset); sprintf(strmac,"%2.1f",offset); strcat(str,strmac); strcat(str,"%"); }
					if (all_sensors[a].type==3) { offset=(all_sensors[a].value+all_sensors[a].offset); sprintf(strmac,"%3.1f",offset); strcat(str,strmac); strcat(str,"mm"); }
					if (all_sensors[a].type==4) { offset=(all_sensors[a].value+all_sensors[a].offset); sprintf(strmac,"%3.1f",offset); strcat(str,strmac); strcat(str,"W"); }
					
				    strcat(str,"</td><td><input type=submit name=ssave value=\"Save\">");	
					strcat_P(str,PSTR("<input type=hidden name=sid value=\""));
					strcat(str,usersession);
					strcat_P(str,PSTR("\">"));
				
					strcat_P(str,PSTR("<input type=hidden name=rnd value=\"")); // Случайое значение ...
					itoa_long(Time_ms,strmac,10);
					strcat(str,strmac);
					strcat_P(str,PSTR("\">"));
					
					strcat(str,"</form></td></tr>\r\n");
				}
				strcat_P(str,PSTR("</table>"));				
				strcat_P(str,PSTR("</form></td></tr></table></body></html>"));			
			}
			
			
			if ((flag==23) || (flag==24)) // Show Logger Setup 
			{
				if (flag==23) b=0; else b=1; // Указатель на элемент массива server_data
				str[0]=0;
				get_style(str);
				get_title(str);
//---------------- Код основной таблицы с пунктами меню! begin--------
				strcat_P(str,PSTR("<center><table border=1 cellspacing=0 cellpadding=10><tr><td width=180 valign=top>"));
				return_menu(sessionid,usersession);
				strcat_P(str,PSTR("</td><td width=250 align=left>"));
				itoa((b+1),str2,10);
				strcat_P(str,PSTR("<u>Setup Logger "));
				strcat(str,str2);
				strcat_P(str,PSTR("</u><hr><center>"));
				
				strcat_P(str,PSTR("<form method=\"get\" action=\"\">\n"));	
				
				strcat_P(str,PSTR("<input type=hidden name=sid value=\""));
				strcat(str,usersession);
				strcat_P(str,PSTR("\">"));
				strcat_P(str,PSTR("<input type=checkbox name=en"));
				if (server_data[b].enable==0xFF) strcat_P(str,PSTR(" checked"));
				strcat_P(str,PSTR("> <b>Enable/Disabe</b><br><br>"));	
				strcat_P(str,PSTR("<b>Remote server IP<br>(For use DNS server enter IP 0.0.0.0)</b><br><input type=text name=i maxlength=25 value=\"")); // IP
				
				strmac[0]=0;
				for (a = 0; a < 4; a++) // делаем строку с IP
				{				  
				  itoa(server_data[b].server_ip[a],str2,10);				  
				  strcat(strmac,str2);
				  if (a != 3)  strcat(strmac,".");
				}
				strcat(str,strmac);
				strcat_P(str,PSTR("\">"));
				
				strcat_P(str,PSTR("<br><b>Host name</b><br><input type=text name=host maxlength=29 value=\"")); // Host name
				strcat(str,server_data[b].server_name);

				strcat_P(str,PSTR("\"><br><br><b>Port</b> "));
				
				itoa(server_data[b].port,strmac,10);
				strcat_P(str,PSTR("<input type=number name=port maxlength=5 max=65535 size=5 value=\""));
				strcat(str,strmac);				
				strcat_P(str,PSTR("\"><br><br>"));
				
				
				//strcat_P(str,PSTR("<b>Device ID</b><br><input type=text name=name maxlength=12 SIZE=15 value=\"")); // Name
				//strcat(str,server_data.name);
				//strcat_P(str,PSTR("\"><br><br>"));
				
				strcat_P(str,PSTR("<b>Path to script</b><input type=text maxlength=50 SIZE=40 NAME=path value=\"")); // Path
				
				strcat(str,server_data[b].script_path);				
				strcat_P(str,PSTR("\"><br>Example: /path/script.php<br><br>"));	
				

				strcat_P(str,PSTR("<b>Update time</b><br><SELECT name=sp>")); 
				
				for (a=0; a<COUNT_AUTOUPDATE; a++) // Массив speed
				{				
					strcat_P(str,PSTR("<option value="));
					itoa(a,str2,10);
					strcat(str,str2);
					
					if (server_data[b].refresh_time==update_time[a].id) strcat_P(str,PSTR(" selected"));	
					//if (server_data[0].refresh_time==update_time[a].id) strcat_P(str,PSTR(" selected"));	
					//if (b==1) strcat_P(str,PSTR(" disabled"));
					
					strcat_P(str,PSTR(">"));
					strcat(str,update_time[a].name);
					strcat_P(str,PSTR("</option>\n"));				
				}
				strcat_P(str,PSTR("</SELECT><BR><BR>"));

				
				strcat_P(str,PSTR("<br><input type=submit name=wiresave"));
				itoa(b,str2,10);
				strcat(str,str2);
				strcat_P(str,PSTR(" value=Save><br><br></form>"));
				
				strcat_P(str,PSTR("</td></tr></table></body></html>"));			
			}
			
			
			
			if (flag==25) // Relay module configuration 18/02/2013
			{
				str[0]=0;
				get_style(str);
				get_title(str);
//---------------- Код основной таблицы с пунктами меню! begin--------
				strcat_P(str,PSTR("<center><table border=1 cellspacing=0 cellpadding=5><tr><td width=180 valign=top>"));
				return_menu(sessionid,usersession);
				strcat_P(str,PSTR("</td><td width=250 align=left>"));
				strcat_P(str,PSTR("<u>Relay configuration</u><br>"));
				strcat_P(str,PSTR("<table border=1 cellspacing=0 cellpadding=10>"));
				strcat_P(str,PSTR("<tr><td align=center>Relay num</td>"));
				strcat_P(str,PSTR("<td align=center>State</td>"));
				strcat_P(str,PSTR("<td align=center><nobr>Sensor trigger</nobr></td>"));
				strcat_P(str,PSTR("<td align=center>Min<br>Max</td>"));
				strcat_P(str,PSTR("<td align=center>&nbsp;</td></tr>"));
				
				if (strstr(request.URI,"button=Save"))
				{
					
					strcpy(strmac,findstr(request.URI,"min=","&"));
					strcpy(str2  ,findstr(request.URI,"max=","&"));
					b=ATOI(findstr(request.URI,"sensor=","&"),10);
					
					if (strstr(request.URI,"rx=0")) // реле 0
					{
						all_relay[0].min=myatof(strmac);
						all_relay[0].max=myatof(str2);
						if (b==100) all_relay[0].flag=0; else all_relay[0].flag=0xFF;
						
						memcpy(all_relay[0].id,all_sensors[b].id,8);
						
						eeprom_write_block(&all_relay[0], (unsigned char*)eeprom_relay_config, sizeof(all_relay[0]));
					}
					
					if (strstr(request.URI,"rx=1")) // реле 1
					{
						all_relay[1].min=myatof(strmac);
						all_relay[1].max=myatof(str2);		
						if (b==100) all_relay[1].flag=0; else all_relay[1].flag=0xFF;
						
						memcpy(all_relay[1].id,all_sensors[b].id,8);
						
						eeprom_write_block(&all_relay[1], (unsigned char*)eeprom_relay_config+sizeof(all_relay[0]), sizeof(all_relay[0]));
					}
					
					
					if (strstr(request.URI,"rx=2")) // реле 2
					{
						all_relay[2].min=myatof(strmac);
						all_relay[2].max=myatof(str2);
						if (b==100) all_relay[2].flag=0; else all_relay[2].flag=0xFF;
						
						memcpy(all_relay[2].id,all_sensors[b].id,8);
						
						eeprom_write_block(&all_relay[2], (unsigned char*)eeprom_relay_config+sizeof(all_relay[0])*2, sizeof(all_relay[0]));
					}
					
					if (strstr(request.URI,"rx=3")) // реле 3
					{
						all_relay[3].min=myatof(strmac);
						all_relay[3].max=myatof(str2);		
						if (b==100) all_relay[3].flag=0; else all_relay[3].flag=0xFF;
						
						memcpy(all_relay[3].id,all_sensors[b].id,8);
						
						eeprom_write_block(&all_relay[3], (unsigned char*)eeprom_relay_config+sizeof(all_relay[0])*3, sizeof(all_relay[0]));
					}
					
					
				}
				
				
				if (strstr(request.URI,"button=On")) // Вкл Выкл реле!!!
				{
					if (strstr(request.URI,"rx=0")) PB_WRITE(all_relay[0].pio,!PB_READ(all_relay[0].pio));
					if (strstr(request.URI,"rx=1")) PB_WRITE(all_relay[1].pio,!PB_READ(all_relay[1].pio));
					if (strstr(request.URI,"rx=2")) PB_WRITE(all_relay[2].pio,!PB_READ(all_relay[2].pio));
					if (strstr(request.URI,"rx=3")) PB_WRITE(all_relay[3].pio,!PB_READ(all_relay[3].pio));					
				}				
				
				for (b=0; b<MAX_RELAY; b++)
				{
					asm("WDR");
					strcat_P(str,PSTR("<tr><td align=center><nobr>Relay "));
					itoa(b,str2,10);
					strcat(str,str2);
					strcat_P(str,PSTR("</nobr></td>"));
					strcat_P(str,PSTR("<td align=center>"));
					strcat_P(str,PSTR("<font style=\"background-color:"));
					
					if (PB_READ(all_relay[b].pio)==0) { strcat(str,"#00FF00;"); strcpy(strmac,"ON"); } else { strcat(str,"#FF0000;"); strcpy(strmac,"OFF"); }
					
					strcat_P(str,PSTR("font-size:12px;\"><b>&nbsp;&nbsp;"));
					strcat(str,strmac);
					strcat_P(str,PSTR("&nbsp;&nbsp;</b></font><br>"));
					strcat_P(str,PSTR("<form method=\"get\" action=\"\">"));
					strcat_P(str,PSTR("<input type=hidden name=act value=relay>"));
					strcat_P(str,PSTR("<input type=hidden name=rx value=\""));
					strcat(str,str2);
					strcat_P(str,PSTR("\">"));
					strcat_P(str,PSTR("<input type=hidden name=sid value=\""));
					strcat(str,usersession);
					strcat_P(str,PSTR("\">"));
					strcat_P(str,PSTR("<input type=submit value=\"On/Off\" style=\"margin-top: 5px;\" name=button></form></td>"));
					
					strcat_P(str,PSTR("<td align=center>"));
					
					strcat_P(str,PSTR("<form method=\"get\" action=\"\">"));
					strcat_P(str,PSTR("<input type=hidden name=act value=relay><select name=sensor>"));
					
					strcat_P(str,PSTR("<option value=\"100\">Not used</option>"));
					for (a=0; a<count_sensors; a++) 
					{
						strcat_P(str,PSTR("<option value="));
						itoa(a,str2,10);
						strcat(str,str2);
						
						if (memcmp(all_sensors[a].id,all_relay[b].id,8)==0) strcat_P(str,PSTR(" selected"));
						
						strcat_P(str,PSTR(">"));
						strcat(str,all_sensors[a].name);
						
						offset=all_sensors[a].value+all_sensors[a].offset;
						sprintf(strmac," [%2.2f]",offset);
						strcat(str,strmac);
						
						strcat_P(str,PSTR("</option>"));
					}
					strcat_P(str,PSTR("</select></td><td>"));
					
					
					strcat_P(str,PSTR("<pre>Min:<input type=text maxlength=5 size=5 name=min value=\""));
					offset=all_relay[b].min;
					sprintf(strmac,"%2.2f",offset);
					strcat(str,strmac); 
					
					strcat_P(str,PSTR("\"><br>Max:<input type=text maxlength=5 size=5 name=max value=\""));					
					offset=all_relay[b].max;
					sprintf(strmac,"%2.2f",offset);
					strcat(str,strmac); 
					
					strcat_P(str,PSTR("\"></pre></td><td>"));
					
					strcat_P(str,PSTR("<input type=hidden name=rx value=\""));
					itoa(b,str2,10);
					strcat(str,str2);
					strcat_P(str,PSTR("\">"));
					strcat_P(str,PSTR("<input type=hidden name=sid value=\""));
					strcat(str,usersession);
					strcat_P(str,PSTR("\">"));
					strcat_P(str,PSTR("<input type=submit value=\"Save\" style=\"margin-top: 5px;\" name=button></form>"));
					
					strcat_P(str,PSTR("</td></tr>"));
					
				}
				
				strcat_P(str,PSTR("</table>"));
				strcat_P(str,PSTR("</form></td></tr></table></body></html>"));			
			}


			
			if ((flag == 47) || (flag==48))	// Нажали Save в меню настроек работы с удалённым сервером
		    {				
				if (flag==47) b=0; else b=1; // Указатель на элемент массива server_data
				strcpy(strmac,findstr(request.URI,"i=","&"));			  
			    data_get(strmac,10);					
				memcpy(server_data[b].server_ip,d,4);								
	
				if (strstr(request.URI,"en=")) server_data[b].enable=0xFF; else server_data[b].enable=0;
				
				strcpy(strmac,findstr(request.URI,"sp=","&"));	
				server_data[b].refresh_time=update_time[ATOI(strmac,10)].id;
				
				strcpy(strmac,findstr(request.URI,"port=","&"));	
				server_data[b].port=ATOI(strmac,10);
				
				memset(server_data[b].script_path,0x00,sizeof(server_data[b].script_path));
				strcpy(server_data[b].script_path,findstr(request.URI,"path=","&"));			
				urldecode(server_data[b].script_path);
				
				strcpy(server_data[b].server_name,findstr(request.URI,"host=","&"));	
				
				
				if (b==0) { 
					WebrefreshTime0=server_data[b].refresh_time*1000;
					eeprom_write_block(&server_data[b], (unsigned char*)eeprom_server_config, sizeof(server_data[0]));
				}
				else {
					WebrefreshTime1=server_data[b].refresh_time*1000;
					eeprom_write_block(&server_data[b], (unsigned char*)eeprom_server_config+sizeof(server_data[0]), sizeof(server_data[0]));
				}
				
				strmac[0]=0;
				for (a = 0; a < 4; a++) // делаем строку с IP
				{				  
				  itoa(IINCHIP_READ(SIPR0+a),str2,10);				  
				  strcat(strmac,str2);
				  if (a != 3)  strcat(strmac,".");
				}
				
				strcpy_P(str,PSTR("<html><head></head><body>\n<SCRIPT Language=\"javascript\">window.location.href = 'http://"));
				strcat(str,strmac);
				strcat_P(str,PSTR("/?act=logger"));
				itoa((b+1),str2,10);
				strcat(str,str2);
				strcat_P(str,PSTR("&sid="));
				strcat(str,usersession);
				strcat_P(str,PSTR("&rnd="));
				itoa_long(Time_ms,strmac,10);
				strcat(str,strmac);
				strcat_P(str,PSTR("'</script>\n</body></html>"));
			}	
			
			
			if (flag == 49)	// Нажали Save Sensors information ... после вывода сообщения перезагружеам 
		    {			  
			  
				for (a=0; a<count_sensors; a++) {
					itoa(a,str2,10);
					strcpy(strmac,"s");  // собираем строку для поиска вида s0=...s10=
					strcat(strmac,str2);			  				
					strcat(strmac,"=");
					strcpy(str2,strmac);
					
					if (strstr(request.URI,strmac))
					{					
						strcpy(strmac,findstr(request.URI,str2,"&"));
						
						//strcpy_P(str,PSTR("<html><head></head><body>"));
						//strcat(str,"-------------------");
						//strcat(str,strmac);
						//strcat(str,"-------------------");
						
						if (strlen(strmac)>0)
						{					
							urldecode(strmac);
							eeprom_write_block(strmac, (unsigned char*)(eeprom_sensors_config+12)+(a*sizeof(all_sensors[0])), sizeof(all_sensors[0].name));
						
							WDI();
							
							itoa(a,str2,10);
							strcpy(strmac,"o");  // собираем строку для поиска вида o0=...o10=
							strcat(strmac,str2);			  				
							strcat(strmac,"=");
							strcpy(str2,strmac);
							strcpy(strmac,findstr(request.URI,str2,"&"));
							offset=myatof(strmac);
							eeprom_write_block((unsigned char*)(&offset), (unsigned char*)(eeprom_sensors_config+29)+(a*sizeof(all_sensors[0])), 4);
							break;
						}
					}
				}
				
				strmac[0]=0;
				for (a = 0; a < 4; a++) // делаем строку с IP
				{				  
				  itoa(IINCHIP_READ(SIPR0+a),str2,10);				  
				  strcat(strmac,str2);
				  if (a != 3)  strcat(strmac,".");
				}
				
				
				strcpy_P(str,PSTR("<html><head></head><body>\n<SCRIPT Language=\"javascript\">window.location.href = 'http://"));
				strcat(str,strmac);
				strcat_P(str,PSTR("/?act=sens&rnd="));
				itoa_long(Time_ms,strmac,10);
				strcat(str,strmac);
				strcat_P(str,PSTR("&sid="));
				strcat(str,usersession);
				strcat_P(str,PSTR("'</script>\n</body></html>"));
				
		    }
			
			
			
			if (flag == 2)	// Нажали Save... после вывода сообщения перезагружеам 
		    {			  
			
/*
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
*/
			  d[0] = SPECIALOP;  d[1] = SPECIALOP;  d[2] = SPECIALOP;  d[3] = SPECIALOP; 
			  eeprom_write_block(d, (unsigned char*)EEPOP, 4);
			  
			  eeprom_write_byte((unsigned char*)EEPADDR_VER, VER_H);
	          eeprom_write_byte((unsigned char*)(EEPADDR_VER+1), VER_L);
			  
			  strcpy(strmac,findstr(name,"i=","&"));			  
			  data_get(strmac,10);			  
			  //printf("ip: %d-%d-%d-%d\n",d[0],d[1],d[2],d[3]); 
			  eeprom_write_block(d, (unsigned char*)(EEPOP+4+2+6), 4);
			  
			  
			  strcpy(strmac,findstr(name,"g=","&"));			 
			  data_get(strmac,10);			  
			  //printf("gateway %d-%d-%d-%d\n",d[0],d[1],d[2],d[3]); 
			  eeprom_write_block(d, (unsigned char*)(EEPOP+4+2+6+4+4), 4);
			  //Print("Save 2\n\r");
			  
			  my_wait_1ms(1);	
			  asm("WDR");
			  
			  strcpy(strmac,findstr(name,"m=","&"));
			  data_get(strmac,16);
			  //printf("mac %x-%x-%x-%x-%x-%x\n",d[0],d[1],d[2],d[3],d[4],d[5]);
			  eeprom_write_block(d, (unsigned char*)(EEPOP+4+2), 6);	
			  
			  
			  //Print("Save 3\n\r");
			  strcpy(strmac,findstr(name,"s=","&"));
			  data_get(strmac,10);
			  //printf("subnet %d-%d-%d-%d\n",d[0],d[1],d[2],d[3]);
			  eeprom_write_block(d, (unsigned char*)(EEPOP+4+2+6+4), 4);

			  strcpy(strmac,findstr(name,"d=","&")); // find DNS
			  data_get(strmac,10);			  
			  eeprom_write_block(d, (unsigned char*)(EEPOP+4+2+6+4+4+4), 4);
		    }
			
			
			
			if (flag <50)
			{
				fl_len = strlen(str);
				
				
				make_head(fl_len);
				
				send(s, request.URI, strlen(request.URI));
				
				content = str;
				
				while (fl_len) {
					if (fl_len >= MAX_BUF_SIZE) {
						tx_len = MAX_BUF_SIZE;
					} else {
						tx_len = fl_len;
					}
					memcpy(tx_buf, content, tx_len);
					asm("WDR");
					send(s, tx_buf, tx_len);
					asm("WDR");
					content += tx_len;
					fl_len -= tx_len;
				}
			}
			
			if (flag == 2)	// Нажали Save... после вывода сообщения перезагружеам 
		    {
				my_wait_1ms(50);	
			    close(s);		  
			    cli();
			    ((void (*)())0x0000)();	
			}
			
			
			if (flag==10) { auth=0; strcpy(usersession,""); strcpy(sessionid,""); my_wait_1ms(50);  close(s); cli(); (*appptr)(); } // Reboot CPU
			if (flag==12) { auth=0; strcpy(usersession,""); strcpy(sessionid,""); my_wait_1ms(100); close(s); cli(); GoBoot(); }  // Enter BootLoader mode
			
			
		break;
	case METHOD_HEAD :
		//printf("METHOD_HEAD\r\n");
		break;
	case METHOD_POST :
		//printf("METHOD_POST\r\n");
		//name = (u_char *)&(request.URI[1]);
		//printf("%s\n\r",name);
		break;
	default :
		break;
	}
	
}

/*
********************************************************************************
*                  MAKE HTTP HEADER
*
* Description: This function makes a HTTP header
* Arguments  : Tx_Buf       - is a pointer to Tx buffer
*              content_type - is a type of HTML
* Returns    : header size.
* Note       : 
********************************************************************************
*/
/*
void make_head(u_char * buf, u_char type, u_int len)
{
	u_char tmp[10];
		
	//head_ptr=PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\nContent-Length: ");

	sprintf(tmp, "%d", len);	
	
	strcpy_P(buf,PSTR("xHTTP/1.0 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\nContent-Length: "));
	strcat(buf, tmp);
	strcat_P(buf,PSTR("\r\n\r\n"));
}
*/

//#define RES_HTMLHEAD_OK	"HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\nContent-Length: " //05.04.2013

void make_head(u_int len)
{
	u_char tmp[10];	

	sprintf(tmp, "%d", len);	
	
	strcpy_P(request.URI,PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\nContent-Length: "));
	strcat(request.URI, tmp);
	strcat_P(request.URI,PSTR("\r\n\r\n"));
}


/*
********************************************************************************
*                   PARSE REQUEST
*
* Description: This function analyzes a request from the browser
*              For simple implementatioon, compare first character of file name
* Arguments  : Data_Buf - a pointer to Rx buffer which has a request from the browser
* Returns    : request type.
* Note       : 
********************************************************************************
*/
unsigned char parse_request(HTTP_REQ * request, u_char * buf)
{
	int i;
	u_char method[TMP_BUF_SIZE];

	i = 0;

	while (1) {
		if (*buf == ' ') {
			method[i] = '\0';
			break;
		} else if (*buf == '\r' || *buf == '\0') {
			request->METHOD = METHOD_ERR;
			return 0;
		}

		if (i >= TMP_BUF_SIZE) {
			request->METHOD = METHOD_ERR;
			return 0;
		}

		method[i++] = *buf++;
	}

	buf++;

	i = 0;

	while (1) {
		if (*buf == ' ') {
			request->URI[i] = '\0';
			break;
		} else if (*buf == '\r' || *buf == '\0') {
			request->METHOD = METHOD_ERR;
			return 0;
		}

		if (i >= TMP_BUF_SIZE) {
			request->METHOD = METHOD_ERR;
			return 0;
		}

		request->URI[i++] = *buf++;
	}
	
	if (!strcmp(method, "GET")) {
		request->METHOD = METHOD_GET;
	} else if (!strcmp(method, "HEAD")) {
		request->METHOD = METHOD_HEAD;
	} else if (!strcmp(method, "POST")) {
		request->METHOD = METHOD_POST;
	} else {
		request->METHOD = METHOD_ERR;
	}
	
	return 1;
}

