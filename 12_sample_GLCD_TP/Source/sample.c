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
#include <time.h> // for random power pill position we use this to generate random positions
#include <../music/music.h>
#include <math.h>
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


int pillCount=0;
volatile int score = 0;
volatile int lives = 1;
volatile int countdown = 60; // Global for timer use
volatile int powerPillsSpawned = 0;

volatile int down_0 = 0;  // For button debouncing
volatile bool gamePaused = true;
volatile bool gameOver = false;
int totalPills = 240;  // Track remaining pills
int pillsEaten = 0;


int offsetX; 
int offsetY;

volatile int mazeGrid[ROWS][COLS];

// Example maze layout (28 chars wide each line):
// 'X' = wall, ' ' = empty space, 'G' = ghost house area
static const char mazeDef[ROWS][COLS+1] = {
"XXXXXXXXXXXXXXXXXXXXXXXXXXXX", // 28 'X'
"X            XX           XX",
"X XXXX XXXXX XX XXXXX XXXXXX",
"X XXXX XXXXX XX XXXXX XXXXXX",
"X     G              G     X",
"X XXXX XX XXXXXXXX XX XXXX X",
"X XXXX XX XXXXXXXX XX XXXX X",
"X      XX    XX    XX      X",
"XXXXXX XXXXX XX XXXXX XXXXXX",
"XXXXXX XXXXX XX XXXXX XXXXXX",
"XXXXXX XXGGGGGGGGGGXX XXXXXX",
"XXXXXX XXGXXXGGXXXGXX XXXXXX",
"XXXXXX XXGXGGGGGGXGXX XXXXXX",
"GGGG   GGGXGGGGGGXGG    GGGG",
"XXXXXX XXGXGGGGGGXGXX XXXXXX",
"XXXXXX XXGXGGGGGGXGXX     XX",
"XXXXXX XXGXXXXXXXXGXX XXX XX",
"X      XXGGGGGGGGGGXX XXX XX",
"X XXXX XX XXXXXXXX XX XXX XX",
"X XXXX XX XXXXXXXX XX XXX XX",
"X XXXX XX          XX     XX",
"X XXXX XX XXXXXXXX XX XXXXXX",
"X     G      XX      G     X",
"X XXXX XXXXX XX XXXXX XXXX X",
"X XXXX XXXXX XX XXXXX XXXX X",
"X   XX                XXXX X",
"XXX XX XX XXXXXXXX XX XXXX X",
"XXX    XX    XX    XX      X",
"XXXXXXXXXXXXXXXXXXXXXXXXXXXX"
};



NOTE pacman_wakka[] = {
    {e4, time_semibiscroma},
    {pause, time_semibiscroma}
};

NOTE power_pill_sound[] = {
    {b4, time_semicroma},
    {e4, time_semicroma},
    {b4, time_semicroma}
};



NOTE game_start[] = {
    {c4, time_semicroma},
    {e4, time_semicroma},
    {g4, time_semicroma},
    {c5, time_semiminima}
};

NOTE victory_sound[] = {
    {c4, time_semicroma},
    {e4, time_semicroma},
    {g4, time_semicroma},
    {c5, time_semicroma}
};

// // Pac-Man starting position 
int pacmanRow = 1;
int pacmanCol = 1;

volatile int pacmanDirRow=0;
volatile int pacmanDirCol=0;
int lastLifeMilestone = 0; // Tracks the last score milestone where a life was added


// forward declarations
void drawUI(void);
void fillCell(int row, int col, int offsetX, int offsetY, uint16_t color);
void drawPill(int row, int col, int offsetX, int offsetY, uint16_t color, int pillsize);
void initMazeGrid(void);
void drawMazeFromGrid(int offsetX, int offsetY);


// Draw Score, Time, Lives
void drawUI() {
    char buffer[40];

    sprintf(buffer, "SCORE: %4d", score);
	//	replace_zero(buffer);
    GUI_Text(10, 0, (uint8_t *)buffer, White, Black);

    sprintf(buffer, "TIME: %2d", countdown);
	//	replace_zero(buffer);
    GUI_Text(240 - 100, 0, (uint8_t *)buffer, White, Black);

    sprintf(buffer, "LIVES: %d", lives);
	//	replace_zero(buffer);
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
		srand(time(NULL));
    int r, c;

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
    char buffer[20];

  // sprintf(buffer, "PILLS: %02d", pillCount);
  // GUI_Text(20, 0, (uint8_t *)buffer, White, Black);
		
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
    int centerX = startX + (CELL_WIDTH / 2);
    int centerY = startY + (CELL_HEIGHT / 2);
    const int radius = 3; // Slightly larger
    int dx, dy;

    // Animation variables
    static int frame = 0;
    frame = (frame + 1) % 20; // Cycle through 20 frames
    float mouthAngle = (frame < 10) ? (frame * 3.6f) : ((20 - frame) * 3.6f); // Opens and closes

    // Draw Pac-Man body
    for (dy = -radius; dy <= radius; dy++) {
        for (dx = -radius; dx <= radius; dx++) {
            if (dx * dx + dy * dy <= radius * radius) {
                // Calculate angle for the current point
                float angle = atan2f(dy, dx) * 180.0f / 3.14159f;

                // Skip drawing in the mouth area
                if (!(angle > -mouthAngle && angle < mouthAngle)) {
                    int drawX = centerX + dx;
                    int drawY = centerY + dy;
                    if (drawX >= 0 && drawX < 240 && drawY >= 0 && drawY < 320) {
                        LCD_SetPoint(drawX, drawY, Yellow);
                    }
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
		if(pillsEaten >= pillCount){
				GUI_Text((240/2)-30, (320/2)-10, (uint8_t *)"Victory!", Yellow, Black);
				// Stop the game
				gamePaused=true;
				disable_RIT();      // 
				disable_timer(2);   // so countdown stops, etc.
				disable_timer(0);
				disable_timer(3);
				NVIC_DisableIRQ(EINT0_IRQn);
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
						ghostFrightenedMode();
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
						ghostFrightenedMode();
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
								playSoundEffect(pacman_wakka, sizeof(pacman_wakka) / sizeof(pacman_wakka[0]));
                score += 10;
								pillsEaten++;
                mazeGrid[newRow][newCol] = EMPTY;
            } else if(mazeGrid[newRow][newCol] == POWER_PILL) {
								playSoundEffect(power_pill_sound, sizeof(power_pill_sound) / sizeof(power_pill_sound[0]));
                score += 50;
								pillsEaten++;
                mazeGrid[newRow][newCol] = EMPTY;
								ghostFrightenedMode();
            }
						
						// Check for extra lives
						if (score >= lastLifeMilestone + 1000) {
								lives += 1;
								lastLifeMilestone += 1000;
								drawUI(); // Update the UI with the new life count
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
		init_RIT(0x01538400); // 300ms for board
	  //init_RIT(0x004C4B40);	// 50ms
		//init_RIT(0x000F4240 );	// 10ms for emulator 
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
    //init_timer(0, 0x00B6F1A0); // for board
		init_timer(2, 0x17D7840); // for emulator. matches the actual seconds in a clock.
		CAN_Init();

		
    //enable_timer(0);
		//init_timer(1, 0x1312D0);
		//enable_timer(1);
    // Set initial direction to nothing
    pacmanDirRow = 0;
    pacmanDirCol = 0;

		init_timer(3, 0x004C4B40); // Adjust this value based on your desired music timing


		
		LPC_SC->PCON |= 0x1;  /* Enter power-down mode */
    LPC_SC->PCON &= ~(0x2);

		// DAC Related. 
		LPC_PINCON->PINSEL1 |= (1<<21);
		LPC_PINCON->PINSEL1 &= ~(1<<20);
		LPC_GPIO0->FIODIR |= (1<<26);
		while (1) {
				__ASM("wfi");}
    return 0;
}


/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
