#define ROM_FNAMELEN    20  /* Maximum filename size */
#define FILE_HEAD_SIZE			(ROM_FNAMELEN +11)

typedef struct          /* Filename block structure */
{
    unsigned long len;               /* Length of file in bytes */
    unsigned long start;             /* Start address of file data in ROM */
    unsigned short check;             /* TCP checksum of file */
    unsigned char flags;            /* Embedded Gateway Interface (EGI) flags */
    char name[ROM_FNAMELEN];		/* Lower-case filename with extension */
} ROM_FNAME;

#define HTTP_FAIL       "HTTP/ 200 OK\r\n\r\nNo Web pages!\r\n"
#define MAXFILES    	100 		// Limit on ROM file count (to stop runaway)

typedef union               		// ROM file directory entry format
{
    ROM_FNAME f;                	// Union of filename..
    unsigned char b[sizeof(ROM_FNAME)];  	// ..with byte values for i2c transfer
} ROM_DIR;

#define read_byte(VAL)				read_from_flash(m_CurRdAddress++)
#define PAGE_PER_BYTE				264



void InitRomFile(void);
void InitDataFlash(void);
void write_to_flash(unsigned char flash_data);
unsigned char read_from_flash(unsigned long Address);
unsigned char search_file_rom(unsigned char *FileName, unsigned long *Address, unsigned long *Size);
void read_from_flashbuf(unsigned long Address, unsigned char* Buffer, unsigned int Size);

