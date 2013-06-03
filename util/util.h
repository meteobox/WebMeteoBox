/*
*
@file		util.h
*/

#ifndef _UTIL_H_
#define _UTIL_H_


#define NO_USE_UTIL_FUNC


extern u_char D2C(char c); 					/* Convert HEX(0-F) to a character */
extern char C2D(u_char c); 					/* Convert a character to HEX */
extern unsigned int ATOI(char* str,u_int base); 			/* Convert a string to integer number */
extern int ValidATOI(char* str, int base, unsigned int* ret); 		/* Verify character string and Convert it to (hexa-)decimal. */
extern char * strtok(char * s,const char * ct); 		/* Tokenize a string */
extern void replacetochar(char * str, char oldchar, char newchar); /* Replace old character with new character in the string */

extern u_short swaps(u_int i);
extern u_long swapl(u_long l);

#ifndef NO_USE_UTIL_FUNC
extern char* ITOA(u_int value,char* str,u_int base);		/* Convert Decimal Number to string */
#endif

#endif /* _UTIL_H */

