#include "button.h"
#include "LPC17xx.h"
#include "GLCD/GLCD.h"
#include <stdbool.h>

extern volatile bool gamePaused;
bool firstUnpauseDone;

// Modified EINT0_IRQHandler with debouncing
void EINT0_IRQHandler(void) {
    // Disable Button interrupts
    NVIC_DisableIRQ(EINT0_IRQn);
    
    // Toggle pause state
    gamePaused = !gamePaused;
			
	
		
	
		if(gamePaused) {
				GUI_Text((240/2)-40, (320/2)-10, (uint8_t *)"PAUSE", Yellow, Black);
				
		} else {
				// Clear the PAUSE text by drawing a black rectangle
				 int x, y;
        for(y = (320/2)-10; y < (320/2)-10 + 16; y++){
          for(x = (240/2)-40; x < (240/2)-40 + 80; x++){
            LCD_SetPoint(x, y, Black);
          }
        }

        // Also clear the "READY!" if not yet cleared
        if(!firstUnpauseDone) {
            // "READY!" was at ( (240/2)-20, (320/2)-20 ) ~ (120, 140)
            // Let’s assume it’s 16 px high, 40 px wide
            for(y = (320/2)-20; y < (320/2)-20 + 16; y++){
              for(x = (240/2)-20; x < (240/2)-20 + 40; x++){
                LCD_SetPoint(x, y, Black);
              }
            }
						enable_timer(0);
            firstUnpauseDone = true;
        }
    }

    LPC_SC->EXTINT &= (1 << 0); // Clear pending interrupt
    enable_RIT();
}

void EINT1_IRQHandler (void)	  	/* KEY1														 */
{
	NVIC_DisableIRQ(EINT1_IRQn);		/* disable Button interrupts			 */
	LPC_PINCON->PINSEL4    &= ~(1 << 22);     /* GPIO pin selection */
	LPC_SC->EXTINT &= (1 << 1);     /* clear pending interrupt         */
}

void EINT2_IRQHandler (void)	  	/* KEY2														 */
{
  LPC_SC->EXTINT &= (1 << 2);     /* clear pending interrupt         */    
}


