#include "OWIHighLevelFunctions.h"
#include "OWIPolled.h"

#define MAX_DEVICES       10
#define MAX_SENSORS       15 
#define MAX_RELAY   	  4 // 2
#define SIZE_SENSOR_STRUCT  sizeof(sensor_structure)


//#define BUS   		      OWI_PIN_5 // нужно OWI_PIN_4  21.12.12
// Порты датчиков
// PORT D pin 1 
// PORT D pin 2

#define DS18B20_FAMILY_ID                0x28 
#define DS18B20_CONVERT_T                0x44
#define DS18B20_READ_SCRATCHPAD          0xbe
#define DS18B20_WRITE_SCRATCHPAD         0x4e
#define DS18B20_COPY_SCRATCHPAD          0x48
#define DS18B20_RECALL_E                 0xb8
#define DS18B20_READ_POWER_SUPPLY        0xb4

#define READ_SUCCESSFUL   0x00
#define READ_CRC_ERROR    0x01

unsigned char DS18B20_ReadTemperature(unsigned char bus, unsigned char * id, unsigned int* temperature);
unsigned char DS18B20_ReadTemperature_Fast_Float(unsigned char bus, unsigned char * id, float* temperature);
void DS18B20_to_float(unsigned int temperature, float * out);
void DS18B20_PrintTemperature(unsigned int temperature, char * out);
unsigned char Read_scratchpad(unsigned char bus, unsigned char num);
unsigned char Write_scratchpad(unsigned char bus, unsigned char num);
void readsingle();
void my_wait_1ms(int cnt);

OWI_device allDevices[MAX_DEVICES];

unsigned char count_sensors; // Сколько всего датчиков подключено в систему.
sensor_structure all_sensors[MAX_SENSORS];
io_structure all_relay[MAX_RELAY]; 