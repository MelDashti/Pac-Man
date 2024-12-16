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

    int textOffset = 5; // Space between text and maze borders

    // SCORE (Top-Left)
    sprintf(buffer, "SCORE: %04d", score);
    GUI_Text(10, 0, (uint8_t *)buffer, White, Black); // Placed above top-left corner

    // TIME (Top-Right)
    sprintf(buffer, "TIME: %02d", countdown);
    GUI_Text(MAX_X - 100, 0, (uint8_t *)buffer, White, Black); // Placed above top-right corner

    // LIVES (Bottom-Left)
    sprintf(buffer, "LIVES: %d", lives);
    GUI_Text(10, MAX_Y - 15, (uint8_t *)buffer, White, Black); // Placed below bottom-left corner
}


void drawCentralBox(void) {
    int boxWidth = 40;
    int boxHeight = 40;

    int boxLeft = (MAX_X / 2) - (boxWidth / 2);
    int boxRight = (MAX_X / 2) + (boxWidth / 2);
    int boxTop = (MAX_Y / 2) - (boxHeight / 2);
    int boxBottom = (MAX_Y / 2) + (boxHeight / 2);

    LCD_DrawLine(boxLeft, boxTop, boxRight, boxTop, Blue);
    LCD_DrawLine(boxLeft, boxBottom, boxRight, boxBottom, Blue);
    LCD_DrawLine(boxLeft, boxTop, boxLeft, boxBottom, Blue);
    LCD_DrawLine(boxRight, boxTop, boxRight, boxBottom, Blue);
}


void drawSmallRectangle(int startX, int startY, int width, int height, uint16_t color) {
    int endX = startX + width;
    int endY = startY + height;

    // Draw rectangle
    LCD_DrawLine(startX, startY, endX, startY, color); // Top side
    LCD_DrawLine(startX, endY, endX, endY, color);    // Bottom side
    LCD_DrawLine(startX, startY, startX, endY, color); // Left side
    LCD_DrawLine(endX, startY, endX, endY, color);    // Right side
}

void drawLShapedBox(int startX, int startY, int width, int height, int thickness, uint16_t color) {
    // Vertical segment of the L (with closed ends)
    LCD_DrawLine(startX, startY, startX + thickness, startY, color);                  // Top edge (closed)
    LCD_DrawLine(startX, startY, startX, startY + height, color);                     // Left edge
    LCD_DrawLine(startX + thickness, startY, startX + thickness, startY + height, color); // Right edge
    LCD_DrawLine(startX, startY + height, startX + thickness, startY + height, color);    // Bottom edge

    // Horizontal segment of the L (with closed ends)
    int bottomY = startY + height - thickness; // Align horizontal part to bottom of vertical
    LCD_DrawLine(startX, bottomY, startX + width, bottomY, color);                    // Top edge of horizontal
    LCD_DrawLine(startX, bottomY + thickness, startX + width, bottomY + thickness, color); // Bottom edge of horizontal
    LCD_DrawLine(startX + width, bottomY, startX + width, bottomY + thickness, color);    // Right edge (closed)
}


void drawTShapedBox(int startX, int startY, int width, int height, int thickness, uint16_t color) {
    int centerX = startX + (width / 2); // Calculate center X for the vertical part

    // Top horizontal rectangle (fully closed sides)
    LCD_DrawLine(startX, startY, startX + width, startY, color);                 // Top edge
    LCD_DrawLine(startX, startY + thickness, startX + width, startY + thickness, color); // Bottom edge
    LCD_DrawLine(startX, startY, startX, startY + thickness, color);             // Left side (closed)
    LCD_DrawLine(startX + width, startY, startX + width, startY + thickness, color); // Right side (closed)

    // Vertical rectangle (centered below the horizontal part)
    LCD_DrawLine(centerX - (thickness / 2), startY + thickness, centerX - (thickness / 2), startY + height, color); // Left side
    LCD_DrawLine(centerX + (thickness / 2), startY + thickness, centerX + (thickness / 2), startY + height, color); // Right side
    LCD_DrawLine(centerX - (thickness / 2), startY + height, centerX + (thickness / 2), startY + height, color);   // Bottom edge
}



void drawMaze(void) {
    uint16_t mazeColor = Blue;

    int topBottomMargin = 20;  
    int leftRightMargin = 2;   
    int offset = 4;            
    int indentWidth = 20;      

    // Vertical boundaries
    int topY = topBottomMargin;
    int bottomY = MAX_Y - topBottomMargin;

    // Define two protrusions on the left side:
    // Each protrusion is defined by a top and bottom Y coordinate where indentation occurs.
    int protrusion1TopLeft = 80;
    int protrusion1BottomLeft = 120;
    int protrusion2TopLeft = 160;
    int protrusion2BottomLeft = 200;

    // Same protrusions on the right side (mirrored)
    int protrusion1TopRight = protrusion1TopLeft;
    int protrusion1BottomRight = protrusion1BottomLeft;
    int protrusion2TopRight = protrusion2TopLeft;
    int protrusion2BottomRight = protrusion2BottomLeft;

    // ==== Outer Border ====
    // Top horizontal line
    LCD_DrawLine(leftRightMargin, topY, MAX_X - leftRightMargin, topY, mazeColor);

    // Bottom horizontal line
    LCD_DrawLine(leftRightMargin, bottomY, MAX_X - leftRightMargin, bottomY, mazeColor);

    // ---- LEFT OUTER BORDER with two protrusions ----
    // From topY down to protrusion1Top
    LCD_DrawLine(leftRightMargin, topY, leftRightMargin, protrusion1TopLeft, mazeColor);
    // Protrusion 1 (left side)
    LCD_DrawLine(leftRightMargin, protrusion1TopLeft, leftRightMargin + indentWidth, protrusion1TopLeft, mazeColor);
    LCD_DrawLine(leftRightMargin + indentWidth, protrusion1TopLeft, leftRightMargin + indentWidth, protrusion1BottomLeft, mazeColor);
    LCD_DrawLine(leftRightMargin + indentWidth, protrusion1BottomLeft, leftRightMargin, protrusion1BottomLeft, mazeColor);

    // Continue down to protrusion2Top
    LCD_DrawLine(leftRightMargin, protrusion1BottomLeft, leftRightMargin, protrusion2TopLeft, mazeColor);

    // Protrusion 2 (left side)
    LCD_DrawLine(leftRightMargin, protrusion2TopLeft, leftRightMargin + indentWidth, protrusion2TopLeft, mazeColor);
    LCD_DrawLine(leftRightMargin + indentWidth, protrusion2TopLeft, leftRightMargin + indentWidth, protrusion2BottomLeft, mazeColor);
    LCD_DrawLine(leftRightMargin + indentWidth, protrusion2BottomLeft, leftRightMargin, protrusion2BottomLeft, mazeColor);

    // Down to bottom
    LCD_DrawLine(leftRightMargin, protrusion2BottomLeft, leftRightMargin, bottomY, mazeColor);

    // ---- RIGHT OUTER BORDER with two protrusions (mirrored) ----
    // From topY down to protrusion1Top
    LCD_DrawLine(MAX_X - leftRightMargin, topY, MAX_X - leftRightMargin, protrusion1TopRight, mazeColor);
    // Protrusion 1 (right side) mirrored inward
    LCD_DrawLine(MAX_X - leftRightMargin, protrusion1TopRight, MAX_X - leftRightMargin - indentWidth, protrusion1TopRight, mazeColor);
    LCD_DrawLine(MAX_X - leftRightMargin - indentWidth, protrusion1TopRight, MAX_X - leftRightMargin - indentWidth, protrusion1BottomRight, mazeColor);
    LCD_DrawLine(MAX_X - leftRightMargin - indentWidth, protrusion1BottomRight, MAX_X - leftRightMargin, protrusion1BottomRight, mazeColor);

    // Down to protrusion2Top
    LCD_DrawLine(MAX_X - leftRightMargin, protrusion1BottomRight, MAX_X - leftRightMargin, protrusion2TopRight, mazeColor);

    // Protrusion 2 (right side)
    LCD_DrawLine(MAX_X - leftRightMargin, protrusion2TopRight, MAX_X - leftRightMargin - indentWidth, protrusion2TopRight, mazeColor);
    LCD_DrawLine(MAX_X - leftRightMargin - indentWidth, protrusion2TopRight, MAX_X - leftRightMargin - indentWidth, protrusion2BottomRight, mazeColor);
    LCD_DrawLine(MAX_X - leftRightMargin - indentWidth, protrusion2BottomRight, MAX_X - leftRightMargin, protrusion2BottomRight, mazeColor);

    // Down to bottom
    LCD_DrawLine(MAX_X - leftRightMargin, protrusion2BottomRight, MAX_X - leftRightMargin, bottomY, mazeColor);


    // ==== Inner Border (Parallel to Outer, with offset) ====

    // Inner border top and bottom lines
    int topY_in = topY + offset;
    int bottomY_in = bottomY - offset;

    LCD_DrawLine(leftRightMargin + offset, topY_in, MAX_X - leftRightMargin - offset, topY_in, mazeColor);
    LCD_DrawLine(leftRightMargin + offset, bottomY_in, MAX_X - leftRightMargin - offset, bottomY_in, mazeColor);

    // Inner border protrusions:
    // For the left side, adjust protrusion coordinates to prevent intersection
    int leftMargin_in = leftRightMargin + offset;
    int leftIndent_in = leftRightMargin + indentWidth - offset;

    // Left inner border with two protrusions
    // Segment above protrusion1
    LCD_DrawLine(leftMargin_in, topY_in, leftMargin_in, protrusion1TopLeft - offset, mazeColor);
    // Protrusion 1 (inner)
    LCD_DrawLine(leftMargin_in, protrusion1TopLeft - offset, leftIndent_in, protrusion1TopLeft - offset, mazeColor);
    LCD_DrawLine(leftIndent_in, protrusion1TopLeft - offset, leftIndent_in, protrusion1BottomLeft + offset, mazeColor);
    LCD_DrawLine(leftIndent_in, protrusion1BottomLeft + offset, leftMargin_in, protrusion1BottomLeft + offset, mazeColor);

    // Between protrusions
    LCD_DrawLine(leftMargin_in, protrusion1BottomLeft + offset, leftMargin_in, protrusion2TopLeft - offset, mazeColor);

    // Protrusion 2 (inner)
    LCD_DrawLine(leftMargin_in, protrusion2TopLeft - offset, leftIndent_in, protrusion2TopLeft - offset, mazeColor);
    LCD_DrawLine(leftIndent_in, protrusion2TopLeft - offset, leftIndent_in, protrusion2BottomLeft + offset, mazeColor);
    LCD_DrawLine(leftIndent_in, protrusion2BottomLeft + offset, leftMargin_in, protrusion2BottomLeft + offset, mazeColor);

    // Down to bottom_in
    LCD_DrawLine(leftMargin_in, protrusion2BottomLeft + offset, leftMargin_in, bottomY_in, mazeColor);

    // For the right side inner border:
    int rightMargin_in = MAX_X - leftRightMargin - offset;
    int rightIndent_in = MAX_X - leftRightMargin - indentWidth + offset;

    // Right inner border with two protrusions
    // Segment above protrusion1 (right)
    LCD_DrawLine(rightMargin_in, topY_in, rightMargin_in, protrusion1TopRight - offset, mazeColor);
    // Protrusion 1 (inner, right)
    LCD_DrawLine(rightMargin_in, protrusion1TopRight - offset, rightIndent_in, protrusion1TopRight - offset, mazeColor);
    LCD_DrawLine(rightIndent_in, protrusion1TopRight - offset, rightIndent_in, protrusion1BottomRight + offset, mazeColor);
    LCD_DrawLine(rightIndent_in, protrusion1BottomRight + offset, rightMargin_in, protrusion1BottomRight + offset, mazeColor);

    // Between protrusions on right
    LCD_DrawLine(rightMargin_in, protrusion1BottomRight + offset, rightMargin_in, protrusion2TopRight - offset, mazeColor);

    // Protrusion 2 (inner, right)
    LCD_DrawLine(rightMargin_in, protrusion2TopRight - offset, rightIndent_in, protrusion2TopRight - offset, mazeColor);
    LCD_DrawLine(rightIndent_in, protrusion2TopRight - offset, rightIndent_in, protrusion2BottomRight + offset, mazeColor);
    LCD_DrawLine(rightIndent_in, protrusion2BottomRight + offset, rightMargin_in, protrusion2BottomRight + offset, mazeColor);

    // Down to bottom_in
    LCD_DrawLine(rightMargin_in, protrusion2BottomRight + offset, rightMargin_in, bottomY_in, mazeColor);


		// we add other shapes
		// Add Central Box
    drawCentralBox();

  // ==== Small Rectangles ====
    // Adjust the positions and sizes to prevent overlap
    drawSmallRectangle(30, 60, 30, 20, mazeColor);                   // Top-left
    drawSmallRectangle(MAX_X - 60, 60, 30, 20, mazeColor);           // Top-right
    drawSmallRectangle(30, MAX_Y - 70, 30, 20, mazeColor);           // Bottom-left
    drawSmallRectangle(MAX_X - 60, MAX_Y - 70, 30, 20, mazeColor);   // Bottom-right

    // ==== L-Shaped Boxes ====
    // Adjust the positions and sizes to prevent overlap
    drawLShapedBox(40, 100, 30, 40, 8, mazeColor);                   // Left-middle
    drawLShapedBox(MAX_X - 100, 100, 50, 30, 6, mazeColor);          // Right-middle
    drawLShapedBox(40, MAX_Y - 150, 40, 50, 8, mazeColor);           // Bottom-left
    drawLShapedBox(MAX_X - 110, MAX_Y - 100, 40, 50, 10, mazeColor);  // Bottom-right

    // ==== T-Shaped Boxes ====
    // Adjust the positions and sizes to prevent overlap
    drawTShapedBox((MAX_X / 2) - 25, 70, 60, 50, 9, mazeColor);      // Top-center
    drawTShapedBox(70, MAX_Y - 85, 50, 40, 6, mazeColor);           // Bottom-left-center
    drawTShapedBox(MAX_X - 100, MAX_Y - 115, 50, 40, 6, mazeColor);  // Bottom-right-center

}





#define PILL_COLOR Yellow
#define PILL_SIZE 1 // better to use define as it will be constant and set at compile time.

void drawFilledSquare(int x, int y, uint16_t color) {
  int hx, vy;  // hx is horizontal offset relative to center and vy is to vertical 
	for (hx = -PILL_SIZE; hx <= PILL_SIZE; hx++) {
        for (vy = -PILL_SIZE; vy <= PILL_SIZE; vy++) {
            if ((x + hx >= 0 && x + hx < MAX_X) && (y + vy>= 0 && y + vy < MAX_Y)) {
                LCD_SetPoint(x + hx, y + vy, color); // Draw a pixel within bounds
            }
        }
    }
}
int isInsideWall(int x, int y) {
    // ==== Check for Outer Boundaries ====
    if (x <= 2 || x >= MAX_X - 2 || y <= 20 || y >= MAX_Y - 20) {
        return 1; // Outside maze boundaries
    }

    // ==== Check for Left Protrusions ====
    if ((x >= 2 && x <= 22 && y >= 80 && y <= 120) ||     // Left Protrusion 1
        (x >= 2 && x <= 22 && y >= 160 && y <= 200)) {    // Left Protrusion 2
        return 1;
    }

    // ==== Check for Right Protrusions ====
    if ((x >= MAX_X - 22 && x <= MAX_X - 2 && y >= 80 && y <= 120) ||     // Right Protrusion 1
        (x >= MAX_X - 22 && x <= MAX_X - 2 && y >= 160 && y <= 200)) {    // Right Protrusion 2
        return 1;
    }

    // ==== Check for Central Box ====
    int boxWidth = 40, boxHeight = 40;
    int boxLeft = (MAX_X / 2) - (boxWidth / 2);
    int boxRight = (MAX_X / 2) + (boxWidth / 2);
    int boxTop = (MAX_Y / 2) - (boxHeight / 2);
    int boxBottom = (MAX_Y / 2) + (boxHeight / 2);
    if (x >= boxLeft && x <= boxRight && y >= boxTop && y <= boxBottom) {
        return 1;
    }

    // ==== Check for Small Rectangles ====
    if ((x >= 30 && x <= 60 && y >= 60 && y <= 80) ||                         // Top-left
        (x >= MAX_X - 60 && x <= MAX_X - 30 && y >= 60 && y <= 80) ||         // Top-right
        (x >= 30 && x <= 60 && y >= MAX_Y - 70 && y <= MAX_Y - 50) ||         // Bottom-left
        (x >= MAX_X - 60 && x <= MAX_X - 30 && y >= MAX_Y - 70 && y <= MAX_Y - 50)) { // Bottom-right
        return 1;
    }

    // ==== Check for L-Shaped Boxes ====
    if ((x >= 40 && x <= 70 && y >= 100 && y <= 140) ||                       // Left-middle
        (x >= MAX_X - 100 && x <= MAX_X - 50 && y >= 100 && y <= 130) ||      // Right-middle
        (x >= 40 && x <= 80 && y >= MAX_Y - 150 && y <= MAX_Y - 100) ||       // Bottom-left
        (x >= MAX_X - 110 && x <= MAX_X - 70 && y >= MAX_Y - 100 && y <= MAX_Y - 50)) { // Bottom-right
        return 1;
    }

    // ==== Check for T-Shaped Boxes ====
    if ((x >= (MAX_X / 2) - 25 && x <= (MAX_X / 2) + 35 && y >= 70 && y <= 120) ||  // Top-center
        (x >= 70 && x <= 120 && y >= MAX_Y - 85 && y <= MAX_Y - 45) ||             // Bottom-left-center
        (x >= MAX_X - 100 && x <= MAX_X - 50 && y >= MAX_Y - 115 && y <= MAX_Y - 75)) { // Bottom-right-center
        return 1;
    }

    return 0; // Not inside any wall or shape
}




static uint16_t pillPositions[240][2]; // Track pill coordinates

void placeStandardPills(void) {
    int count = 0;
    int x, y;

    // Dynamically calculate spacing to distribute exactly 240 pills
    int rows = 22; // Number of rows (adjust based on maze height)
    int cols = 25; // Number of columns (adjust based on maze width)

    int pillSpacingX = (MAX_X - 40) / cols; // Horizontal spacing
    int pillSpacingY = (MAX_Y - 40) / rows; // Vertical spacing

    // Start placing pills row by row
		for (y = 40; y <= MAX_Y - 40 && count < 250; y += pillSpacingY) {
				for (x = 40; x <= MAX_X - 40 && count < 250; x += pillSpacingX) {
						if (!isInsideWall(x, y)) { // Check if position is valid
                drawFilledSquare(x, y, PILL_COLOR);
                pillPositions[count][0] = x;
                pillPositions[count][1] = y;
                count++;
            }
        }
    }
		while (count < 250) {
        for (y = 40; y <= MAX_Y - 40; y++) {
            for (x = 40; x <= MAX_X - 40; x++) {
                if (!isInsideWall(x, y)) {
                    drawFilledSquare(x, y, PILL_COLOR);
                    pillPositions[count][0] = x;
                    pillPositions[count][1] = y;
                    count++;
                    if (count >= 250) {
                        return;
                    }
                }
            }
        }
    }

		
}


#define POWER_PILL_COLOR Magenta
void placePowerPills(void) {
    int placed = 0;

    while (placed < 6) {
        int idx = rand() % 240;
        int px = pillPositions[idx][0];
        int py = pillPositions[idx][1];

        if (!isInsideWall(px, py)) { // Check if position is valid
            drawFilledSquare(px, py, POWER_PILL_COLOR);
            placed++;
        }
    }
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
	drawCentralBox();
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
