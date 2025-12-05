#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 160000000UL

volatile int MSEC = 0; //represent milliseconds

volatile int one = 0; //bools to enable delays
volatile int one_point_five = 0;

void timer1_init_ctc(){//initialize timer1 in ctc mode
	TCCR1A= 0x00; //wgm10=wgm11=0 (ctc mode)
	TCCR1B |= (1<<WGM12); //wgm12 = 1 (ctc mode)
	TCNT1 = 0; //initialize timer1 to 0
	TIMSK1 |= (1<<OCIE1A); //activate interrupt for timer1
	OCR1A = 0x00FA; //to introduce 1ms delay in ctc mode 64 prescalar
}
void timer1_start(){//start timer1
    //prescalar of 64
	TCCR1B |= (1 << CS11) | (1 << CS10); //cs11=cs10=1
}
void timer1_stop(){//stop timer1
	TCCR1B &= ~ ((1<<CS12) | (1<<CS10) | (1<<CS11)); //no clock
}

int flip(int x){
    temp = ~x;
    // Reverse bits
    temp = (temp & 0b11110000) >> 4 | (temp & 0b00001111) << 4; // Swap Nibbles
    temp = (temp & 0b11001100) >> 2 | (temp & 0b00110011) << 2; // Swap every other 2 bits
    temp = (temp & 0b10101010) >> 1 | (temp & 0b01010101) << 1; // Swap every other bit
    num1 = (temp & 0b11100000) >> 5;
    num2 = temp & 0b00000111;
    return num1 * num2;
}

ISR(TIMER1_COMPA_vect)//represent the interrupt service routine (this interrupt is related to timer1 in ctc mode)
{
	TCNT1 = 0;
	MSEC++;
	while(MSEC < 1000 && one)
	{}
	while(MSEC < 1500 && one_point_five)
	{}
}

int main(void)
{
	sei(); //enable the interrupt at the microcontroller level
	timer1_init_ctc();
}

Buzz_one_point_five(){
    for (int i = 0; i < 10; i++){
        for (int i = 0; i < 100; i++){
        PORTE &= ~(1 << 4);
		one_point_five = 1;
        timer1_start();
		timer1_stop();
		MSEC = 0;
		one_point_five = 0;
        PORTE |= (1 << 4);
        one_point_five = 1;
        timer1_start();
		timer1_stop();
		MSEC = 0;
		one_point_five = 0;
        }
    }
}

Buzz_one(){
    for (int i = 0; i < 10; i++){
        for (int i = 0; i < 100; i++){
        PORTE &= ~(1 << 4);
        one = 1;
        timer1_start();
		timer1_stop();
		MSEC = 0;
		one = 0;
        PORTE |= (1 << 4);
        one = 1;
        timer1_start();
		timer1_stop();
		MSEC = 0;
		one = 0;
        }
    }
}
