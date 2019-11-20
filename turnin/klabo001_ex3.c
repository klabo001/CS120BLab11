//Scheduler Code
#include <avr/io.h>
//#include "io.c" //included from directory

// Permission to copy is granted provided that this header remains intact.
// This software is provided with no warranties.
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
////////////////////////////////////////////////////////////////////////////////

#ifndef BIT_H
#define BIT_H

////////////////////////////////////////////////////////////////////////////////
//Functionality - Sets bit on a PORTx
//Parameter: Takes in a uChar for a PORTx, the pin number and the binary value
//Returns: The new value of the PORTx
unsigned char SetBit(unsigned char pin, unsigned char number, unsigned char bin_value)
{
	return (bin_value ? pin | (0x01 << number) : pin & ~(0x01 << number));
}

////////////////////////////////////////////////////////////////////////////////
//Functionality - Gets bit from a PINx
//Parameter: Takes in a uChar for a PINx and the pin number
//Returns: The value of the PINx
unsigned char GetBit(unsigned char port, unsigned char number)
{
	return ( port & (0x01 << number) );
}

#endif //BIT_H
// Returns '\0' if no key pressed, else returns char '1', '2', ... '9', 'A', ... 
// If multiple keys pressed, returns leftmost-topmost one
// Keypad must be connected to port C
/* Keypad Arrangement
	   PC4 PC5 PC6 PC7
  col	1	2	3	4
row
PC0 1 	1	2	3	A
PC1 2	4	5	6	B
PC2 3	7	8	9	C
PC3 4	*	0	#	D
*/

unsigned char GetKeypadKey()
{
	PORTA = 0xEF; //Enable col 4 with 0, disable others with 1's
	asm("nop"); //add a delay to allow PORTC to stabilize before checking
	if(GetBit(PINA,0) == 0) {return ('1');}
	if(GetBit(PINA,1) == 0) {return ('4');}
	if(GetBit(PINA,2) == 0) {return ('7');}
	if(GetBit(PINA,3) == 0) {return ('*');}
	
	//Check keys in column 2
	PORTA = 0xDF; // Enable col 5 with 0, disable others with 1's
	asm("nop"); //add a delay to allow PORTC to stabilize before checking
	if(GetBit(PINA,0) == 0) {return('2');}
	if(GetBit(PINA,1) == 0) {return('5');}
	if(GetBit(PINA,2) == 0) {return('8');}
	if(GetBit(PINA,3) == 0) {return('0');}
	
	//Check keys in column 3
	PORTA = 0xBF; // Enable col 6 with 0, disable others with 1's
	asm("nop"); //add a delay to allow PORTC to stabilize before checking
	if(GetBit(PINA,0) == 0) {return('3');}
	if(GetBit(PINA,1) == 0) {return('6');}
	if(GetBit(PINA,2) == 0) {return('9');}
	if(GetBit(PINA,3) == 0) {return('#');}
	
	//Check keys in column 4
	PORTA = 0x7F; // Enable col 7 with 0, disable others with 1's
	asm("nop"); //add a delay to allow PORTC to stabilize before checking
	if(GetBit(PINA,0) == 0) {return('A');}
	if(GetBit(PINA,1) == 0) {return('B');}
	if(GetBit(PINA,2) == 0) {return('C');}
	if(GetBit(PINA,3) == 0) {return('D');}
	
	return('\0'); // default
}
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

//------------- Display SM ---------------------------------------------

enum keypad_states {keypad_input};
unsigned char x;
//combine blinking LED outputs from toggleLED0 SM and toggleLED1 SM, and output on PORTB

int keypadTick(int state)
{
	//Local Variables
	unsigned char output;
	
	switch (state) 
	{ //transitions
		case keypad_input: state = keypad_input; break;
		default: state = keypad_input; break;
	}
	switch (state) 
	{
		case keypad_input:
				x = GetKeypadKey();
				
				switch (x) 
				{
					case '\0': output = 0x1F; x = ' '; break; //All 5 LEDs on
					case '1': output = 0x01; break; //hex equivalent
					case '2': output = 0x02; break;
					case '3': output = 0x03; break;
					case '4': output = 0x04; break;
					case '5': output = 0x05; break;
					case '6': output = 0x06; break;
					case '7': output = 0x07; break;
					case '8': output = 0x08; break;
					case '9': output = 0x09; break;
					case 'A': output = 0x0A; break;
					case 'B': output = 0x0B; break;
					case 'C': output = 0x0C; break;
					case 'D': output = 0x0D; break;
					case '*': output = 0x0E; break;
					case '0': output = 0x00; break;
					case '#': output = 0x0F; break;
					default: output = 0x1B; break; //should never occur. Middle LED off.
				}
				break;
	}
	PORTB = output;
	if(x != ' ')
	{
		LCD_ClearScreen();
		LCD_WriteData(x);
	}
	
	//LCD_ClearScreen();
	//LCD_WriteData(x);
	
	return state;
}


int main(){
	DDRA = 0xF0; PORTA = 0x0F;
	DDRB = 0xFF; PORTB = 0x00; // PORTB set to output, outputs init 0s
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	// Declare an array of tasks
	static task task4;
	task *tasks[] = {&task4};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	unsigned long GCD = 10;

	// Task 4 (displaySM)
	task4.state = keypad_input; //Task initial state.
	task4.period = 50; //Task Period
	task4. elapsedTime = task4.period; //Task current elapsed time.
	task4.TickFct = &keypadTick; //Function pointer for the tick.
	
	// Set the timer and turn it on
	LCD_init();
	LCD_ClearScreen();
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
