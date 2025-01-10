#include "ghost.h"
#include "GLCD/GLCD.h"
#include <stdlib.h>
#include <stdbool.h>
#include <music/music.h>
// external variables
#define ROWS 29
#define COLS 28
#define WALL  1
#define EMPTY 0
#define PILL  2
// Optionally define POWER_PILL if you want to add them later
#define POWER_PILL 3
extern volatile int mazeGrid[ROWS][COLS];
extern int pacmanRow;
extern int pacmanCol;
extern int score;
extern int lives;
extern volatile bool gamePaused;
extern int offsetX;
extern int offsetY;
#define CELL_WIDTH 8
#define CELL_HEIGHT 10
Ghost blinky;


NOTE death_sound[] = {
    {b4, time_semicroma},
    {b3, time_semicroma},
    {a3, time_semicroma},
    {g3, time_semicroma},
    {f3, time_semiminima},
    {pause, time_semicroma}
};

// For BFS we store row/col in a queue
typedef struct {
  int row;
  int col;
} BFSPos;

// We'll store up to ROWS*COLS positions in the queue
#define QSIZE (ROWS * COLS)

// BFS-based findNextMove
void findNextMoveBFS(Ghost *ghost, int targetRow, int targetCol, int *nextRow, int *nextCol) 
{
    // If already at target, stay put
    if (ghost->row == targetRow && ghost->col == targetCol) {
        *nextRow = ghost->row;
        *nextCol = ghost->col;
        return;
    }

    // Simple direction calculation
//    int rowDiff = targetRow - ghost->row;
//    int colDiff = targetCol - ghost->col;
		int rowDiff, colDiff;
		if (ghost->isChasing) {
    // normal chase: move toward (pacmanRow, pacmanCol)
    rowDiff = targetRow - ghost->row;
    colDiff = targetCol - ghost->col;
		} else {
				// frightened: move AWAY from (pacmanRow, pacmanCol)
				rowDiff = ghost->row - targetRow;
				colDiff = ghost->col - targetCol;
		}
    
    // Try vertical movement first if there's vertical distance
    if (rowDiff != 0) {
        int tryRow = ghost->row + (rowDiff > 0 ? 1 : -1);
        // Check if move is valid
        if (tryRow >= 0 && tryRow < ROWS && mazeGrid[tryRow][ghost->col] != WALL) {
            *nextRow = tryRow;
            *nextCol = ghost->col;
            return;
        }
    }
    
    // Try horizontal movement if vertical wasn't possible
    if (colDiff != 0) {
        int tryCol = ghost->col + (colDiff > 0 ? 1 : -1);
        // Check if move is valid
        if (tryCol >= 0 && tryCol < COLS && mazeGrid[ghost->row][tryCol] != WALL) {
            *nextRow = ghost->row;
            *nextCol = tryCol;
            return;
        }
    }
    
    // If no valid moves found, stay in place
    *nextRow = ghost->row;
    *nextCol = ghost->col;
}

/* The rest of your ghost stuff: */
void initGhost(void)
{
    blinky.row = 13; 
    blinky.col = 14;
    blinky.isChasing = true;
    blinky.isActive = true;
    blinky.respawnTimer = 0;
    blinky.frightenedTimer = 0;
	  blinky.underlyingCell = EMPTY; 
}

void ghostFrightenedMode(void){
		blinky.isChasing = false;
		blinky.frightenedTimer = 10;
}



void updateGhost(void) 
{
    if (!blinky.isActive || gamePaused) {
        return;
    }

    // 1) Restore the OLD cell from underlyingCell
    //    (Instead of painting it black, we redraw the correct tile).
    int oldRow = blinky.row;
    int oldCol = blinky.col;
    int oldCellType = blinky.underlyingCell;

    // RE-DRAW whatever was in oldCellType:
    if (oldCellType == PILL) {
        fillCell(oldRow, oldCol, offsetX, offsetY, Black);
        // standard small pill
        drawPill(oldRow, oldCol, offsetX, offsetY, Yellow, 1);
    }
    else if (oldCellType == POWER_PILL) {
        fillCell(oldRow, oldCol, offsetX, offsetY, Black);
        // bigger pill
        drawPill(oldRow, oldCol, offsetX, offsetY, Red, 3);
    }
    else {
        // just empty floor
        fillCell(oldRow, oldCol, offsetX, offsetY, Black);
    }

    // 2) Figure out the ghost's next move
    int nextRow, nextCol;
    findNextMoveBFS(&blinky, pacmanRow, pacmanCol, &nextRow, &nextCol);
    
    // 3) Update ghost position
    blinky.row = nextRow;
    blinky.col = nextCol;

    // 4) Remember what cell type is at the new location
    blinky.underlyingCell = mazeGrid[nextRow][nextCol];

    // 5) Draw the ghost at the new position
    drawGhost(offsetX, offsetY);

    // 6) Check collision with Pac-Man
    if (blinky.row == pacmanRow && blinky.col == pacmanCol) {
        if (!blinky.isChasing) {
            // Ghost is frightened -> Pac-Man eats ghost
            score += 100;
            blinky.isActive = false;
            blinky.respawnTimer = 3;
            // Place ghost in spawn, but offscreen or invisible
            blinky.row = 13;
            blinky.col = 14;
        } else {
            // Normal chase -> ghost kills Pac-Man
            lives--;
            if (lives <= 0) {
                gamePaused = true;
								playSoundEffect(death_sound, sizeof(death_sound) / sizeof(death_sound[0]));

								// Stop the game
								gamePaused=true;
								disable_RIT();      // 
								disable_timer(2);   // so countdown stops, etc.
								disable_timer(0);
								disable_timer(3);
								NVIC_DisableIRQ(EINT0_IRQn);
                GUI_Text((240/2)-40, (320/2)-10, (uint8_t *)"GAME OVER!", Red, Black);
								
            } else {
								// Clear the ghost's current position before moving it
								fillCell(blinky.row, blinky.col, offsetX, offsetY, Black);
								
								// Reset positions
								pacmanRow = 1;
								pacmanCol = 1;
								blinky.row = 13;
								blinky.col = 14;
								
								// Reset the underlying cell value
								blinky.underlyingCell = EMPTY;
								playSoundEffect(death_sound, sizeof(death_sound) / sizeof(death_sound[0]));

								// Redraw the ghost in its new position
								drawGhost(offsetX, offsetY);
}
        }
    }
}

void drawGhost(int offsetX, int offsetY) {
    if (!blinky.isActive) return;
    
    uint16_t color = blinky.isChasing ? Red : Blue;
    int startX = offsetX + blinky.col * CELL_WIDTH;
    int startY = offsetY + blinky.row * CELL_HEIGHT;

    const int width = 3;  // Fixed ghost half-width
    const int height = 4; // Fixed ghost height
    const int waveHeight = 2; // Height of the wavy bottom
    const int eyeRadius = 1;
    const int eyeOffsetX = 1;
    const int eyeOffsetY = -1;

    int dx, dy;

    // Draw main body (rounded top)
    for (dy = -height; dy <= 0; dy++) {
        for (dx = -width; dx <= width; dx++) {
            if ((dx * dx) / (float)(width * width) + 
                (dy * dy) / (float)(height * height) <= 1.0f) {
                int drawX = startX + (CELL_WIDTH / 2) + dx;
                int drawY = startY + (CELL_HEIGHT / 2) + dy;
                if (drawX >= 0 && drawX < 240 && drawY >= 0 && drawY < 320) {
                    LCD_SetPoint(drawX, drawY, color);
                }
            }
        }
    }

    // Draw wavy bottom
    for (dy = 1; dy <= waveHeight; dy++) {
        for (dx = -width; dx <= width; dx++) {
            int waveOffset = (dx + dy) % 2; // Alternating wave pattern
            int drawX = startX + (CELL_WIDTH / 2) + dx;
            int drawY = startY + (CELL_HEIGHT / 2) + dy + waveOffset;
            if (drawX >= 0 && drawX < 240 && drawY >= 0 && drawY < 320) {
                LCD_SetPoint(drawX, drawY, color);
            }
        }
    }

    // Draw eyes
    for (dy = -eyeRadius; dy <= eyeRadius; dy++) {
        for (dx = -eyeRadius; dx <= eyeRadius; dx++) {
            if (dx * dx + dy * dy <= eyeRadius * eyeRadius) {
                // Left eye
                int leftEyeX = startX + (CELL_WIDTH / 2) - eyeOffsetX + dx;
                int leftEyeY = startY + (CELL_HEIGHT / 2) + eyeOffsetY + dy;
                if (leftEyeX >= 0 && leftEyeX < 240 && leftEyeY >= 0 && leftEyeY < 320) {
                    LCD_SetPoint(leftEyeX, leftEyeY, White);
                }

                // Right eye
                int rightEyeX = startX + (CELL_WIDTH / 2) + eyeOffsetX + dx;
                int rightEyeY = startY + (CELL_HEIGHT / 2) + eyeOffsetY + dy;
                if (rightEyeX >= 0 && rightEyeX < 240 && rightEyeY >= 0 && rightEyeY < 320) {
                    LCD_SetPoint(rightEyeX, rightEyeY, White);
                }
            }
        }
    }
}


void handleGhostTimer(void) 
{
		if (!blinky.isActive) {
				blinky.respawnTimer--;
				if (blinky.respawnTimer <= 0) {
						blinky.isActive = true;
						blinky.row = 13;
						blinky.col = 14;
						blinky.isChasing = true;  // ghost reverts to normal chase
				}
		}
    if (blinky.frightenedTimer > 0) {
        blinky.frightenedTimer--;
        if (blinky.frightenedTimer <= 0) {
            blinky.isChasing = true;  // Reset to chase mode after frightened expires
        }
    }
}
