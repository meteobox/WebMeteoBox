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
#include "../../iinchip/socket.h"
#include "../../util/wiz_util.h"
#include "protocol.h"

#include "task_config.h"
#include "term.h"

StrConfigParam NetworkParam, MSGConfig;
CONFIG_MSG my_msg;

Server_config server_data[2]; // 2 логгера


extern unsigned char m_FlagFlashWrite;

//#define DefPassword			"1\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
//#define DefPassword			"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
#define DefPassword	  		    "admin\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //пароль по умолчанию admin

#define DefConfigParam		"\x82\x82\x82\x82\x00\x00\x00\x08\xDC\x00\x02\x05\xc0\xa8\x01\xde\xFF\xFF\xFF\x00\xc0\xa8\x01\x01\xc0\xa8\x01\x01\x00"
//#define DefPassword			"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

extern void InitFlash(void);
//-----------------------------------------------------------------------------
//http://mon.dubna.tk/script/add.php
//


void default_network(void)
{
    char a;	
	char * b;
	

	memset(&all_relay,0x00,sizeof(all_relay));
	
	
	all_relay[0].pio=2; // Присваиваем к номерам реле pio atmega
	all_relay[1].pio=3;
	all_relay[2].pio=4;
	all_relay[3].pio=5;
	
	
	b=(char *)(all_relay);
	for (a=0; a<sizeof(all_relay); a++) {
		eeprom_write_byte((unsigned char*)eeprom_relay_config+a,*(b+a));
		asm("WDR");
	}
	
	
	
	//memset(server_data,0x00,sizeof(server_data));
	server_data[0].enable=0x00; // enable=0xff
	strcpy(server_data[0].server_name,"narodmon.ru");
	strcpy(server_data[0].script_path,"/post.php");
	
	//strcpy(server_data[0].server_name,"inflnr.jinr.ru");
	//strcpy(server_data[0].script_path,"/test/add.php");
	
	//используйте IP 91.122.49.168 или 94.19.113.221 до которого у Вас наименьший ping
	
	server_data[0].server_ip[0]=0;  // ip 0.0.0.0 = use dns
	server_data[0].server_ip[1]=0;
	server_data[0].server_ip[2]=0;
	server_data[0].server_ip[3]=0;
	

	server_data[0].refresh_time=30; // 60=1 раз в 10 минут, 1=раз в 10 сек
	server_data[0].port=80;
	
	memcpy(&server_data[1],&server_data[0],sizeof(server_data[0]));
	
	
	server_data[1].server_ip[0]=0;
	server_data[1].server_ip[1]=0;
	server_data[1].server_ip[2]=0;
	server_data[1].server_ip[3]=0;
	strcpy(server_data[1].server_name,"meteobox.tk");
	strcpy(server_data[1].script_path,"/script/add.php");
	
	//strcpy(server_data[1].server_name,"dubna.tk");
	//strcpy(server_data[1].script_path,"/");
	
	
	//strcpy(server_data[1].server_name,"");
	
	server_data[1].refresh_time=6; // 1 раз в минуту
	
	
	//eeprom_write_block(&server_data[0], (unsigned char*)eeprom_server_config, sizeof(server_data[0]));
	//eeprom_write_block((const void*)&server_data[0], (unsigned char*)eeprom_server_config, 85);
	b=(char *)(server_data);
	for (a=0; a<sizeof(server_data); a++) {
		eeprom_write_byte((unsigned char*)eeprom_server_config+a,*(b+a));
		asm("WDR");
	}
	
	//asm("WDR");
	//eeprom_write_block(&server_data[1], (unsigned char*)eeprom_server_config+sizeof(server_data[0]), sizeof(server_data[0]));
	
	memcpy(&NetworkParam, DefConfigParam, sizeof(NetworkParam));
	
	
	my_wait_1ms(5);	
	asm("WDR");
	
	eeprom_write_block(&NetworkParam, (unsigned char*)EEPOP, sizeof(NetworkParam));
	eeprom_write_byte((unsigned char*)EEPADDR_VER, VER_H);
	eeprom_write_byte((unsigned char*)(EEPADDR_VER+1), VER_L);

	my_wait_1ms(5);	
	asm("WDR");
	
	eeprom_write_block((unsigned char*)DefPassword,(unsigned char*)eeprom_password, 15);
    eeprom_write_block((unsigned char*)DefPassword, (unsigned char*)eeprom_login,   15);
	
	strcpy(all_sensors[0].name,"No name");
	strcpy(all_sensors[0].id,"NONE");
	all_sensors[0].value=0;
	all_sensors[0].type=0;
	all_sensors[0].flag=0;
	all_sensors[0].offset=0.0000f;

	for (a=0; a<MAX_SENSORS; a++) {
		
		memcpy(&all_sensors[a],&all_sensors[0],sizeof(all_sensors[0]));
		
		eeprom_write_block(all_sensors[0].name, (unsigned char*)(eeprom_sensors_config+12)+(a*sizeof(all_sensors[0])), sizeof(all_sensors[0].name));
		my_wait_1ms(5);	
		asm("WDR");
		eeprom_write_block((unsigned char*)(&all_sensors[0].offset), (unsigned char*)(eeprom_sensors_config+29)+(a*sizeof(all_sensors[0])), 4);
		my_wait_1ms(5);	
		asm("WDR");
	}	
	
}


//-----------------------------------------------------------------------------
void SetConfig(void)
{
	unsigned char temp;
	

	// Get Network Parameters
	temp = eeprom_read_byte((unsigned char*)EEPOP);

	if( temp != SPECIALOP) 
	{
		// This board is initial state
		default_network();
	}
	else
	{
  	    eeprom_read_block(&all_relay, (unsigned char*)eeprom_relay_config, sizeof(all_relay));
		asm("WDR");
		
		for (temp=0; temp<MAX_SENSORS; temp++)
		{ 
			eeprom_read_block(&all_sensors[temp].name,   (unsigned char*)(eeprom_sensors_config+12)+(temp*sizeof(all_sensors[0])), sizeof(all_sensors[0].name));
			eeprom_read_block(&all_sensors[temp].offset, (unsigned char*)(eeprom_sensors_config+29)+(temp*sizeof(all_sensors[0])), 4);
			//strcpy(all_sensors[temp].name,"12345");
			//my_wait_1ms(1);
			asm("WDR");
		}
		
		eeprom_read_block(&server_data, (unsigned char*)eeprom_server_config, sizeof(server_data)); 
		asm("WDR");
		
		eeprom_read_block(&NetworkParam,(unsigned char*)EEPOP, sizeof(NetworkParam));
		asm("WDR");
		//check version
		if( (eeprom_read_byte((unsigned char*)EEPADDR_VER) != VER_H) || (eeprom_read_byte((unsigned char*)(EEPADDR_VER+1)) != VER_L) ) {
			eeprom_write_byte((unsigned char*)EEPADDR_VER, VER_H);
			eeprom_write_byte((unsigned char*)(EEPADDR_VER+1), VER_L);
		}

	}
	
	
	setSIPR(NetworkParam.ip);
	setGAR(NetworkParam.gw);
	setSUBR(NetworkParam.subnet);
	setSHAR(NetworkParam.mac);
	
	
	
	
}
//-----------------------------------------------------------------------------


//UDP
void ProcessConfig(unsigned char Sock)
{
	int len;							
	unsigned char serverip[4];
	uint16 serverport = CONFIG_SERVER_PORT;
	
	switch (getSn_SR(Sock))
	{
		case SOCK_UDP:
			if((len = getSn_RX_RSR(Sock)) > 0)
			{
				LED_ON_OFF(1);
				if(len > sizeof(my_msg))len = sizeof(my_msg);

				len = recvfrom(Sock, (unsigned char*)&my_msg, len, serverip, &serverport);
				
				if(serverport == CONFIG_SERVER_PORT)
				{
					if(memcmp((unsigned char*)my_msg.op, (unsigned char*)"SRCH", 4)==0)
					{						
						MSGConfig = NetworkParam;
						memcpy((unsigned char*)my_msg.op,"MBOX", 4);
						memcpy((unsigned char*)my_msg.Gw,MSGConfig.gw,4);
						memcpy((unsigned char*)my_msg.Subnet,MSGConfig.subnet,4);
						memcpy((unsigned char*)my_msg.Mac,MSGConfig.mac,6);
						memcpy((unsigned char*)my_msg.Lip,MSGConfig.ip,4);
						memcpy((unsigned char*)my_msg.dns,MSGConfig.dns,4);
						
						serverip[0] = 0xff;serverip[1] = 0xff;serverip[2] = 0xff;serverip[3] = 0xff;
						
						sendto(Sock, (unsigned char*)&my_msg, sizeof(my_msg), serverip, serverport);
					}			
					else if(memcmp((unsigned char*)my_msg.op, (unsigned char*)"SETQ", 4)==0)
					{
					
					
			            if ((my_msg.Mac[0] == MSGConfig.mac[0]) && 
						    (my_msg.Mac[1] == MSGConfig.mac[1]) && 
							(my_msg.Mac[2] == MSGConfig.mac[2]) && 
						    (my_msg.Mac[3] == MSGConfig.mac[3]) && 
							(my_msg.Mac[4] == MSGConfig.mac[4]) && 
						    (my_msg.Mac[5] == MSGConfig.mac[5]))
						   
			            {					  
					
							memcpy((unsigned char*)my_msg.op, "SETR", 4);
							serverip[0] = 0xff;serverip[1] = 0xff;serverip[2] = 0xff;serverip[3] = 0xff;
							sendto(Sock, (unsigned char*)&my_msg, sizeof(my_msg), serverip, serverport);

							MSGConfig.op[0] = SPECIALOP;
							MSGConfig.op[1] = SPECIALOP;
							MSGConfig.op[2] = SPECIALOP;
							MSGConfig.op[3] = SPECIALOP;
							
							memcpy((unsigned char*)MSGConfig.gw,my_msg.Gw,4);
							memcpy((unsigned char*)MSGConfig.subnet,my_msg.Subnet,4);
							memcpy((unsigned char*)MSGConfig.mac,my_msg.Mac,6);
							memcpy((unsigned char*)MSGConfig.ip,my_msg.Lip,4);
							memcpy((unsigned char*)MSGConfig.dns,my_msg.dns,4);
							
							if (MSGConfig.ip[0] > 254) MSGConfig.ip[0]  = 254; 
							if (MSGConfig.gw[0] > 254) MSGConfig.gw[0]  = 254; 
						
							eeprom_write_block(&MSGConfig, (unsigned char*)EEPOP, sizeof(MSGConfig));
	
							// Need reboot
							close(Sock);

							//printf("Set Command.\r\n");
							cli();
							((void (*)())0x0000)();		// reset
						
						}
						
					}					
				    else if(memcmp((unsigned char*)my_msg.op, (unsigned char*)"R**T", 4)==0) //20.10.2010 сброс к настройкам по умолчанию
					{
						//Print("Cmd ok.\r\n");
						if ((my_msg.Mac[0] == MSGConfig.mac[0]) && 
						    (my_msg.Mac[1] == MSGConfig.mac[1]) && 
							(my_msg.Mac[2] == MSGConfig.mac[2]) && 
						    (my_msg.Mac[3] == MSGConfig.mac[3]) && 
							(my_msg.Mac[4] == MSGConfig.mac[4]) && 
						    (my_msg.Mac[5] == MSGConfig.mac[5]))
						   
			            {						
						  // Print("Mak ok.\r\n");						   
						   default_network();
						   close(Sock);
						   cli();
						   ((void (*)())0x0000)();	
						}
					}
					else if(memcmp((unsigned char*)my_msg.op, (unsigned char*)"FWUP", 4)==0) //16.03.2011 Go BootLoader
					{
						if ((my_msg.Mac[0] == MSGConfig.mac[0]) && 
						    (my_msg.Mac[1] == MSGConfig.mac[1]) && 
							(my_msg.Mac[2] == MSGConfig.mac[2]) && 
						    (my_msg.Mac[3] == MSGConfig.mac[3]) && 
							(my_msg.Mac[4] == MSGConfig.mac[4]) && 
						    (my_msg.Mac[5] == MSGConfig.mac[5]))
						   
			            {						
						   close(Sock);
						   cli();
						   GoBoot();	
						}
					}							
				}
				LED_ON_OFF(0);
			}
			break;	
			
		case SOCK_CLOSED:     
			if(socket(Sock, Sn_MR_UDP, CONFIG_CLIENT_PORT, 0x00)== 0)
			//printf("Socket[%d] - Config Socket Started.\r\n", Sock);
			break;
	}	
}
//-----------------------------------------------------------------------------
