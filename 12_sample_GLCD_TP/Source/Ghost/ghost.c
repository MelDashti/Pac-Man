#include "ghost.h"
#include "GLCD/GLCD.h"
#include <stdlib.h>
#include <stdbool.h>

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

Ghost blinky;


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
                GUI_Text((240/2)-40, (320/2)-10, (uint8_t *)"GAME OVER!", Red, Black);
            } else {
                pacmanRow = 1;
                pacmanCol = 1;
                blinky.row = 13;
                blinky.col = 14;
            }
        }
    }
}


void drawGhost(int offsetX, int offsetY) 
{
    if (!blinky.isActive) return;

    int radius = 3;
    uint16_t color = blinky.isChasing ? Red : Blue;
    
    int startX = offsetX + blinky.col * 8;  // if each cell is 8 px wide
    int startY = offsetY + blinky.row * 10; // if each cell is 10 px tall
    int dy, dx;
    for (dy = -radius; dy <= radius; dy++) {
        for (dx = -radius; dx <= radius; dx++) {
            if (dx*dx + dy*dy <= radius*radius) {
                int drawX = startX + 4 + dx;  // offset to center in the cell
                int drawY = startY + 5 + dy;
                if (drawX >= 0 && drawX < 240 && drawY >= 0 && drawY < 320) {
                    LCD_SetPoint(drawX, drawY, color);
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
