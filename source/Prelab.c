//Scheduler Code

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

unsigned long int findGCD(unsigned long int a, unsigned long int b)
{
	unsigned long int c;
	while(1)
	{
		c = a%b;
		if (c==0) return b;
		a = b;
		b = c;
		
	}
	return 0;
	
}
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

int toggleLED0SMTick(){
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

int toggleLED1SMTick(){
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
			output = led0_output | led1_output << ; //write shared outputs to local variable
			break;
	}
	PORTB = output;
	return state;
}


int main(){
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	
	// Declare an array of tasks
	static _task task1, task2, task3, task4;
	_task *tasks[] = {&task1, &task2, &task3, &task4};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	
	unsigned long GCD = tasks[0]->period;
	for (i = 1; i < numTasks; i++)
	{
		GCD = findGCD(GCD, tasks[i]->period);
	}
	
	// Task 1 (pauseButtonToggleSM)
	task1.state = start; //Task initial state.
	task1.period = 50; //Task Period
	task1. elapsedTime = task1.period; //Task current elapsed time.
	task1.TickFct = &pauseButtonToggleSMTick; //Function pointer for the tick.
	
	// Task 2 (toggleLED0SM)
	task2.state = start; //Task initial state.
	task2.period = 500; //Task Period
	task2. elapsedTime = task2.period; //Task current elapsed time.
	task2.TickFct = &toggleLED0SMTick; //Function pointer for the tick.
	 
	// Task 3 (toggleLED1SM)
	task3.state = start; //Task initial state.
	task3.period = 1000; //Task Period
	task3. elapsedTime = task3.period; //Task current elapsed time.
	task3.TickFct = &toggleLED1SMTick; //Function pointer for the tick.
	
	// Task 4 (toggleLED1SM)
	task4.state = start; //Task initial state.
	task4.period = 10; //Task Period
	task4. elapsedTime = task4.period; //Task current elapsed time.
	task4.TickFct = &SMTick4; //Function pointer for the tick.
	
	// Set the timer and turn it on
	
	TimerSet(GCD);
	TimerOn();
	
	unsigned short i;// scheduler for loop iterator
	
	while(1){
		for(i = 0; i < numTasks; i++)
		{
			if ( tasks[i]->elapsedTime == task[i]->period)
			{
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state); //Set next State
				tasks[i]->elapsedTime = 0; //reset the elapsed time for the next tick
			}
			tasks[i]-> elapsedTime+= GCD;
		}
		While(!TimerFlag);
		TimerFlag = 0;
	}
	return 0; //Error: Program should not exit
}














