#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include "MMC.h"

#define F_CPU 8000000UL

//-----UART--------------------
void uart_init(unsigned int BAUD)
{	
	unsigned long int temp_BAUD;
	temp_BAUD=(F_CPU)/16;
	temp_BAUD/=BAUD;
	temp_BAUD--;
	UBRRH=(unsigned char)(temp_BAUD>>8);
	UBRRL=(unsigned char)(temp_BAUD);
	//set khung truyen va kich hoat bo nhan du lieu
	UCSRA=0x00;
	UCSRC=(1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);
	UCSRB=(1<<TXEN);
}
// chuong trinh con phat ky tu UART
void uart_char_tx(unsigned char chr)
{
	if(chr=='\n')
	uart_char_tx('\r');
	while (!(UCSRA & (1<<UDRE))){};			//cho den khi bit UDRE=1
	UDR=chr;
}

static FILE uartstd=FDEV_SETUP_STREAM(uart_char_tx,NULL,_FDEV_SETUP_WRITE);

unsigned char buff[512],tchr;		//bo dem phat va nhan
const char hocavr[]="Hello\n	World!\n";

void main(void)
{
	uart_init(38400);	
	DDRC=0xFF;PORTC=0x00;		
	int i,j;
	char Res;
	Res=mmc_init();             //khoi dong MMC va SPI
	PORTC++;                //xog buoc 1,khoi dong card thanh cong
	if(Res!=0)goto Finish;

	
	//doc sector 1
	Res=mmc_readblock(1,buff);		//doc sector 1
	PORTC++;						//xog buoc 6,doc sector 1 thanh cong
	fprintf(&uartstd,buff);
	
	//doc sector 2
	Res=mmc_readblock(2,buff);
	PORTC++;
	fprintf(&uartstd,buff);		//in toan bo chuoi doc ve len Terminal
	
	//doc sector 3
	Res=mmc_readblock(3,buff);
	PORTC++;
	fprintf(&uartstd,"\n\n");
	fprintf(&uartstd,buff);

	//doc sector 4
	Res=mmc_readblock(4,buff);
	PORTC++;
	fprintf(&uartstd,buff);	
		
	Finish:
	      PORTC|=(Res<<4);      // hien thi trang thai MMC ra LED 2

	while (1)
	{
		// Place your code here

	}
	return 0;
	
}
