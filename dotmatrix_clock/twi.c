#include<avr/io.h>
#include<stdlib.h>
void init_timer(void);
void delay_us(unsigned int);
void delay_ms(unsigned int);

void lcd_write(char data,char rs);
void lcd_display(char code);
void lcd_control(char code);
void lcd_clear(void);
void lcd_init(void);
void lcd_print(char *str);
void twi_init(void);

char twi_sendaddr_t(unsigned char s_addr);
char twi_sendaddr_r(unsigned char s_addr);
char twi_senddata(unsigned char data);
char twi_recvdata(unsigned char*);
void twi_stop(void);
void error(char);


void twi_init(void){
	TWCR=0x00;
	TWBR=0x10;	//100khz
	TWSR=0x00;
	TWAR=0x00;
	TWCR=0x04;
}

char twi_sendaddr_t(unsigned char s_addr){
	TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN); 
	while(!(TWCR&(1<<TWINT)));

	if((TWSR&0xf8)!=0x08);
	if((TWSR&0xf8)!=0x10);//repeated start

	TWDR=(s_addr<<1)|0; //send slave address and write bit
	TWCR=(1<<TWINT)|(1<<TWEN);
	while(!(TWCR&(1<<TWINT)));

	switch(TWSR&0xf8){
		case 0x18:	return 0;
			break;
		case 0x20:	return -1;
			break;
		default:	return TWSR&0xf8;

	}
	
}

char twi_sendaddr_r(unsigned char s_addr){
	TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN); 
	while(!(TWCR&(1<<TWINT)));

	if((TWSR&0xf8)!=0x08);
	if((TWSR&0xf8)!=0x10);//repeated start


	TWDR=(s_addr<<1)|1; //send slave address and read bit
	TWCR=(1<<TWINT)|(1<<TWEN);
	while(!(TWCR&(1<<TWINT)));

	switch(TWSR&0xf8){
		case 0x40:	return 0;
			break;
		case 0x48:	return -1;
			break;
		default:	return TWSR&0xf8;

	}
	return 1;
}

char twi_senddata(unsigned char data){
	TWDR=data;					//send data
	TWCR=(1<<TWINT)|(1<<TWEN);
	while(!(TWCR&(1<<TWINT)));

	switch(TWSR&0xf8){
		case 0x28:	return 0;	
				break;
		case 0x30:	return -1;
				break;				
		default: return TWSR&0xf8;
	}
	return 1;
}

char twi_recvdata(unsigned char *data){			
	TWCR=(1<<TWINT)|(1<<TWEN);
	while(!(TWCR&(1<<TWINT)));
	*data=TWDR;//recieve data;
	switch(TWSR&0xf8){
		case 0x50:	return 0;	
				break;
		case 0x58:	return -1;
				break;
		default: return TWSR&0xf8;
	}
	
}

void twi_stop(void){
	TWCR=(1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
}

void error(char n){
	char str[2];
	lcd_control(0x80);
	lcd_print("error");
	lcd_print(" 0x");
	itoa(n,str,16);
	lcd_print(str);
}
