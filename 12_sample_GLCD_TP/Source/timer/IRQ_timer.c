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
#include <stdbool.h>
extern volatile int countdown;
extern volatile powerPillsSpawned;
extern volatile bool gamePaused;
extern void drawUI(void);
extern volatile gameOver;

/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

uint16_t SinTable[45] =                                      
{
    410, 467, 523, 576, 627, 673, 714, 749, 778,
    799, 813, 819, 817, 807, 789, 764, 732, 694, 
    650, 602, 550, 495, 438, 381, 324, 270, 217,
    169, 125, 87 , 55 , 30 , 12 , 2  , 0  , 6  ,   
    20 , 41 , 70 , 105, 146, 193, 243, 297, 353
};

// Update Timer1 frequency for different sound effects
void update_timer1_frequency(uint32_t frequency) {
    if (frequency == 0) {
        // Disable sound
        disable_timer(1);
        LPC_DAC->DACR = 0;
        return;
    }
    
    disable_timer(1);
    // Calculate match value for desired frequency
    // Clock = 25MHz, Table size = 45 samples
    uint32_t matchValue = 25000000 / (frequency * 45);
    LPC_TIM1->MR0 = matchValue;
    reset_timer(1);
    enable_timer(1);
}
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
				gameOver=true;
				disable_timer(0);
				disable_RIT(0);
			
    }
		
		// *** ADD THIS LINE! ***
    handleGhostTimer();
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
void TIMER1_IRQHandler(void) {

		static int ticks = 0;

    /* Update DAC with sinusoidal value */
    LPC_DAC->DACR = (SinTable[ticks] << 6);

    /* Increment index for SinTable */
    ticks++;
    if (ticks == 45) ticks = 0;

    /* Clear interrupt flag */
    LPC_TIM1->IR = 1;
    return;
}
/******************************************************************************
**                            End Of File
******************************************************************************/
