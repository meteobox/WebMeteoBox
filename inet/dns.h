/**
 @file		dns.h
 @brief 		Implement Simple Domain Name System Protocol 
 */

#ifndef __DNS_H
#define __DNS_H

 /*
       <Message Format>
    +---------------------+
    |        Header       |
    +---------------------+
    |       Question      | the question for the name server
    +---------------------+
    |        Answer       | Resource Records answering the question            
    +---------------------+                                        
    |      Authority      | Resource Records pointing toward an authority      
    +---------------------+                                        
    |      Additional     | Resource Records holding additional information
    +---------------------+

               As follow, example of DNS Standard Query
               +---------------------------------------------------+
    Header     | OPCODE=SQUERY                                     |
               +---------------------------------------------------+
    Question   | QNAME=SRI-NIC.ARPA., QCLASS=IN, QTYPE=A           |
               +---------------------------------------------------+
    Answer     | <empty>                                           |
               +---------------------------------------------------+
    Authority  | <empty>                                           |
               +---------------------------------------------------+
    Additional | <empty>                                           |
               +---------------------------------------------------+

               As follow, example of DNS response
               +---------------------------------------------------+
    Header     | OPCODE=SQUERY, RESPONSE, AA                       |
               +---------------------------------------------------+
    Question   | QNAME=SRI-NIC.ARPA., QCLASS=IN, QTYPE=A           |
               +---------------------------------------------------+
    Answer     | SRI-NIC.ARPA. 86400 IN A 26.0.0.73                |
               |               86400 IN A 10.0.0.51                |
               +---------------------------------------------------+
    Authority  | <empty>                                           |
               +---------------------------------------------------+
    Additional | <empty>                                           |
               +---------------------------------------------------+
*/

/*	QCLASS values

CLASS fields appear in resource records.  The following CLASS mnemonics
and values are defined: 
*/
/* CLASS */
#define	CLASS_IN	1 	/**< the Internet */
#define CLASS_CS	2 	/**< the CSNET class (Obsolete - used only for examples in some obsolete RFCs) */
#define CLASS_CH	3 	/**< the CHAOS class */
#define CLASS_HS	4 	/**< Hesiod [Dyer 87] */
/* QCLASS */
#define QCLASS_ANY	255 	/**< any class */

/* 	QTYPE values

TYPE fields are used in resource records.  Note that these types are a subset of QTYPEs.
*/
/* TYPE */
#define		TYPE_A		1 	/**< The ARPA Internet */
#define		TYPE_NS		2 	/**< an authoritative name server */
#define		TYPE_MD		3 	/**< a mail destination (Obsolete - use MX) */
#define		TYPE_MF		4 	/**< a mail forwarder (Obsolete - use MX) */
#define		TYPE_CNAME	5 	/**< the canonical name for an alias */
#define		TYPE_SOA	6 	/**< marks the start of a zone of authority */
#define		TYPE_MB		7	/**< a mailbox domain name (EXPERIMENTAL) */
#define		TYPE_MG		8 	/**< a mail group member (EXPERIMENTAL) */
#define		TYPE_MR		9 	/**< a mail rename domain name (EXPERIMENTAL)*/
#define		TYPE_NULL	10 	/**< a null RR (EXPERIMENTAL) */
#define		TYPE_WKS	11 	/**< a well known service description */
#define		TYPE_PTR	12 	/**< a domain name pointer */
#define		TYPE_HINFO	13 	/**< host information */
#define		TYPE_MINFO	14 	/**< mailbox or mail list information */
#define		TYPE_MX		15 	/**< mail exchange */
#define		TYPE_TXT	16 	/**< text strings */
/* QTYPE */
#define		QTYPE_AXFR		252	/**< A request for a transfer of an entire zone */
#define		QTYPE_MAILB		253 	/**< A request for mailbox-related records (MB, MG or MR) */
#define		QTYPE_MAILA		254 	/**< A request for mail agent RRs (Obsolete - see MX) */
#define		QTYPE_TYPE_ALL		255 	/**< A request for all records */



#define	INITRTT		2000L	/**< Initial smoothed response time */
#define	MAXCNAME	10	/**< Maximum amount of cname recursion */


/* Round trip timing parameters */
#define	AGAIN	8		/**< Average RTT gain = 1/8 */
#define	LAGAIN	3		/**< Log2(AGAIN) */
#define	DGAIN	4		/**< Mean deviation gain = 1/4 */
#define	LDGAIN	2		/**< log2(DGAIN) */

#define	IPPORT_DOMAIN	53

/* Header for all domain messages */
/*
                                    1  1  1  1  1  1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      ID                       |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    QDCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ANCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    NSCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ARCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*/    
/*A four bit field that specifies kind of query in this
  message.  This value is set by the originator of a query
  and copied into the response.  The values are:
 */
#define OP_QUERY		0	/**< a standard query (QUERY) */
#define OP_IQUREY		1	/**< an inverse query (IQUERY) */
#define OP_STATUS		2	/**< a server status request (STATUS)*/

/* A one bit field that specifies whether this message is a query (0), or a response (1). */
#define	QR_QUERY		0	
#define	QR_RESPONSE		1

/* Response code - this 4 bit field is set as part of
                   responses.  The values have the following interpretation:
*/

#define	RC_NO_ERROR		0	/**< No error condition */
#define	RC_FORMAT_ERROR		1	/**< Format error - The name server was unable to interpret the query. */
#define	RC_SERVER_FAIL		2	/**< Server failure - The name server was unable to process this query due to a problem with the name server. */
#define	RC_NAME_ERROR		3	/**< Name Error - Meaningful only for responses from an authoritative name server, this code signifies that the domain name referenced in the query does not exist.*/
#define	RC_NOT_IMPL		4	/**< Not Implemented - The name server does not support the requested kind of query.*/
#define	RC_REFUSED		5	/**< Refused - The name server refuses to perform the specified operation for policy reasons. 
 							For example, a name server may not wish to provide the information to the particular requester,
							or a name server may not wish to perform a particular operation (e.g., zone */

#define DHDR_SIZE		12


/**
 @brief 	Header for all domain messages
 */
typedef struct _DHDR 
{
	u_int	id;				/**< Identification */
	u_char flag0;
	u_char flag1;
	u_int	qdcount;	/**< Question count */
	u_int	ancount;	/**< Answer count */
	u_int	nscount;	/**< Authority (name server) count */
	u_int	arcount;	/**< Additional record count */
}DHDR;

/* rd : Recursion desired , tc : Truncation, aa	: Authoratative answer, opcode : op code = OP_QUREY, OP_IQUREY, OP_STAUTS, qr : Query/Response */
#define MAKE_FLAG0(qr,op,aa,tc,rd)	( ((qr & 0x01) << 7) + ((op & 0x0F) << 3) + ((aa & 0x01) << 2) + ((tc & 0x01) << 1) + (rd & 0x01) )

/* rcode : Response code, z : Reserved for future use.  Must be zero in all queries and responses, */
/* ra : Recursion Available - this be is set or cleared in a response, and denotes whether recursive query support is available in the name server.*/
#define MAKE_FLAG1(ra,z,rcode)		( ((ra & 0x01) << 7) + ((z & 0x07) << 4) +  (rcode & 0x0F) )

/*
		  <QUESTION FORMAT >
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                                               |
    /                     QNAME                     /
    /                                               /
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     QTYPE                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     QCLASS                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

where:

QNAME           a domain name represented as a sequence of labels, where
                each label consists of a length octet followed by that
                number of octets.  The domain name terminates with the
                zero length octet for the null label of the root.  Note
                that this field may be an odd number of octets; no
                padding is used.

QTYPE           a two octet code which specifies the type of the query.
                The values for this field include all codes valid for a
                TYPE field, together with some more general codes which
                can match more than one type of RR.

QCLASS          a two octet code that specifies the class of the query.
                For example, the QCLASS field is IN for the Internet.
*/

/**
 @brief 	QUESTION FORMAT
 */
typedef struct _QUESTION
{
//	char* qname;		// Variable length data
	u_int qtype;
	u_int qclass;
}DQST;


/*
	 Resource record format

The answer, authority, and additional sections all share the same
format: a variable number of resource records, where the number of
records is specified in the corresponding count field in the header.
Each resource record has the following format:
                                    1  1  1  1  1  1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                                               |
    /                                               /        
    /                      NAME                     /
    |                                               |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      TYPE                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     CLASS                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      TTL                      |
    |                                               |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                   RDLENGTH                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     RDATA                     /
    /                                               /
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

where:

NAME            a domain name to which this resource record pertains.
			In order to reduce the size of messages, the domain system utilizes a
			compression scheme which eliminates the repetition of domain names in a
			message.  In this scheme, an entire domain name or a list of labels at
			the end of a domain name is replaced with a pointer to a prior occurance
			of the same name.
			The pointer takes the form of a two octet sequence:
			    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
			    | 1  1|                OFFSET                   |
			    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
			The first two bits are ones.  This allows a pointer to be distinguished
			from a label, since the label must begin with two zero bits because
			labels are restricted to 63 octets or less.  (The 10 and 01 combinations
			are reserved for future use.)  The OFFSET field specifies an offset from
			the start of the message (i.e., the first octet of the ID field in the
			domain header).  A zero offset specifies the first byte of the ID field,
			etc.
			The compression scheme allows a domain name in a message to be
			represented as either:
			   - a sequence of labels ending in a zero octet
			   - a pointer
			   - a sequence of labels ending with a pointer

TYPE            two octets containing one of the RR type codes.  This
                field specifies the meaning of the data in the RDATA
                field.

CLASS           two octets which specify the class of the data in the
                RDATA field.

TTL             a 32 bit unsigned integer that specifies the time
                interval (in seconds) that the resource record may be
                cached before it should be discarded.  Zero values are
                interpreted to mean that the RR can only be used for the
                transaction in progress, and should not be cached.

RDLENGTH        an unsigned 16 bit integer that specifies the length in
                octets of the RDATA field.

RDATA           a variable length string of octets that describes the
                resource.  The format of this information varies
                according to the TYPE and CLASS of the resource record.
                For example, the if the TYPE is A and the CLASS is IN,
                the RDATA field is a 4 octet ARPA Internet address.
*/

#define	COMPRESSION_SCHEME	0xC0

/**
 @brief 	Resource record format

 The answer, authority, and additional sections all share the same
format: a variable number of resource records, where the number of
records is specified in the corresponding count field in the header.
Each resource record has the following format:
 */
typedef struct _RESOURCE_RECORD
{
//	char* 	_name;		// Variable length data
	u_int	_type;
	u_int	_class;
	u_int	_ttl;
	u_int   _rdlen;
//	char*    _rdata;	// Variable length data
}DRR;




#define MAX_DNSMSG_SIZE 	512			/**< Maximum size of DNS message */
#define MAX_DOMAINNAME_LEN	50			/**< Maximum size of domain name */
#define MAX_QNAME_LEN		128			/**< Maximum size of qname */


typedef enum _QUERYDATA{BYNAME,BYIP}QUERYDATA;		/* Query type */



/* Resolve domain name or ip address from DNS server */
extern u_char dns_query(SOCKET s, u_long dnsip, u_char * domain_name, u_long* domain_ip,QUERYDATA querydata, u_int elapse);

extern int gethostbyaddr(u_long ipaddr,char* domain);		// gethostbyaddr function retrieves the host domain name corresponding to a network address
extern u_long gethostbyname(char* hostname);			// gethostbyname function retrieves host ip address corresponding to a host name


#endif
