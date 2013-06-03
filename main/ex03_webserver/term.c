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
 
#include <util/delay.h>
#include "OWIPolled.h"
#include "OWIHighLevelFunctions.h"
#include "OWIBitFunctions.h"
#include "common_files\OWIcrc.h"
#include "term.h"
//#include "protocol.h"


extern float myatof(char *s);
extern float powi(int x, int y);

void my_wait_1ms(int cnt)
{
	for (; cnt; cnt--) _delay_us(1000);
}

extern OWI_device allDevices[MAX_DEVICES];

void readsingle()
{
  unsigned int tmp = 0;
  unsigned char temperature;
  unsigned char scratchpad[9];
  
  
    OWI_DetectPresence(OWI_PIN_2);
    OWI_SkipRom(OWI_PIN_2);
    OWI_SendByte(DS18B20_CONVERT_T ,OWI_PIN_2);

    while (!OWI_ReadBit(OWI_PIN_2));

    OWI_DetectPresence(OWI_PIN_2);
    OWI_SkipRom(OWI_PIN_2);
    OWI_SendByte(DS18B20_READ_SCRATCHPAD, OWI_PIN_2);
    scratchpad[0] = OWI_ReceiveByte(OWI_PIN_2);
    scratchpad[1] = OWI_ReceiveByte(OWI_PIN_2);
      
    if ((scratchpad[1]&128) == 0){
      //LCD_WriteData('+');
    }
    else{
      //LCD_WriteData('-');
      tmp = ((unsigned int)scratchpad[1]<<8)|scratchpad[0];
      tmp = ~tmp + 1;
      scratchpad[0] = tmp;
      scratchpad[1] = tmp>>8;  
    }
      
    /*auaiaei cia?aiea oaeia cia?. oaiia?aoo?u*/ 
    temperature = (scratchpad[0]>>4)|((scratchpad[1]&7)<<4);
	//sprintf(printbuf,"t=%0d\r\n",temperature);	Print(printbuf); 
      
    /*auaiaei a?iaio? ?anou cia?. oaiia?aoo?u*/
    temperature = (scratchpad[0]&15);
    temperature = (temperature<<1) + (temperature<<3);
    temperature = (temperature>>4);
  
}


/*****************************************************************************
*   Function name :   DS18B20_ReadTemperature
*   Returns :       eiau - READ_CRC_ERROR, anee n?eoaiiua aaiiua ia i?ioee i?iaa?eo
*                          READ_SUCCESSFUL, anee aaiiua i?ioee i?iaa?eo    
*   Parameters :    bus - auaia iee?ieiio?ieea?a, eioi?ue auiieiyao ?ieu 1WIRE oeiu
*                   *id - eiy ianneaa ec 8-ie yeaiaioia, a eioi?ii o?aieony
*                         aa?an aao?eea DS18B20
*                   *temperature - oeacaoaeu ia oanoiaaoaoe ?ac?yaio? ia?aiaiio?
*                                a eioi?ie aoaao nio?aiaii n?eoaiiiai ci. oaiia?aoo?u
*   Purpose :      Aa?anoao aao?ee DS18B20, aaao eiiaiao ia i?aia?aciaaiea oaiia?aoo?u
*                  ?aao, n?eouaaao aai iaiyou - scratchpad, i?iaa?yao CRC,
*                  nio?aiyao cia?aiea oaiia?aoo?u a ia?aiaiiie, aica?auaao eia ioeaee             
*****************************************************************************/
unsigned char DS18B20_ReadTemperature(unsigned char bus, unsigned char * id, unsigned int* temperature)
{
    unsigned char scratchpad[9];
    unsigned char i;
  
    /*iiaaai neaiae na?ina
    eiiaiao aey aa?anaoee anao ono?ienoa ia oeia
    iiaaai eiiaiao - caioe i?aia?aciaaiey */
    OWI_DetectPresence(bus);
    OWI_MatchRom(id, bus);
	//OWI_SkipRom(BUS);
    OWI_SendByte(DS18B20_CONVERT_T ,bus);

    /*?aai, eiaaa aao?ee caaa?oeo i?aia?aciaaiea*/ 
    while (!OWI_ReadBit(bus));

    /*iiaaai neaiae na?ina
    eiiaiao aey aa?anaoee anao ono?ienoa ia oeia
    eiiaiao - ?oaiea aioo?aiiae iaiyoe
    caoai n?eouaaai aioo?aii?? iaiyou aao?eea a iannea
    */
    OWI_DetectPresence(bus);
    OWI_MatchRom(id, bus);
    OWI_SendByte(DS18B20_READ_SCRATCHPAD, bus);
    for (i = 0; i<=8; i++){
      scratchpad[i] = OWI_ReceiveByte(bus);
    }
    
    if(OWI_CheckScratchPadCRC(scratchpad) != OWI_CRC_OK){
      return READ_CRC_ERROR;
    }
    
    *temperature = (unsigned int)scratchpad[0];
    *temperature |= ((unsigned int)scratchpad[1] << 8);
    
    return READ_SUCCESSFUL;
}


void Start_18b20_Convert(unsigned char bus)
{
    OWI_DetectPresence(bus);    
	OWI_SkipRom(OWI_PIN_2);
    OWI_SendByte(DS18B20_CONVERT_T ,bus);
	my_wait_1ms(751);     
    //while (!OWI_ReadBit(bus));
}

/*
unsigned char DS18B20_ReadTemperature_Fast(unsigned char bus, unsigned char * id, unsigned int* temperature)
{
    unsigned char scratchpad[9];
    unsigned char i;
  
    OWI_DetectPresence(bus);
    OWI_MatchRom(id, bus);
    OWI_SendByte(DS18B20_READ_SCRATCHPAD, bus);
    for (i = 0; i<=8; i++){
      scratchpad[i] = OWI_ReceiveByte(bus);
    }
    
    if(OWI_CheckScratchPadCRC(scratchpad) != OWI_CRC_OK){
      return READ_CRC_ERROR;
    }
    
    *temperature = (unsigned int)scratchpad[0];
    *temperature |= ((unsigned int)scratchpad[1] << 8);
    
    return READ_SUCCESSFUL;
}
*/

unsigned char DS18B20_ReadTemperature_Fast_Float(unsigned char bus, unsigned char * id, float* temperature)
{
    unsigned char scratchpad[9];
    unsigned char i;
	unsigned int  xx;
	float temp=0.0;
  
    OWI_DetectPresence(bus);
    OWI_MatchRom(id, bus);
    OWI_SendByte(DS18B20_READ_SCRATCHPAD, bus);
	my_wait_1ms(10);
    for (i = 0; i<=8; i++){
      scratchpad[i] = OWI_ReceiveByte(bus);
    }
    
    if(OWI_CheckScratchPadCRC(scratchpad) != OWI_CRC_OK){
      return READ_CRC_ERROR;
    }
    

  //scratchpad[1]=0x01;
  //scratchpad[0]=0x91;

  
  xx=(unsigned int)(((unsigned int)scratchpad[1] << 8) | scratchpad[0]);
  
  if ((scratchpad[1] & 0xF0) == 0xF0)
  {
    temp = (4096-(xx & 0x0FFF))/16.00;
    temp =-temp;
  }
  else temp = xx/16.00;
  
  
  
  *temperature=temp;
    
  return (READ_SUCCESSFUL);
}





unsigned char Read_scratchpad(unsigned char bus, unsigned char num)
{
    unsigned char i;
  
    /*iiaaai neaiae na?ina
    eiiaiao aey aa?anaoee anao ono?ienoa ia oeia
    iiaaai eiiaiao - caioe i?aia?aciaaiey */
	
    OWI_DetectPresence(bus);
    OWI_MatchRom(allDevices[num].id, bus);    
  
    OWI_SendByte(DS18B20_READ_SCRATCHPAD, bus);
    for (i = 0; i<=8; i++){
      allDevices[num].scratchpad[i] = OWI_ReceiveByte(bus);
    }
    
    if(OWI_CheckScratchPadCRC(allDevices[num].scratchpad) != OWI_CRC_OK){
      return READ_CRC_ERROR;
    }
	
    
    return READ_SUCCESSFUL;
}


unsigned char Write_scratchpad(unsigned char bus, unsigned char num)
{
//    unsigned char i;
  
    /*iiaaai neaiae na?ina
    eiiaiao aey aa?anaoee anao ono?ienoa ia oeia
    iiaaai eiiaiao - caioe i?aia?aciaaiey */
	
    OWI_DetectPresence(bus);
    OWI_MatchRom(allDevices[num].id, bus);    
  
    OWI_SendByte(DS18B20_WRITE_SCRATCHPAD, bus);
    
    OWI_SendByte(0x0e, bus);
	OWI_SendByte(0x08, bus);
	OWI_SendByte(0x7f, bus);	
    
    return READ_SUCCESSFUL;
}





/*****************************************************************************
*   Function name :  DS18B20_PrintTemperature 
*   Returns :         iao       
*   Parameters :     temperature - oaiia?aoo?a aao?eea DS18B20     
*   Purpose :        Auaiaeo cia?aiea oaiia?aoo?u aao?eea DS18B20
*                    ia LCD. Aa?an ciaeiianoa io?ii aunoaaeyou ca?aiaa.
*****************************************************************************/
void DS18B20_PrintTemperature(unsigned int temperature, char * out)
{
  unsigned char tmp = 0;
  unsigned int a=0;
  char t1[7];
  char t2[3];
  /*auaiaei ciae oaiia?aoo?u
  *anee iia io?eoaoaeuiay 
  *aaeaai i?aia?aciaaiea*/  
  if ((temperature & 0x8000) == 0){
   // LCD_WriteData('+');
  }
  else{
    //LCD_WriteData('-');
    temperature = ~temperature + 1;
  }
     
	
  //auaiaei cia?aiea oaeia cia?. oaiia?aoo?u      
  tmp = (unsigned char)(temperature>>4);
  
  sprintf(t1,"%d.",tmp);
        
  //auaiaei a?iaio? ?anou cia?. oaiia?aoo?u
  tmp = (unsigned char)(temperature&15);
  
  a=tmp*100;
  a = (a>>4);
 
  sprintf(t2,"%02d",a);
  
  strcat(t1,t2);
  strcpy(out,t1);
  //Print(printbuf);
}


void DS18B20_to_float(unsigned int temperature, float * out)
{
  unsigned char tmp = 0;
  unsigned int a=0;
  char t1[7];
  char t2[3];

  if ((temperature & 0x8000) == 0){
  }
  else{
    temperature = ~temperature + 1;
  }
     
	
  tmp = (unsigned char)(temperature>>4);
  
  sprintf(t1,"%d.",tmp);
        
  tmp = (unsigned char)(temperature&15);
  
  a=tmp*100;
  a = (a>>4);
 
  sprintf(t2,"%02d",a);
  
  strcat(t1,t2);
  
  *out=myatof(t1);
}