/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               main.c
** Descriptions:            The GLCD application function
**
**--------------------------------------------------------------------------------------------------------
** Created by:              AVRman
** Created date:            2010-11-7
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             Paolo Bernardi
** Modified date:           03/01/2020
** Version:                 v2.0
** Descriptions:            basic program for LCD and Touch Panel teaching
**
*********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "LPC17xx.h"
#include "GLCD/GLCD.h" 
#include "TouchPanel/TouchPanel.h"
#include "timer/timer.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef SIMULATOR
extern uint8_t ScaleFlag; // <- ScaleFlag needs to visible in order for the emulator to find the symbol (can be placed also inside system_LPC17xx.h but since it is RO, it needs more work)
#endif


int score = 0;
int lives = 1;
volatile int countdown = 60; // Global for timer use

void drawUI(void) {
    char buffer[20];

    // SCORE
    sprintf(buffer, "SCORE: %04d", score);
    GUI_Text(10, 0, (uint8_t *)buffer, White, Black);

    // TIME
    sprintf(buffer, "TIME: %02d", countdown);
    GUI_Text(240, 0, (uint8_t *)buffer, White, Black);

    // LIVES
    sprintf(buffer, "LIVES: %d", lives);
    GUI_Text(10, 220, (uint8_t *)buffer, White, Black);
}


void drawMaze(void) {
    uint16_t mazeColor = Blue;
    // Outer boundary (adjust coordinates as needed)
    LCD_DrawLine(10, 10, 310, 10, mazeColor);
    LCD_DrawLine(10, 10, 10, 230, mazeColor);
    LCD_DrawLine(310, 10, 310, 230, mazeColor);
    LCD_DrawLine(10, 230, 310, 230, mazeColor);

    // Internal “box” walls (example)
    LCD_DrawLine(140, 100, 180, 100, mazeColor);
    LCD_DrawLine(140, 140, 180, 140, mazeColor);
    LCD_DrawLine(140, 100, 140, 140, mazeColor);
    LCD_DrawLine(180, 100, 180, 140, mazeColor);


}

#define PILL_COLOR Yellow
static uint16_t pillPositions[240][2]; // Track pill coordinates

void placeStandardPills(void) {
    int count = 0;
    int x, y;
    for (y = 20; y < 220 && count < 240; y += 20) {
        for (x = 20; x < 300 && count < 240; x += 20) {
            LCD_SetPoint(x, y, PILL_COLOR);
            pillPositions[count][0] = x;
            pillPositions[count][1] = y;
            count++;
        }
    }
}

#define POWER_PILL_COLOR Magenta


void placePowerPills(void) {
    for (int i = 0; i < 6; i++) {
        int idx = rand() % 240; // Random index for power pill
        int px = pillPositions[idx][0];
        int py = pillPositions[idx][1];
        LCD_SetPoint(px, py, POWER_PILL_COLOR);
    }
		
}

void drawCentralBox(void){
		// example coordinates for a central box around the screen center
		int boxLeft = 140;
		int boxRight = 180;
		int boxTop = 100;
		int boxBottom = 140;
	
		LCD_DrawLine(boxLeft, boxTop, boxRight, boxTop, Blue);
		LCD_DrawLine(boxLeft, boxBottom, boxRight, boxBottom, Blue);
		LCD_DrawLine(boxLeft, boxBottom, boxLeft, boxBottom, Blue);
		LCD_DrawLine(boxRight, boxTop, boxRight, boxBottom, Blue);
}


int main(void)
{
  SystemInit();  												/* System Initialization (i.e., PLL)  */
	
  LCD_Initialization();
	
  	TP_Init();
//	TouchPanel_Calibrate();
	
	LCD_Clear(Black);
	
	// here we draw the maze
	drawMaze();
	placeStandardPills();
	placePowerPills();
	drawUI();
	
	
  GUI_Text(140, 180, (uint8_t *)"READY!", Yellow, Black);
	
	
	//GUI_Text(0, 280, (uint8_t *) " touch here : 1 sec to clear  ", Red, White);
	//LCD_DrawLine(0, 0, 200, 200, White);
	//init_timer(0, 0x1312D0 ); 						/* 50ms * 25MHz = 1.25*10^6 = 0x1312D0 */
	//init_timer(0, 0x6108 ); 						  /* 1ms * 25MHz = 25*10^3 = 0x6108 */
	init_timer(0, 0x4E2 ); 						    /* 500us * 25MHz = 1.25*10^3 = 0x4E2 */
	//init_timer(0, 0xC8 ); 						    /* 8us * 25MHz = 200 ~= 0xC8 */
	
	enable_timer(0);
	
	LPC_SC->PCON |= 0x1;									/* power-down	mode										*/
	LPC_SC->PCON &= ~(0x2);						
	
  while (1)	
  {
		__ASM("wfi");
  }
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
