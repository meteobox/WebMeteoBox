#include <stdio.h>

#include "../mcu/types.h"
#include "w5300.h"
#include "socket.h"


#define SWAP16(A)		((((A << 8 ) & 0xFF00)) | ((A >> 8)& 0x00FF))

uint16 local_port;
uint8 first_send_flag[MAX_SOCK_NUM];
uint16 cmd_cnt[MAX_SOCK_NUM] ={0,0};

/** 
 * The flag to check if first send or not.
 */
uint8    check_sendok_flag[MAX_SOCK_NUM];

/**
* Internal Functions
*/

/*
*********************************************************************
This Socket function initialize the channel in perticular mode, and set the port
and wait for W3150A+ done it.
And the Parameters "s" is for socket number, "protocol" is for socket protocol,
port is the source port for the socket and flag is the option for the socket.
This function return 1 for sucess else 0.
*********************************************************************
*/

uint8 socket(SOCKET s, uint8 protocol, uint16 port, uint16 flag)
{
	uint8 ret;
#ifdef __DEF_IINCHIP_DBG__
	printf("socket()\r\n");
#endif
	if ((protocol == Sn_MR_TCP) || (protocol == Sn_MR_UDP) || (protocol == Sn_MR_IPRAW) || (protocol == Sn_MR_MACRAW))
	{
		close(s);
		IINCHIP_WRITE(Sn_MR(s), protocol | flag );
#ifdef __DEF_IINCHIP_DBG__
		printf("Sn_MR(%d) = 0x%02x\r\n", s, IINCHIP_READ(Sn_MR(s)));
#endif
		if (port != 0) {
			IINCHIP_WRITE(Sn_PORTR(s),(uint8)((port & 0xff00) >> 8));
			IINCHIP_WRITE((Sn_PORTR(s) + 1),(uint8)(port & 0x00ff));
		} else {
			local_port++; // if don't set the source port, set local_port number.
			IINCHIP_WRITE(Sn_PORTR(s),(uint8)((local_port & 0xff00) >> 8));
			IINCHIP_WRITE((Sn_PORTR(s) + 1),(uint8)(local_port & 0x00ff));
		}
		IINCHIP_WRITE(Sn_CR(s), Sn_CR_OPEN); // run sockinit Sn_CR
		ret = 1;
		first_send_flag[s] = 1;
	}
	else
	{
		ret = 0;
	}
#ifdef __DEF_IINCHIP_DBG__
	printf("Sn_SSR = 0x%02x , Protocol = 0x%02x\r\n", getSn_SSR(s), IINCHIP_READ(Sn_MR(s)));
#endif
	return ret;
}


/*
*********************************************************************
This function close the socket and parameter is "s" which represent the socket number
*********************************************************************
*/


void close(SOCKET s)
{
	IINCHIP_WRITE(Sn_CR(s), Sn_CR_CLOSE);
}

/*
void close(SOCKET s) // 18.03.2013
{
	// M_08082008 : It is fixed the problem that Sn_SSR cannot be changed a undefined value to the defined value.
	//              Refer to Errata of W5300
	//Check if the transmit data is remained or not.
	if( ((getSn_MR(s)& 0x0F) == Sn_MR_TCP) && (getSn_TX_FSR(s) != getIINCHIP_TxMAX(s)) ) 
	{ 
		uint16 loop_cnt =0;
		while(getSn_TX_FSR(s) != getIINCHIP_TxMAX(s))
		{
			if(loop_cnt++ > 10)
			{
				uint8 destip[4];
				// M_11252008 : modify dest ip address
				//getSIPR(destip);
				destip[0] = 0;destip[1] = 0;destip[2] = 0;destip[3] = 1;
				socket(s,Sn_MR_UDP,0x3000,0);
				sendto(s,(uint8*)"x",1,destip,0x3000); // send the dummy data to an unknown destination(0.0.0.1).
				break; // M_11252008 : added break statement
			}
		//wait_10ms(10);
		my_wait_1ms(100);
		asm("WDR");
		}
	};
	setSn_IR(s ,0x00FF);          // Clear the remained interrupt bits.
	setSn_CR(s ,Sn_CR_CLOSE);     // Close s-th SOCKET     
}
*/

/*
*********************************************************************
This function established  the connection for the channel in passive (server) mode.
This function waits for the request from the peer. The parameter "s" is the socket number
This function return 1 for success else 0.
*********************************************************************
*/

uint8 listen(SOCKET s)
{
	uint8 ret;
#ifdef __DEF_IINCHIP_DBG__
	printf("listen()\r\n");
#endif
	if( getSn_SSR(s) == SOCK_INIT )
	{
		IINCHIP_WRITE(Sn_CR(s), Sn_CR_LISTEN);
		ret = 1;
	}
	else
	{
		ret = 0;
#ifdef __DEF_IINCHIP_DBG__
	printf("listen() - Failed. getSn_SSR(s) = 0x%02x \r\n", getSn_SSR(s));
#endif
	}
	printf("Sn_PORTR(%d) : 0x%02x\r\n", s, IINCHIP_READ(Sn_PORTR(s)));
	printf("Sn_PORTR(%d)+1 : 0x%02x\r\n", s, IINCHIP_READ(Sn_PORTR(s)+1));
	return ret;
}


/*
*********************************************************************
This function established  the connection for the channel in Active (client) mode. 
This function waits for the untill the connection is established. The parameter "s" is the socket number
This function return 1 for success else 0.
*********************************************************************
*/

uint8    connect(SOCKET s, uint8 * addr, uint16 port)
{
   if
   (
      ((addr[0] == 0xFF) && (addr[1] == 0xFF) && (addr[2] == 0xFF) && (addr[3] == 0xFF)) ||
      ((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) ||
      (port == 0x00) 
   )
   {
      #ifdef __DEF_IINCHIP_DBG__
         printf("%d : Fail[invalid ip,port]\r\n",s);
      #endif
      return 0;
   }
   
		// set destination IP
		IINCHIP_WRITE(Sn_DIPR(s), addr[0]);
		IINCHIP_WRITE(Sn_DIPR1(s), addr[1]);
		IINCHIP_WRITE(Sn_DIPR2(s), addr[2]);
		IINCHIP_WRITE(Sn_DIPR3(s), addr[3]);
	
		// set destination PORT
		IINCHIP_WRITE(Sn_DPORTR(s), port >> 8);
		IINCHIP_WRITE(Sn_DPORTR1(s), port & 0xff);

		// Connect
		IINCHIP_WRITE(Sn_CR(s), Sn_CR_CONNECT);

   return 1;   
}


/*
*********************************************************************
This function used for disconnect the socket and parameter is "s" which represent the socket number
*********************************************************************
*/

void disconnect(SOCKET s)
{
#ifdef __DEF_IINCHIP_DBG__
	printf("disconnect()\r\n");
#endif
	IINCHIP_WRITE(Sn_CR(s), Sn_CR_DISCON);
}

/*
*********************************************************************
This function used to send the data in TCP mode and the parameter "s" represents
the socket number and "buf" a pointer to data and "len" is the data size to be send.
This function return 1 for success else 0.
*********************************************************************
*/

uint32   send(SOCKET s, uint8 * buf, uint32 len)
{
   uint8 status=0;
   uint32 ret=0;
   uint32 freesize=0;
   #ifdef __DEF_IINCHIP_DBG__
      uint32 loopcnt = 0;

      printf("%d : send()\r\n",s);
   #endif
   
   ret = len;
   if (len > getIINCHIP_TxMAX(s)) ret = getIINCHIP_TxMAX(s); // check size not to exceed MAX size.
   
   

   /*
    * \note if you want to use non blocking function, <b>"do{}while(freesize < ret)"</b> code block 
    * can be replaced with the below code. \n
    * \code 
    *       while((freesize = getSn_TX_FSR(s))==0); 
    *       ret = freesize;
    * \endcode
    */
   // -----------------------
   // NOTE : CODE BLOCK START
   do                                   
   {
      freesize = getSn_TX_FSR(s);
      status = getSn_SSR(s);
      #ifdef __DEF_IINCHIP_DBG__
         printf("%d : freesize=%ld\r\n",s,freesize);
         if(loopcnt++ > 0x0002000)
         { 
            printf("%d : freesize=%ld,status=%04x\r\n",s,freesize,status);
            printf("%d:Send Size=%08lx(%d)\r\n",s,ret,ret);
            printf("MR=%04x\r\n",*((vuint16*)MR));
            loopcnt = 0;
         }
      #endif
      if ((status != SOCK_ESTABLISHED) && (status != SOCK_CLOSE_WAIT)) return 0;
   } while (freesize < ret);
   // NOTE : CODE BLOCK END
   // ---------------------
   
	 if(ret & 0x01) wiz_write_buf(s, buf, (ret+1));
	 else wiz_write_buf(s,buf,ret);                   // copy data

   #ifdef __DEF_IINCHIP_DBG__
      loopcnt=0;
   #endif   
   
   if(!first_send_flag[s])                	 // if first send, skip.
   {
      while (!(getSn_IR(s) & Sn_IR_SENDOK))  // wait previous SEND command completion.
      {
      #ifdef __DEF_IINCHIP_DBG__

         if(loopcnt++ > 0x1ffff)
         {
            printf("%d:Sn_SSR(%04x)\r\n",s,status);
            printf("%d:Send Size=%08lx(%d)\r\n",s,ret,ret);
            printf("MR=%04x\r\n",*((vuint16*)MR));
            loopcnt = 0;
         }
      #endif
         if (getSn_SSR(s) == SOCK_CLOSED)    // check timeout or abnormal closed.
         {
            #ifdef __DEF_IINCHIP_DBG__
               printf("%d : Send Fail. SOCK_CLOSED.\r\n",s);
            #endif
            return 0;
         }
      }
      setSn_IR(s, Sn_IR_SENDOK);             // clear Sn_IR_SENDOK	
   }
   else first_send_flag[s] = 0;
   
   // send
   setSn_TX_WRSR(s,ret);   
   setSn_CR(s,Sn_CR_SEND);
   while(!(getSn_IR(s) & Sn_IR_SENDOK));
   return ret;
}

/*
*********************************************************************
This function is an application I/F function which is used to receive the data in TCP mode.
It continues to wait for data as much as the application wants to receive and the 
parameter "s" represents the socket number and "buf" a pointer to copy the data
to be received and "len" is the data size to be read. This function return received data size
for success else -1.
*********************************************************************
*/
uint32 recv(SOCKET s, uint8 * buf, uint32 len)
{
	uint16 pack_size = 0;

#ifdef __DEF_IINCHIP_DBG__
	printf("recv() : len=%08lx\r\n",len);
#endif

	if(IINCHIP_READ(Sn_MR(s)) & Sn_MR_ALIGN)
	{
		wiz_read_buf(s, buf, len);

		IINCHIP_WRITE(Sn_CR(s),Sn_CR_RECV);
		return len;
	}
	wiz_read_buf(s, (uint8*)&pack_size, 2);
	pack_size = SWAP16(pack_size);

	len = pack_size;
	if(pack_size & 0x01) len += 1;
#ifdef __DEF_IINCHIP_DBG__   
	printf("%u:pack_size=%d\r\n", s, pack_size);
#endif

	wiz_read_buf(s, buf, len);

	IINCHIP_WRITE(Sn_CR(s), Sn_CR_RECV);
	return (uint32)pack_size;
}

/*
*********************************************************************
This function is an application I/F function which is used to send the data for other then
TCP mode. Unlike TCP transmission, The peer's destination address and the port is needed.
and the parameter "s" represents the socket number and "buf" a pointer to the data 
and "len" is the data size to send and addr is the peer's Destination IP address and port is
the peer's destination port number. This function return send data size for success else -1.
*********************************************************************
*/
uint16 sendto(SOCKET s, uint8 * buf, uint16 len, uint8 * addr, uint16 port)
{
	uint8 status=0;
	uint8 isr=0;
	uint16 ret=0;

#ifdef __DEF_IINCHIP_DBG__
//	printf("sendto()\r\n");
#endif
	if (len > getIINCHIP_TxMAX(s)) ret = getIINCHIP_TxMAX(s); // check size not to exceed MAX size.
	else ret = len;

	if
		(
		 	((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) ||
		 	((port == 0x00)) ||(ret == 0)
		) 
 	{
#ifdef __DEF_IINCHIP_DBG__
	printf("%d Fail[%.2x.%.2x.%.2x.%.2x, %.d, %d]\r\n",s, addr[0], addr[1], addr[2], addr[3] , port, len);
	printf("Fail[invalid ip,port]\r\n");
#endif
	}
	else
	{
		IINCHIP_WRITE(Sn_DIPR(s), addr[0]);
		IINCHIP_WRITE(Sn_DIPR1(s), addr[1]);
		IINCHIP_WRITE(Sn_DIPR2(s), addr[2]);
		IINCHIP_WRITE(Sn_DIPR3(s), addr[3]);

		IINCHIP_WRITE(Sn_DPORTR(s), port >> 8);
		IINCHIP_WRITE(Sn_DPORTR1(s), port & 0xff);

		// copy data
		setSn_TX_WRSR(s, ret);

		wiz_write_buf(s, buf, ret+(ret & 0x01)); 
		IINCHIP_WRITE(Sn_CR(s),Sn_CR_SEND);
		while (!((isr = getSn_IR(s)) & Sn_IR_SENDOK))
		{
			status = getSn_SSR(s);

			if ((status == SOCK_CLOSED) || (isr & Sn_IR_TIMEOUT))
			{
#ifdef __DEF_IINCHIP_DBG__
//				printf("send fail.\r\n");
#endif
				ret = 0; break;
			}
		}
		setSn_IR(s, Sn_IR_SENDOK);
	}
	return ret;
}


/*
*********************************************************************
This function is an application I/F function which is used to receive the data in other then
TCP mode. This function is used to receive UDP, IP_RAW and MAC_RAW mode, and handle 
the header as well. 
The parameter "s" represents the socket number and "buf" a pointer to copy the data to be 
received and "len" is the data size to read and and addr is a pointer to store the peer's 
IP address and port is a pointer to store the peer's port number.
This function return received data size for success else -1.
*********************************************************************
*/
uint16 recvfrom(SOCKET s,	uint8 * buf, uint16 len, uint8 * addr, uint16 *port)
{
	uint8 head[8];
	uint16 data_len=0;

#ifdef __DEF_IINCHIP_DBG__
//	printf("recvfrom()\r\n");
#endif

	if ( len > 0 )
	{
		switch (IINCHIP_READ(Sn_MR(s)) & 0x07)
		{
			case Sn_MR_UDP :
				wiz_read_buf(s, head, 0x08);
				// read peer's IP address, port number.
				addr[0] = head[0];
				addr[1] = head[1];
				addr[2] = head[2];
				addr[3] = head[3];
				*port = head[4];
				*port = (*port << 8) + head[5];
				data_len = (uint16)head[6];
				data_len = (data_len << 8) + (uint16)head[7];

#ifdef __DEF_IINCHIP_DBG__
//				printf("UDP msg arrived\r\n");
//				printf("source Port : %d\r\n", *port);
//				printf("source IP : %d.%d.%d.%d\r\n", addr[0], addr[1], addr[2], addr[3]);
#endif
				break;
			case Sn_MR_IPRAW :
				wiz_read_buf(s, head, 0x06);
				addr[0] = head[0];
				addr[1] = head[1];
				addr[2] = head[2];
				addr[3] = head[3];
				data_len = (uint16)head[4];
				data_len = (data_len << 8) + (uint16)head[5];

#ifdef __DEF_IINCHIP_DBG__
				printf("IP RAW msg arrived\r\n");
				printf("source IP : %d.%d.%d.%d\r\n", addr[0], addr[1], addr[2], addr[3]);
#endif
				break;
			case Sn_MR_MACRAW :
				wiz_read_buf(s, head, 2);
				data_len = (uint16)head[0];
				data_len = (data_len<<8) + (uint16)head[1];
		
				break;

			default :
				break;
		}

		wiz_read_buf(s, buf, data_len+(data_len & 0x01)); // data copy.
		if((IINCHIP_READ(Sn_MR(s)) & 0x07)==Sn_MR_MACRAW)
		{
			uint8 crc[4];
#ifndef __DEF_IINCHIP_DGB__
				printf("MAC RAW msg arrived\r\n");
				printf("dest mac=%.2X.%.2X.%.2X.%.2X.%.2X.%.2X\r\n",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);
				printf("src  mac=%.2X.%.2X.%.2X.%.2X.%.2X.%.2X\r\n",buf[6],buf[7],buf[8],buf[9],buf[10],buf[11]);
				printf("type    =%.2X%.2X\r\n",buf[12],buf[13]); 
#endif				
			wiz_read_buf(s, crc, 4);
		}
		IINCHIP_WRITE(Sn_CR(s), Sn_CR_RECV);
	}
#ifdef __DEF_IINCHIP_DBG__
//	printf("recvfrom() end ..\r\n");
#endif
	return data_len;
}

uint16 igmpsend(SOCKET s, uint8 * buf, uint16 len)
{
	uint8 status=0;
	uint8 isr=0;
	uint16 ret=0;
	
#ifdef __DEF_IINCHIP_DBG__
	printf("igmpsend()\r\n");
#endif
   if (len > getIINCHIP_TxMAX(s)) ret = getIINCHIP_TxMAX(s); // check size not to exceed MAX size.
   else ret = len;

	if	(ret == 0) 
 	{
 	   
#ifdef __DEF_IINCHIP_DBG__
	printf("%d Fail[%d]\r\n",s, len);
#endif
	}
	else
	{
		// copy data
      setSn_TX_WRSR(s,ret);
	  
      wiz_write_buf(s, buf, ret+(ret & 0x01));	  
	  
  	   IINCHIP_WRITE(Sn_CR(s),Sn_CR_SEND);
      while (!((isr=getSn_IR(s)) & Sn_IR_SENDOK))
      {
      	status = getSn_SSR(s);
			  if ((status == SOCK_CLOSED) || (isr & Sn_IR_TIMEOUT))
			    {
          #ifdef __DEF_IINCHIP_DBG__
			    printf("igmpsend fail.\r\n");
          #endif
				  ret = 0; break;
			    }
		  }
		setSn_IR(s, Sn_IR_SENDOK);
	  
	}
	return ret;
}
