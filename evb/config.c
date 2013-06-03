/**
 * @file		config.c
 * @brief 		relate to the configure varibles
 */


#include <avr/io.h>
#include <stdio.h> 
#include <string.h>
#include <avr/eeprom.h>

#include "../mcu/types.h"

#include "../util/sockutil.h"
#include "../util/myprintf.h"
#include "../iinchip/socket.h"

#include "../evb/config.h"
#include "../iinchip/w5100.h"

SYSINFO	SysInfo;		/**< system info.(version, ...) */
NETCONF	NetConf;	/**< network info.(mac, ip,...) */
CHCONF	ChConf;		/**< channel info.(port, channel,...) */


/**
 * @brief	initialize configuration variables
*/
void init_conf_variables(void)
{
	memset((void*)&SysInfo,0,sizeof(SYSINFO));	
	memset((void*)&NetConf,0,sizeof(NETCONF));
	memset((void*)&ChConf,0,sizeof(CHCONF));
	get_sysinfo(&SysInfo);
	get_netconf(&NetConf);
	get_chconf(&ChConf);
}


/**
 * @brief	set system infomation
*/
void set_sysinfo(SYSINFO* pSysInfo)
{
	eeprom_write_block((void*)pSysInfo,(void*)SYS_INFO, sizeof(SYSINFO));	
}


/**
 * @brief	get system infomation
*/
void get_sysinfo(
	SYSINFO* pSysInfo	/**< system infomation pointer */
	)
{
	if(eeprom_read_word((u_int*)SYS_TEST) != TEST_VALUE)
	{
		pSysInfo->test = TEST_VALUE;
		pSysInfo->ver  = FW_VERSION;
		pSysInfo->auto_reset = SYSTEM_MANUAL_RESET;
		pSysInfo->any_port = SYSTEM_ANY_PORT;
		set_sysinfo(pSysInfo);
	}
	else
	{
		eeprom_read_block((void*)pSysInfo,(void*)SYS_INFO, sizeof(SYSINFO));

		pSysInfo->ver = FW_VERSION;
	}
}


/**
 * @brief		get reset info.(use SYSINFO structure)
 * @return	get reset method.
*/
u_char get_reset_flag(void)
{
	SysInfo.auto_reset = eeprom_read_byte((u_char*)SYS_AUTORESET);
	return SysInfo.auto_reset;
}


/**
 * @brief	set reset info.(use SYSINFO structure)
*/
void set_reset_flag(u_char flag)
{
	eeprom_write_byte((u_char*)SYS_AUTORESET,flag);	
}

/**
 * @brief		get unused port
 * @return 	port number
*/
u_int  get_system_any_port(void)
{
	u_int i;
	SysInfo.any_port = eeprom_read_word((u_int*)SYS_ANY_PORT)+1;
	for(i=0; i < MAX_SOCK_NUM; i++)
	{
		if(SysInfo.any_port == ChConf.ch[i].port) 
		{
			if(SysInfo.any_port++ > 1000) SysInfo.any_port = 1000;
		}
	}
	eeprom_write_word((u_int*)SYS_ANY_PORT,SysInfo.any_port);
	return SysInfo.any_port;
}

/**
 * @brief	set network configuration
*/
void set_netconf(NETCONF* pNetConf)
{
	eeprom_write_block((void*)pNetConf,(void*)NET_CONF, sizeof(NETCONF));	
}

/**
 * @brief		get network configuration
 * @return	pointer to be saved
*/
void get_netconf(NETCONF* pNetConf)
{
	if(eeprom_read_word((void*)NET_TEST) != TEST_VALUE)
	{
		pNetConf->test = TEST_VALUE;
		eeprom_read_block((void*)pNetConf->mac, (void*)NET_MAC,6);
		if(!memcmp(pNetConf->mac,"\xFF\xFF\xFF\xFF\xFF\xFF",6) || !memcmp(pNetConf->mac,"\x00\x00\x00\x00\x00\x00",6))
		{
			memcpy(pNetConf->mac,DEFAULT_NET_MAC,6);
		}

		pNetConf->sip	= htonl(DEFAULT_NET_SIP);
		pNetConf->gwip	= htonl(DEFAULT_NET_GWIP);
		pNetConf->sn	= htonl(DEFAULT_NET_SN);
		pNetConf->dns	= htonl(DEFAULT_NET_DNS);
		pNetConf->Mem_alloc = DEFAULT_NET_MEMALLOC;

		
		set_netconf(pNetConf);
	}
	else eeprom_read_block((u_char*)pNetConf,(void*)NET_CONF, sizeof(NETCONF));
}

/**
 * @brief		set channel configuration
 * @return	pointer to be saved
*/
void set_chconf(CHCONF* pChConf)
{
	eeprom_write_block((void*)pChConf,(void*)CH_CONF, sizeof(CHCONF));
}

/**
 * @brief		get channel configuration
 * @return	pointer to be saved
*/
void get_chconf(CHCONF* pChConf)
{
	if(eeprom_read_word((void*)CH_CONF) != TEST_VALUE)
	{
		u_int i;
		pChConf->test = TEST_VALUE;
		for(i=0;i < 4; i++)
		{
			pChConf->ch[i].type  = DEFAULT_CH_TYPE;
			pChConf->ch[i].port = DEFAULT_LISTEN_PORT;
			pChConf->ch[i].destip = 0;
		}
	}
	else eeprom_read_block((void*)pChConf,(void*)CH_CONF, sizeof(CHCONF));
}


/**
 * @brief		set to be factory setting (network config value)
*/
void load_factory_netconf(void)
{
	u_int erase_value = 0x0000;
	eeprom_write_word((u_int*)NET_TEST,erase_value);
	get_netconf(&NetConf);		
}

/**
 * @brief		set to be factory setting (channel config value)
*/
void load_factory_chconf(void)
{
	u_int erase_value = 0x0000;
	eeprom_write_word((u_int*)CH_CONF,erase_value);
	get_chconf(&ChConf);		
}


/**
 * @brief		show the  network coniguration
*/
void display_netconf(NETCONF* pNetConf)
{
	u_int i;
	PRINT("MAC Addr      : ");
	for(i=0; i<5;i++) PRINT1("%02X.",pNetConf->mac[i]);
	PRINTLN1("%02X",pNetConf->mac[i]);
	PRINTLN1("Source IP     : %s",inet_ntoa(ntohl(pNetConf->sip)));
	PRINTLN1("Gateway IP    : %s",inet_ntoa(ntohl(pNetConf->gwip)));
	PRINTLN1("Subnet Mask   : %s",inet_ntoa(ntohl(pNetConf->sn)));
	PRINTLN1("DNS Server IP : %s",inet_ntoa(ntohl(pNetConf->dns)));
	PRINTLN1("Mem alloc     : %.2x", pNetConf->Mem_alloc);
}

/**
 * @brief		show the  channel configuration
*/
void display_chconf(CHCONF* pChConf)
{
	u_int i;
	for(i=0; i < MAX_SOCK_NUM; i++)
	{
		PRINT1("CH%d : ",i); 
		switch(pChConf->ch[i].type)
		{
		case NOTUSE: PRINTLN("Not Used");
			break;
		case DHCP_CLIENT: PRINTLN("DHCP Client");
			break;
		case LB_TCPS:
			PRINTLN1("Loob-Back TCP Server : Listen Port %d",pChConf->ch[i].port);
			break;
		case LB_TCPC:
			PRINT1("Loob-Back TCP Client : Server Port %d, ",pChConf->ch[i].port);
			PRINTLN1("Dest IP %s", inet_ntoa(ntohl(pChConf->ch[i].destip)));
			break;
		case LB_UDP:
			PRINTLN1("Loop-Back UDP : Source Port %d", pChConf->ch[i].port);
			break;
		case WEB_SERVER:
			PRINTLN1("Web Server : HTTP Port %d", pChConf->ch[i].port);
			break;
		}
	}
}


//net config + 2007.12.13 [jhpark]
/**
 * @brief		read the net configuration form reg
*/
void get_netconf_reg(NETCONF* pNetConf)
{
	u_char i = 0;
	u_long ip=0;

	//get mac address
	for(i=0; i<6;i++)pNetConf->mac[i] = IINCHIP_READ(SHAR0+i);

	ip = 0;
	//get gateway
	for(i=0; i < 4; i++)
	{
		ip <<= 8;
		ip += (u_char)IINCHIP_READ(GAR0+i);
	}
	pNetConf->gwip = ntohl(ip);

	ip = 0;
	//get ip
	for(i=0; i < 4; i++)
	{
		ip <<= 8;
		ip += (u_char)IINCHIP_READ(SIPR0+i);
	}
	pNetConf->sip = ntohl(ip);

	ip = 0;
	//get sunet
	for(i=0; i < 4; i++)
	{
		ip <<= 8;
		ip += (u_char)IINCHIP_READ(SUBR0+i);
	}
	pNetConf->sn = ntohl(ip);
}

/**
 * @brief		compare net configuration
*/
u_char comp_net_conf(NETCONF *pConfA, NETCONF *pConfB)
{
	u_char i;


	for(i=0;i<6;i++)
	{
		if(pConfA->mac[i] != pConfB->mac[i])return 1;
	}
	
	if(pConfA->sip != pConfB->sip)return 2;
	if(pConfA->gwip != pConfB->gwip)return 3;
	if(pConfA->sn != pConfB->sn)return 4;

	return 0;
}

