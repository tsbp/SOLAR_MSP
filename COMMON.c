#include "io430.h"
#include "COMMON.h"
//==============================================================================
uSTATE state = {.mPres = 0};
unsigned long operation_duration = 0;
unsigned char ledStt = 0;
unsigned int blink = BLINK_WAIT;
//==============================================================================

void leds(unsigned char aStt)
{
  if(!aStt) P1DIR &= ~BIT4;
  else
  {    
    if (aStt == 0x03)        P1OUT ^= BIT4;
    else if(aStt & BIT0)     P1OUT |= BIT4;
    else                     P1OUT &= ~BIT4;    
    P1DIR |= BIT4;
  }
}
//==============================================================================
void blinking(unsigned int blnk)
{
  static unsigned int bStage, bCntr = BLNK_MAX;
  
  if(bCntr) bCntr--;
  else
  {
    bCntr = BLNK_MAX;    
    if(blnk & (1 << bStage)) ledStt |= BIT0;
    else                     ledStt &= ~BIT0;
    
    if((blnk >> 8) & (1 << bStage)) ledStt |= BIT1;
    else                             ledStt &= ~BIT1;
    
    bStage++;
    bStage %= 8;    
  }  
}
//== motor move ================================================================
void move(unsigned int aPlane, unsigned int aDir)
{
  if(!state.moving)
  {
    state.moving = 1;
    operation_duration =  5000L;
    //STOP_MOTORS
    if(aPlane == VERTICAL)
    {
      if(aDir == BACKWARD) P2OUT |= REL_PWR1;
      __delay_cycles(DIRECTION_DELAY);
      P2OUT |= REL_DIR1;
    }
    else
    {
      if(aDir == BACKWARD) P2OUT |=  REL_PWR2;
      __delay_cycles(DIRECTION_DELAY);
      P1OUT |=  REL_DIR2;
    }
  }
}