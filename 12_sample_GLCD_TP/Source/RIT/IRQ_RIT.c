#include "LPC17xx.h"
#include "GLCD/GLCD.h" 
#include "Ghost/ghost.h"
#include "../joystick/joystick.h"
#include "RIT/RIT.h"
#include <stdbool.h> // For boolean data type
extern volatile int pacmanDirRow;
extern volatile int pacmanDirCol;
extern volatile bool gamePaused;

// Flags to track button state
static bool upPressedFlag = false;
static bool downPressedFlag = false;
static bool leftPressedFlag = false;
static bool rightPressedFlag = false;

void RIT_IRQHandler(void) {
    // Read joystick inputs (active low)
    int upPressed    = !(LPC_GPIO1->FIOPIN & (1 << 29));  // Joystick UP
    int downPressed  = !(LPC_GPIO1->FIOPIN & (1 << 26));  // Joystick DOWN
    int leftPressed  = !(LPC_GPIO1->FIOPIN & (1 << 27));  // Joystick LEFT
    int rightPressed = !(LPC_GPIO1->FIOPIN & (1 << 28));  // Joystick RIGHT

	// only process the joystick if the game is not paused
		
	
    // Handle UP
    if (upPressed && !upPressedFlag) {
        pacmanDirRow = -1;
        pacmanDirCol = 0;
        upPressedFlag = true; // Mark as handled
    } else if (!upPressed) {
        upPressedFlag = false; // Reset when released
    }

    // Handle DOWN
    if (downPressed && !downPressedFlag) {
        pacmanDirRow = 1;
        pacmanDirCol = 0;
        downPressedFlag = true;
    } else if (!downPressed) {
        downPressedFlag = false;
    }

    // Handle LEFT
    if (leftPressed && !leftPressedFlag) {
        pacmanDirRow = 0;
        pacmanDirCol = -1;
        leftPressedFlag = true;
    } else if (!leftPressed) {
        leftPressedFlag = false;
    }

    // Handle RIGHT
    if (rightPressed && !rightPressedFlag) {
        pacmanDirRow = 0;
        pacmanDirCol = 1;
        rightPressedFlag = true;
    } else if (!rightPressed) {
        rightPressedFlag = false;
    }

    // Move Pacman in the current direction
    if (!gamePaused) {
				movePacMan();
				updateGhost();
		} 

    // Clear the RIT interrupt flag
    LPC_RIT->RICTRL |= 0x1;
}
