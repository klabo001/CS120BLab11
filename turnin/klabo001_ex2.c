//Scheduler Code
#include <avr/io.h>
//#include "io.c" //included from directory

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#ifndef __io_h__
#define __io_h__

void LCD_init();
void LCD_ClearScreen(void);
void LCD_WriteCommand (unsigned char Command);
void LCD_Cursor (unsigned char column);
void LCD_DisplayString(unsigned char column ,const unsigned char *string);
void delay_ms(int miliSec);
#endif


#define SET_BIT(p,i) ((p) |= (1 << (i)))
#define CLR_BIT(p,i) ((p) &= ~(1 << (i)))
#define GET_BIT(p,i) ((p) & (1 << (i)))

/*-------------------------------------------------------------------------*/

#define DATA_BUS PORTC		// port connected to pins 7-14 of LCD display
#define CONTROL_BUS PORTD	// port connected to pins 4 and 6 of LCD disp.
#define RS 6			// pin number of uC connected to pin 4 of LCD disp.
#define E 7			// pin number of uC connected to pin 6 of LCD disp.

/*-------------------------------------------------------------------------*/

void LCD_ClearScreen(void) {
	LCD_WriteCommand(0x01);
}

void LCD_init(void) {

	//wait for 100 ms.
	delay_ms(100);
	LCD_WriteCommand(0x38);
	LCD_WriteCommand(0x06);
	LCD_WriteCommand(0x0f);
	LCD_WriteCommand(0x01);
	delay_ms(10);
}

void LCD_WriteCommand (unsigned char Command) {
	CLR_BIT(CONTROL_BUS,RS);
	DATA_BUS = Command;
	SET_BIT(CONTROL_BUS,E);
	asm("nop");
	CLR_BIT(CONTROL_BUS,E);
	delay_ms(2); // ClearScreen requires 1.52ms to execute
}

void LCD_WriteData(unsigned char Data) {
	SET_BIT(CONTROL_BUS,RS);
	DATA_BUS = Data;
	SET_BIT(CONTROL_BUS,E);
	asm("nop");
	CLR_BIT(CONTROL_BUS,E);
	delay_ms(1);
}

void LCD_DisplayString( unsigned char column, const unsigned char* string) {
	LCD_ClearScreen();
	unsigned char c = column;
	while(*string) {
		LCD_Cursor(c++);
		LCD_WriteData(*string++);
	}
}

void LCD_Cursor(unsigned char column) {
	if ( column < 17 ) { // 16x1 LCD: column < 9
		// 16x2 LCD: column < 17
		LCD_WriteCommand(0x80 + column - 1);
		} else {
		LCD_WriteCommand(0xB8 + column - 9);	// 16x1 LCD: column - 1
		// 16x2 LCD: column - 9
	}
}

void delay_ms(int miliSec) //for 8 Mhz crystal

{
	int i,j;
	for(i=0;i<miliSec;i++)
	for(j=0;j<775;j++)
	{
		asm("nop");
	}
}

#ifndef TIMER_H
#define TIMER_H

#include <avr/interrupt.h>

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1ms
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B 	= 0x0B;	// bit3 = 1: CTC mode (clear timer on compare)
					// bit2bit1bit0=011: prescaler /64
					// 00001011: 0x0B
					// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
					// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A 	= 125;	// Timer interrupt will be generated when TCNT1==OCR1A
					// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
					// So when TCNT1 register equals 125,
					// 1 ms has passed. Thus, we compare to 125.
					// AVR timer interrupt mask register

	TIMSK1 	= 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1 = 0;

	// TimerISR will be called every _avr_timer_cntcurr milliseconds
	_avr_timer_cntcurr = _avr_timer_M;

	//Enable global interrupts
	SREG |= 0x80;	// 0x80: 1000000
}

void TimerOff() {
	TCCR1B 	= 0x00; // bit3bit2bit1bit0=0000: timer off
}

void TimerISR() {
	TimerFlag = 1;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect)
{
	// CPU automatically calls when TCNT0 == OCR0 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; 			// Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { 	// results in a more efficient compare
		TimerISR(); 				// Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

#endif //TIMER_H
//-------------------End Timer Directory--------------------------------
// ------Task Scheduler Data Structure ---------------------------------
/* Struct for Tasks represent a running process in our simple real-time
			operating system. */

typedef struct _task {
	/*Tasks should have members that include state, period,
		a measurement of elapsed time, and a function pointer.*/
		
		signed char state; //Task's current state
		unsigned long int period; //Task period
		unsigned long int elapsedTime; //Time elapsed since last tick
		int (*TickFct)(int);
} task;

// ------End Task Scheduler Data structure -----------------------------

//------------- Display SM ---------------------------------------------

enum display_states {display_display1, display_display2};



int displayNumTick(int state)
{
	//Local Variables
	
	
	switch (state) 
	{ //transitions
		case display_display1: state = display_display2; break;
		case display_display2: state = display_display1; break;

		default: state = display_display1; break;
	}
	switch (state) 
	{//Actions
		
		case display_display1: 
			LCD_DisplayString(1, "CS120B is Legend...");
			break;
		case display_display2: 
			LCD_DisplayString(1, "Wait for it DARY");
			break;
	}
	
	return state;
}


int main(){
		DDRA = 0x00; PORTA = 0xFF;
		DDRC = 0xFF; PORTC = 0x00;
		DDRD = 0xFF; PORTD = 0x00;

	// Declare an array of tasks
	static task task4;
	task *tasks[] = {&task4};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	unsigned long GCD = 400;

	// Task 4 (displaySM)
	task4.state = display_display1; //Task initial state.
	task4.period = 800; //Task Period
	task4. elapsedTime = task4.period; //Task current elapsed time.
	task4.TickFct = &displayNumTick; //Function pointer for the tick.
	
	// Set the timer and turn it on
	
	TimerSet(GCD);
	TimerOn();
	
	//Initialize LCD
	LCD_init();
	unsigned short i;// scheduler for loop iterator
	
	while(1){
		for(i = 0; i < numTasks; i++)
		{
			if ( tasks[i]->elapsedTime == tasks[i]->period)
			{
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state); //Set next State
				tasks[i]->elapsedTime = 0; //reset the elapsed time for the next tick
			}
			tasks[i]-> elapsedTime+= GCD;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
	return 0; //Error: Program should not exit
}
