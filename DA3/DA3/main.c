
#define F_CPU 16000000UL //16MHz
#define BAUD 9600 //Baud Rate
#define MYUBRR F_CPU/16/BAUD-1 //calculate Baud

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

void read_adc(void);  //Read ADC
void adc_init(void);  //initialize ADC
void USART_init( unsigned int ubrr ); //initialize USART
void USART_tx_string(char *data); //Print String USART
volatile unsigned int adc_temp;
char outs[20]; //array

int main(void) {
	
	adc_init(); // Initialize the ADC (Analog / Digital Converter)
	USART_init(MYUBRR); // Initialize the USART (RS232 interface)
	USART_tx_string("Connected!\r\n"); // shows theres a connection with USART
	_delay_ms(125); // wait a bit

	sei(); //interrupt
	while(1)
	{
	}
}

void adc_init(void)
{
	
	ADMUX = (0<<REFS1)| // Reference Selection Bits

	(1<<REFS0)| // AVcc - external cap at AREF
	(0<<ADLAR)| // ADC Left Adjust Result
	(0<<MUX2)| // ANalog Channel Selection Bits
	(1<<MUX1)| // ADC2 (PC2 PIN25)
	(0<<MUX0);

	ADCSRA = (1<<ADEN)| // ADC ENable

	(0<<ADSC)| // ADC Start Conversion
	(0<<ADATE)| // ADC Auto Trigger Enable
	(0<<ADIF)| // ADC Interrupt Flag
	(0<<ADIE)| // ADC Interrupt Enable
	(1<<ADPS2)| // ADC Prescaler Select Bits
	(0<<ADPS1)|
	(1<<ADPS0);

	// Timer/Counter1 Interrupt Mask Register

	TIMSK1 |= (1<<TOIE1); // enable overflow interrupt
	TCCR1B |= (1<<CS12)|(1<<CS10); // native clock
	TCNT1 = 49911; //((16MHz/1024)*1)-1 = 15624

}


void read_adc(void) {
	unsigned char i =4;
	adc_temp = 0; //initialize
	while (i--) {
		ADCSRA |= (1<<ADSC);
		while(ADCSRA & (1<<ADSC));
		adc_temp+= ADC;
		_delay_ms(50);
	}
	adc_temp = adc_temp / 4; // Average a few samples

}

/* INIT USART (RS-232) */
void USART_init( unsigned int ubrr ) {
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	UCSR0B = (1 << TXEN0); // Enable receiver, transmitter & RX interrupt
	UCSR0C = (3 << UCSZ00); //asynchronous 8 N 1
}

void USART_tx_string( char *data ) {
	while ((*data != '\0')) {
		while (!(UCSR0A & (1 <<UDRE0)));
		UDR0 = *data;
		data++;
	}
}

ISR(TIMER1_OVF_vect) //timer overflow interrupt to delay for 1 second
{
	read_adc();//read ADC
	snprintf(outs,sizeof(outs),"%3d F \r\n", adc_temp);// print it
	USART_tx_string(outs);
	TCNT1 = 49911; //reset tcnt value for delay
}