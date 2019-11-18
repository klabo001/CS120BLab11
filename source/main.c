//Scheduler Code
#include <avr/io.h>
//#include "io.c" //included from directory


//------------------------------Timer Directory Start-------------------
// Permission to copy is granted provided that this header remains intact. 
// This software is provided with no warranties.

////////////////////////////////////////////////////////////////////////////////

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

//----------Find GCD function-------------------------------------------

// unsigned long int findGCD(unsigned long int a, unsigned long int b)
// {
// 	unsigned long int c;
// 	while(1)
// 	{
// 		c = a%b;
// 		if (c==0) return b;
// 		a = b;
// 		b = c;
// 		
// 	}
// 	return 0;
// 	
// }
/*
unsigned long GCD = tasks[0]->period;
for (i = 1; i < numTasks; i++)
{
	GCD = findGCD(GCD, tasks[i]->period);
}
*/
//------------End GCD function -----------------------------------------

// ------Shared Variables ----------------------------------------------
unsigned char led0_output = 0x00;
unsigned char led1_output = 0x00;
unsigned char pause = 0;
// ------End Shared Variables-------------------------------------------
// --------Pause Button SM ---------------------------------------------
enum pauseButtonSM_states { pauseButton_wait, pauseButton_press, pauseButton_release};
	//Monitors button connected to PA0
	//When button is pressed, shared variable "pause" is toggled.
int pauseButtonSMTick(int state) {
	unsigned char press  = ~PINA &0x01;
	
	switch (state) { //transitions
		case pauseButton_wait:
			state = press = 0x01? pauseButton_press: pauseButton_wait; break;
		case pauseButton_press: 
			state = pauseButton_release; break;
		case pauseButton_release:
			state = press == 0x00? pauseButton_wait: pauseButton_press; break;
		default: state = pauseButton_wait; break;
	}
	
	switch (state) {
		case pauseButton_wait: break;
		case pauseButton_press: 
			pause = (pause == 0) ? 1 : 0; // toggle pause
		case pauseButton_release: break;
	}
	return state;
}

// ----------End Pause Button SM ---------------------------------------

// ----------Toggle LED0 SM --------------------------------------------

enum toggleLED0_states { toggleLED0_wait, toggleLED0_blink};

int toggleLED0SMTick(int state){
	switch (state) {//transitions
		case toggleLED0_wait: state = !pause? toggleLED0_blink: toggleLED0_wait; break;
		case toggleLED0_blink: state = pause? toggleLED0_wait: toggleLED0_blink; break;
		default: state = toggleLED0_wait; break;
	}
	switch (state) {// actions
		case toggleLED0_wait: break;
		case toggleLED0_blink:
			led0_output = (led0_output == 0x00) ? 0x01 : 0x00; //toggle LED
			break;
	}
	return state;
}
//---------------end Toggle LED0 SM------------------------------------

//---------------Toggle LED1 SM ----------------------------------------

enum toggleLED1_states { toggleLED1_wait, toggleLED1_blink};

int toggleLED1SMTick(int state){
	switch (state) {//transitions
		case toggleLED1_wait: state = !pause? toggleLED1_blink: toggleLED1_wait; break;
		case toggleLED1_blink: state = pause? toggleLED1_wait: toggleLED1_blink; break;
		default: state = toggleLED1_wait; break;
	}
	switch (state) {// actions
		case toggleLED1_wait: break;
		case toggleLED1_blink:
			led1_output = (led1_output == 0x00) ? 0x01 : 0x00; //toggle LED
			break;
	}
	return state;
	
}

//------------------------End Toggle LED1 SM ---------------------------

//------------- Display SM ---------------------------------------------

enum display_states {display_display};

//combine blinking LED outputs from toggleLED0 SM and toggleLED1 SM, and output on PORTB

int displaySMTick(int state){
	//Local Variables
	unsigned char output;
	
	switch (state) { //transitions
		case display_display: state = display_display; break;
		default: state = display_display; break;
	}
	switch (state) {
		case display_display:
			output = led0_output | led1_output << 1; //write shared outputs to local variable
			break;
	}
	PORTB = output;
	return state;
}


int main(){
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	
	// Declare an array of tasks
	static task task1, task2, task3, task4;
	task *tasks[] = {&task1, &task2, &task3, &task4};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
//	int iter = 0;
	unsigned long GCD = 10;
// 	for (iter = 1; iter < numTasks; iter++)
// 	{
// 		GCD = findGCD(GCD, tasks[iter]->period);
// 	}
// 	
	// Task 1 (pauseButtonToggleSM)
	task1.state = pauseButton_wait; //Task initial state.
	task1.period = 50; //Task Period
	task1. elapsedTime = task1.period; //Task current elapsed time.
	task1.TickFct = &pauseButtonSMTick; //Function pointer for the tick.
	
	// Task 2 (toggleLED0SM)
	task2.state = toggleLED0_wait; //Task initial state.
	task2.period = 500; //Task Period
	task2. elapsedTime = task2.period; //Task current elapsed time.
	task2.TickFct = &toggleLED0SMTick; //Function pointer for the tick.
	 
	// Task 3 (toggleLED1SM)
	task3.state = toggleLED1_wait; //Task initial state.
	task3.period = 1000; //Task Period
	task3. elapsedTime = task3.period; //Task current elapsed time.
	task3.TickFct = &toggleLED1SMTick; //Function pointer for the tick.
	
	// Task 4 (displaySM)
	task4.state = display_display; //Task initial state.
	task4.period = 10; //Task Period
	task4. elapsedTime = task4.period; //Task current elapsed time.
	task4.TickFct = &displaySMTick; //Function pointer for the tick.
	
	// Set the timer and turn it on
	
	TimerSet(GCD);
	TimerOn();
	
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
