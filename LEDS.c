#include "io430.h"
#include "LEDS.h"
//==============================================================================
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