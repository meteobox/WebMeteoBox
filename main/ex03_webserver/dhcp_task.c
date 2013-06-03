#include "task_config.h"

#include "../../iinchip/socket.h"
#include "../../mcu/timer.h"
#include "../../inet/dhcp.h"
#include "dhcp_task.h"
#include "../../evb/config.h"
#include "../../evb/evb.h"

#include <stdio.h>
#include <avr/eeprom.h>
#include <string.h>

extern u_char SRC_MAC_ADDR[6];		/**< Source MAC Address */
extern u_char GET_SN_MASK[4];		/**< Subnet mask received from the DHCP server */
extern u_char GET_GW_IP[4];		/**< Gateway IP address received from the DHCP server */
extern u_char GET_DNS_IP[4];		/**< DNS server IP address received from the DHCP server */
extern u_char GET_SIP[4];		/**< Leased source IP address received from the DHCP server */


extern NETCONF	NetConf;
extern SYSINFO	SysInfo;
extern CHCONF	ChConf;
extern StrConfigParam NetworkParam;

void sys_soft_reset(void)
{
	asm volatile("jmp 0x0000");	
}

unsigned char m_FlagDhcpInit = 0;

void dhcp_task_init(void)
{
	unsigned int i = 2;

	//Default MAC Load
	eeprom_read_block(&NetworkParam, (unsigned char*)EEPOP, sizeof(NetworkParam));
	memcpy(SRC_MAC_ADDR, NetworkParam.mac, 6);
	
	if(NetworkParam.dhcp)
	{
		init_timer();

		printf("Wait for DHCP server");
						  //0123456789abcdef	
		evb_set_lcd_text(1,(u_char*)"WaitForDHCP Ser");
		
		init_dhcp_client(i, sys_soft_reset, sys_soft_reset);

	
		if(!getIP_DHCPS())
		{
			printf("Fail get a IP from DHCP\r\n");

							  //0123456789abcdef				
			evb_set_lcd_text(1,(u_char*)"Fail get DHCP Se");
		}
		else
		{
			printf("Get inform from DHCP Server\r\n");	

			memcpy(NetworkParam.gw, (u_char*)GET_GW_IP, 4); 
			memcpy(NetworkParam.subnet,(u_char*)GET_SN_MASK, 4);
			memcpy(NetworkParam.ip, (u_char*)GET_SIP, 4);
			
			eeprom_write_block(&NetworkParam, (unsigned char*)EEPOP, sizeof(NetworkParam));
		}
	}
}

//
void dhcp_task(void)
{
	check_DHCP_state(2);
}


