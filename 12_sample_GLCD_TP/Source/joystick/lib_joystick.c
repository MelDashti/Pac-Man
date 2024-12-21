/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           joystick.h
** Last modified Date:  2018-12-30
** Last Version:        V1.00
** Descriptions:        Atomic joystick init functions
** Correlated files:    lib_joystick.c, funct_joystick.c
**--------------------------------------------------------------------------------------------------------       
*********************************************************************************************************/

#include "LPC17xx.h"
#include "joystick.h"

/*----------------------------------------------------------------------------
  Function that initializes joysticks and switch them off
 *----------------------------------------------------------------------------*/

// Using the defines from joystick.h (optional)
void joystick_init(void) {
    // Configure UP (P1.29)
    LPC_PINCON->PINSEL3 &= ~(3 << 26); // Clear function bits for P1.29 (bits 26,27)
    LPC_GPIO1->FIODIR   &= ~(1 << 29); // Set P1.29 as input

    // Configure DOWN (P1.26) - P1.26 is controlled by bits 20 and 21 in PINSEL3
    LPC_PINCON->PINSEL3 &= ~(3 << 20); // Clear function bits for P1.26
    LPC_GPIO1->FIODIR   &= ~(1 << 26); // Set P1.26 as input

    // Configure LEFT (P1.27) - PINSEL3 bits for P1.27 are 22 and 23
    LPC_PINCON->PINSEL3 &= ~(3 << 22); 
    LPC_GPIO1->FIODIR   &= ~(1 << 27);

    // Configure RIGHT (P1.28) - PINSEL3 bits for P1.28 are 24 and 25
    LPC_PINCON->PINSEL3 &= ~(3 << 24);
    LPC_GPIO1->FIODIR   &= ~(1 << 28);
}

