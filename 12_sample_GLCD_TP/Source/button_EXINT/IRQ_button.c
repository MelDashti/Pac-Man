#include "button.h"
#include "LPC17xx.h"
#include "GLCD/GLCD.h"
#include <stdbool.h>

extern volatile bool gamePaused;
extern volatile bool gameOver;
extern int offsetX;
extern int offsetY;
bool firstUnpauseDone;

extern volatile bool debouncing; // here we add this line


// EINT0 Handles the pause/start game button. 
void EINT0_IRQHandler(void) {
			debouncing = true;
			NVIC_DisableIRQ(EINT0_IRQn); // here we disable the button interupt 
			LPC_PINCON->PINSEL4    &= ~(1 << 20);     				/* GPIO pin selection 			*/
			LPC_SC->EXTINT &= (1 << 0); // Clear pending interrupt
}

void EINT1_IRQHandler (void)	  	/* KEY1														 */
{
	NVIC_DisableIRQ(EINT1_IRQn);		/* disable Button interrupts			 */
	LPC_PINCON->PINSEL4    &= ~(1 << 22);     /* GPIO pin selection */
	LPC_SC->EXTINT &= (1 << 1);     /* clear pending interrupt         */
}

void EINT2_IRQHandler (void)	  	/* KEY2														 */
{
  LPC_SC->EXTINT &= (1 << 2);     /* clear pending interrupt         */    
}


