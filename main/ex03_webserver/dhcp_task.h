#ifndef _CONFIG_TASK_
#define _CONFIG_TASK_

//#include "../../mcu/config.h"
/*
#define SOCK_CONFIG			3

//FLUSB에서 변경
#define CONFIG_CLIENT_PORT	48713  // <- 48723
#define CONFIG_SERVER_PORT	48714  // <- 48724

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


*/

void dhcp_task_init(void);
void dhcp_task(void);
#endif
