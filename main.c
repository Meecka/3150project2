#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL

void timer0_init_ctc();
void timer0_start();
uint8_t timer0_expired(int ms_delay);
void timer0_stop();
void timer0_reset();
ISR(TIMER0_COMPA_vect);
void timer2_init_ctc(int compare_value);
void timer2_start(int prescale);
uint8_t timer2_expired();
void timer2_stop();
void timer2_reset();
ISR(TIMER2_COMPA_vect);
void delay(int ms);
void play_tone(uint16_t frequency, uint16_t duration_ms);
int flip(int x);
void CheckHighLow();
void HHigh();
void LLow();

static volatile int compare_count = 0;
volatile int CountingVar = 0x00;
volatile int CountingMax = 25;
volatile int CountingChange = 1;
volatile int CountersEqual = 1;

int main(void)
{
	DDRA = 0x00;
	PORTA = 0xFF;
	DDRD = 0xFF;
	DDRE = 0x10; // make all PE output
	// PORTD = CountingVar;
	// CountingVar = 0xFF;
	PORTD = 0b01111000;
	sei();
	timer0_init_ctc();
	// timer2_init_ctc();

	while (1)
	{
		CountersEqual = 0;
		if (~PINA & (1 << 5)) // SW7
		{
			CountingVar++;
		}
		if (~PINA & (1 << 7)) // SW9
		{
			CountingVar--;
		}
		if (~PINA & (1 << 0)) // SW1
		{
			CountingMax++;
		}
		if (~PINA & (1 << 2)) // SW 3
		{
			CountingMax -= CountingChange;
		}
		PORTD = flip(CountingVar);
		PORTA = 0xFF;
		delay(296);
		CheckHighLow();
	}
	return 0;
}

void timer0_init_ctc()
{
	TCCR0A |= (1 << WGM01);
	TCCR0B = 0x00; // CTC mode
	TCNT0 = 0;
	OCR0A = 249; // 1ms with prescalar 64
}

void timer0_start()
{
	TIFR0 &= ~(1 << OCF0A);
	TCNT0 = 0;
	TCCR0B |= ((1 << CS01) | (1 << CS00)); // prescalar 64
	TIMSK0 |= (1 << OCIE0A);			   // enable interrupt
}

uint8_t timer0_expired(int ms_delay)
{
	return (compare_count >= ms_delay); // if interrupt ms_delay times for longer delay
}

void timer0_stop()
{
	TCCR0B = 0;
	TIMSK0 &= ~(1 << OCIE0A);
}

void timer0_reset()
{
	TCNT0 = 0;
	TIFR0 |= (1 << OCF0A);
	compare_count = 0;
}

ISR(TIMER0_COMPA_vect)
{
	compare_count++; // timer interrupt
}

void timer2_init_ctc(int compare_value)
{
	TCCR2A |= (1 << WGM21);
	TCCR2B = 0x00; // CTC mode
	TCNT2 = 0;
	OCR2A = compare_value; // 1ms with prescalar 64
}

void timer2_start(int prescale)
{
	TIFR2 &= ~(1 << OCF2A);
	TCNT2 = 0;
	if (prescale == 1)
	{
		TCCR2B |= (1 << CS20); // prescalar 1
	}
	else if (prescale == 8)
	{
		TCCR2B |= (1 << CS21); // prescalar 8
	}
	else if (prescale == 64)
	{
		TCCR2B |= ((1 << CS21) | (1 << CS20)); // prescalar 64
	}
	else if (prescale == 256)
	{
		TCCR2B |= (1 << CS22); // prescalar 256
	}
	else if (prescale == 1024)
	{
		TCCR2B |= ((1 << CS22) | (1 << CS20)); // prescalar 1024
	}
	TIMSK2 |= (1 << OCIE2A); // enable interrupt
}

uint8_t timer2_expired()
{
	return (compare_count == 1); // if interrupt ms_delay times for longer delay
}

void timer2_stop()
{
	TCCR2B = 0;
	TIMSK2 &= ~(1 << OCIE2A);
}

void timer2_reset()
{
	TCNT2 = 0;
	TIFR2 |= (1 << OCF2A);
	TCCR2B &= ~((1 << CS22) | (1 << CS21) | (1 << CS20)); // clear prescale bits
	compare_count = 0;
}

ISR(TIMER2_COMPA_vect)
{
	compare_count = 1;
	timer2_stop();
}

void delay(int ms)
{ // wait until user responds
	timer0_stop();
	timer0_reset();
	timer0_start();
	while (!timer0_expired(ms))
		;
	timer0_stop();
	return;
}

void play_tone(uint16_t frequency, uint16_t duration_ms)
{ // play a given frequency for a given amount of time
	if (frequency == 0 || duration_ms == 0)
		return;
	uint32_t duration_us = (uint32_t)duration_ms * 1000;
	uint16_t half_period_us = 500000 / frequency;
	int machineCycles = half_period_us / .0625;
	int prescale = 0;
	int timer_preload_val = 0;
	if (machineCycles <= 255)
	{
		prescale = 1; // no prescaling
		timer_preload_val = 0 - machineCycles;
	}
	else if (machineCycles / 8 <= 255)
	{
		prescale = 8;
		timer_preload_val = 0 - (machineCycles / 8);
	}
	else if (machineCycles / 64 <= 255)
	{
		prescale = 64;
		timer_preload_val = 0 - (machineCycles / 64);
	}
	else if (machineCycles / 256 <= 255)
	{
		prescale = 256;
		timer_preload_val = 0 - (machineCycles / 256);
	}
	else if (machineCycles / 1024 <= 255)
	{
		prescale = 1024;
		timer_preload_val = 0 - (machineCycles / 1024);
	}
	else
	{
		return; // no valid prescale for Timer0
	}

	timer2_stop();
	timer2_reset();
	timer2_init_ctc(timer_preload_val);

	uint32_t total_toggles = duration_us / (2 * half_period_us); // each cycle = 2 toggles

	for (uint32_t i = 0; i < total_toggles; i++)
	{
		PORTE |= (1 << 4);
		timer2_start(prescale);
		while (!timer2_expired())
			;
		timer2_reset();
		PORTE &= ~(1 << 4);
		timer2_start(prescale);
		while (!timer2_expired())
			;
		timer2_reset();
	}
}

int flip(int x)
{
	int temp = ~x;
	// Reverse bits
	temp = (temp & 0b11110000) >> 4 | (temp & 0b00001111) << 4; // Swap Nibbles
	temp = (temp & 0b11001100) >> 2 | (temp & 0b00110011) << 2; // Swap every other 2 bits
	temp = (temp & 0b10101010) >> 1 | (temp & 0b01010101) << 1; // Swap every other bit
	return temp;
}

void CheckHighLow()
{
	if (CountingVar >= CountingMax)
	{
		HHigh();
	}
	else if (CountingVar < 0x00)
	{
		LLow();
	}
	return;
}

void HHigh()
{
	if (CountingVar == CountingMax)
	{
		CountersEqual = 1;
		return;
	}
	else
	{
		CountingVar = 0;
		play_tone(1500, 1500);
	}
	return;
}

void LLow()
{
	CountingVar = CountingMax;
	play_tone(1000, 1000);
}