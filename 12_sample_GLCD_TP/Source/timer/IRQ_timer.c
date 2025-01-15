/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_timer.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    timer.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include <string.h>
#include "LPC17xx.h"
#include "timer.h"
#include "Ghost/ghost.h"
#include "../GLCD/GLCD.h" 
#include "../TouchPanel/TouchPanel.h"
#include <stdio.h> /*for sprintf*/
#include <stdbool.h>
#include <../CAN/CAN.h>


extern volatile int countdown;
extern volatile powerPillsSpawned;
extern volatile bool gamePaused;
extern void drawUI(void);
extern volatile gameOver;
extern handleGhostTimer();

#define RIT_SEMIMINIMA 8
#define RIT_MINIMA 16
#define RIT_INTERA 32

#define UPTICKS 1



uint16_t SinTable[45] = {
    410, 467, 523, 576, 627, 673, 714, 749, 778,
    799, 813, 819, 817, 807, 789, 764, 732, 694, 
    650, 602, 550, 495, 438, 381, 324, 270, 217,
    169, 125, 87 , 55 , 30 , 12 , 2  , 0  , 6  ,   
    20 , 41 , 70 , 105, 146, 193, 243, 297, 353
};

// Original sine wave/DAC logic
void TIMER0_IRQHandler(void) {
    static int sineticks = 0;
    /* DAC management */    
    static int currentValue; 
    
    currentValue = SinTable[sineticks];
    currentValue -= 410;
    currentValue /= 1;
    currentValue += 410;
    LPC_DAC->DACR = currentValue << 6;
    
    sineticks++;
    if (sineticks == 45) sineticks = 0;
    
    LPC_TIM0->IR = 1; // Clear interrupt flag
    return;
}

// Original disable timer logic
void TIMER1_IRQHandler(void) {
    disable_timer(0);
    LPC_TIM1->IR = 1; // Clear interrupt flag
    return;
}

// Game logic (moved from original Timer 0)
void TIMER2_IRQHandler(void) {
    if (countdown > 0) {
        countdown--;
        // Instead of directly calling drawUI, send values over CAN
       send_values_CAN();
    } else {
        GUI_Text((240/2)-30, (320/2)-20, (uint8_t *)"GAME OVER!", Red, Black);
        gameOver = true;
        disable_timer(2);
        disable_RIT();
        disable_timer(3);
    }
    
    if (!blinky.isChasing) {
        handleGhostTimer();
    }
    
    if (!gamePaused && powerPillsSpawned < 6) {
        drawPowerPills();
    }
    
    LPC_TIM2->IR = 1;  // Clear interrupt flag
}
static NOTE* currentBackgroundSong = NULL; // Background song
static NOTE* currentEffect = NULL;         // Sound effect buffer
static int   bgSongLength = 0;             // Background song length
static int   effectLength = 0;             // Sound effect length
static int   bgNoteIndex = 0;              // Background song index
static int   effectNoteIndex = 0;          // Sound effect index
static bool  effectPlaying = false;        // Flag for sound effect state
static int   ticks = 0;                    // Tick counter for timing notes

void playBackgroundMusic(NOTE* song, int length) {
    currentBackgroundSong = song;
    bgSongLength = length;
    bgNoteIndex = 0;
    effectPlaying = false; // Ensure no effect is playing
    enable_timer(3);       // Start the timer
}

void playSoundEffect(NOTE* effect, int length) {
    if (!effectPlaying) { // Only play if no other effect is playing
        currentEffect = effect;
        effectLength = length;
        effectNoteIndex = 0;
        effectPlaying = true;
    }
}

void TIMER3_IRQHandler(void) {
    if (!isNotePlaying()) {
        ++ticks;

        if (ticks == UPTICKS) {
            ticks = 0;

            if (effectPlaying) {
                // Play the current note of the sound effect
                playNote(currentEffect[effectNoteIndex++]);

                // Check if the effect is complete
                if (effectNoteIndex >= effectLength) {
                    effectPlaying = false; // Stop effect playback
                    effectNoteIndex = 0;  // Reset effect index
                }
            } else if (currentBackgroundSong != NULL) {
                // Play the current note of the background music
                playNote(currentBackgroundSong[bgNoteIndex++]);

                // Loop the background music
                if (bgNoteIndex >= bgSongLength) {
                    bgNoteIndex = 0;
                }
            }
        }
    }

    LPC_TIM3->IR = 1; // Clear interrupt flag
}

