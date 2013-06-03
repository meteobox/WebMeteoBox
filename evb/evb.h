/**
 @file		evb.h
 @brief 		
 */

#ifndef _EVB_H
#define _EVB_H



#define IICHIP_RESET_INIT()					(DDRE |= 0x40)
#define IICHIP_RESET()						(PORTE |= 0x40)

#define LED_AVR_PORT_VAL					PORTB
#define LED_AVR_PORT_DIR					DDRB

#define LED_PIN_0							4
#define LED_PIN_1							5
#define LED_PIN_2							6
#define LED_PIN_3							7

#define LED_ON								1
#define LED_OFF								0	

#define led_off_all()						led_off(0);led_off(1);led_off(2);led_off(3)


#define SW_AVR_PORT_VAL						PORTE
#define SW_AVR_PORT_DIR						DDRE

#define SW_VALUE							PINE


#define SW_PIN_0							5
#define SW_PIN_1							6


#define SW_MASK								0x60//(1<<SW_PIN_0)|(1<<SW_PIN_1)


//-----------------------------------------------------------------------------
// AVR ADC Defination
#define ADC_SR_REG							ADCSR

#define ADC_PRESCALE_DIV2                   1
#define ADC_PRESCALE_DIV4                   2
#define ADC_PRESCALE_DIV8                   3
#define ADC_PRESCALE_DIV16                  4
#define ADC_PRESCALE_DIV32                  5
#define ADC_PRESCALE_DIV64                  6
#define ADC_PRESCALE_DIV128                 7

#define ADC_ENABLE                          0x80
#define ADC_START_CONVERSION                0x40
#define ADC_FREE_RUNNING                    0x20
#define ADC_COMPLETE                        0x10
#define ADC_INT_ENABLE                      0x08

#define ADC_REF_VREF                        0
#define ADC_REF_AVCC                        0x40
#define ADC_REF_INTERNAL                    0xC0

#define adc_WaitForCoversion()				while(!(ADC_SR_REG & ADC_COMPLETE))
#define adc_GetData() 						((ADCH<<8) | ADCL)
#define adc_SetChannel(Ch)					(ADMUX = (Ch)&7)
#define adc_Run()							(ADC_SR_REG |= ADC_START_CONVERSION)


extern void mcu_init(void);
extern void evb_init(void);

void led_init(void);
void led_toggle(u_char led);
void led_off(u_char led);
void led_on(u_char led);
u_char led_state(u_char led);

void evb_set_lcd_text(u_char row, u_char* lcd);
u_char* evb_get_lcd_text(u_char row);

void sw_init(void);
u_char sw_state(unsigned char num);

void AdcInit(void);
unsigned int AdcRead(unsigned char Channel);
void evb_soft_reset(void);
#endif
