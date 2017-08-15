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

//---------tao 1 stream ten mystdout thuoc loai FILE,connect voi ham uart_char_tx

static FILE uartstd=FDEV_SETUP_STREAM(uart_char_tx,NULL,_FDEV_SETUP_WRITE);
//---------------------

unsigned char buff[512],tchr;		//bo dem phat va nhan
const char hocavr[]="\nK韓h gui quy anh, chi.\n Trong thoi gian toi, khoi uo?ng c鬾g viec cua Mr. Bao qu?nhiu\n.V?vay lop Tieng Anh se tam dung v?cho den khi co thong bao moi.";

void main(void)
{
	// Declare your local variables here
	uart_init(38400);
	
	// khoi dong UART, BAUD =38400
	DDRC=0xFF;PORTC=0x00;
		
	int i,j;
	char Res;
	Res=mmc_init();             //khoi dong MMC va SPI
	PORTC++;                //xog buoc 1,khoi dong card thanh cong
	if(Res!=0)goto Finish;
	
	//------------Phan ghi du lieu----------------
	//Ghi sector 1
	char *teststring="\nWelcome to this site \n";
	Res=mmc_writeblock(1,teststring);           //ghi vao sector 1
	PORTC++;
	if(Res!=0)goto Finish;
	
	// ghi sector 2
	teststring="Phan noi dung nay dc luu o sector 2 cua card";
	Res=mmc_writeblock(2,teststring);      //ghi vao sector 2
	PORTC++;
	if(Res!=0)goto Finish;
	
	//-----ghi sector 3 va 4
	tchr=pgm_read_byte(&hocavr[0]);
	for(i=0;tchr>0;i++)
	{
		if(i<512)
		{
			tchr=pgm_read_byte(&hocavr[i]);
			buff[i]=tchr;
		}
		else break;
	}
	Res=mmc_writeblock(3,buff);				//ghi vao sector 3
	PORTC++;
	if(Res!=0)goto Finish;
	
	for(j=0;tchr>0;j++)
	{
		if(j<512)
		{
			tchr=pgm_read_byte(&hocavr[i+j]);
			buff[j]=tchr;
		}
		else break;
	}
	Res=mmc_writeblock(4,buff);			//ghi vao sector 4
	PORTC++;					//xog buoc 5, ghi vao sector 4 thanh cong
	if(Res!=0)goto Finish;
	
	//----******Phan doc du lieu ****----------
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
