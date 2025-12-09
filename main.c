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

static volatile int compare_count = 0; // timer compare count for delays
volatile int CountingVar = 0x00; // current count
volatile int CountingMax = 25; // max count
volatile int CountingChange = 1; //amount to change count by
volatile int CountersEqual = 1; // flag to see if countingVar has reached max

int main(void)
{
	DDRA = 0x00; // make all PORTE input
	PORTA = 0xFF; // pull-ups enabled
	DDRD = 0xFF; // make all PORTD output
	DDRE = 0x10; // make all PORTE output
	sei(); //enable global interrupt at the micro controller level
	timer0_init_ctc(); //initialize timer0 for delay

	while (1)
	{
		CountersEqual = 0; //reset countersEqual flag
		if (~PINA & (1 << 5)) //SW7 increase current count
		{
			CountingVar++;
		}
		if (~PINA & (1 << 7)) //SW9 decrease current count
		{
			CountingVar--;
		}
		if (~PINA & (1 << 0)) //SW1 increase countingMax
		{
			CountingMax++;
		}
		if (~PINA & (1 << 2)) //SW3 decrease countingMax by countingChange
		{
			CountingMax -= CountingChange;
		}
		PORTD = flip(CountingVar); //bit inversion for output
		delay(296); //delay to see visible changes in count
		CheckHighLow();
	}
	return 0;
}

void timer0_init_ctc() //initialize timer0 in ctc mode
{
	TCCR0A |= (1 << WGM01); // CTC mode wgm01=1
	TCCR0B = 0x00; // CTC mode wg02=wg00=0
	TCNT0 = 0;  //initialize timer0 to 0
	OCR0A = 249; // 1ms with prescalar 64
}

void timer0_start() //start timer0
{
	TCNT0 = 0; //initialize timer0 to 0
	TCCR0B |= ((1 << CS01) | (1 << CS00)); // prescalar 64
	TIMSK0 |= (1 << OCIE0A); // enable interrupt
}

uint8_t timer0_expired(int ms_delay) //return with timer0 expired
{
	return (compare_count >= ms_delay); // if interrupt ms_delay times for longer delay
}

void timer0_stop() //stop timer0
{
	TCCR0B = 0; //stop clock
	TIMSK0 &= ~(1 << OCIE0A); //disable interrupt
}

void timer0_reset() //reset timer0
{
	TCNT0 = 0; //initialize timer0 to 0
	TIFR0 |= (1 << OCF0A); //set output compare flag A
	compare_count = 0; //timer compare count to 0
}

ISR(TIMER0_COMPA_vect)//interrupt service routine
{
	compare_count++; // timer interrupt
}

void timer2_init_ctc(int compare_value) //initialize timer2
{
	TCCR2A |= (1 << WGM21);// CTC mode wgm21=1
	TCCR2B = 0x00; // wgm20=wgm22=0
	TCNT2 = 0; //initialize timer2 to 0
	OCR2A = compare_value; // 1ms with prescalar 64
}

void timer2_start(int prescale) //start timer2 with custom prescalar
{
	TCNT2 = 0; //initialize timer2 to 0
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

uint8_t timer2_expired() //return when timer2 expired
{
	return (compare_count == 1); // if interrupt ms_delay times for longer delay
}

void timer2_stop() //stop timer2
{
	TCCR2B = 0; //stop clock
	TIMSK2 &= ~(1 << OCIE2A); //disable interrupt
}

void timer2_reset() //reset timer0
{
	TCNT2 = 0; //initialize timer0 to 0
	TIFR2 |= (1 << OCF2A); // set the output compare flag
	TCCR2B &= ~((1 << CS22) | (1 << CS21) | (1 << CS20)); // clear prescale bits
	compare_count = 0; //reset compare count to 0
}

ISR(TIMER2_COMPA_vect) //interrupt service routine
{
	compare_count = 1; //set compare count to 1
	timer2_stop(); //stop timer2
}

void delay(int ms) //generate custom delay
{
	timer0_stop(); //stop timer0
	timer0_reset(); //reset timer0
	timer0_start(); //start timer0
	while (!timer0_expired(ms)) //wait until timer0 is expired
	;
	timer0_stop(); //stop timer0
	return;
}

void play_tone(uint16_t frequency, uint16_t duration_ms) // play a given frequency for a given amount of time
{
	if (frequency == 0 || duration_ms == 0)
	return;
	uint32_t duration_us = (uint32_t)duration_ms * 1000; //convert milliseconds to microseconds
	uint16_t half_period_us = 500000 / frequency; //calculate half period
	int machineCycles = half_period_us / .0625; //calculate machine cycles
	int prescale = 0; //initialize prescale
	int timer_preload_val = 0; //initialize timer preload
	if (machineCycles <= 255) //set to no prescaling
	{
		prescale = 1; // no prescaling for timer0
		timer_preload_val = 0 - machineCycles;
	}
	else if (machineCycles / 8 <= 255)
	{
		prescale = 8; //set timer0 prescalar 8
		timer_preload_val = 0 - (machineCycles / 8); //set timer0 preload value
	}
	else if (machineCycles / 64 <= 255)
	{
		prescale = 64;//set timer0 prescalar 64
		timer_preload_val = 0 - (machineCycles / 64); //set timer0 preload value
	}
	else if (machineCycles / 256 <= 255)
	{
		prescale = 256; //set timer0 prescalar 256
		timer_preload_val = 0 - (machineCycles / 256); //set timer0 preload value
	}
	else if (machineCycles / 1024 <= 255)
	{
		prescale = 1024; //set timer0 prescalar 1024
		timer_preload_val = 0 - (machineCycles / 1024); //set timer0 preload value
	}
	else
	{
		return; // no valid prescale for Timer0
	}

	timer2_stop(); //stop timer2
	timer2_reset(); //reset timer2
	timer2_init_ctc(timer_preload_val); //initialize timer2 with preload value

	uint32_t total_toggles = duration_us / (2 * half_period_us); // each cycle = 2 toggles

	for (uint32_t i = 0; i < total_toggles; i++)
	{
		PORTE |= (1 << 4); //speaker on
		timer2_start(prescale); //start timer2 with prescalar
		while (!timer2_expired()) //wait square wave
		;
		timer2_reset();
		PORTE &= ~(1 << 4); //speaker off
		timer2_start(prescale); //start timer2 with prescalar
		while (!timer2_expired()) //wait square wave
		;
		timer2_reset(); //reset timer2
	}
}

int flip(int x) //flip bits of x
{
	int temp = ~x; //inverts bits
	// Reverse order of bits
	temp = (temp & 0b11110000) >> 4 | (temp & 0b00001111) << 4; // Swap Nibbles
	temp = (temp & 0b11001100) >> 2 | (temp & 0b00110011) << 2; // Swap every other 2 bits
	temp = (temp & 0b10101010) >> 1 | (temp & 0b01010101) << 1; // Swap every other bit
	return temp;
}

void CheckHighLow() // check if countingVar is >= countingMax or < 0
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

void HHigh() //CountingVar is >= CountingMax
{
	if (CountingVar == CountingMax)
	{
		CountersEqual = 1; // set flag if current count equals max count
		return;
	}
	else
	{
		CountingVar = 0; // if it exceeded max, reset count to 0
		play_tone(1500, 1500); //play high tone for 1.5 seconds
	}
	return;
}

void LLow() //if counter dropped below zero
{
	CountingVar = CountingMax; //roll it over to max
	play_tone(1000, 1000); //play low tone for 1 second
}
