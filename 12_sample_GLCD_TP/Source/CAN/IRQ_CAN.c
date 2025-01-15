/*----------------------------------------------------------------------------
 * Name:    Can.c
 * Purpose: CAN interface for for LPC17xx with MCB1700
 * Note(s): see also http://www.port.de/engl/canprod/sv_req_form.html
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <LPC17xx.h>                  /* LPC17xx definitions */
#include "CAN.h"                      /* LPC17xx CAN adaption layer */
#include "../GLCD/GLCD.h"

extern uint8_t icr ; 										//icr and result must be global in order to work with both real and simulated landtiger.
extern uint32_t result;
extern CAN_msg       CAN_TxMsg;    /* CAN message for sending */
extern CAN_msg       CAN_RxMsg;    /* CAN message for receiving */                                
extern volatile gameOver;
extern volatile int countdown;
extern volatile int score;
extern volatile int lives;
extern void drawUI(void);
static int puntiRicevuti1 = 0;
static int puntiInviati1 = 0;

static int puntiRicevuti2 = 0;
static int puntiInviati2 = 0;

uint16_t val_RxCoordX = 0;            /* Locals used for display */
uint16_t val_RxCoordY = 0;

/*----------------------------------------------------------------------------
  CAN interrupt handler
 *----------------------------------------------------------------------------*/
void CAN_IRQHandler (void)  {
   /* check CAN controller 1 */
  icr = 0;
  icr = (LPC_CAN1->ICR | icr) & 0xFF;               /* clear interrupts */
  
  if (icr & (1 << 0)) {                              /* CAN Controller #1 meassage is received */
    CAN_rdMsg (1, &CAN_RxMsg);                      /* Read the message */
    LPC_CAN1->CMR = (1 << 2);                        /* Release receive buffer */
  }
  if (icr & (1 << 1)) {                         /* CAN Controller #1 meassage is transmitted */
    // do nothing in this example
  }
    
  /* check CAN controller 2 */
  icr = 0;
  icr = (LPC_CAN2->ICR | icr) & 0xFF;             /* clear interrupts */
  if (icr & (1 << 0)) {            /* CAN Controller #2 meassage is received */
    CAN_rdMsg (2, &CAN_RxMsg);    /* Read the message */
    LPC_CAN2->CMR = (1 << 2);      /* Release receive buffer */

    uint8_t time = (uint8_t) CAN_RxMsg.data[0] ;
    uint8_t lifes = (uint8_t) CAN_RxMsg.data[1] ;
    uint16_t score = (uint16_t) ( (uint8_t) CAN_RxMsg.data[2] << 8 ) | (uint8_t) CAN_RxMsg.data[3] ;
    
    if(!gameOver){
      drawUI();
    }
  }
  if (icr & (1 << 1)) {                         /* CAN Controller #2 meassage is transmitted */
    // do nothing in this example
  }
}

void send_values_CAN(){
  uint8_t buffer[4];

  buffer[0] = (uint8_t) countdown;
  buffer[1] = (uint8_t) lives;
  buffer[2] = (uint8_t) ( ( score & 0xFF00 ) >> 8);
  buffer[3] = (uint8_t) ( score & 0xFF);

  int i;
  for(i = 0; i<4; i++){
    CAN_TxMsg.data[i] = buffer[i];
  }

  CAN_TxMsg.id = 2;
  CAN_TxMsg.len = 4;
  CAN_TxMsg.format = STANDARD_FORMAT;
  CAN_TxMsg.type = DATA_FRAME;
  CAN_wrMsg (1, &CAN_TxMsg);
}