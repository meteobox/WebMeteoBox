/**
 @file		httpd.h
 @brief 		Define Constants and fucntions associated with HTTP protocol.
 */

#ifndef	__HTTPD_H__
#define	__HTTPD_H__


#define HTTP_SERVER_PORT		80		/**< Http server well-known port number */

/* HTTP Method */
#define		METHOD_ERR		0		/**< Error Method. */
#define		METHOD_GET		1		/**< GET Method.   */
#define		METHOD_HEAD		2		/**< HEAD Method.  */
#define		METHOD_POST		3		/**< POST Method.  */

/* HTTP GET Method */
#define		PTYPE_ERR		0		/**< Error file. */
#define		PTYPE_HTML		1		/**< HTML file.  */
#define		PTYPE_GIF		2		/**< GIF file.   */
#define		PTYPE_TEXT		3		/**< TEXT file.  */
#define		PTYPE_JPEG		4		/**< JPEG file.  */
#define		PTYPE_FLASH		5		/**< FLASH file. */
#define		PTYPE_MPEG		6		/**< MPEG file.  */
#define		PTYPE_PDF		7		/**< PDF file.   */
#define 	PTYPE_CGI		8		/**< CGI */

/* HTTP response */
#define		STATUS_OK		200
#define		STATUS_CREATED		201
#define		STATUS_ACCEPTED		202
#define		STATUS_NO_CONTENT	204
#define		STATUS_MV_PERM		301
#define		STATUS_MV_TEMP		302
#define		STATUS_NOT_MODIF	304
#define		STATUS_BAD_REQ		400
#define		STATUS_UNAUTH		401
#define		STATUS_FORBIDDEN	403
#define		STATUS_NOT_FOUND	404
#define		STATUS_INT_SERR		500
#define		STATUS_NOT_IMPL		501
#define		STATUS_BAD_GATEWAY	502
#define		STATUS_SERV_UNAVAIL	503

#define		MAX_INT_STR			80

/* HTML Doc. for ERROR */
//#define ERROR_HTML_PAGE "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 78\r\n\r\n<HTML>\r\n<BODY>\r\nSorry, the page you requested was not found.\r\n</BODY>\r\n</HTML>\r\n\0"
//static PROGMEM char  ERROR_HTML_PAGE[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 78\r\n\r\n<HTML>\r\n<BODY>\r\nSorry, the page you requested was not found.\r\n</BODY>\r\n</HTML>\r\n\0";

//#define ERROR_REQUEST_PAGE "HTTP/1.1 400 OK\r\nContent-Type: text/html\r\nContent-Length: 50\r\n\r\n<HTML>\r\n<BODY>\r\nInvalid request.\r\n</BODY>\r\n</HTML>\r\n\0"
//static PROGMEM char ERROR_REQUEST_PAGE[] = "HTTP/1.1 400 OK\r\nContent-Type: text/html\r\nContent-Length: 50\r\n\r\n<HTML>\r\n<BODY>\r\nInvalid request.\r\n</BODY>\r\n</HTML>\r\n\0";

/* HTML Doc. for CGI result  */
//#define HTML_HEADER "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: "
//static PROGMEM char HTML_HEADER[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ";

//#define RETURN_CGI_PAGE "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 59\r\n\r\n<HTML>\r\n<BODY>\r\nCGI command is completed.\r\n</BODY>\r\n</HTML>\r\n\0"
//static PROGMEM char RETURN_CGI_PAGE[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 59\r\n\r\n<HTML>\r\n<BODY>\r\nCGI command is completed.\r\n</BODY>\r\n</HTML>\r\n\0";

/* Response header for HTML*/


//#define RES_HTMLHEAD_OK	"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: "

//#define RES_HTMLHEAD_OK	"HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\nContent-Length: " //05.04.2013


//static PROGMEM char RES_HTMLHEAD_OK[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ";
/* Response head for TEXT */
//#define RES_TEXTHEAD_OK	"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "

/* Response head for GIF */
//#define RES_GIFHEAD_OK	"HTTP/1.1 200 OK\r\nContent-Type: image/gif\r\nContent-Length: "

/* Response head for JPEG */
//#define RES_JPEGHEAD_OK	"HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: "		

/* Response head for FLASH */
//#define RES_FLASHHEAD_OK "HTTP/1.1 200 OK\r\nContent-Type: application/x-shockwave-flash\r\nContent-Length: "
//static PROGMEM char RES_FLASHHEAD_OK[] = "HTTP/1.1 200 OK\r\nContent-Type: application/x-shockwave-flash\r\nContent-Length: ";

/* Response head for MPEG */
//#define RES_MPEGHEAD_OK "HTTP/1.1 200 OK\r\nContent-Type: video/mpeg\r\nContent-Length: "	

/* Response head for PDF */
//#define RES_PDFHEAD_OK "HTTP/1.1 200 OK\r\nContent-Type: application/pdf\r\nContent-Length: "


//#define TX_RX_MAX_BUF_SIZE 512

#define MAX_URI_SIZE	(TX_RX_MAX_BUF_SIZE/2 - sizeof(char)*2)		

/**
 @brief 	Structure of HTTP REQUEST 
 */
 
typedef struct _st_http_request
{
	u_char	METHOD;						
	u_char	TYPE;						
	char	URI[MAX_URI_SIZE];				
}st_http_request;



extern void unescape_http_url(char * url);					/* convert escape character to ascii */

extern void parse_http_request(st_http_request *, u_char *);		/* parse request from peer */

extern void find_http_uri_type(u_char *, char *);				/* find MIME type of a file */

extern void make_http_response_head(char *, char, u_long);				/* make response header */

extern char* get_http_param_value(char* uri, char* param_name);	/* get the user-specific parameter value */

extern char* get_http_uri_name(char* uri);
#endif	/* end of __HTTPD_H__ */ 
