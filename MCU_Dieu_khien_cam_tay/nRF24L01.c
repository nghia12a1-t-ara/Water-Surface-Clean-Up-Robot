#include <avr/io.h>
#include <util/delay.h>
#include "nRF24L01.h"
#include "PORT.h"

unsigned char TxBuf[32]=
{
	0x01,0x02,0x03,0x4,0x05,0x06,0x07,0x08,
	0x09,0x10,0x11,0x12,0x13,0x14,0x15,0x16,
	0x17,0x18,0x19,0x20,0x21,0x22,0x23,0x24,
	0x25,0x26,0x27,0x28,0x29,0x30,0x31,0x32,
};

unsigned char const TX_ADDRESS[TX_ADR_WIDTH]= {0x34,0x43,0x10,0x10,0x01};
unsigned char const RX_ADDRESS[RX_ADR_WIDTH]= {0x34,0x43,0x10,0x10,0x01};

void nRF24L01_Init(void)
{
	_delay_us(100);
	CLEARBIT(PORTD, CE);	//CE=0;    // chip enable
	SETBIT(PORTB, CSN);		//CSN=1;   // Spi disable
	CLEARBIT(PORTB, SCK);	//SCK=0;   // Spi clock line init high
	SPI_Write_Buf(WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);    //
	SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH); //
	SPI_RW_Reg(WRITE_REG + EN_AA, 0x01);      // EN P0, 2-->P1
	SPI_RW_Reg(WRITE_REG + EN_RXADDR, 0x01);  // Enable data P0
	SPI_RW_Reg(WRITE_REG + RF_CH, 0);         // Chanel 0 RF = 2400 + RF_CH* (1or 2 M)
	SPI_RW_Reg(WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH); // Do rong data truyen 32 byte
	SPI_RW_Reg(WRITE_REG + RF_SETUP, 0x07);   		  // 1M, 0dbm
	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0e);   		  // Enable CRC, 2 byte CRC, Send
}

unsigned char SPI_RW(unsigned char Buff)
{
	unsigned char bit_ctr;
	for(bit_ctr = 0; bit_ctr < 8; bit_ctr++) // output 8-bit
	{
		//MOSI = (Buff & 0x80);       // output 'uchar', MSB to MOSI
		if (Buff & 0x80)
		{
			SETBIT(PORTB, MOSI);
		}
		else
		{
			CLEARBIT(PORTB, MISO);
		}
		_delay_us(5);
		Buff = (Buff << 1);         // shift next bit into MSB..
		SETBIT(PORTB, SCK);			// SCK = 1;  // Set SCK high..
		_delay_us(5);
		Buff |= MISO;       		// capture current MISO bit
		CLEARBIT(PORTB, SCK);		// SCK = 0;  // ..then set SCK low again
	}
	return(Buff);           		// return read uchar
}

unsigned char SPI_Read(unsigned char reg)
{
	unsigned char reg_val;

	CLEARBIT(PORTB, CSN);	// CSN = 0; // CSN low, initialize SPI communication...
	SPI_RW(reg);            // Select register to read from..
	reg_val = SPI_RW(0);    // ..then read registervalue
	SETBIT(PORTB, CSN);		// CSN = 1; // CSN high, terminate SPI communication

	return(reg_val);        // return register value
}

unsigned char SPI_RW_Reg(unsigned char reg, unsigned char value)
{
	unsigned char status;

	CLEARBIT(PORTB, CSN);		// CSN = 0; // CSN low, init SPI transaction
	status = SPI_RW(reg);		// select register
	SPI_RW(value);				// ..and write value to it..
	SETBIT(PORTB, CSN);			// CSN = 1; // CSN high again

	return(status);				// return nRF24L01 status uchar
}

unsigned char SPI_Read_Buf(unsigned char reg, unsigned char *pBuf, unsigned char uchars)
{
	unsigned char status, uchar_ctr;

	CLEARBIT(PORTB, CSN);		// CSN = 0; // Set CSN low, init SPI tranaction
	status = SPI_RW(reg);       // Select register to write to and read status uchar

	for(uchar_ctr = 0; uchar_ctr < uchars; uchar_ctr++)
	pBuf[uchar_ctr] = SPI_RW(0);    //

	SETBIT(PORTB, CSN);			// CSN = 1;
	return(status);             // return nRF24L01 status uchar
}

unsigned char SPI_Write_Buf(unsigned char reg, unsigned char *pBuf, unsigned uchars)
{
	unsigned char status,uchar_ctr;
	CLEARBIT(PORTB, CSN);		// CSN = 0; //SPI
	
	status = SPI_RW(reg);
	for(uchar_ctr = 0; uchar_ctr < uchars; uchar_ctr++)		//
	SPI_RW(*pBuf++);
	
	SETBIT(PORTB, CSN);			// CSN = 1; //SPI
	return(status);				//
}

void SetRX_Mode(void)
{
	CLEARBIT(PORTB, CE);		// CE=0;
	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0f);   	// Enable CRC, 2 byte CRC, Recive
	SETBIT(PORTB, CE);			// CE=1;
	_delay_us(130);				//
}

void SetTX_Mode(void)
{
	CLEARBIT(PORTB, CE);		// CE=0;
	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0e);   	// Enable CRC, 2 byte CRC, Send
	SETBIT(PORTB, CE);			// CE=1;
	_delay_us(130);    //
}

unsigned char nRF24L01_RxPacket(unsigned char* rx_buf)
{
	unsigned char revale = 0;
	unsigned char sta;
	sta = SPI_Read(STATUS);	// Read Status
	//if(RX_DR)				// Data in RX FIFO
	if((sta & 0x40) != 0)		// Data in RX FIFO
	{
		CLEARBIT(PORTB, CE);		// CE=0;
		SPI_Read_Buf(RD_RX_PLOAD, rx_buf, TX_PLOAD_WIDTH);	// read receive payload from RX_FIFO buffer
		revale = 1;
	}
	SPI_RW_Reg(WRITE_REG+STATUS, sta);
	return revale;
}

void nRF24L01_TxPacket(unsigned char * tx_buf)
{
	CLEARBIT(PORTB, CE);		// CE=0;
	SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH); // Send Address
	SPI_Write_Buf(WR_TX_PLOAD, tx_buf, TX_PLOAD_WIDTH); 			 //send data
	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0e);   		 // Send Out
	SETBIT(PORTB, CE);			// CE=1;
}

int RF_available(uint8_t* pipe_num)
{
	if (!(SPI_Read(FIFO_STATUS) & (1 << RX_EMPTY)))
	{
		if (pipe_num){
			uint8_t status = get_status();
			*pipe_num = ( status >> RX_P_NO ) & 0x07;
		}
		return 1;
	}
	return 0;
}

uint8_t get_status(void)
{
	return SPI_RW(NOP);
}
