#include "LPC17xx.h"
#include "GLCD/GLCD.h" 
#include "joystick/joystick.h"
#include "Ghost/ghost.h"
#include "RIT/RIT.h"
#include "button.h"
#include "TouchPanel/TouchPanel.h"
#include "timer/timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>   // For boolean data type

#ifdef SIMULATOR
extern uint8_t ScaleFlag;
#endif

#define ROWS 29
#define COLS 28

#define EMPTY 0
#define WALL  1
#define PILL  2
// Optionally define POWER_PILL if you want to add them later
#define POWER_PILL 3

// Cell dimensions (adjust as needed)
#define CELL_WIDTH 8
#define CELL_HEIGHT 10

int score = 0;
int lives = 1;
volatile int countdown = 60; // Global for timer use
volatile int powerPillsSpawned = 0;

volatile int down_0 = 0;  // For button debouncing
volatile bool gamePaused = true;
int totalPills = 240;  // Track remaining pills
int pillsEaten = 0;


int offsetX; 
int offsetY;

extern void initGhost(void);
extern void drawGhost(int offsetX, int offsetY);

volatile int mazeGrid[ROWS][COLS];

// Example maze layout (28 chars wide each line):
// 'X' = wall, ' ' = empty space, 'G' = ghost house area
static const char mazeDef[ROWS][COLS+1] = {
"XXXXXXXXXXXXXXXXXXXXXXXXXXXX", // 28 'X'
"X            XX            X",
"X XXXX XXXXX XX XXXXX XXXX X",
"X XXXX XXXXX XX XXXXX XXXX X",
"X                          X",
"X XXXX XX XXXXXXXX XX XXXX X",
"X XXXX XX XXXXXXXX XX XXXX X",
"X      XX    XX    XX      X",
"XXXXXX XXXXX XX XXXXX XXXXXX",
"XXXXXX XXXXX XX XXXXX XXXXXX",
"XXXXXX XX          XX XXXXXX",
"XXXXXX XX XXX  XXX XX XXXXXX",
"XXXXXX XX XGGGGGGX XX XXXXXX",
"          XGGGGGGX          ",
"XXXXXX XX XGGGGGGX XX XXXXXX",
"XXXXXX XX XGGGGGGX XX     XX",
"XXXXXX XX XXXXXXXX XX XXX XX",
"X      XX          XX XXX XX",
"X XXXX XX XXXXXXXX XX XXX XX",
"X XXXX XX XXXXXXXX XX XXX XX",
"X XXXX XX          XX     XX",
"X XXXX XX XXXXXXXX XX XXXXXX",
"X            XX            X",
"X XXXX XXXXX XX XXXXX XXXX X",
"X XXXX XXXXX XX XXXXX XXXX X",
"X   XX                XX   X",
"XXX XX XX XXXXXXXX XX XX XXX",
"X      XX    XX    XX      X",
"XXXXXXXXXXXXXXXXXXXXXXXXXXXX"
};


// // Pac-Man starting position 
int pacmanRow = 1;
int pacmanCol = 1;

volatile int pacmanDirRow=0;
volatile int pacmanDirCol=0;


// forward declarations
void drawUI(void);
void fillCell(int row, int col, int offsetX, int offsetY, uint16_t color);
void drawPill(int row, int col, int offsetX, int offsetY, uint16_t color, int pillsize);
void initMazeGrid(void);
void drawMazeFromGrid(int offsetX, int offsetY);

// Draw Score, Time, Lives
void drawUI(void) {
    char buffer[20];

    sprintf(buffer, "SCORE: %04d", score);
    GUI_Text(10, 0, (uint8_t *)buffer, White, Black);

    sprintf(buffer, "TIME: %02d", countdown);
    GUI_Text(240 - 100, 0, (uint8_t *)buffer, White, Black);

    sprintf(buffer, "LIVES: %d", lives);
    GUI_Text(10, 320 - 15, (uint8_t *)buffer, White, Black);
}

// Fill a cell with solid color
void fillCell(int row, int col, int offsetX, int offsetY, uint16_t color) {
    int px, py;
    int startX = offsetX + col * CELL_WIDTH;
    int startY = offsetY + row * CELL_HEIGHT;
    for (py = 0; py < CELL_HEIGHT; py++) {
        for (px = 0; px < CELL_WIDTH; px++) {
            int drawX = startX + px;
            int drawY = startY + py;
            if (drawX < 240 && drawY < 320) {
                LCD_SetPoint(drawX, drawY, color);
            }
        }
    }
}

void drawPill(int row, int col, int offsetX, int offsetY, uint16_t color, int pillSize) {
    int hx, vy;
    int startX = offsetX + col * CELL_WIDTH;
    int startY = offsetY + row * CELL_HEIGHT;
    int centerX = startX + (CELL_WIDTH / 2);
    int centerY = startY + (CELL_HEIGHT / 2);
    for (hx = -pillSize; hx <= pillSize; hx++) {
        for (vy = -pillSize; vy <= pillSize; vy++) {
            int drawX = centerX + hx;
            int drawY = centerY + vy;
            if (drawX >= 0 && drawX < 240 && drawY >= 0 && drawY < 320) {
                LCD_SetPoint(drawX, drawY, color);
            }
        }
    }
}

void initMazeGrid(void) {
    int r, c;
    int pillCount = 0;

    for (r = 0; r < ROWS; r++) {
        for (c = 0; c < COLS; c++) {
            char cell = mazeDef[r][c];
            if (cell == 'X') {
                mazeGrid[r][c] = WALL;
            } else if (cell == 'G') {
                // Ghost house area as WALL so no pills appear inside
                mazeGrid[r][c] = EMPTY;
            } else {
                // ' ' = empty floor, initially mark EMPTY
                mazeGrid[r][c] = EMPTY; 
            }
        }
    }

		for (r = 0; r < ROWS; r++) {
				for (c = 0; c < COLS; c++) {
						if (mazeGrid[r][c] == EMPTY && mazeDef[r][c] != 'G') {  // Check original maze definition
								mazeGrid[r][c] = PILL;
								pillCount++;
						}
				}
		}

    // Print out the pill count for debugging, giving error
    //printf("Total Pills: %d\n", pillCount);

//		srand(LPC_TIM0->TC); // Here we can assign a seed for randomizing check later how 
//    int powerPillsNeeded = 6;
//    while (powerPillsNeeded > 0) {
//        int rr = rand() % ROWS;
//        int cc = rand() % COLS;
//        if (mazeGrid[rr][cc] == PILL) {
//            mazeGrid[rr][cc] = POWER_PILL;
//            powerPillsNeeded--;
//        }
//    }
}

void drawPowerPills(){
		if ((rand() % 5) == 0) {  
            // find a random cell that is still a standard pill
            while (1) {
                int rr = rand() % ROWS;
                int cc = rand() % COLS;
                if (mazeGrid[rr][cc] == PILL) {
                    mazeGrid[rr][cc] = POWER_PILL;
                    // Immediately draw it:
                    fillCell(rr, cc, offsetX, offsetY, Black);
                    drawPill(rr, cc, offsetX, offsetY, Red, 3);
                    powerPillsSpawned++;
                    break;
                }
            }
        }
}

void drawMazeFromGrid(int offsetX, int offsetY) {
    int r, c;
    for (r = 0; r < ROWS; r++) {
        for (c = 0; c < COLS; c++) {
            int cellVal = mazeGrid[r][c];
            if (cellVal == WALL) {
                fillCell(r, c, offsetX, offsetY, Blue);
					} else if (cellVal == PILL) {
							fillCell(r, c, offsetX, offsetY, Black);
							// Standard pill size (e.g., 1)
							drawPill(r, c, offsetX, offsetY, Yellow, 1);
					} else if (cellVal == POWER_PILL) {
							fillCell(r, c, offsetX, offsetY, Black);
							// Power pill larger size (e.g., 3)
							drawPill(r, c, offsetX, offsetY, Red, 3);


            } else {
                // EMPTY or any other cell
                fillCell(r, c, offsetX, offsetY, Black);
            }
        }
    }
}

void drawPacMan(int row, int col, int offsetX, int offsetY) {
    int startX = offsetX + col * CELL_WIDTH;
    int startY = offsetY + row * CELL_HEIGHT;
    int radius = 3; // small radius
		int dy, dx;
    for (dy = -radius; dy <= radius; dy++) {
        for (dx = -radius; dx <= radius; dx++) {
            if (dx*dx + dy*dy <= radius*radius) {
                int drawX = startX + (CELL_WIDTH/2) + dx;
                int drawY = startY + (CELL_HEIGHT/2) + dy;
                if (drawX >= 0 && drawX < 240 && drawY >= 0 && drawY < 320) {
                    LCD_SetPoint(drawX, drawY, Yellow);
                }
            }
        }
    }
}



bool movePacMan(void){
    // Here we calculate the new position based on the current position
    int newRow = pacmanRow + pacmanDirRow;
    int newCol = pacmanCol + pacmanDirCol;
			
		// Here we also check if we have won
		if(pillsEaten >= totalPills){
				GUI_Text((240/2)-30, (320/2)-10, (uint8_t *)"Victory!", Yellow, Black);
				
				// Stop the game
				gamePaused=true;
				disable_RIT();      // 
				disable_timer(0);   // so countdown stops, etc.

				return false;
		}
    // here we check if its teleport location
    if(newRow == 13 && newCol == 28){
        fillCell(pacmanRow, pacmanCol, offsetX, offsetY, Black); // Clear old position
        pacmanRow = newRow;
        pacmanCol = 0;
        // Check if there's a pill at the new position
        if(mazeGrid[pacmanRow][pacmanCol] == PILL) {
            score += 10;
						pillsEaten++;
            mazeGrid[pacmanRow][pacmanCol] = EMPTY;
        } else if(mazeGrid[pacmanRow][pacmanCol] == POWER_PILL) {
            score += 50;
						pillsEaten++;
            mazeGrid[pacmanRow][pacmanCol] = EMPTY;
						blinky.isChasing = false;
						blinky.frightenedTimer = 200;
        }
        drawPacMan(pacmanRow, pacmanCol, offsetX, offsetY);
        drawUI();
        return true;
    }
    if(newRow == 13 && newCol == -1){
        fillCell(pacmanRow, pacmanCol, offsetX, offsetY, Black); // Clear old position
        pacmanRow = newRow;
        pacmanCol = 27;
        // Check if there's a pill at the new position
        if(mazeGrid[pacmanRow][pacmanCol] == PILL) {
            score += 10;
            mazeGrid[pacmanRow][pacmanCol] = EMPTY;
        } else if(mazeGrid[pacmanRow][pacmanCol] == POWER_PILL) {
            score += 50;
            mazeGrid[pacmanRow][pacmanCol] = EMPTY;
        }
        drawPacMan(pacmanRow, pacmanCol, offsetX, offsetY);
        drawUI();
        return true;
    }

    // here we check if the new position is within the bounds and not a wall
    if(newRow >= 0 && newRow < ROWS && newCol >= 0 && newCol < COLS && mazeGrid[newRow][newCol] != WALL){
        // here we clear the old pacman position 
        if (pacmanRow != newRow || pacmanCol != newCol) {
            // Check for pills before moving
            if(mazeGrid[newRow][newCol] == PILL) {
                score += 10;
								pillsEaten++;
                mazeGrid[newRow][newCol] = EMPTY;
            } else if(mazeGrid[newRow][newCol] == POWER_PILL) {
                score += 50;
								pillsEaten++;
                mazeGrid[newRow][newCol] = EMPTY;
            }
						
						// check for extra lives
						if(score>0 && score%1000==0){
							lives++;
							drawUI();
						}
            
            fillCell(pacmanRow, pacmanCol, offsetX, offsetY, Black); // Clear old position
            pacmanRow = newRow;
            pacmanCol = newCol;
            drawPacMan(pacmanRow, pacmanCol, offsetX, offsetY); // Draw at new position
            
            // Update UI if we collected anything
            if(mazeGrid[newRow][newCol] != EMPTY) {
                drawUI();
            }
        }
        return true; 
    }
    return false;
}


int main(void) {

    SystemInit();   
    LCD_Initialization();
    TP_Init();
		BUTTON_init();
    LCD_Clear(Black);
	//	init_RIT(0x004C4B40);	// 50ms
		init_RIT(0x000F4240 );	// 50ms
//	
		enable_RIT();
    joystick_init(); // NEW: Initialize joystick
		
		offsetX = (240 - (COLS * CELL_WIDTH)) / 2; 
    offsetY = (320 - (ROWS * CELL_HEIGHT)) / 2;

    initMazeGrid();
    drawMazeFromGrid(offsetX, offsetY);
		drawUI();
    drawPacMan(pacmanRow, pacmanCol, offsetX, offsetY);
		initGhost(); // initializes the ghost blinky
		drawGhost(offsetX, offsetY); // draws the initial ghost position

    // ready message
    GUI_Text((240/2)-23, (320/2)-10, (uint8_t *)"READY", Yellow, Black);

    init_timer(0, 0x1312D0);
    //enable_timer(0);

    // Set initial direction to nothing
    pacmanDirRow = 0;
    pacmanDirCol = 0;

		LPC_SC->PCON |= 0x1;  /* Enter power-down mode */
    LPC_SC->PCON &= ~(0x2);

		while (1) {
				__ASM("wfi");}
    return 0;
}


/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
