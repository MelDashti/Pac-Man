/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           lib_timer.h
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        atomic functions to be used by higher sw levels
** Correlated files:    lib_timer.c, funct_timer.c, IRQ_timer.c
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "LPC17xx.h"
#include "timer.h"

/******************************************************************************
** Function name:		enable_timer
**
** Descriptions:		Enable timer
**
** parameters:			timer number: 0 or 1
** Returned value:		None
**
******************************************************************************/


void enable_timer(char timer_num) {
    switch(timer_num) {
        case 0:
            LPC_TIM0->TCR = 1;
            break;
        case 1:
            LPC_TIM1->TCR = 1;
            break;
        case 2:
            LPC_TIM2->TCR = 1;
            break;
        case 3:
            LPC_TIM3->TCR = 1;
            break;
    }
}

/******************************************************************************
** Function name:		disable_timer
**
** Descriptions:		Disable timer
**
** parameters:			timer number: 0 or 1
** Returned value:		None
**
******************************************************************************/
void disable_timer(char timer_num) {
    switch(timer_num) {
        case 0:
            LPC_TIM0->TCR = 0;
            break;
        case 1:
            LPC_TIM1->TCR = 0;
            break;
        case 2:
            LPC_TIM2->TCR = 0;
            break;
        case 3:
            LPC_TIM3->TCR = 0;
            break;
    }
}

/******************************************************************************
** Function name:		reset_timer
**
** Descriptions:		Reset timer
**
** parameters:			timer number: 0 or 1
** Returned value:		None
**
******************************************************************************/
void reset_timer(char timer_num) {
    uint32_t regVal;
    
    switch(timer_num) {
        case 0:
            regVal = LPC_TIM0->TCR;
            regVal |= 0x02;
            LPC_TIM0->TCR = regVal;
            break;
        case 1:
            regVal = LPC_TIM1->TCR;
            regVal |= 0x02;
            LPC_TIM1->TCR = regVal;
            break;
        case 2:
            regVal = LPC_TIM2->TCR;
            regVal |= 0x02;
            LPC_TIM2->TCR = regVal;
            break;
        case 3:
            regVal = LPC_TIM3->TCR;
            regVal |= 0x02;
            LPC_TIM3->TCR = regVal;
            break;
    }
}

unsigned int init_timer ( char timer_num, unsigned int TimerInterval )
{
  if ( timer_num == 0 )
  {
	LPC_TIM0->MR0 = TimerInterval;

// <<< Use Configuration Wizard in Context Menu >>> 
// <h> timer0 MCR
//   <e.0> MR0I
//	 <i> 1 Interrupt on MR0: an interrupt is generated when MR0 matches the value in the TC. 0
//	 <i> 0 This interrupt is disabled
//   </e>
//   <e.1> MR0R
//	 <i> 1 Reset on MR0: the TC will be reset if MR0 matches it.
//	 <i> 0 Feature disabled.
//   </e>
//   <e.2> MR0S
//	 <i> 1 Stop on MR0: the TC and PC will be stopped and TCR[0] will be set to 0 if MR0 matches the TC
//	 <i> 0 Feature disabled.
//   </e>
//   <e.3> MR1I
//	 <i> 1 Interrupt on MR1: an interrupt is generated when MR0 matches the value in the TC. 0
//	 <i> 0 This interrupt is disabled
//   </e>
//   <e.4> MR1R
//	 <i> 1 Reset on MR1: the TC will be reset if MR0 matches it.
//	 <i> 0 Feature disabled.
//   </e>
//   <e.5> MR1S
//	 <i> 1 Stop on MR1: the TC and PC will be stopped and TCR[1] will be set to 0 if MR1 matches the TC
//	 <i> 0 Feature disabled.
//   </e>
//   <e.6> MR2I
//	 <i> 1 Interrupt on MR2: an interrupt is generated when MR2 matches the value in the TC.
//	 <i> 0 This interrupt is disabled
//   </e>
//   <e.7> MR2R
//	 <i> 1 Reset on MR2: the TC will be reset if MR2 matches it.
//	 <i> 0 Feature disabled.
//   </e>
//   <e.8> MR2S
//	 <i> 1 Stop on MR2: the TC and PC will be stopped and TCR[2] will be set to 0 if MR2 matches the TC
//	 <i> 0 Feature disabled.
//   </e>
//   <e.9> MR3I
//	 <i> 1 Interrupt on MR3: an interrupt is generated when MR3 matches the value in the TC.
//	 <i> 0 This interrupt is disabled
//   </e>
//   <e.10> MR3R
//	 <i> 1 Reset on MR3: the TC will be reset if MR3 matches it.
//	 <i> 0 Feature disabled.
//   </e>
//   <e.11> MR3S
//	 <i> 1 Stop on MR3: the TC and PC will be stopped and TCR[3] will be set to 0 if MR3 matches the TC
//	 <i> 0 Feature disabled.
//   </e>
	LPC_TIM0->MCR = 3;
// </h>
// <<< end of configuration section >>>    

	NVIC_EnableIRQ(TIMER0_IRQn);
	return (1);
  }
  else if ( timer_num == 1 )
  {
	LPC_TIM1->MR0 = TimerInterval;
	LPC_TIM1->MCR = 7;				/* Interrupt reset and stop on MR1 */

	NVIC_EnableIRQ(TIMER1_IRQn);
	return (1);
  }
	else if (timer_num == 2){
		LPC_TIM2->MR0 = TimerInterval;
		LPC_TIM2->MCR = 3;          // Like timer 0: Interrupt and reset on MR0
		NVIC_EnableIRQ(TIMER2_IRQn);
		return(1);
	}
	else if (timer_num == 3){
		LPC_TIM3->MR0 = TimerInterval;
		LPC_TIM3->MCR = 7;          // Like timer 1: Interrupt, reset and stop on MR0
    NVIC_EnableIRQ(TIMER3_IRQn);
		return(1);
	}
  return (0);
}

/******************************************************************************
**                            End Of File
******************************************************************************/
