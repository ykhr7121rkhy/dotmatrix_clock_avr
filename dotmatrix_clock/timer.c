
/********timer.c***********/
#include<avr/io.h>


void init_timer(void);
void delay_us(unsigned int us);
void delay_ms(unsigned int ms);


void init_timer(void){
	TCCR0A=0x02;
	TCCR0B=0x01;
	TCCR1A=0x00;
	TCCR1B=0x0c; //f/256
	TCCR2A=0x02;
	TCCR2B=0x06;//256div
	ASSR=0x20;
	OCR0A=20-1;	//20MHz
	OCR2A=63;
//	OCR0A=8-1; //8MHz
//	OCR1A=15624;
//	OCR2A=100-1;
//	TIMSK0=0x02;
//	TIMSK1=0x02;
	TIMSK2=0x02;
	
		
}

void delay_us(unsigned int us){
	int i;
	for(i=0;i<us;i++){
		while(!(TIFR0&0x02));
		TIFR0|=0x02; //write 1 to clear OCF0A
	}		
}

void delay_ms(unsigned int ms){
	int i,j;
	for(i=0;i<ms;i++)
		for(j=0;j<1000;j++){
			while(!(TIFR0&0x02));
			TIFR0|=0x02; //write 1 to clear OCF0A
		}
}
