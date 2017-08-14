/*
 * MMC.h
 *
 * Created: 8/10/2017 12:44:41 PM
 *  Author: nguyenhoanganhkhoa
 */ 

#include <avr/io.h>
#include <stdio.h>

#define SPI_PORT    PORTB
#define SPI_DDR     DDRB
#define SCK_PIN     7
#define MISO_PIN    6
#define MOSI_PIN    5
#define SS_PIN      4

#define Block_len   512		// dinh nghia chiu dai 1 sector

#ifndef cbi
#define cbi(port,bit)   (port)&=~(1<<(bit))
#endif
#ifndef sbi
#define sbi(port,bit)   (port)|=(1<<(bit))
#endif

// Dinh nghia cac lenh co ban giao tiep MMC
#define CMD0        0x00             // GO_IDLE_STATE , reset va dua vao trang thai nghi
#define CMD1        0x01            // SEND_OP_COND, yeu cau Card tra loi ve trang thai cua CARD
#define CMD16       0x10            //SET_BLOCK_LEN, Dat do dai(tinh theo byte)cho 1 khoi du lieu
#define CMD17       0x11           // READ_SINGLE_BLOCK, doc 1 block du lieu cua Card
#define CMD24       0x18           //WRITE_BLOCK, ghi 1 mang du lieu vao 1 block cua Card

char mmc_status=0;                 // cac trang thai tra ve khi thao tac voi card
// cai dat cho SPI o che do Master
void SPI_MesterInit(void);
void SPI_tByte(unsigned char data);
unsigned char SPI_rByte(void);
//end of SPI----------------

// cac lenh giao tiep MMC
unsigned char mmc_rResponse(unsigned char Response);            //yeu cau Response tu Card
void mmc_tCommand(unsigned char Cmd, unsigned long int arg);    //goi 1 lenh den card
char mmc_init(void);        // khoi dong card
char mmc_writeblock(unsigned long int LBAddress,unsigned char *buff);       //ghi 1 sector ( 1 khoi du lieu)
char mmc_readblock(unsigned long int LBAddress,unsigned char *buff);        //doc 1 sector(1 khoi du lieu)



#ifndef MMC_H_
#define MMC_H_

void SPI_MasterInit(void)
{
	SPI_DDR|=(1<<SCK_PIN)|(1<<MOSI_PIN)|(1<<SS_PIN);
	SPI_PORT|=(1<<MISO_PIN);                // dien tro keo len cho chan MISO
	SPCR=(1<<SPIE)|(1<<SPE)|(1<<MSTR);      //chu y la nen set CPOL=0,CPHA=0
}

void SPI_tByte(unsigned char data)             // transmite 1 byte
{
	SPDR=data;
	//while (bit_is_clear(SPSR,SPIF));
	while (!(SPSR & (1<<SPIF)));          // cho den khi bit SPIF duoc set, qua trinh ket thuc
}

unsigned char SPI_rByte(void)                 // recieve 1 byte
{
	unsigned char data;
	SPDR=0xFF;
	// dummy value
	while (!(SPSR & (1<<SPIF)));        // cho den khi bit SPIF duoc set, truyen dummy ket thuc
	data=SPDR;
	return data;
}
//------ nhan va so sanh Response tu MMC
unsigned char mmc_rResponse(unsigned char Response)
{
	int Timeout=0x0fff;
	unsigned char Res;
	while((Timeout--)>0)
	{
		Res=SPI_rByte();
		if(Res==Response)break;     //da dung yeu cau,thoat khoi while
	}
	if (Timeout==0)return 1;        //tra ve 1 neu timeout
	else return 0;
}

void mmc_tCommand(unsigned char Cmd,unsigned long int arg)
{
	cbi(SPI_PORT,SS_PIN);       // Kich hoat duong SS cua SPI,MMC duoc chon
	SPI_tByte(0xFF);            //dummy, 1 lenh luon bat dau 0 nen gui FF thi MMC ko dap ung
	SPI_tByte(Cmd|0x40);        // 0x40=01000000 la ma bat buoc khi goi lenh
	SPI_tByte((unsigned char)(arg>>24));
	SPI_tByte((unsigned char)(arg>>16));
	SPI_tByte((unsigned char)(arg>>8));
	SPI_tByte((unsigned char)arg);
	SPI_tByte(0x95);        //CRC
	SPI_tByte(0xFF);        // ko quan cam return
}

char mmc_init(void)
{
	unsigned int i;
	SPI_MasterInit();
	sbi(SPI_PORT,SS_PIN);       // disable SPI MMC
	for(i=0;i<10;i++)SPI_tByte(0xFF);       //MMC se vao SPI mode, 80 clock
	cbi(SPI_PORT,SS_PIN);       //cho phep MMC hoat dong
	mmc_tCommand(CMD0,0);       //lenh CMD0,reset MMC
	if(mmc_rResponse(0x01)==1)
	{
		mmc_status=1;       //timeout khi reset
		sbi(SPI_PORT,SS_PIN);        //disable SPI MMC
		return mmc_status;
		
	}
	//goi lenh CMD1 cho den khi nhan duoc Response =0 hoac timeout
	i=0xffff;       //max timeout
	do
	{
		mmc_tCommand(CMD1,0);       // lenh CMD1
		i--;
	}
	while ((SPI_rByte()!=0)&&i>0);
	if(i == 1)       // co loi khi goi CMD1
	{
		mmc_status=2;   //loi CMD1
		sbi(SPI_PORT,SS_PIN);       //disable SPI MMC
		return mmc_status;
	}
	mmc_tCommand(CMD16,Block_len);      //lenh CMD16, set do dai sector, Block_len = 512
	if(mmc_rResponse(0x00)==1)
	{
		mmc_status=3;       // timeout khi set len
		sbi(SPI_PORT,SS_PIN);           // disable SPI MMC
		return mmc_status;
	}
	sbi(SPI_PORT,SS_PIN);   //disable SPI MMC
	return 0;       //no error
}
//---------Ham ghi 1 khoi du lieu len MMC---------------
char mmc_writeblock(unsigned long int LBAddress, unsigned char *buff)
{
	unsigned char status;
	unsigned long int tempA;
	unsigned int i;
	tempA=512*LBAddress;
	
	cbi(SPI_PORT,SS_PIN);       //cho phep MMC hoat dong
	mmc_tCommand(CMD24,tempA);      //goi lenh cho phep ghi 1 khoi du lieu
	if(mmc_rResponse(0x00)==1)          //co loi kiem tra Response
	{
		mmc_status=4;               //timeout khi goi lenh write block
		sbi(SPI_PORT,SS_PIN);       //disable SPI MMC
		return mmc_status;
	}
	SPI_tByte(0xFE);        //goi dau hieu(token) bao cho mmc biet sap co data den, dau hieu = 0xFE
	for(i=0;i<Block_len;i++)    SPI_tByte(buff[i]);
	SPI_tByte(0xFF);            //goi 2 byte check sum
	SPI_tByte(0xFF);           //goi 2 byte check sum
	
	//------doc trang thai response, phai la 0x05
	
	status=SPI_rByte();
	if((status&0x0F)!=0x05)     //co loi kiem tra response
	{
		mmc_status=5;   // loi khi goi datablock
		sbi(SPI_PORT,SS_PIN);       //disable SPI MMC
		return mmc_status;
	}
	//----- cho den khi MMC ko busy
	i=0xffff;
	while(!SPI_rByte()&&(--i)){;}       //cho den khi mmc khong busy
	if(i==0)
	{
		mmc_status=6;       //loi timeout khi cho busy
		sbi(SPI_PORT,SS_PIN);
		return mmc_status;
	}
	sbi(SPI_PORT,SS_PIN);       //disable SPI MMC
	return 0;
}
//--------------Ham doc 1 khoi du lieu tu card ------------------
char mmc_readblock(unsigned long int LBAddress,unsigned char *buff)
{
	unsigned long int tempA;
	unsigned int i;
	tempA=512*LBAddress;
	
	cbi(SPI_PORT,SS_PIN);       //cho phep MMC hoat dong
	mmc_tCommand(CMD17,tempA);    //goi lenh cho phep doc 1 khoi du lieu
	if(mmc_rResponse(0x00)==1)
	
	{
		mmc_status=7;
		sbi(SPI_PORT,SS_PIN);
		return mmc_status;
	}
	// kiem tra dau hieu (token) read
	if(mmc_rResponse(0xFE)==1)      // co loi,kiem tra Response
	{
		mmc_status=8;       //timeout khi goi len read block
		sbi(SPI_PORT,SS_PIN);        //disable SPI MMC
		return mmc_status;
	}
	
	for(i=0;i<Block_len;i++)buff[i]=SPI_rByte();     //doc khoi du lieu
	//goi 2 byte check sum
	SPI_tByte(0xFF);
	SPI_tByte(0xFF);
	
	sbi(SPI_PORT,SS_PIN);       //disable SPI MMC
	return 0;
}

#endif /* MMC_H_ */
