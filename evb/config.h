#ifndef _CONFIG_H
#define _CONFIG_H


#define SYS_INFO	0x00
#define SYS_TEST	(SYS_INFO)
#define SYS_VER		(SYS_TEST + 2)
#define SYS_AUTORESET	(SYS_VER + 4)
#define SYS_ANY_PORT	(SYS_AUTORESET + 1)

#define NET_CONF	0x20
#define NET_TEST	(NET_CONF)
#define NET_MAC		(NET_TEST+2)
#define NET_SIP		(NET_MAC + 6)
#define NET_GWIP	(NET_SIP + 4)
#define NET_SN		(NET_GWIP + 4)
#define NET_DNS		(NET_SN + 4)
#define NET_MEMALLOC	(NET_DNS + 4)

#define CH_CONF		0x50
#define CH_TEST		(CH_CONF)
#define CH_TYPE_0	(CH_TEST + 2)
#define CH_PORT_0	(CH_TYPE_0 + 1)
#define CH_DESTIP_0	(CH_PORT_0 + 2)
#define CH_TYPE_1	(CH_DESTIP_0 + 4)
#define CH_PORT_1	(CH_TYPE_1 + 1)
#define CH_DESTIP_1	(CH_PORT_1 + 2)
#define CH_TYPE_2	(CH_DESTIP_1 + 4)
#define CH_PORT_2	(CH_TYPE_2 + 1)
#define CH_DESTIP_2	(CH_PORT_2 + 2)
#define CH_TYPE_3	(CH_DESTIP_2 + 4)
#define CH_PORT_3	(CH_TYPE_3 + 2)
#define CH_DESTIP_3	(CH_PORT_3 + 2)

#define TEST_VALUE	0xA5A5

#define DEFAULT_NET_MAC		"\x00\x08\xDC\x00\x00\x00"	/**< Default Mac Address : 00.08.DC.00.00.00 */
#define DEFAULT_NET_SIP		0xC0A80002	/**< Default Source IP     : 192.168.0.2    */
#define DEFAULT_NET_GWIP	0xC0A80001	/**< Default Gateway IP    : 192.168.0.1    */
#define DEFAULT_NET_SN		0xFFFFFF00	/**< Default Subnet Mask   : 255.255.255.0  */
#define DEFAULT_NET_DNS		0x00000000	/**< Default DNS Server IP : 0.0.0.0	  */
#define DEFAULT_NET_MEMALLOC	0x55		/**< Default iinchip memory allocation */

#define DEFAULT_CH_TYPE		LB_TCPS		/**< TCP Loop-Back Server */
#define DEFAULT_LISTEN_PORT	5000		/**< Default Listen Port  : 5000 */
#define DEFAULT_CONNECT_PORT	3000		/**< Default Connect Port : 3000 */
#define DEFAULT_SOURCE_PORT	3000		/**< Default Source Port  : 3000 */
#define DEFAULT_HTTP_PORT	80		/**< Default HTTP Port    : 80 */
#define DEFAULT_CH_DESTIP	0xC0A80003	/**< Default Destination IP : 192.168.0.3 */

#define SYSTEM_ANY_PORT		1000		/**< Default System Any Port Number  */
#define SYSTEM_AUTO_RESET	1		
#define SYSTEM_MANUAL_RESET	0


typedef enum _APPTYPE{NOTUSE, DHCP_CLIENT, LB_TCPS, LB_TCPC, LB_UDP, WEB_SERVER}APPTYPE;

typedef struct _SYSINFO
{
	u_int	test;
	u_long	ver;
	u_char	auto_reset;
	u_int	any_port;
}SYSINFO;

typedef struct _NETCONF
{
	u_int test;
	u_char mac[6];
	u_long sip;
	u_long gwip;
	u_long sn;
	u_long dns;
	u_char Mem_alloc;
}NETCONF;


typedef struct _CHCONF
{
	u_int test;
	struct _CH_CONF
	{
		u_char	type;
		u_int	port;
		u_long	destip;
	}ch[MAX_SOCK_NUM];
}CHCONF;


extern void init_conf_variables(void);

extern void set_sysinfo(SYSINFO* pSysInfo);
extern void get_sysinfo(SYSINFO* pSysInfo);
extern u_char get_reset_flag(void);
extern void set_reset_flag(u_char flag);
extern u_int  get_system_any_port(void);

extern void set_netconf(NETCONF* pNetConf);
extern void get_netconf(NETCONF* pNetConf);
extern void load_factory_netconf(void);
extern void display_netconf(NETCONF* pNetConf);

extern void set_chconf(CHCONF* pChConf);
extern void get_chconf(CHCONF* pChConf);
extern void load_factory_chconf(void);
extern void display_chconf(CHCONF* pChConf);

//net config + 2007.12.13 [jhpark]
extern void get_netconf_reg(NETCONF* pNetConf);
extern u_char comp_net_conf(NETCONF *pConfA, NETCONF *pConfB);
#endif 


