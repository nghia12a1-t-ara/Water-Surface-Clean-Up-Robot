/*
 * BDK_Cam_tay.c
 *
 * Created: 31/10/2020 4:23:05 AM
 * Author : Nghia
 */ 

#define F_CPU 8000000ul
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include "nRF24L01.h"	// Cac ham lquan SPI va nRF
#include "PORT.h"		// Dinh nghia cac cong

char xyData[32] = "";
int xAxis, yAxis, xRei;

char Buffer[32] = "";
char xBuffer[4] = "";
char yBuffer[4] = "";

char BTON[] = "2500   2500";
char BTOFF[] = "3500   3500";

// Nguyen mau ham, cac ham ADC, IO
void ADC_Init(void);
int ADC_Read(char);
void IO_Init(void);

int main(void)
{
	// Khoi tao IO, ADC, nRF
	IO_Init();
	ADC_Init();
	nRF24L01_Init();
	
	while (1)		// Super loop
	{
		SetTX_Mode();
		// Doc gia tri 2 joystick
		xAxis = ADC_Read(0);
		yAxis = ADC_Read(1);
		
		// Xu ly chuoi
		itoa(xAxis, xBuffer, 4);
		itoa(yAxis, yBuffer, 4);
		
		strcpy(Buffer, xBuffer); 
		strcpy(Buffer, yBuffer);
		
		// Truyen du lieu
		nRF24L01_TxPacket(Buffer);
		
		// Ktra nut bam dieu khien bang tai
		if (!(PORTD & (1 << ON_Bangtai)))
		{
			_delay_ms(20);			// Chong doi phim
			if (!(PORTD & (1 << ON_Bangtai)))
			{
				PORTD |= (1 << LED_bangtai);
				nRF24L01_TxPacket(BTON);
				while(!(PORTD & (1 << ON_Bangtai)));
			}
		}
		if (!(PORTD & (1 << OFF_Bangtai)))
		{
			_delay_ms(20);			// Chong doi phim
			if (!(PORTD & (1 << OFF_Bangtai)))
			{
				PORTD &= ~(1 << LED_bangtai);
				nRF24L01_TxPacket(BTOFF);
				while(!(PORTD & (1 << OFF_Bangtai)));
			}
		}
		
		// Nhan tin hieu canh bao tu robot
		SetRX_Mode();
		if (RF_available(TxBuf))		// Neu nhan duoc tin hieu canh bao
		{
			char text[32];
			text[0] = nRF24L01_RxPacket(TxBuf);
			xRei = atoi(&text[0]);
			if (xRei == 123)
			{
				PORTD |= (1 << LED_bangtai);
			}
			if (xRei == 456)
			{
				PORTD |= (1 << LED_racday);
			}
		}
		
	}
}

void IO_Init()
{
	// D2,3,4,5 = output, D6,7 = input
	DDRD |= (1 << LED_bangtai) | (1 << LED_power) |(1 << LED_racday) | (1 << LED_suco);
	DDRD &= ~((1 << ON_Bangtai) | (1 << OFF_Bangtai));
	// Cac khoi tao ban dau
	PORTD &= ~(1 << LED_suco);
	PORTD &= ~(1 << LED_racday);
	PORTD &= ~(1 << LED_bangtai);
	PORTD |= (1 << LED_power);
}

void ADC_Init()
{
	DDRC = 0x0;				// Port C_ADC = input
	ADCSRA = 0x87;			// Enable ADC, fr/128
	ADMUX = 0x40;			// Vref: Avcc
	
}

int ADC_Read(char channel)
{
	int Ain, AinLow;
	
	ADMUX = ADMUX|(channel & 0x0f);
	ADCSRA |= (1 << ADSC);		// Bat dau chuyen doi
	while((ADCSRA & (1 << ADIF)) == 0);	// Doi EOC_Ket thuc chuyen doi
	
	_delay_us(10);
	AinLow = (int)ADCL;			// Doc byte thap
	Ain = (int)ADCH*256;		// Doc byte cao
	
	Ain = Ain + AinLow;
	return(Ain);				// Gia tri so tra ve
}



