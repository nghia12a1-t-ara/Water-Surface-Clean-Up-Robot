/*
 * MCU_Bang_tai.c
 *
 * Created: 20/11/2020 05:26:09 AM
 * Author : Nghia
 */ 
#define F_CPU 8000000ul
#include <avr/io.h>
#include <util/delay.h>
#include "PORT.h"
#include "Nen_rac.h"

extern long duration;
extern int distance;
int kc = 0;		// Bien dem de chay ctrinh con nen rac & bao rac day
int y = 0;		// Dai dien cho tin hieu dong/mo bang tai
int dem = 0;			// Bien dem so lan do dong dien
int kq[10];				// Luu gia tri trung binh 10 lan do dau tien
int a = 0;				// Gia tri trung binh 10 lan do dau tien
int period = 1000;		// Chu ky dong co step moi buoc

	// Cac nguyen mau ham
int trung_binh(int*, int);
void ADC_Init(void);
int ADC_Read(char);
void IO_Init(void);
void UART_Init(uint16_t);
void UART_putc(unsigned char);
char UART_getc(void);
extern void Nen_rac(void);

int main(void)
{
    // Khoi tao IO, UART
	IO_Init();
	UART_Init(9600);
	
    while (1) 
    {
		char Rei = UART_getc();
		if(Rei == '1')
		{
			y = 1;
		}
		if(Rei == '2')
		{
			y = 0;
		}
	}
	kc = 0;
	if(y == 1)
	{
		kc++;
		// Chay dong co step _ half step
		for(int j = 0; j<12; j++)
		{
			PORTB &= ~(1 << Coil4);
			PORTB &= ~(1 << Coil3);
			PORTB &= ~(1 << Coil2);
			PORTD |= (1 << Coil1);
			_delay_ms(period);
			PORTB &= ~(1 << Coil4);
			PORTB &= ~(1 << Coil3);
			PORTD |= (1 << Coil2);
			PORTD |= (1 << Coil1);
			_delay_ms(period);
			PORTB &= ~(1 << Coil4);
			PORTB &= ~(1 << Coil3);
			PORTD |= (1 << Coil2);
			PORTD &= ~(1 << Coil1);
			_delay_ms(period);
			PORTB &= ~(1 << Coil4);
			PORTD |= (1 << Coil3);
			PORTD |= (1 << Coil2);
			PORTD &= ~(1 << Coil1);
			_delay_ms(period);			
			PORTB &= ~(1 << Coil4);
			PORTD |= (1 << Coil3);
			PORTB &= ~(1 << Coil2);
			PORTD &= ~(1 << Coil1);
			_delay_ms(period);
			PORTD |= (1 << Coil4);
			PORTD |= (1 << Coil3);
			PORTB &= ~(1 << Coil2);
			PORTD &= ~(1 << Coil1);
			_delay_ms(period);			
			PORTD |= (1 << Coil4);
			PORTB &= ~(1 << Coil3);
			PORTB &= ~(1 << Coil2);
			PORTD &= ~(1 << Coil1);
			_delay_ms(period);			
			PORTD |= (1 << Coil4);
			PORTB &= ~(1 << Coil3);
			PORTB &= ~(1 << Coil2);
			PORTD |= (1 << Coil1);
			_delay_ms(period);
		}
		PORTB &= ~(1 << Coil4);
		PORTB &= ~(1 << Coil3);
		PORTB &= ~(1 << Coil2);
		PORTD |= (1 << Coil1);
		_delay_ms(period - 50);

		// Doc tin hieu dien ap
		int x = ADC_Read(0);
		if (dem < 10)
		{
			kq[dem] = x;
		}
		if (dem == 10)
		{
			a = trung_binh(kq, 10);
		}
		if (dem > 10)
		{
			if (x > (a+4))		// Su co ket bang tai
			{
				dem = 0;
				y = 0;
				_delay_ms(10);
				// Dung bang tai
				PORTD &= ~(1 << Coil4);
				PORTD &= ~(1 << Coil3);
				PORTD &= ~(1 << Coil2);
				PORTD &= ~(1 << Coil1);
			}
		}
		if (dem > 500)
		{
			dem = 0;		// reset bien dem khi no qua lon > 500
		}
		dem++;		// tang bien dem 

		// Kiem tra rac day moi 100 vong lap
		if (kc == 100);
		{
			Nen_rac();
		}
    }
}

void IO_Init()
{
	PORTD |= (1 << Coil1) | (1 << trigPin);
	PORTB &= ~((1 << Coil2) | (1 << Coil3) | (1 << Coil4));
	PORTD &= ~(1 << echoPin);
}

int trung_binh(int* a, int n)
{
	int tong = 0;
	for (int i = 0; i < n; i++)
	{
		tong += a[i];
	}
	return (tong/n);
}

void UART_Init(uint16_t baudrate)
{
	// set baudrate
	UBRR0L = (uint8_t)(baudrate & 0xFF);
	UBRR0H = (uint8_t)(baudrate >> 8);
	
	// enable the trans_receier
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
}

void UART_putc(unsigned char data)
{
	// wait for transmit buffer to be empty
	while(!(UCSR0A & (1 >> UDRE0)));

	// load data into transmit register
	UDR0 = data;
}

char UART_getc(void)
{
	// wait for data
	while(!(UCSR0A & (1 << RXC0)));
	// return data
	return UDR0;
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

extern void Nen_rac()
{
	kc = 0;
	// Chay dong co nen rac
	// delay 1s
	// Do khoang cach
	PORTD &= ~(1 << trigPin);
	_delay_ms(2);
	// kich xung 10ms cho chan Trig
	PORTD |= (1 << trigPin);
	_delay_ms(10);
	PORTD &= ~(1 << trigPin);
	// Doc gia tri chan echoPin, doc thoi gian Echo o muc cao
	// Su dung Input Capture
	TCCR1A = 0;
	TIFR1 = (1 << ICF1);				// Xoa co Input capture
	TCCR1B = 0x41;						// Phat hien canh len cua Echo
	while ((TIFR1 & (1 << ICF1)) == 0);	// Doi den khi phat hien canh len
	long t = ICR1;
	TIFR1 = (1<<ICF1);					// Xoa co Input capture
	TCCR1B = 0x01;						// Phat hien canh xuong cua Echo khi
	while ((TIFR1 & (1 << ICF1)) == 0);	// Doi den khi phat hien canh xuong

	duration = ICR1 - t;  				// Gia tri xung cao tra ve
	// Tinh toan khoang cach
	distance = duration * 0.034 / 2;
	if(distance < 15)
	{
		UART_putc('2');
	}
	else
	{
		// dua nen rac ve vi tri ban dau
		// doi nen rac ve vi tri ban dau bang cong tac hanh trinh
	}
}


