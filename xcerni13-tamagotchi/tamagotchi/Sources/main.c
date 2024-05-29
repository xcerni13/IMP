/*
 * IMP Projekt 2021/2022
 * Hra Tamagotchi na platforme FITkit 3 s maticovym displejom
 * Rebeka Èernianska, xcerni13
 * 18.12.2021
 * */

/* Header file with all the essential definitions for a given type of MCU */
#include "MK60D10.h"
#include <stdio.h>

/* Macros for bit-level registers manipulation */
#define GPIO_PIN_MASK	0x1Fu
#define GPIO_PIN(x)		(((1)<<(x & GPIO_PIN_MASK)))


/* Constants specifying delay loop duration */
#define	tdelay1			10000
#define tdelay2 		20

/* Macros for buttons */
#define BTN_RIGHT 0x400     // Port E, bit 10
#define BTN_LEFT 0x8000000 // Port E, bit 27
#define BTN_ENTER 0x800     // Port E, bit 11

#define SPK 0x10          // Speaker is on PTA4

/* Macros for direction of movement */
#define LEFT -1
#define RIGHT 1

//character 3
int postava3[8][16] = {{0,0,0,0,0,0,1,1,1,1,},
    					{0,0,0,0,1,1,0,1,0,0,1,},
						{0,0,0,1,0,0,0,0,0,1,0,1,},
						{0,0,0,0,1,1,0,0,0,0,0,1,},
						{0,0,0,1,0,0,0,0,0,0,0,1,},
						{0,0,0,0,1,1,0,0,0,0,1,},
						{0,0,0,0,0,0,1,1,1,1,},
						{0,}};

//character 2
int postava2[8][16] = {{0,0,0,0,0,0,1,0,0,0,1,},
					   {0,0,0,0,0,0,1,1,1,1,1,},
					   {0,0,0,0,0,0,1,0,1,0,1,},
					   {0,0,0,0,0,0,1,1,1,1,1,},
					   {0,0,0,0,0,1,1,1,0,1,1,1,},
					   {0,0,0,0,0,0,1,1,1,1,1,},
					   {0,0,0,0,0,0,1,1,1,1,1,},
					   {0,0,0,0,0,0,0,1,0,1,}};

//character 1
int postava1[8][16] = {{0,0,0,0,0,1,1,1,1,1,1,},
					   {0,0,0,0,1,0,0,0,0,0,0,1,},
					   {0,0,0,1,0,0,1,0,0,1,0,0,1,},
					   {0,0,0,1,0,0,0,0,0,0,0,0,1,},
					   {0,0,0,1,0,0,1,1,1,1,0,0,1,},
					   {0,0,0,0,1,0,0,0,0,0,0,1,},
					   {0,0,0,0,0,1,1,1,1,1,1,},
					   {0,}};

int death[8][16] = {{0,},
    				{0,0,0,0,0,0,0,0,1,},
					{0,0,0,0,0,0,0,0,1,},
					{0,0,0,0,0,0,1,1,1,1,1,},
					{0,0,0,0,0,0,0,0,1,},
					{0,0,0,0,0,0,0,0,1,},
					{0,0,0,0,0,0,0,0,1,},
					{0,}};

int current_character[8][16] = {{0,}, {0,},	{0,}, {0,},	{0,}, {0,},	{0,}, {0,}};

//global variables
int row_nums[8] = {26, 24, 9, 25, 28, 7, 27, 29};
int direction = RIGHT;
int button_pressed = 0;
int character_chosen = 0;
int current_character_int = 1;
int type = 1;
int fed = 0;
int dead = 0;
int beep_flag = 0;
int compare = 0x100;


/* Configuration of the necessary MCU peripherals */
void SystemConfig() {
	/* Turn on all port clocks */
	SIM->SCGC5 = SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTE_MASK | SIM_SCGC5_PORTA_MASK;

	/* Set corresponding PTA pins (column activators of 74HC154) for GPIO functionality */
	PORTA->PCR[8] = ( 0|PORT_PCR_MUX(0x01) );  // A0
	PORTA->PCR[10] = ( 0|PORT_PCR_MUX(0x01) ); // A1
	PORTA->PCR[6] = ( 0|PORT_PCR_MUX(0x01) );  // A2
	PORTA->PCR[11] = ( 0|PORT_PCR_MUX(0x01) ); // A3

	/* Set corresponding PTA pins (rows selectors of 74HC154) for GPIO functionality */
	PORTA->PCR[26] = ( 0|PORT_PCR_MUX(0x01) );  // R0
	PORTA->PCR[24] = ( 0|PORT_PCR_MUX(0x01) );  // R1
	PORTA->PCR[9] = ( 0|PORT_PCR_MUX(0x01) );   // R2
	PORTA->PCR[25] = ( 0|PORT_PCR_MUX(0x01) );  // R3
	PORTA->PCR[28] = ( 0|PORT_PCR_MUX(0x01) );  // R4
	PORTA->PCR[7] = ( 0|PORT_PCR_MUX(0x01) );   // R5
	PORTA->PCR[27] = ( 0|PORT_PCR_MUX(0x01) );  // R6
	PORTA->PCR[29] = ( 0|PORT_PCR_MUX(0x01) );  // R7

	/* Set corresponding PTE pins (output enable of 74HC154) for GPIO functionality */
	PORTE->PCR[28] = ( 0|PORT_PCR_MUX(0x01) ); // #EN

	/* Change corresponding PTE port pins as outputs */
	PTE->PDDR = GPIO_PDDR_PDD(GPIO_PIN(28));

	PORTA->PCR[4] = PORT_PCR_MUX(0x01);  // Speaker

	/* Change corresponding PTA port pins as outputs */
	PTA->PDDR = GPIO_PDDR_PDD(0x3F000FC0);
	PTA->PDDR |= GPIO_PDDR_PDD(0b10000); //add speaker as output

	PTA->PDOR &= GPIO_PDOR_PDO(~SPK);   // Speaker off, beep_flag is false

	/* Configuration of buttons */
    PORTE->PCR[10] = (PORT_PCR_ISF(0x01) | PORT_PCR_IRQC(0x0A) | PORT_PCR_MUX(0x01) | PORT_PCR_PE(0x01) | PORT_PCR_PS(0x01)); // SW2
    PORTE->PCR[27] = (PORT_PCR_ISF(0x01) | PORT_PCR_IRQC(0x0A) | PORT_PCR_MUX(0x01) | PORT_PCR_PE(0x01) | PORT_PCR_PS(0x01)); // SW4
    PORTE->PCR[11] = (PORT_PCR_ISF(0x01) | PORT_PCR_IRQC(0x0A) | PORT_PCR_MUX(0x01) | PORT_PCR_PE(0x01) | PORT_PCR_PS(0x01)); // SW6

	/* Enabling NVIC - interruptions */
	NVIC_ClearPendingIRQ(PORTE_IRQn);
	NVIC_EnableIRQ(PORTE_IRQn);

	//Enable clock to PIT module
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK; //Enable module
	PIT_MCR &= ~PIT_MCR_MDIS_MASK; //enable mdis
	PIT_MCR |= PIT_MCR_FRZ_MASK;
}



/* Variable delay loop */
void delay(int t1, int t2)
{
	int i, j;

	for(i=0; i<t1; i++) {
		for(j=0; j<t2; j++);
	}
}


/* Conversion of requested column number into the 4-to-16 decoder control.  */
void column_select(unsigned int col_num)
{
	unsigned i, result, col_sel[4];

	for (i =0; i<4; i++) {
		result = col_num / 2;	  // Whole-number division of the input number
		col_sel[i] = col_num % 2;
		col_num = result;

		switch(i) {

			// Selection signal A0
		    case 0:
				((col_sel[i]) == 0) ? (PTA->PDOR &= ~GPIO_PDOR_PDO( GPIO_PIN(8))) : (PTA->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(8)));
				break;

			// Selection signal A1
			case 1:
				((col_sel[i]) == 0) ? (PTA->PDOR &= ~GPIO_PDOR_PDO( GPIO_PIN(10))) : (PTA->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(10)));
				break;

			// Selection signal A2
			case 2:
				((col_sel[i]) == 0) ? (PTA->PDOR &= ~GPIO_PDOR_PDO( GPIO_PIN(6))) : (PTA->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(6)));
				break;

			// Selection signal A3
			case 3:
				((col_sel[i]) == 0) ? (PTA->PDOR &= ~GPIO_PDOR_PDO( GPIO_PIN(11))) : (PTA->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(11)));
				break;

			// Otherwise nothing to do...
			default:
				break;
		}
	}
}

void print_row(int row)
{
	PTA->PDOR |= GPIO_PDOR_PDO(GPIO_PIN(row));
}

void draw_character()
{
	for (int k = 0; k < 1000; k++)
	{
		for (int i = 15; i >= 0; i--)
		{
			if (!character_chosen)
			{
				//opening menu arrows
				column_select(1);
				print_row(9);
				print_row(28);
				print_row(25);
				PTA->PDOR &= GPIO_PDOR_PDO(0x0);
				column_select(0);
				print_row(25);
				PTA->PDOR &= GPIO_PDOR_PDO(0x0);
				column_select(14);
				print_row(9);
				print_row(28);
				print_row(25);
				PTA->PDOR &= GPIO_PDOR_PDO(0x0);
				column_select(15);
				print_row(25);
				PTA->PDOR &= GPIO_PDOR_PDO(0x0);
			}

			column_select(i);
			for (int j = 7; j >= 0; j--)
			{
				if (current_character[j][i] == 1)
				{
					print_row(row_nums[7-j]);
				}
			}
			PTA->PDOR &= GPIO_PDOR_PDO(0x0);
		}
	}

}


void move_character_one_led(int direction) //performs the movement which was set by character_movement()
{
	if (direction == RIGHT) //right
	{
		for (int i = 0; i < 8; i++)
		{
			int first = current_character[i][0];
			for (int j = 0; j < 15; j++)
			{
				current_character[i][j] = current_character[i][j+1];
			}
			current_character[i][15] = first;
		}
	}
	else if (direction == LEFT) //left - pretoze cele to zobrazujem od COL15 do COL 0 perspektivy
	{
		for (int i = 0; i < 8; i++)
		{
			int last = current_character[i][15];
			for (int j = 15; j >= 1; j--)
			{
				current_character[i][j] = current_character[i][j-1];
			}
			current_character[i][0] = last;
		}
	}
}

void character_movement() //different movements of characters - can apply to any character - only changes the 2D array
{
	if (type == 1)
	{
		int check_for_end = 0;
		for (int i = 0; i < 8; i++)
		{
			if (current_character[i][0] == 1)
			{
				check_for_end = 1;
			}
			if (current_character[i][15] == 1)
			{
				check_for_end = 1;
			}
		}
		if (check_for_end == 1)
		{
			direction *= -1;
		}

		move_character_one_led(direction);
		draw_character();
	}
	else if (type == 2)
	{
		move_character_one_led(direction);
		draw_character();
	}
	else if (type == 3)
	{
		int check_for_end = 0;
		for (int i = 0; i < 8; i++)
		{
			if (current_character[i][0] == 1)
			{
				check_for_end = 1;
			}
			if (current_character[i][15] == 1)
			{
				check_for_end = 1;
			}
		}
		if (check_for_end == 1)
		{
			direction *= -1;
			int flipped[8][16] = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	    					{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
							{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
							{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
							{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
							{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
							{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
							{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
			if (direction == LEFT)
			{
				for (int k = 0; k < 8; k++)
				{
					for (int m = 0; m < 10; m++)
					{
						flipped[k][m] = current_character[k][9-m];
					}
				}
				for (int k = 0; k < 8; k++)
				{
					for (int m = 0; m < 16; m++)
					{
						current_character[k][m] = flipped[k][m];
					}
				}
			}
		}
		move_character_one_led(direction);
		draw_character();
	}
}


void choose_character(int direction) //used for character choice in the beginning of game
{
	//which arrow was pressed, in that direction the characters rotate
	if (direction == LEFT)
	{
		current_character_int -= 1;
		if (current_character_int == 0)
		{
			current_character_int = 3;
		}
	}
	else
	{
		current_character_int += 1;
		if (current_character_int == 4)
		{
			current_character_int = 1;
		}
	}

	//assigning the specific character design
	if (current_character_int == 1)
	{
		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 16; j++)
			{
				current_character[i][j] = postava1[i][j];
			}
		}
	}
	else if (current_character_int == 2)
	{
		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 16; j++)
			{
				current_character[i][j] = postava2[i][j];
			}
		}
	}
	else if (current_character_int == 3)
	{
		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 16; j++)
			{
				current_character[i][j] = postava3[i][j];
			}
		}
	}

	//setting the behaviour for the specific character
	if (current_character_int == 1)
	{
		type = 1;
	}
	if (current_character_int == 2)
	{
		type = 2;
	}
	if (current_character_int == 3)
	{
		type = 3;
	}
}

void beep(tone)
{
	if (beep_flag)
	{
		GPIOA_PDOR ^= SPK;   // invert speaker state
		delay(1000, tone);
	}
	else GPIOA_PDOR &= ~SPK; // logic 0 on speaker port if beep is false
}


void init_characters()
{
	current_character_int = 1;
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			current_character[i][j] = postava1[i][j];
		}
	}
}

void TimerConfig() /* Setting up the display counter */
{
	//Initialize PIT0 to count down from starting_value - no need for second channel
	PIT_LDVAL0 = 471859197;
	PIT_TCTRL0 &= PIT_TCTRL_CHN_MASK;
	PIT_TCTRL0 |= PIT_TCTRL_TEN_MASK; //Let the PIT channel generate interrupt requests
	PIT_TCTRL0 |= PIT_TCTRL_TIE_MASK;
	NVIC_SetPriority(PIT0_IRQn, 3); //Clear any pending IRQ from PIT
	NVIC_ClearPendingIRQ(PIT0_IRQn); //Enable the PIT interrupt in the NVIC
	NVIC_EnableIRQ(PIT0_IRQn);
}


void LPTMR0Init(int count)
{
    SIM_SCGC5 |= SIM_SCGC5_LPTIMER_MASK; // Enable clock to LPTMR
    LPTMR0_CSR &= ~LPTMR_CSR_TEN_MASK;   // Turn OFF LPTMR to perform setup
    LPTMR0_PSR = ( LPTMR_PSR_PRESCALE(0) // 0000 is div 2
                 | LPTMR_PSR_PBYP_MASK   // LPO feeds directly to LPT
                 | LPTMR_PSR_PCS(1)) ;   // use the choice of clock
    LPTMR0_CMR = count;                  // Set compare value
    LPTMR0_CSR =(  LPTMR_CSR_TCF_MASK    // Clear any pending interrupt (now)
                 | LPTMR_CSR_TIE_MASK    // LPT interrupt enabled
                );
    NVIC_EnableIRQ(LPTMR0_IRQn);         // enable interrupts from LPTMR0
    LPTMR0_CSR |= LPTMR_CSR_TEN_MASK;    // Turn ON LPTMR0 and start counting
}

/* Handler of life cycle timer*/
void PIT0_IRQHandler()
{
	if (!fed)
	{
		dead = 1;
	}
	else //reset the timer if the character was fed, another timer will be set after the first one ends
	{
		PIT_TCTRL0 |= PIT_TCTRL_TEN_MASK;
		fed = 0;
	}

	PIT_TFLG0 &= PIT_TFLG_TIF_MASK;	// reset timer flag
}

void LPTMR0_IRQHandler()
{
    LPTMR0_CMR = compare;
    LPTMR0_CSR |=  LPTMR_CSR_TCF_MASK;   // writing 1 to TCF tclear the flag
    beep_flag = !beep_flag;              // see beep_flag test in main()
}

/* Handler of the button interruptions */
void PORTE_IRQHandler()
{
	if (PORTE->ISFR &= BTN_LEFT)
	{
		button_pressed = 1;
		if (!character_chosen)
		{
			choose_character(LEFT);
		}
	}
	else if (PORTE->ISFR &= BTN_RIGHT)
	{
		button_pressed = 1;
		if (!character_chosen)
		{
			choose_character(RIGHT);
		}
	}
	else if (PORTE->ISFR &= BTN_ENTER) //used in all phases of life, usage depends on the phase
	{
		for (int k = 0; k < 100; k++)
		{
			beep(1);
		}
		button_pressed = 1;

		if (!character_chosen)
		{
			character_chosen = 1;
		}
		else
		{
			fed = 1;
		}
		if (dead)
		{
			dead = 0;
		}
	}
}


int main(void)
{
	SystemConfig();
	LPTMR0Init(compare);
	init_characters();

	//infinite loop to be able to play multiple times
	while (1) {
		if (!character_chosen) //first phase of game - choosing the character
		{
			draw_character();
			TimerConfig();
		}
		else //second phase - life cycle of the character, if it is fed, it will live another cycle, if not, it dies
		{
			character_movement();
			delay(10000, 20);
		}
		while (dead) //last phase, when the character dies, to show the game over display and to reset the character positions
		{
			for (int k = 0; k < 1000; k++)
			{
				for (int i = 15; i >= 0; i--)
				{
					column_select(i);
					for (int j = 7; j >= 0; j--)
					{
						if (death[j][i] == 1)
						{
							print_row(row_nums[7-j]);
						}
					}
					PTA->PDOR &= GPIO_PDOR_PDO(0x0);
				}
			}
			character_chosen = 0;
			init_characters();
		}

		dead = 0;

	}


    /* Never leave main */
    return 0;
}
