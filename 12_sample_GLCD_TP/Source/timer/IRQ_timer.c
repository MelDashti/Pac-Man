/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_timer.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    timer.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include <string.h>
#include "LPC17xx.h"
#include "timer.h"
#include "../GLCD/GLCD.h" 
#include "../TouchPanel/TouchPanel.h"
#include <stdio.h> /*for sprintf*/
extern volatile int countdown;
extern volatile powerPillsSpawned;
extern volatile gamePaused;
extern void drawUI(void);


/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/


void TIMER0_IRQHandler (void)
{
    // Decrement the countdown if it's greater than 0
    if (countdown > 0) {
        countdown--;
        drawUI(); // Update the displayed countdown
    } else {
        // If countdown reached 0, you can show "Game Over!" or handle end condition here.
        // For now, just keep it simple.
				GUI_Text((240/2)-30, (320/2)-20, (uint8_t *)"GAME OVER!", Red, Black);
    }
		
		 // Random spawn of power pill logic 
    // Only spawn if game not paused, and if we haven’t spawned all 6
    if (!gamePaused && powerPillsSpawned < 6) {
        // e.g. a small chance each second.  You decide the probability; here 1 in 5:
				drawPowerPills();
    }
		
		
    LPC_TIM0->IR = 1; // Clear interrupt flag
    return;
}

/******************************************************************************
** Function name:		Timer1_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void TIMER1_IRQHandler (void)
{
  LPC_TIM1->IR = 1;			/* clear interrupt flag */
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
