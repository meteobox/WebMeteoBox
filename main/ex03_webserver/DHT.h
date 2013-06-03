#include "OWIHighLevelFunctions.h"
#include "OWIPolled.h"


#define DHTPIN    1	//OWI_PIN_4   0x10


extern unsigned char bGlobalErr;
extern unsigned char dht_dat[5];

extern signed   int dht_term;
extern unsigned int dht_hum;

void InitDHT(unsigned char pins);
void ReadDHT(unsigned char pins);