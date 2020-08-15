#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>

#define BAUDRATE 9600
#define BAUD_PRESCALLER (((F_CPU / (BAUDRATE * 16UL))) - 1)


uint16_t adc_value;
char buffer[5];                //Output of the itoa function


void adc_init(void);
uint16_t read_adc(uint8_t channel);

void USART_init(void);
void USART_send(unsigned char data);
void USART_putstring(char* StringPtr);

void adc_init(void)
{
    ADCSRA |= ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));    //16Mhz/128 = 125Khz
    ADMUX |= (1<<REFS0);                //Voltage reference from Avcc (5v)
    ADCSRA |= (1<<ADEN);                //Turn on ADC
}

uint16_t read_adc(uint8_t channel)
{
    ADMUX |= channel;                //ADC channel to be read
    ADCSRA |= (1<<ADSC);             //Starts  conversion
    while(ADCSRA & (1<<ADSC));       //Wait until the conversion is done
    return ADCW;                     //Returns the ADC value
}


int main(void)
{
    adc_init();
    USART_init();

    USART_putstring("Oscilloscope\n");

    while(1)
    {
            adc_value = read_adc(0b00000000);        //Read one ADC channel

            char print_buf1[40];
            sprintf(print_buf1, "Analog value=%d\n", adc_value);
            USART_putstring(print_buf1);

            _delay_ms(500);
    }
    return 0;
}



void USART_init(void)
{
    UBRR0H = (uint8_t)(BAUD_PRESCALLER>>8);
    UBRR0L = (uint8_t)(BAUD_PRESCALLER);
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);
    UCSR0C = (3<<UCSZ00);
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
