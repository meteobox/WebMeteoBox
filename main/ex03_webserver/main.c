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
#include <stdio.h> 
#include <string.h>
#include "../../mcu/types.h"
#include "../../mcu/delay.h"
#include "../../inet/httpd.h"
#include "../../evb/evb.h"
#include "../../inet/dns.h" // 13.12.2012

#include "../../iinchip/socket.h"
#include "../../util/sockutil.h"

#include "task_config.h"

#include "protocol.h"
#include "DHT.h"

#include "OWIPolled.h"
#include "OWIHighLevelFunctions.h"
#include "OWIBitFunctions.h"
#include "common_files\OWIcrc.h"
#include "term.h"
#include "i2csoft.h"
//-----------------------------------------------------------------------------

//#define  WEB_DEBUG


#define MY_NET_MEMALLOC		0x55					// MY iinchip memory allocation

#define MY_LISTEN_PORT		5000					// MY Listen Port  : 5000 
#define MY_CONNECT_PORT		3000					// MY Connect Port : 3000

#define SOCK_TCPC			0
#define SOCK_TCPS			1


#define DEFAULT_HTTP_PORT			80
//-----------------------------------------------------------------------------


OWI_device allDevices[MAX_DEVICES];

void ProcessWebSever(u_char ch);
void PROCESS_RSPORT0(unsigned char Sock, unsigned int Port);
void PROCESS_RSPORT1(unsigned char Sock, unsigned int Port);
char TCP_SEND_DATA(char num);


static st_http_request *http_request;	/**< Pointer to HTTP request */
extern StrConfigParam NetworkParam, MSGConfig;
extern Server_config server_data[2];

unsigned char bchannel_start;

unsigned char m_FlagWrite = 0;
unsigned char m_FlagFlashWrite = 0;

unsigned long m_FileSize = 0;
unsigned long m_FileWriteCount = 0;

extern unsigned long int WebrefreshTime0;
extern unsigned long int WebrefreshTime1;

extern volatile char Wt0;
extern volatile char Wt1; 

extern unsigned long int TimeWt0;
extern unsigned long int TimeWt1;


void WIZNET_RST( char ckl )
{
  if (ckl == 1) PORTE |= (1<<7);
  else PORTE &= ~(1<<7);
}

//  
//-----------------------------------------------------------------------------
void NetInit(void)
{
	volatile unsigned long dd = 300000;
  /* 
	unsigned char mac[6]	= MY_NET_MAC;
	unsigned char sm[4]	= MY_SUBNET;
	unsigned char gwip[4]	= MY_NET_GWIP;
	unsigned char m_sip[4]	= MY_SOURCEIP;
*/
	unsigned char tx_mem_conf[8] = {8,8,8,8,8,8,8,8};          // for setting TMSR regsiter
//	unsigned char rx_mem_conf[8] = {8,8,8,8,8,8,8,8};          // for setting RMSR regsiter
	
	//W5300 Chip Init
	iinchip_init();

	while(dd--);
	/*
	//Set MAC Address
	setSHAR(mac);

	//Set Gateway
	setGAR(gwip);

	//Set Subnet Mask
	setSUBR(sm);

	//Set My IP
	setSIPR(m_sip);*/
	

#ifdef __DEF_IINCHIP_INT__
	setIMR(0xEF);
#endif
	
	//sysinit(MY_NET_MEMALLOC, MY_NET_MEMALLOC);

   /* allocate internal TX/RX Memory of W5300 */
   if(!sysinit(tx_mem_conf, tx_mem_conf))           
   {
//      Print("MEMORY CONFIG ERR.\r\n");
      while(1);
   }	
}
//-----------------------------------------------------------------------------




//______________________________________________________________________

void WDI (void) // сброс  WATCHDOGa!!!
{
  /* reset WDT */
  asm("WDR");
}

void mydbg(void) // Зависаем и мигаем лампочкой...
{
	while(1){LED_ON_OFF(1);_delay_us(100);LED_ON_OFF(0);_delay_us(100);WDI();	}
}


void leddebugsignal (char a) //Мигалка для отладки
{
char b;
  for (b=0; b<a; b++)
  {
	LED_ON_OFF(1);
	my_wait_1ms(50);
	LED_ON_OFF(0);
	my_wait_1ms(50); 
	asm("WDR");
  }
}


void getdns (void) 
{
	char dnsaddr[18];
	extern u_long dns_server_ip;	
	//dns_server_ip = htonl(inet_addr((u_char*) "8.8.8.8" ));
	sprintf(dnsaddr,"%d.%d.%d.%d",NetworkParam.dns[0],NetworkParam.dns[1],NetworkParam.dns[2],NetworkParam.dns[3]);
	dns_server_ip = htonl(inet_addr((u_char*) dnsaddr ));
}

//-----------------------------------------------------------------------------


char RST_CHECK()
{
  return ((PIND&(1<<7))>0);
}



//u_char URL[] = "narodmon.ru";
extern u_long remote_server_ip = 0;




//just taken from the BMP085 datasheet
 int ac1;
 int ac2;
 int ac3;
 unsigned int ac4;
 unsigned int ac5;
 unsigned int ac6;
 int b1;
 int b2;
 int mb;
 int mc;
 int md;
 int temperature;
 long pressure;


void bmp085_read_temperature_and_pressure(int* temperature, long int* pressure) {
 int ut= bmp085_read_ut();
 long up;
 long x1, x2, x3, b3, b5, b6, p;
 unsigned long b4, b7;
 char oversampling_setting = 3;
 
 bmp085_read_up(&up);

 //calculate the temperature
 x1 = ((long int)ut - ac6) * ac5 >> 15;
 x2 = ((long int) mc << 11) / (x1 + md);
 b5 = x1 + x2;
 *temperature = (b5 + 8) >> 4;

 //calculate the pressure
 b6 = b5 - 4000;
 x1 = (b2 * (b6 * b6 >> 12)) >> 11;
 x2 = ac2 * b6 >> 11;
 x3 = x1 + x2;

 //b3 = (((int32_t) ac1 * 4 + x3)<> 2;

 //if (oversampling_setting == 3) 
 b3 = ((unsigned long int) ac1 * 4 + x3 + 2) << 1;
 //if (oversampling_setting == 2) b3 = ((int32_t) ac1 * 4 + x3 + 2);
 //if (oversampling_setting == 1) b3 = ((int32_t) ac1 * 4 + x3 + 2) >> 1;
 //if (oversampling_setting == 0) b3 = ((int32_t) ac1 * 4 + x3 + 2) >> 2;

 
 x1 = ac3 * b6 >> 13;
 x2 = (b1 * (b6 * b6 >> 12)) >> 16;
 x3 = ((x1 + x2) + 2) >> 2;
 b4 = (ac4 * (unsigned long int) (x3 + 32768)) >> 15;
 b7 = ((unsigned long int) up - b3) * (50000 >> oversampling_setting);
 p = b7 < 0x80000000 ? (b7 * 2) / b4 : (b7 / b4) * 2;

 x1 = (p >> 8) * (p >> 8);
 x1 = (x1 * 3038) >> 16;
 x2 = (-7357 * p) >> 16;
 *pressure = p + ((x1 + x2 + 3791) >> 4);

}


int main(void) {
char a;
char msb,lsb;


//________________init_watchdog______________________
/* Write logical one to WDCE and WDE */
WDI(); // сбрасыввем WATCHDOG
WDTCR |= (1<<WDCE) | (1<<WDE);
  /* Turn off WDT */
// WDTCR = 0x07;
WDTCR = (1<<WDCE) | 0x07;
//___________________________________________________
WDI(); // сбрасыввем WATCHDOG

	
	
	
	evb_init();
	
	

	if (RST_CHECK()==0) {
		default_network();
		while (1) leddebugsignal(1); 
	}
	
	
	

//--- bmp085 ----------------------------------------------
	SoftI2CInit();
	SoftI2CStart();	

	ac1=read_bmp085_int_register(0xAA);
	ac2=read_bmp085_int_register(0xAC);
	ac3=read_bmp085_int_register(0xAE);
	ac4=read_bmp085_int_register(0xB0);
	ac5=read_bmp085_int_register(0xB2);
	ac6=read_bmp085_int_register(0xB4);	
	b1 = read_bmp085_int_register(0xB6);
	b2 = read_bmp085_int_register(0xB8);
	mb = read_bmp085_int_register(0xBA);
	mc = read_bmp085_int_register(0xBC);
	md = read_bmp085_int_register(0xBE);
//--- bmp085 ----------------------------------------------	
	

	WIZNET_RST(0);
	my_wait_1ms(300);asm("WDR");
	WIZNET_RST(1);
	my_wait_1ms(100);asm("WDR");
	
	NetInit();
	
	my_wait_1ms(300);asm("WDR");
	my_wait_1ms(300);asm("WDR");
	my_wait_1ms(300);asm("WDR");
	
	
	SetConfig();	
	
	my_wait_1ms(200);asm("WDR");	
	
	
	WebrefreshTime0  = server_data[0].refresh_time*1000;
	WebrefreshTime1  = server_data[1].refresh_time*1000;
	

	getdns(); 
	

	//remote_server_ip = gethostbyname((char*)URL);
	
//--- 1-Wire-----------------------------------------------  
  
  InitDHT(OWI_PIN_1);  // DHT

  OWI_Init(OWI_PIN_2); // 1-Wire
  
  
   
  OWI_SearchDevices(allDevices, MAX_DEVICES, OWI_PIN_2, &count_ds18b2);
  
  for (a=0; a<count_ds18b2; a++) 
  {
    Read_scratchpad(OWI_PIN_2,a);
	if (allDevices[a].scratchpad[4] != 0x7F) Write_scratchpad(OWI_PIN_2,a);
	asm("WDR");
	//sprintf(printbuf,"%02X ",allDevices[port_tcp0].scratchpad[4]);	Print(printbuf);	 
  }	
  
  
//--- 1-Wire-----------------------------------------------	

	//bmp085_read_temperature_and_pressure(&temperature,&pressure);



	
	

	eeprom_read_block(&MLogin,    (void *)eeprom_login,   15);
	eeprom_read_block(&MPasswd,   (void *)eeprom_password,  15);

	//mydbg();
	
	leddebugsignal(30);

	while (1)
	{
		exe_sensors();
		WDI();		
		ProcessConfig(1);
		WDI();		
		ProcessWebSever(5);		
		WDI();
		ProcessWebSever(6);
		WDI();
		ProcessWebSever(7);
		WDI();
		TCP_SEND_DATA(0); // 3 socket
		WDI();
		TCP_SEND_DATA(1); // 4 socket
		WDI();
		exe_relay();
		WDI();		
	}		
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
/*
char checkdnsorip(unsigned char *buf) // Проверяем что у нас ip или доменое имя? 
{
	char a,flag=0;
	for (a=0; a<strlen(buf); a++)	
		switch (buf[a]) {
			case 48 ... 57: break; //0..9
			case '.'      : break; 
			default       : return(1); // Домен
		}
	
	
	return(0); // Букв нету, значит IP
}
*/

//processing http protocol , and excuting the followed fuction.
void ProcessWebSever(u_char ch)
{
	int len;
	u_int wait_send=0;

	http_request = (st_http_request*)RX_BUF;		// struct of http request
	/* http service start */
	
	
	switch(getSn_SR(ch))
	{
	case SOCK_ESTABLISHED:
		if(bchannel_start==1)
		{
			bchannel_start = 2;
		}
		if ((len = getSn_RX_RSR(ch)) > 0)		
		{
			if ((u_int)len > MAX_URI_SIZE) len = MAX_URI_SIZE;				
			len = recv(ch, (u_char*)http_request, len);
			*(((u_char*)http_request)+len) = 0;
			
			proc_http2(ch, (u_char*)http_request, len);	// request is processed
			
			//printf( "- HTTP REQUEST DONE -\n");
			
			while(getSn_TX_FSR(ch)!= getIINCHIP_TxMAX(ch))
			{
			
				if(wait_send++ > 1500)
				{
					break;
				}
				//my_wait_1ms(1); // NEW 27.12.2012
				
				asm("WDR");
			}
			
			disconnect(ch);
			my_wait_1ms(100); // NEW 27.12.2012
		}
		break;


	case SOCK_CLOSE_WAIT:   

		//printf("CLOSE_WAIT : %d",ch);	// if a peer requests to close the current connection

		disconnect(ch);
		bchannel_start = 0;
	case SOCK_CLOSED:                   
		if(!bchannel_start)
		{

			//printf("%d : Web Server Started.\r\n",ch);

			bchannel_start = 1;
		}

		if(socket(ch, Sn_MR_TCP, DEFAULT_HTTP_PORT, 0x00) == 0)    /* reinitialize the socket */
			bchannel_start = 0;
		else
			listen(ch);
		
		break;
	}	// end of switch 
}



//----- New 06.02.2009 ----------------- память для буфера приёмника! с запасом 2к
u_char *data_buf = (u_char *)(0x3100);

/*
POST http://narodmon.ru/post.php HTTP/1.0\r\n
Host: narodmon.ru\r\n
Content-Type: application/x-www-form-urlencoded\r\n
Content-Length: NN\r\n
\r\n
ID=MAC&mac1=value1&...&macN=valueN[&time=time1]
*/

char TCP_SEND_DATA(char num)
{
   int  len;
   int wait_send=0;
   char tt,a,b;
   char tmp[33];
   char stbuf[18];
   char flag=1;
   static uint16 any_port = 1000;   
   unsigned char ip[4];
   float offset;
   char s=2;      // Ноер сокета 2
   
   s=s+num;
   
   tt=getSn_SSR(s);
   
   if (count_sensors==0) return;  

	   switch(tt)                   // check SOCKET status
	   {                                      // ------------
		  case SOCK_ESTABLISHED:              // ESTABLISHED?
			 if(getSn_IR(s) & Sn_IR_CON)      // check Sn_IR_CON bit
			 {
				//Print("+");
				setSn_IR(s,Sn_IR_CON);        // clear Sn_IR_CON
			 }
				  // recv
				strcpy(data_buf,"POST ");
				strcat(data_buf,server_data[num].script_path);
				strcat(data_buf," HTTP/1.0\r\n");
				strcat(data_buf,"Host: ");
				strcat(data_buf,server_data[num].server_name);
				strcat(data_buf,"\r\n");
				
				strcat(data_buf,"Content-Type: application/x-www-form-urlencoded\r\n");
				strcat(data_buf,"Content-Length:   \r\n\r\n");
				
				len=0; // сюда будем класть Content-Length
				tmp[0]=0x00;
				for (a = 0; a < 6; a++)
				{
					itoa(NetworkParam.mac[a],stbuf,16);
					if (strlen(stbuf)<2) strcat(tmp,"0");
					strcat(tmp,strupr(stbuf));
				}
				
				strcat(data_buf,"ID=");
				strcat(data_buf,tmp);
				
				len=strlen(tmp)+3; // Начинаем вычислять общую длинну посылки
				
				for (a=0; a<count_sensors; a++) 
				{
					tmp[0]=0x00;
					for (b = 0; b < 8; b++)
					{
						itoa(all_sensors[a].id[b],stbuf,16);
						if (strlen(stbuf)<2) strcat(tmp,"0");				
						strcat(tmp,strupr(stbuf));
					}
					strcat(data_buf,"&");
					strcat(data_buf,tmp);
					strcat(data_buf,"=");
					
					len+=strlen(tmp)+2;
					
					if (all_sensors[a].type==1) { offset=all_sensors[a].value+all_sensors[a].offset; sprintf(tmp,"%2.2f",offset); }
					if (all_sensors[a].type==2) { offset=all_sensors[a].value+all_sensors[a].offset; sprintf(tmp,"%2.1f",offset); }
					if (all_sensors[a].type==3) { offset=all_sensors[a].value+all_sensors[a].offset; sprintf(tmp,"%3.1f",offset); } // Mm
					if (all_sensors[a].type==4) { offset=all_sensors[a].value+all_sensors[a].offset; sprintf(tmp,"%3.1f",offset); } // Watt
					
					len+=strlen(tmp);
					
					strcat(data_buf,tmp);				
				}

				
				
				itoa(len,tmp,10);
				for (a=80; a<250; a++)
				if (*(data_buf+a)==':') memcpy((data_buf+a+1),tmp,strlen(tmp));
					
				len=send(s,data_buf,strlen(data_buf));     // send
				
				while(getSn_TX_FSR(s)!= getIINCHIP_TxMAX(s))
				{			
					if(wait_send++ > 100)	break;
					asm("WDR");
				}
				
				disconnect(s);	
				//close(s);		// 18.03.2013

				flag=0;

				if (num==0) Wt0=1;
				if (num==1) Wt1=1;
				
			 break;
											  // ---------------
	   case SOCK_CLOSE_WAIT:                  // PASSIVE CLOSED
			 disconnect(s);                   // disconnect 
			 break;
											  // --------------
	   case SOCK_CLOSED:                      // CLOSED
		  close(s);                           // close the SOCKET
		  socket(s,Sn_MR_TCP,any_port++,0);  // open the SOCKET with TCP mode and any source port number
		  break;
											  // ------------------------------
	   case SOCK_INIT:                        // The SOCKET opened with TCP mode
	  
//------------ Проверяем looger 1

		  if ((num==0) && (Wt0==0) && (server_data[num].enable==0xFF)) { 
			
			//if (checkdnsorip(server_data[num].server_name)) 
			if (server_data[num].server_ip[0]==0) 
			{
				remote_server_ip = gethostbyname(server_data[num].server_name);
				
				strcpy(data_buf,inet_ntoa( htonl(remote_server_ip)));				
				data_get(data_buf,10);	

				if (d[0]==0) // DNS вернули 0.0.0.0
				{
					leddebugsignal(10); //Мигаем тревожной лампочкой!
					TimeWt0=1000; // Попробуем еще раз через 10 секунд!
					Wt0=1;
					break; // и вываливаемся...
				}				
				
				memcpy(ip,d,4);
			}
			else
			{
				memcpy(ip,server_data[num].server_ip,4);  			
			}
			
			connect(s, ip, server_data[num].port);             // Try to connect to "TCP SERVER"
			//leddebugsignal(10);
			TimeWt0=WebrefreshTime0;
			break;
		  }
		  
//------------ Проверяем looger 2		  
		  
		  if ((num==1) && (Wt1==0) && (server_data[num].enable==0xFF)) { 
			
			//if (checkdnsorip(server_data[num].server_name)) 
			if (server_data[num].server_ip[0]==0) 
			{
				remote_server_ip = gethostbyname(server_data[num].server_name);
				
				strcpy(data_buf,inet_ntoa( htonl(remote_server_ip)));				
				data_get(data_buf,10);					
				
				if (d[0]==0) // DNS вернули 0.0.0.0
				{
					leddebugsignal(10); //Мигаем тревожной лампочкой!
					TimeWt1=1000; // Попробуем еще раз через 10 секунд!
					Wt1=1;
					break; // и вываливаемся...
				}
				
				memcpy(ip,d,4);
			}
			else
			{
				memcpy(ip,server_data[num].server_ip,4);  			
				//leddebugsignal(10);
			}
			
			connect(s, ip, server_data[num].port);             // Try to connect to "TCP SERVER"
			//leddebugsignal(10);
			TimeWt1=WebrefreshTime1;
		  }
		  break;
		  
	   default:
		  break;
	   }
		
  return(tt);   
}



