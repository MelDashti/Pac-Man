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
#define RIT_SEMIMINIMA 8
#define RIT_MINIMA 16
#define RIT_INTERA 32

#define UPTICKS 1


//SHORTENING UNDERTALE: TOO MANY REPETITIONS
NOTE song[] = 
{
	// 1
	{d3, time_semicroma},
	{d3, time_semicroma},
	{d4, time_croma},
	{a3, time_croma},
	{pause, time_semicroma},
	{a3b, time_semicroma},
	{pause, time_semicroma},
	{g3, time_croma},
	{f3, time_semicroma*2},
	{d3, time_semicroma},
	{f3, time_semicroma},
	{g3, time_semicroma},
	// 2
	{c3, time_semicroma},
	{c3, time_semicroma},
	{d4, time_croma},
	{a3, time_croma},
	{pause, time_semicroma},
	{a3b, time_semicroma},
	{pause, time_semicroma},
	{g3, time_croma},
	{f3, time_semicroma*2},
	{d3, time_semicroma},
	{f3, time_semicroma},
	{g3, time_semicroma},
	// 3
	{c3b, time_semicroma},
	{c3b, time_semicroma},
	{d4, time_croma},
	{a3, time_croma},
	{pause, time_semicroma},
	{a3b, time_semicroma},
	{pause, time_semicroma},
	{g3, time_croma},
	{f3, time_semicroma*2},
	{d3, time_semicroma},
	{f3, time_semicroma},
	{g3, time_semicroma},
	// 4
	{a2b, time_semicroma},
	{a2b, time_semicroma},
	{d4, time_croma},
	{a3, time_croma},
	{pause, time_semicroma},
	{a3b, time_semicroma},
	{pause, time_semicroma},
	{g3, time_croma},
	{f3, time_semicroma*2},
	{d3, time_semicroma},
	{f3, time_semicroma},
	{g3, time_semicroma},
	// 5
	
};

void RIT_IRQHandler (void)
{
	static int currentNote = 0;
	static int ticks = 0;
	if(!isNotePlaying())
	{
		++ticks;
		if(ticks == UPTICKS)
		{
			ticks = 0;
			playNote(song[currentNote++]);
		}
	}
	
    static bool buttonPressed = false;  // Track button state
    static int debounceCounter = 0;     // Counter for debounce delay

    // Handle button debouncing
    if (debouncing) {
        if ((LPC_GPIO2->FIOPIN & (1 << 10)) == 0) {  // Button pressed
            if (!buttonPressed) {
                buttonPressed = true;  // Mark the button as pressed
                gamePaused = !gamePaused;  // Toggle pause state

                if (gamePaused) {
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

        ADC_start_conversion();
				// play sound
				
        movePacMan();
        updateGhost();
    }

    LPC_RIT->RICTRL |= 0x1;  // Clear RIT interrupt flag
}

