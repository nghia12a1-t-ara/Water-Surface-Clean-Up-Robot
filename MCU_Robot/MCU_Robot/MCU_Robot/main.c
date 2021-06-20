/*
 * GccApplication1.c
 *
 * Created: 05/11/2020 09:05:03 AM
 * Author : Nghia
 */ 
#define F_CPU 8000000ul
#include <avr/io.h>
#include <util/delay.h>
#include "nRF24L01.h"
#include "PORT.h"

char xyData[32] = "";
int xAxis, yAxis, BTai;
int motorSpeedA = 0;
int motorSpeedB = 0;
const char Ketbangtai[32] = "123";
const char Racdayroi[32] = "456";
int y = 2;

// Nguyen mau ham
void IO_Init(void);
void PWM_Init(void);
void UART_Init(uint16_t);
void UART_putc(unsigned char);
char UART_getc(void);
void PWM_Trig(int, int);
int map(int, int, int, int, int);

int main(void)
{
    // Khoi tao IO, nRF, UART
	IO_Init();
	PWM_Init();
	UART_Init(9600);
	nRF24L01_Init();
	
	xAxis = yAxis = 5000;
	int y = 0;
	
    while (1) 
    {
		// Nhan du lieu dieu khien tu bo dk cam tay
		SetRX_Mode();
		if (RF_available(TxBuf))
		{
			char text[32];
			text[0] = nRF24L01_RxPacket(TxBuf);
			xAxis = atoi(&text[0]);
			yAxis = atoi(&text[5]);
			_delay_ms(10);
		}
		
		// Kiem tra du lieu dk bang tai
		if ((xAxis > 2000) && (xAxis < 3000))		// Gui tin hieu chay bang tai
		{
			UART_putc('1');
			xAxis = 5000;
		}
		if ((xAxis > 3000) && (xAxis < 4000))		// Gui tin hieu tat bang tai
		{
			UART_putc('2');
			xAxis = 5000;
		}
		
		// Kiem ta du lieu dieu khien dong co
				/* Du lieu truc Y */
		if ((yAxis < 460) && (yAxis >= 0))
		{
			SETBIT(PORTD, in1);
			CLEARBIT(PORTD, in2);
			SETBIT(PORTD, in3);
			CLEARBIT(PORTD, in4);
			motorSpeedA = map(yAxis, 460, 0, 0, 255);
			motorSpeedB = map(yAxis, 460, 0, 0, 255);
		}
		else if ((yAxis > 550) && (yAxis < 1500))
		{
			CLEARBIT(PORTD, in1);
			SETBIT(PORTD, in2);
			CLEARBIT(PORTD, in3);
			SETBIT(PORTD, in4);
			motorSpeedA = map(yAxis, 550, 1023, 0, 255);
			motorSpeedB = map(yAxis, 550, 1023, 0, 255);
		}
		else
		{
			motorSpeedA = 0;
			motorSpeedB = 0;
		}
				/* Du lieu truc Y */
		if ((xAxis < 460) && (xAxis >= 0))
		{
			int xMapped = map(xAxis, 460, 0, 0, 255);
			
			motorSpeedA = motorSpeedA + xMapped;
			motorSpeedB = motorSpeedB - xMapped;
			
			if (motorSpeedA < 0)
			{
				motorSpeedA = 0;
			}
			if (motorSpeedB > 255)
			{
				motorSpeedB = 255;
			}
		}
		if ((xAxis > 550) && (xAxis < 1500))
		{
			int xMapped = map(xAxis, 550, 1023, 0, 255);
			
			motorSpeedA = motorSpeedA - xMapped;
			motorSpeedB = motorSpeedB + xMapped;
			
			if (motorSpeedA > 255)
			{
				motorSpeedA = 255;
			}
			if (motorSpeedB < 0)
			{
				motorSpeedB = 0;
			}
		}
		if (motorSpeedA < 70)
		{
			motorSpeedA = 0;
		}
		if (motorSpeedB < 70)
		{
			motorSpeedB = 0;
		}	
		PWM_Trig(enA, motorSpeedA); 
		PWM_Trig(enB, motorSpeedB);		// Xuat PWM chay 2 dong co
		
		// Nhan tin hieu canh bao su co tu MCU Bang tai (UART) & Gui ve BDK Cam tay
		char Rei = UART_getc();
		if(Rei == '1') { y = 1;}
		if(Rei == '2') { y = 2;}
		SetRX_Mode();
		if (y == 1)			// Gui bao su co ket bang tai
		{
			nRF24L01_TxPacket(Ketbangtai);
		}
		if (y == 2)			// Gui bao su co rac day
		{
			nRF24L01_RxPacket(Racdayroi);		
		}
		y = 0;
    }
}

void IO_Init()
{
	// cac chan L298 deu la Output
	DDRD |= (1 << enA) | (1 << enB) | (1 << in1) | (1 << in2) | (1 << in3) | (1 << in4);
	PORTD &= ~((1 << in1) | (1 << in2) | (1 << in3) | (1 << in4));
}

void PWM_Init()
{
	TCCR0A |=(1 << WGM00)|(1 << WGM01);
	TCCR0A |= (1 << COM0A1)|(1 << COM0B1);
	TCCR0B |= (1 << CS02);
	// khoi tao duty = 0, f = 0;
	TCNT0 = 0;
	OCR0A = 0;
	OCR0B = 0;
}

void PWM_Trig(int pin_PWM, int speed)
{
	int giatrinap;
	TCNT0 = 20000; 
	giatrinap = map(speed, 0, 255, 0, TCNT0);
	if (pin_PWM == enA)
	{
		OCR0A = giatrinap;
	}
	if (pin_PWM == enB)
	{
		OCR0B = giatrinap;
	}
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

int map(int input, int in_min, int in_max, int out_min, int out_max)
{
	return ((input - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}
