#include <avr/io.h>
#include <util/delay.h>
#include "Nen_rac.h"
#include "PORT.h"

void Nen_rac()
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
	duration = pulseIn(echoPin, HIGH);
	// Calculating the distance
	distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
	if(distance < 15)
	{
		UART_putc('2');
		y = 0;
	}
	else
	{
		// dua nen rac ve vi tri ban dau
		// doi nen rac ve vi tri ban dau bang cong tac hanh trinh
	}
}
