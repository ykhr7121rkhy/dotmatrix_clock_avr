/*
 * dotmatrix_clock.c
 *
 * Created: 2020/04/17 23:37:31
 * Author : ykhr7
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#define ADDRESS_HOUR 0x74
#define ADDRESS_MINUTE 0x70
#define BIT_IS_SET(data,mask) (((data)&(mask)) ? 0x01 : 0)

#define CHAR_1 0
#define CHAR_2 5
#define CHAR_3 26
#define CHAR_4 16
#define CHAR_5 21



void init_timer(void);
void delay_us(unsigned int us);
void delay_ms(unsigned int ms);
void twi_init(void);
char twi_sendaddr_t(unsigned char s_addr);
char twi_sendaddr_r(unsigned char s_addr);
char twi_senddata(unsigned char data);
char twi_recvdata(unsigned char*);
void twi_stop(void);

void HT16K33_command_write(unsigned char slaveaddr,unsigned char data);
void HT16K33_dispdata_write(unsigned char slaveaddr,unsigned char regaddr, unsigned char data);
void display_init(void);
void display_write(unsigned char *disp_ram);
void character_write(unsigned char num,unsigned char offset);

unsigned char display_num[12][5] = {{0x3e,0x51,0x49,0x45,0x3e},{0x00,0x42,0x7f,0x40,0x00},
									{0x42,0x61,0x51,0x49,0x46},{0x22,0x41,0x49,0x49,0x36},
									{0x18,0x14,0x12,0x7f,0x10},{0x27,0x45,0x45,0x45,0x39},
									{0x3c,0x4a,0x49,0x49,0x30},{0x01,0x71,0x09,0x05,0x03},
									{0x36,0x49,0x49,0x49,0x36},{0x06,0x49,0x49,0x29,0x1e},
									{0x00,0x00,0x22,0x00,0x00},{0x00,0x00,0x00,0x00,0x00}};

unsigned char disp_ram[32];
uint16_t bcd_minsec=0x5900;
uint16_t bcd_hour = 0x0023;
uint8_t dot_flag=0,down_button,up_button;

int main(void)
{
	unsigned char i;
	DDRD=0x00;
	PORTD=0xc0;//Pull up resistor enable PD7,PD6
	init_timer();
	display_init();
	sei();
    /* Replace with your application code */
    while (1) 
    {
		if(!(PIND&0x40)){
			down_button = 1;
			up_button = 0;
			if(OCR2A > 2) OCR2A -= 1;
		}
		else if(!(PIND&0x80)){
			up_button = 1;
			down_button = 0;
			if(OCR2A > 2) OCR2A -= 1;
		}
		else if(!(PIND&0x80)&&!(PIND&0x40)){
			up_button = 1;
			down_button = 1;
			if(OCR2A > 2) OCR2A -= 1;
		}
		else{
			up_button = 0;
			down_button = 0;
			OCR2A=63;
		}
		character_write((unsigned char)(bcd_hour>>4)&0xf,CHAR_1);
		character_write((unsigned char)(bcd_hour)&0xf,CHAR_2);
		character_write(dot_flag?10:11,CHAR_3);
		character_write((unsigned char)(bcd_minsec>>12)&0xf,CHAR_4);
		character_write((unsigned char)(bcd_minsec>>8)&0xf,CHAR_5);
		display_write(disp_ram);
    }
}
void character_write(unsigned char num,unsigned char offset){
	unsigned char i;
	for(i=offset;i<(offset+5);i++){
		disp_ram[i]=display_num[num][i-offset];
	}	
}
void HT16K33_command_write(unsigned char slaveaddr,unsigned char data){
	twi_sendaddr_t(slaveaddr);
	twi_senddata(data);
	twi_stop();
}
void HT16K33_dispdata_write(unsigned char slaveaddr,unsigned char regaddr, unsigned char data){
	twi_sendaddr_t(slaveaddr);
	twi_senddata(regaddr);
	twi_senddata(data);
	twi_stop();
}

void display_init(void){
	twi_init();
	HT16K33_command_write(ADDRESS_HOUR,0x21); //Internal clock enable
	HT16K33_command_write(ADDRESS_HOUR,0xa0); //row/int output pin set
	HT16K33_command_write(ADDRESS_HOUR,0xef); //Dimming set
	HT16K33_command_write(ADDRESS_HOUR,0x80); //Blinking set
	HT16K33_command_write(ADDRESS_MINUTE,0x21); //Internal clock enable
	HT16K33_command_write(ADDRESS_MINUTE,0xa0); //row/int output pin set
	HT16K33_command_write(ADDRESS_MINUTE,0xef); //Dimming set
	HT16K33_command_write(ADDRESS_MINUTE,0x80); //Blinking set
}

void display_write(unsigned char *disp_ram){
	unsigned char addr,index,k,data;

	for(addr=0,k=0;addr<0xf,k<8;addr+=2,k++){
		data=0;
		for(index=0;index<8;index++) {
			data|=(BIT_IS_SET(disp_ram[index],1<<k))<<index;
		}
		HT16K33_dispdata_write(ADDRESS_HOUR,addr,data);
	}
	
	for(addr=1,k=0;addr<=0xf,k<8;addr+=2,k++){
		data=0;
		for(index=8;index<16;index++) {
			data|=(BIT_IS_SET(disp_ram[index],1<<k))<<(index-8);
		}
		HT16K33_dispdata_write(ADDRESS_HOUR,addr,data);
	}
	
	for(addr=0,k=0;addr<0xf,k<8;addr+=2,k++){
		data=0;
		for(index=16;index<24;index++) {
			data|=(BIT_IS_SET(disp_ram[index],1<<k))<<(index-16);
		}
		HT16K33_dispdata_write(ADDRESS_MINUTE,addr,data);
	}
	
	for(addr=1,k=0;addr<=0xf,k<8;addr+=2,k++){
		data=0;
		for(index=24;index<32;index++) {
			data|=(BIT_IS_SET(disp_ram[index],1<<k))<<(index-24);
		}
		HT16K33_dispdata_write(ADDRESS_MINUTE,addr,data);
	}
	
	HT16K33_command_write(ADDRESS_HOUR,0x81);
	HT16K33_command_write(ADDRESS_MINUTE,0x81);
}

ISR(TIMER2_COMPA_vect){
	if(dot_flag){
		dot_flag=0;
		if((bcd_minsec&0x0f)>=9){
			if((bcd_minsec&0xf0)>=0x50){
				if((bcd_minsec&0xf00)>=0x900){
					if((bcd_minsec&0xf000)>=0x5000){
						if((bcd_hour&0xf)>=0x9){
							bcd_minsec&=~0x0ffff;
							bcd_hour&=~0x0f;
							bcd_hour+=0x10;
						}
						else if((bcd_hour&0xff)==0x23){
							bcd_hour=0x0000;
							bcd_minsec=0x0000;
						}
						else{
							bcd_minsec&=~0xffff;
							bcd_hour+=0x1;
						}
					}
					else{
						bcd_minsec&=~0x0fff;
						bcd_minsec+=0x1000;
					}
				}
				else{
					bcd_minsec&=~0x0ff;
					bcd_minsec+=0x100;
				}
			}
			else{
				bcd_minsec&=~0x0f;
				bcd_minsec+=0x10;
			}
		}
		else{
			bcd_minsec++;
		}
	}
	else{
		dot_flag=1;
	}
	
	if(up_button){
		dot_flag=0;
		bcd_minsec &=~0x00ff;
		if((bcd_minsec&0xf00)>=0x900){
			if((bcd_minsec&0xf000)>=0x5000){
				if((bcd_hour&0xf)>=0x9){
					bcd_minsec&=~0x0ffff;
					bcd_hour&=~0x0f;
					bcd_hour+=0x10;
				}
				else if((bcd_hour&0xff)==0x23){
					bcd_hour=0x0000;
					bcd_minsec=0x0000;
				}
				else{
					bcd_minsec&=~0xffff;
					bcd_hour+=0x1;
				}
			}
			else{
				bcd_minsec&=~0x0fff;
				bcd_minsec+=0x1000;
			}
		}
		else{
			bcd_minsec&=~0x0ff;
			bcd_minsec+=0x100;
		}
	}
	else if(down_button){
		dot_flag=0;
		bcd_minsec &=~0x00ff;
		if((bcd_minsec&0xf00)==0x000){
			if((bcd_minsec&0xf000)==0x0000){
				if((bcd_hour&0xff)==0x00){
					bcd_hour=0x0023;
					bcd_minsec=0x5900;		//hour 10
				}
				else if((bcd_hour&0x0f)==0x00){
					bcd_minsec|=0x5900;
					bcd_hour|=0x09;
					bcd_hour-=0x10;
				}
				else{
					bcd_hour-=0x1;//hour 1
					bcd_minsec|=0x5900;
				}
			}
			else{
				bcd_minsec|=0x0900;
				bcd_minsec-=0x1000;	//min 10
			}
		}
		else{
			bcd_minsec-=0x100;//min 1
		}
	}
	else if(up_button && down_button){
		dot_flag=0;
		bcd_minsec &=~0x00ff;
	}
}