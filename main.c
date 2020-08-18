#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdint.h>
#include <avr/pgmspace.h>

#define BAUDRATE 115200UL
//#define BAUD_PRESCALLER (((F_CPU / (BAUDRATE * 16UL))) - 1)
#define BAUD_PRESCALLER 16


uint16_t adc_value0, adc_value1;
uint8_t ser[5];
/*
const volatile uint8_t sine_lookup[] PROGMEM=
{
    128,131,134,137,140,143,146,149,
    152,155,158,162,165,167,170,173,
    176,179,182,185,188,190,193,196,
    198,201,203,206,208,211,213,215,
    218,220,222,224,226,228,230,232,
    234,235,237,238,240,241,243,244,
    245,246,248,249,250,250,251,252,
    253,253,254,254,254,255,255,255,
    255,255,255,255,254,254,254,253,
    253,252,251,250,250,249,248,246,
    245,244,243,241,240,238,237,235,
    234,232,230,228,226,224,222,220,
    218,215,213,211,208,206,203,201,
    198,196,193,190,188,185,182,179,
    176,173,170,167,165,162,158,155,
    152,149,146,143,140,137,134,131,
    128,124,121,118,115,112,109,106,
    103,100,97,93,90,88,85,82,
    79,76,73,70,67,65,62,59,
    57,54,52,49,47,44,42,40,
    37,35,33,31,29,27,25,23,
    21,20,18,17,15,14,12,11,
    10,9,7,6,5,5,4,3,
    2,2,1,1,1,0,0,0,
    0,0,0,0,1,1,1,2,
    2,3,4,5,5,6,7,9,
    10,11,12,14,15,17,18,20,
    21,23,25,27,29,31,33,35,
    37,40,42,44,47,49,52,54,
    57,59,62,65,67,70,73,76,
    79,82,85,88,90,93,97,100,
    103,106,109,112,115,118,121,124
};  */


void adc_init(void);
uint16_t read_adc(uint8_t channel);

void USART_init(void);
void USART_send(unsigned char data);
void USART_putstring(char* StringPtr);

void timer0pwm_init();

void adc_init(void)
{
    ADCSRA |= ((1<<ADPS2)|(1<<ADPS1));    //16Mhz/64
    ADMUX |= (1<<REFS0);                //Voltage reference from Avcc (5v)
    ADCSRA |= (1<<ADEN);                //Turn on ADC
}

uint16_t read_adc(uint8_t channel)
{
    ADMUX &= 0xF0;                   //Clear the older channel that was read
    ADMUX |= channel;                //ADC channel to be read
    ADCSRA |= (1<<ADSC);             //Starts  conversion
    while(ADCSRA & (1<<ADSC));       //Wait until the conversion is done
    return ADCW;                     //Returns the ADC value
}

void timer0pwm_init()
{
    TCCR0A |= (1<<COM0A1) | (1<<WGM01) | (1<<WGM00);
    TCCR0B |= (1<<CS02);  //pre scaling 256
  ///  OCR0A = 64;  //25% duty cycle  255*(25/100)
    OCR0A = 127;  //50% duty cycle
    sei();
}



int main(void)
{
   // uint8_t i=0;

    DDRD |= (1<<PD6);   //PWM output OC0A

    adc_init();
    USART_init();
    timer0pwm_init();
    USART_putstring("Oscilloscope\n");

    while(1)
    {
            adc_value0 = read_adc(0);        //Read one ADC channel
            adc_value1 = read_adc(1);        //Read one ADC channel

         ///  char print_buf1[40];
         ///  sprintf(print_buf1, "Analog value=%d\n", adc_value);
         ///  USART_putstring(print_buf1);

            ser[0]=0xF7;
            ser[1]=((uint16_t)adc_value0) >> 8;     // high byte
			ser[2]=((uint16_t)adc_value0) & 0x00FF; // low byte
			ser[3]=((uint16_t)adc_value1) >> 8;     // high byte
			ser[4]=((uint16_t)adc_value1) & 0x00FF; // low byte

			USART_send(ser[0]);
			USART_send(ser[1]);
			USART_send(ser[2]);
			USART_send(ser[3]);
			USART_send(ser[4]);

		//	OCR0A = (pgm_read_byte(&sine_lookup[i]));
        //    i++;
    }
    return 0;
}


void USART_init(void)
{
    UBRR0H = (uint8_t)(BAUD_PRESCALLER>>8);
    UBRR0L = (uint8_t)(BAUD_PRESCALLER);

  //  UCSR0A &= ~(1<<U2X0);   //U2X0=0
    UCSR0A |= (1<<U2X0);      //U2X0=1

    UCSR0B |= (1<<RXEN0)|(1<<TXEN0);
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);
}

void USART_send( unsigned char data)
{
    while(!(UCSR0A & (1<<UDRE0)));
    UDR0 = data;
}

void USART_putstring(char* StringPtr)
{
    while(*StringPtr != 0x00){
    USART_send(*StringPtr);
    StringPtr++;}
}
