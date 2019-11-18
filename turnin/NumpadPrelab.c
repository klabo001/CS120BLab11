/*	Author: klabo001
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif


// Permission to copy is granted provided that this header remains intact.
// This software is provided with no warranties.

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
	PORTC = 0xEF; //Enable col 4 with 0, disable others with 1's
	asm("nop"); //add a delay to allow PORTC to stabilize before checking
	if(GetBit(PINC,0) == 0) {return ('1');}
	if(GetBit(PINC,1) == 0) {return ('4');}
	if(GetBit(PINC,2) == 0) {return ('7');}
	if(GetBit(PINC,3) == 0) {return ('*');}
	
	//Check keys in column 2
	PORTC = 0xDF; // Enable col 5 with 0, disable others with 1's
	asm("nop"); //add a delay to allow PORTC to stabilize before checking
	if(GetBit(PINC,0) == 0) {return('2');}
	if(GetBit(PINC,1) == 0) {return('5');}
	if(GetBit(PINC,2) == 0) {return('8');}
	if(GetBit(PINC,3) == 0) {return('0');}
	
	//Check keys in column 3
	PORTC = 0xBF; // Enable col 6 with 0, disable others with 1's
	asm("nop"); //add a delay to allow PORTC to stabilize before checking
	if(GetBit(PINC,0) == 0) {return('3');}
	if(GetBit(PINC,1) == 0) {return('6');}
	if(GetBit(PINC,2) == 0) {return('9');}
	if(GetBit(PINC,3) == 0) {return('#');}
	
	//Check keys in column 4
	PORTC = 0x7F; // Enable col 7 with 0, disable others with 1's
	asm("nop"); //add a delay to allow PORTC to stabilize before checking
	if(GetBit(PINC,0) == 0) {return('A');}
	if(GetBit(PINC,1) == 0) {return('B');}
	if(GetBit(PINC,2) == 0) {return('C');}
	if(GetBit(PINC,3) == 0) {return('D');}
	
	return('\0'); // default
}

int main(void)
{
	unsigned char x;
	DDRB = 0xFF; PORTB = 0x00; // PORTB set to output, outputs init 0s
	DDRC = 0xF0; PORTC = 0x0F; // PC7..4 outputs init 0s, PC3..0 inputs init 1s
	
	while(1) 
	{
		x = GetKeypadKey();
		switch (x) {
			case '\0': PORTB = 0x1F; break; //All 5 LEDs on
			case '1': PORTB = 0x01; break; //hex equivalent
			case '2': PORTB = 0x02; break; 
			case '3': PORTB = 0x03; break; 
			case '4': PORTB = 0x04; break; 
			case '5': PORTB = 0x05; break; 
			case '6': PORTB = 0x06; break; 
			case '7': PORTB = 0x07; break; 
			case '8': PORTB = 0x08; break; 
			case '9': PORTB = 0x09; break; 
			case 'A': PORTB = 0x0A; break; 
			case 'B': PORTB = 0x0B; break; 
			case 'C': PORTB = 0x0C; break; 
			case 'D': PORTB = 0x0D; break; 
			case '*': PORTB = 0x0E; break; 
			case '0': PORTB = 0x00; break; 
			case '#': PORTB = 0x0F; break; 
			default: PORTB = 0x1B; break; //should never occur. Middle LED off.
		}
	}
	

}
