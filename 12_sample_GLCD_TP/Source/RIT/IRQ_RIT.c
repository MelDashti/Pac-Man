#include "LPC17xx.h"
#include "GLCD/GLCD.h" 
#include "Ghost/ghost.h"
#include "../joystick/joystick.h"
#include "RIT/RIT.h"
#include <stdbool.h> // For boolean data type
#include "../music/music.h"

// here we define the shared variable
volatile bool debouncing = false; 


extern volatile int pacmanDirRow;
extern volatile int pacmanDirCol;
extern volatile bool gamePaused;
extern volatile bool gameOver;
extern int offsetX;
extern int offsetY;


void RIT_IRQHandler (void)
{
    static bool buttonPressed = false;  // Track button state
    static int debounceCounter = 0;     // Counter for debounce delay

    // Handle button debouncing
    if (debouncing) {
        if ((LPC_GPIO2->FIOPIN & (1 << 10)) == 0) {  // Button pressed
            if (!buttonPressed) {
                buttonPressed = true;  // Mark the button as pressed
                gamePaused = !gamePaused;  // Toggle pause state

                if (gamePaused) {
										enable_timer(3);
                    // Display "PAUSE" text
                    GUI_Text((240 / 2) - 23, (320 / 2) - 10, (uint8_t *)"PAUSE", Yellow, Black);
                    disable_timer(2);  // Pause the game timer
                } else {
                    // Clear "PAUSE" text
										int x, y;
                    for (y = (320 / 2) - 10; y < (320 / 2) - 10 + 16; y++) {
                        for (x = (240 / 2) - 23; x < (240 / 2) - 23 + 40; x++) {
                            LCD_SetPoint(x, y, Black);
                        }
                    }
                    enable_timer(2);  // Resume the game timer
                }
            }
        } else {  // Button released
            buttonPressed = false;  // Reset button state
            debounceCounter++;
            if (debounceCounter >= 2) {  // 50 ms debounce delay
                debouncing = false;  // End debouncing
                debounceCounter = 0;  // Reset debounce counter
                NVIC_EnableIRQ(EINT0_IRQn);  // Re-enable EINT0 interrupt
                LPC_PINCON->PINSEL4 |= (1 << 20);  // Restore P2.10 to EINT0 mode
            }
        }
    }

    // Handle joystick input
    if (!gamePaused) {
        int upPressed    = !(LPC_GPIO1->FIOPIN & (1 << 29));  // Joystick UP
        int downPressed  = !(LPC_GPIO1->FIOPIN & (1 << 26));  // Joystick DOWN
        int leftPressed  = !(LPC_GPIO1->FIOPIN & (1 << 27));  // Joystick LEFT
        int rightPressed = !(LPC_GPIO1->FIOPIN & (1 << 28));  // Joystick RIGHT

        if (upPressed) {
            pacmanDirRow = -1;
            pacmanDirCol = 0;  // Move up
        } else if (downPressed) {
            pacmanDirRow = 1;
            pacmanDirCol = 0;  // Move down
        } else if (leftPressed) {
            pacmanDirRow = 0;
            pacmanDirCol = -1;  // Move left
        } else if (rightPressed) {
            pacmanDirRow = 0;
            pacmanDirCol = 1;  // Move right
        }

				// play sound
				
        movePacMan();
        updateGhost();
    }

    LPC_RIT->RICTRL |= 0x1;  // Clear RIT interrupt flag
}

