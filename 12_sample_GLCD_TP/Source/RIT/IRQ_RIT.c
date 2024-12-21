#include "LPC17xx.h"
#include "RIT.h"
#include "../joystick/joystick.h"

extern volatile int pacmanDirRow;
extern volatile int pacmanDirCol;

void RIT_IRQHandler (void) {
    // Read joystick (active low)
    int upPressed    = !(LPC_GPIO1->FIOPIN & (1<<29));
    int downPressed  = !(LPC_GPIO1->FIOPIN & (1<<26));
    int leftPressed  = !(LPC_GPIO1->FIOPIN & (1<<27));
    int rightPressed = !(LPC_GPIO1->FIOPIN & (1<<28));

    // Classic Pac-Man: change direction immediately on press
    // but do NOT zero direction if no press => keep going
    if (upPressed) {
        pacmanDirRow = -1;
        pacmanDirCol = 0;
    }
    else if (downPressed) {
        pacmanDirRow = 1;
        pacmanDirCol = 0;
    }
    else if (leftPressed) {
        pacmanDirRow = 0;
        pacmanDirCol = -1;
    }
    else if (rightPressed) {
        pacmanDirRow = 0;
        pacmanDirCol = 1;
    }
    // else do nothing => keep old direction

    LPC_RIT->RICTRL |= 0x1;  // clear interrupt flag
}
