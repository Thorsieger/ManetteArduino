#include <avr/io.h>		// for the input/output register
#include <util/delay.h>

// For the serial port

#define CPU_FREQ        16000000L       // Assume a CPU frequency of 16Mhz
#define DELAY 10

void init_serial(int speed)
{
/* Set baud rate */
UBRR0 = CPU_FREQ/(((unsigned long int)speed)<<4)-1;

/* Enable transmitter & receiver */
UCSR0B = (1<<TXEN0 | 1<<RXEN0);

/* Set 8 bits character and 1 stop bit */
UCSR0C = (1<<UCSZ01 | 1<<UCSZ00);

/* Set off UART baud doubler */
UCSR0A &= ~(1 << U2X0);
}

void send_serial(unsigned char c)
{
loop_until_bit_is_set(UCSR0A, UDRE0);
UDR0 = c;
}

unsigned char get_serial(void) {
loop_until_bit_is_set(UCSR0A, RXC0);
return UDR0;
}

// For the AD converter

void ad_init(unsigned char channel)   
{   
ADCSRA|=(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);   
ADMUX|=(1<<REFS0)|(1<<ADLAR);
ADMUX=(ADMUX&0xf0)|channel;   
ADCSRA|=(1<<ADEN);
}   

unsigned int ad_sample(void){
ADCSRA|=(1<<ADSC);
while(bit_is_set(ADCSRA, ADSC));
return ((ADCH>>3)|0x20);
}

// For the I/O 

void output_init(void){
DDRB = 0x3F; // PIN 8 to PIN 13 as output
}

void output_set(unsigned char value){
if(value=='A') PORTB &= 0xfe; //Eteindre led
else if(value=='B') PORTB &= 0xfd;
else if(value=='C') PORTB &= 0xfb;
else if(value=='D') PORTB &= 0xf7;
else if(value=='E') PORTB &= 0xef;
else if(value=='F') PORTB &= 0xdf;
else if(value=='a') PORTB |= 0x01;//Allumer led
else if(value=='b') PORTB |= 0x02;
else if(value=='c') PORTB |= 0x04;
else if(value=='d') PORTB |= 0x08;
else if(value=='e') PORTB |= 0x10;
else if(value=='f') PORTB |= 0x20;

}

void input_init(void){
DDRD &= 0b10000011;  // PIN 2,3,4,5,6 as input   1000 0011
PORTD |= 0b01111100; // Pull-up activated on all PIN
}

unsigned char input_get(){
return (((PIND&0x7C)>>2)|0x20);//lecture etat des boutons et mise ne forme des données
}

// Dummy main

int main(void){

//init
init_serial(9600);
output_init();
input_init();

//Valeurs les boutons et des Joystick
int boutons=0,joyA=0,joyB=0;

//loop
for(;;){
	
	//Récupération de la valeur des boutons
	unsigned char tmpB = input_get();

	
	//Récupération de la valeur du joystick
	ad_init(0);
	_delay_ms(DELAY);
	unsigned int tmpJA = ad_sample();

	ad_init(1);
	_delay_ms(DELAY);
	unsigned int tmpJB = ad_sample();
	
	//Envoi des données si modification
	if(tmpJA != joyA || tmpJB !=joyB || tmpB != boutons){
		boutons = tmpB;
		joyA=tmpJA;
		joyB=tmpJB;

		send_serial(boutons);
		send_serial(joyA);
		send_serial(joyB);
	}

	//lecture port serie
	if(UCSR0A & (1<<RXC0)){
	unsigned char test = get_serial();
	if((test>='a' && test<='f') || (test>='A' && test<='F'))output_set(test);
	}
	_delay_ms(DELAY);
}
return 0;
}