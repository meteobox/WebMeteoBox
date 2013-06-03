/**
 @file		evb.c
 @brief 		functions to initialize EVB prepheral equipments
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

#include "../mcu/types.h"


#if (__IINCHIP_TYPE__ == __W5100__)
 #include "../iinchip/w5100.h"
#elif (__IINCHIP_TYPE__ == __W5300__)
 #include "../iinchip/w5300.h"
#else
	#error "unknown iinchip type"
#endif

#include "wiz_util.h"
#include "../iinchip/socket.h"

#include "util.c"
#include "sockutil.c"
        

/**
@brief	ntohs function converts a unsigned short from TCP/IP network byte order to host byte order (which is little-endian on Intel processors).
@return 	a 16-bit number in host byte order
*/




