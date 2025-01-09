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
#include "Ghost/ghost.h"
#include "../GLCD/GLCD.h" 
#include "../TouchPanel/TouchPanel.h"
#include <stdio.h> /*for sprintf*/
#include <stdbool.h>
extern volatile int countdown;
extern volatile powerPillsSpawned;
extern volatile bool gamePaused;
extern void drawUI(void);
extern volatile gameOver;
extern handleGhostTimer();

/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_timer.c
** Descriptions:        functions to manage T0, T1, T2, and T3 interrupts
** Correlated files:    timer.h
*********************************************************************************************************/
#include <string.h>
#include "LPC17xx.h"
#include "timer.h"
#include "Ghost/ghost.h"
#include "../GLCD/GLCD.h" 
#include "../TouchPanel/TouchPanel.h"
#include <stdio.h>
#include <stdbool.h>

extern volatile int countdown;
extern volatile powerPillsSpawned;
extern volatile bool gamePaused;
extern void drawUI(void);
extern volatile gameOver;
extern handleGhostTimer();

#define RIT_SEMIMINIMA 8
#define RIT_MINIMA 16
#define RIT_INTERA 32

#define UPTICKS 1


//SHORTENING UNDERTALE: TOO MANY REPETITIONS
NOTE song[] = 
{
	// 1
	{d3, time_semicroma},
	{d3, time_semicroma},
	{d4, time_croma},
	{a3, time_croma},
	{pause, time_semicroma},
	{a3b, time_semicroma},
	{pause, time_semicroma},
	{g3, time_croma},
	{f3, time_semicroma*2},
	{d3, time_semicroma},
	{f3, time_semicroma},
	{g3, time_semicroma},
	// 2
	{c3, time_semicroma},
	{c3, time_semicroma},
	{d4, time_croma},
	{a3, time_croma},
	{pause, time_semicroma},
	{a3b, time_semicroma},
	{pause, time_semicroma},
	{g3, time_croma},
	{f3, time_semicroma*2},
	{d3, time_semicroma},
	{f3, time_semicroma},
	{g3, time_semicroma},
	// 3
	{c3b, time_semicroma},
	{c3b, time_semicroma},
	{d4, time_croma},
	{a3, time_croma},
	{pause, time_semicroma},
	{a3b, time_semicroma},
	{pause, time_semicroma},
	{g3, time_croma},
	{f3, time_semicroma*2},
	{d3, time_semicroma},
	{f3, time_semicroma},
	{g3, time_semicroma},
	// 4
	{a2b, time_semicroma},
	{a2b, time_semicroma},
	{d4, time_croma},
	{a3, time_croma},
	{pause, time_semicroma},
	{a3b, time_semicroma},
	{pause, time_semicroma},
	{g3, time_croma},
	{f3, time_semicroma*2},
	{d3, time_semicroma},
	{f3, time_semicroma},
	{g3, time_semicroma},
	// 5
	
};


uint16_t SinTable[45] = {
    410, 467, 523, 576, 627, 673, 714, 749, 778,
    799, 813, 819, 817, 807, 789, 764, 732, 694, 
    650, 602, 550, 495, 438, 381, 324, 270, 217,
    169, 125, 87 , 55 , 30 , 12 , 2  , 0  , 6  ,   
    20 , 41 , 70 , 105, 146, 193, 243, 297, 353
};

// Original sine wave/DAC logic
void TIMER0_IRQHandler(void) {
    static int sineticks = 0;
    /* DAC management */    
    static int currentValue; 
    
    currentValue = SinTable[sineticks];
    currentValue -= 410;
    currentValue /= 1;
    currentValue += 410;
    LPC_DAC->DACR = currentValue << 6;
    
    sineticks++;
    if (sineticks == 45) sineticks = 0;
    
    LPC_TIM0->IR = 1; // Clear interrupt flag
    return;
}

// Original disable timer logic
void TIMER1_IRQHandler(void) {
    disable_timer(0);
    LPC_TIM1->IR = 1; // Clear interrupt flag
    return;
}

// Game logic (moved from original Timer 0)
void TIMER2_IRQHandler(void) {
    // Decrement the countdown if it's greater than 0
    if (countdown > 0) {
        countdown--;
        drawUI(); // Update the displayed countdown
    } else {
        // If countdown reached 0, show "Game Over!"
        GUI_Text((240/2)-30, (320/2)-20, (uint8_t *)"GAME OVER!", Red, Black);
        gameOver = true;
        disable_timer(2);  // Changed from 0 to 2 since this is now Timer 2
        disable_RIT(0);
    }
    
    if (!blinky.isChasing) {
        handleGhostTimer();
    }
        
    // Random spawn of power pill logic 
    if (!gamePaused && powerPillsSpawned < 6) {
        drawPowerPills();
    }
    
    LPC_TIM2->IR = 1; // Clear interrupt flag
    return;
}

void TIMER3_IRQHandler(void) {
    static int currentNote = 0;
    static int ticks = 0;
    
    if(!isNotePlaying()) {
        ++ticks;
        if(ticks == UPTICKS) {
            ticks = 0;
            playNote(song[currentNote++]);
        }
    }
    
    // If we've played all notes, you might want to either:
    // Option 1: Loop the song
    if(currentNote >= sizeof(song)/sizeof(song[0])) {
        currentNote = 0;
    }
    
    // Or Option 2: Stop playing
    /*if(currentNote >= sizeof(song)/sizeof(song[0])) {
        disable_timer(3);
    }*/
    
    LPC_TIM3->IR = 1; // Clear interrupt flag
}