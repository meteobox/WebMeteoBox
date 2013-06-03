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


static char * ___strtok=NULL;


/**
@brief	CONVERT HEX INTO CHAR
@return	a character
  
This function converts HEX(0-F) to a character
*/
u_char D2C(
	char c	/**< is a Hex(0x00~0x0F) to convert to a character */
	)
{
	u_int t = (u_int) c;
	if (t >= 0 && t <= 9)
		return '0' + c;
	if (t >= 10 && t <= 15)
		return 'A' + c - 10;

	return c;
}


/**
@brief	CONVERT CHAR INTO HEX
@return	HEX
  
This function converts HEX(0-F) to a character
*/
char C2D(
	u_char c	/**< is a character('0'-'F') to convert to HEX */
	)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return 10 + c -'a';
	if (c >= 'A' && c <= 'F')
		return 10 + c -'A';

	return (char)c;
}


/**
@brief	CONVERT STRING INTO INTEGER
@return	a integer number
*/
/*
u_int ATOI(
	char* str,	
	u_int base	
	)
{
        unsigned int num = 0;
        while (*str !=0)
                num = num * base + C2D(*str++);
	return num;
}
*/
unsigned int ATOI(char* str, u_int base)
{
        u_int num = 0;

        while (*str != 0) num = num * base + C2D(*str++);

	return num;
}	




/**
@brief	CONVERT STRING INTO HEX OR DECIMAL
@return	success - 1, fail - 0
*/
int ValidATOI(
	char* str, 	/**< is a pointer to string to be converted */
	int base, 	/**< is a base value (must be in the range 2 - 16) */
	unsigned int* ret		/**<  is a integer pointer to return */
	)
{
	int c;
	char* tstr = str;
	if(str == 0 || *str == '\0') return 0;
	while(*tstr != '\0')
	{
		c = C2D(*tstr);
		if( c >= 0 && c < base) tstr++;
		else    return 0;
	}
	
	*ret = ATOI(str,base);
	return 1;
}


 /**
@brief	Calculate the length of the initial substring of "s" which only contain letters in "accept"
@return	The string to search for
*/
size_t strspn(
	const char *s, 		/**< The string to be searched */
	const char *accept	/**< The string to search for */
	)
{
	const char *p;
	const char *a;
	size_t count = 0;

	for (p = s; *p != '\0'; ++p) {
		for (a = accept; *a != '\0'; ++a) {
			if (*p == *a)
				break;
		}
		if (*a == '\0')
			return count;
		++count;
	}

	return count;
}

/**
@brief	Find the first occurrence of a set of characters
*/
char * strpbrk(
	const char * cs,	/**< The string to be searched */
	const char * ct	/**< The characters to search for */
	)
{
	const char *sc1,*sc2;

	for( sc1 = cs; *sc1 != '\0'; ++sc1) {
		for( sc2 = ct; *sc2 != '\0'; ++sc2) {
			if (*sc1 == *sc2)
				return (char *) sc1;
		}
	}
	return NULL;
}



/**
@brief	Split a string into tokens

		WARNING: strtok is deprecated, use strsep instead.
*/ 
char * strtok(
	char * s,			/**< The string to be searched */
	const char * ct	/**< The characters to search for */
	)
{
	char *sbegin, *send;

	sbegin  = s ? s : ___strtok;
	if (!sbegin) {
		return NULL;
	}
	sbegin += strspn(sbegin,ct);
	if (*sbegin == '\0') {
		___strtok = NULL;
		return( NULL );
	}
	send = strpbrk( sbegin, ct);
	if (send && *send != '\0')
		*send++ = '\0';
	___strtok = send;
	return (sbegin);
}


/**
@brief	replace the specified character in a string with new character
*/ 
void replacetochar(
	char * str, 		/**< pointer to be replaced */
	char oldchar, 	/**< old character */
	char newchar	/**< new character */
	)
{
	int x;
	for (x = 0; str[x]; x++) 
		if (str[x] == oldchar) str[x] = newchar;	
}


u_short swaps(u_int i)
{
	u_short ret=0;
	ret = (i & 0xFF) << 8;
	ret |= ((i >> 8)& 0xFF);
	return ret;	
}
/*
u_long swapl(u_long l)
{
	u_long ret=0;
	ret = (l & 0xFF) << 24;
	ret |= ((l >> 8) & 0xFF) << 16;
	ret |= ((l >> 16) & 0xFF) << 8;
	ret |= ((l >> 24) & 0xFF);
	return ret;
}*/



#ifndef NO_USE_UTIL_FUNC
/**
@brief	This function converts a integer number to a string.
@return	a pointer to string
*/ 
char* ITOA(
	u_int value,	/**< is a integer value to be converted */
	char* str,	/**< is a pointer to string to be returned */
	u_int base	/**< is a base value (must be in the range 2 - 16) */
	)
{
	char c;
	char* tstr = str;
	char* ret = str;
	if(value == 0) *str++='0';
	while(value > 0)
	{
		*str++ =(char)D2C((char)(value%base));
		value /= base;
	}
	*str-- ='\0';
	while(tstr < str )
	{
		c = *tstr;
		*tstr++ = *str;	
		*str-- = c;
	}
	return ret;
}

#endif

