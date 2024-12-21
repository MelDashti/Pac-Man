
#include "LPC17xx.h"
#include "GLCD/GLCD.h" 
#include "joystick/joystick.h"
#include "RIT/RIT.h"
#include "TouchPanel/TouchPanel.h"
#include "timer/timer.h"
#include <stdio.h>
#include <stdlib.h>

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

static int mazeGrid[ROWS][COLS];

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
"XXXXXX XX XXXXXXXX XX XXXXXX",
"XXXXXX XX XGGGGGGX XX XXXXXX",
"          XGGGGGGX          ",
"XXXXXX XX XGGGGGGX XX XXXXXX",
"XXXXXX XX XGGGGGGX XX     XX",
"XXXXXX XX XXXXXXXX	XX XXX XX",
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
int pacmanRow = 15;
int pacmanCol = 14;


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
                mazeGrid[r][c] = WALL;
            } else {
                // ' ' = empty floor, initially mark EMPTY
                mazeGrid[r][c] = EMPTY; 
            }
        }
    }

    // Fill empty spaces with pills
    for (r = 0; r < ROWS; r++) {
        for (c = 0; c < COLS; c++) {
            if (mazeGrid[r][c] == EMPTY) {
                mazeGrid[r][c] = PILL;
                pillCount++;
            }
        }
    }

    // Print out the pill count for debugging, giving error
    //printf("Total Pills: %d\n", pillCount);

    srand(1); // Here we can assign a seed for randomizing check later how 
    int powerPillsNeeded = 6;
    while (powerPillsNeeded > 0) {
        int rr = rand() % ROWS;
        int cc = rand() % COLS;
        if (mazeGrid[rr][cc] == PILL) {
            mazeGrid[rr][cc] = POWER_PILL;
            powerPillsNeeded--;
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



volatile int pacmanDirRow=0;
volatile int pacmanDirCol=0;

int main(void) {

    SystemInit();   
    LCD_Initialization();
    TP_Init();
    LCD_Clear(Black);

    joystick_init(); // NEW: Initialize joystick
		
		init_RIT(0x004C4B40);  // ~50ms at 100MHz
		enable_RIT();
		int offsetX = (240 - (COLS * CELL_WIDTH)) / 2; 
    int offsetY = (320 - (ROWS * CELL_HEIGHT)) / 2;

    initMazeGrid();
    drawMazeFromGrid(offsetX, offsetY);
    drawUI();
    drawPacMan(pacmanRow, pacmanCol, offsetX, offsetY);

    // ready message
    GUI_Text((240/2)-20, (320/2)-20, (uint8_t *)"READY!", Yellow, Black);

    init_timer(0, 0x1312D0);
    enable_timer(0);

    // Set initial direction to nothing
    pacmanDirRow = 0;
    pacmanDirCol = 0;

		LPC_SC->PCON |= 0x1;  /* Enter power-down mode */
    LPC_SC->PCON &= ~(0x2);

		while (1) {
				__ASM("wfi");

				// After waking up from interrupt:
				// Check if pacmanDirRow or pacmanDirCol changed.
				// Attempt to move Pac-Man one cell in the chosen direction:
				int newRow = pacmanRow + pacmanDirRow;
				int newCol = pacmanCol + pacmanDirCol;

				// Handle wrapping:
				if (newRow < 0) newRow = ROWS - 1;
				if (newRow >= ROWS) newRow = 0;
				if (newCol < 0) newCol = COLS - 1;
				if (newCol >= COLS) newCol = 0;

				// Check if the new cell is a wall:
				if (mazeGrid[newRow][newCol] != WALL) {
						// It's safe to move
						pacmanRow = newRow;
						pacmanCol = newCol;

						// If there's a pill, increment score, remove pill
						if (mazeGrid[newRow][newCol] == PILL || mazeGrid[newRow][newCol] == POWER_PILL) {
								score += (mazeGrid[newRow][newCol] == PILL) ? 10 : 50;
								mazeGrid[newRow][newCol] = EMPTY; 
								drawUI(); // Update score on UI
						}

						// Redraw Pac-Man at new position
						drawMazeFromGrid(offsetX, offsetY);
						drawPacMan(pacmanRow, pacmanCol, offsetX, offsetY);
				}
				// If it's a wall, Pac-Man stays in the same place, no movement.
		}
    return 0;
}


/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
