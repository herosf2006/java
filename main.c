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
	int 	i,j;
	char	res;
	DDRC 	= 0xFF;
	PORTC 	= 0x00;		

	res 	= mmc_init();
	PORTC++;
	if( res != 0 )
		goto finish;

	char *teststring="Welcome to this site\nUnsigned int read_adc";

	res		= mmc_writeblock(1, teststring);
	PORTC++;
	if( res! = 0 )
		goto Finish;	
		
	finish:
		PORTC |= ( res << 4 );

	while (1)
	{

	}
	return 0;	
}
