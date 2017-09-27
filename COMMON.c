#include "io430.h"
#include "COMMON.h"
#include "UART.h"

//==============================================================================
uSTATE state = {.mPres = 0};
unsigned long operation_duration = 0;
unsigned char ledStt = 0;
unsigned int blink = BLINK_WAIT;

int azimuth, angle;
unsigned int light;


int rollArray[FILTER_LENGHT], pitchArray[FILTER_LENGHT], yawArray[FILTER_LENGHT];
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
void move(unsigned int aPlane, unsigned int aDir, unsigned long opDuration)
{
  if(!state.moving)
  {
    state.moving = 1;
    operation_duration =  opDuration;
    //STOP_MOTORS
    if(aPlane == VERTICAL)
    {
      if(aDir == BACKWARD) {P2OUT |= REL_PWR1;  blink = BLINK_DOWN;}
      else blink = BLINK_UP;
      __delay_cycles(DIRECTION_DELAY);
      P2OUT |= REL_DIR1;
    }
    else
    {
      if(aDir == BACKWARD) {P2OUT |=  REL_PWR2; blink = BLINK_BACKWARD;}
      else blink = BLINK_FORWARD;
      __delay_cycles(DIRECTION_DELAY);
      P1OUT |=  REL_DIR2;
    }
  }
}
//==============================================================================
unsigned char termState  = 0;
unsigned char getTermState(void)
{
  unsigned char tmp = 0;
  if(P2IN & BIT4) tmp |= BIT0;
  if(P2IN & BIT5) tmp |= BIT1;
  if(P2IN & BIT7) tmp |= BIT2;
  if(P2IN & BIT6) tmp |= BIT3;
  return tmp;
}

//==============================================================================
unsigned char txBuf[20] = {ID_SLAVE, 0, 0};
//==============================================================================
void cmdExecute(void)
{
  unsigned int dataLng = 0;
      
      state.mPres = 1; 
      state.cmdIn = 0;      
      if(rxBuf[0] == ID_MASTER)
        switch(rxBuf[1])
        {      

          case CMD_ANGLE: 
                  if(!state.moveV)
                  {
                    state.moveV = 1;
                    angle = rxBuf[3] | (rxBuf[4] << 8); 
                  }
                  break;
                  
          case CMD_AZIMUTH: 
                  if(!state.moveH)
                  {
                    state.moveH = 1;
                    azimuth = rxBuf[3] | (rxBuf[4] << 8);
                  }      
                  break;
                  
          case CMD_LEFT: 
                  dataLng   = 1;
                  txBuf[3]  = OK;
                  move(HORIZONTAL, BACKWARD, 5000);
                  break;
                  
          case CMD_RIGHT:
                  move(HORIZONTAL, FORWARD, 5000); 
                  break;
                  
          case CMD_UP:
                  dataLng   = 1;
                  txBuf[3]  = OK; 
                  move(VERTICAL, FORWARD, 5000);
                  break;
                  
          case CMD_DOWN: 
                  dataLng   = 1;
                  txBuf[3]  = OK;
                  move(VERTICAL, BACKWARD, 5000);
                  break;
                  
          case CMD_STATE: 
            {
                  dataLng   = 9;   
                  txBuf[3]  = _roll;
                  txBuf[4]  = _roll >> 8;
                  txBuf[5]  = _pitch;
                  txBuf[6]  = _pitch >> 8;
                  txBuf[7]  = _heading;
                  txBuf[8]  = _heading >> 8;
                  txBuf[9]  = light;
                  txBuf[10] = light >> 8;
                  txBuf[11] = getTermState();
            }
                  break;
                  
          case CMD_CFG: 
                  break;
//          case CMD_SET_AZIMUTH:
//            if(!state.moveH)
//            {
//              state.moveH = 1;
//              azimuth = rxBuf[1] | (rxBuf[2] << 8);
//            }           
//            break;
//            
//          case CMD_SET_ANGLE:
//            if(!state.moveV)
//            {
//              state.moveV = 1;
//              angle = rxBuf[1] | (rxBuf[2] << 8); 
//            }
//            break;
//            
//          case CMD_UP:
//            //blink = BLINK_UP;            
//            move(VERTICAL, FORWARD, 5000);
//            break;
//            
//          case CMD_DOWN:
//            //blink = BLINK_DOWN;
//            move(VERTICAL, BACKWARD, 5000);
//            break;
//            
//          case CMD_RIGHT:
//            //blink = BLINK_BACKWARD;
//            move(HORIZONTAL, FORWARD, 5000);
//            break;
//            
//          case CMD_LEFT:
//            //blink = BLINK_FORWARD;
//            move(HORIZONTAL, BACKWARD, 5000);
//            break;
      }
      txBuf[1] = rxBuf[1]; // add cmd;
      txBuf[2] = dataLng;     // add length;
      // add crc16      
      txBuf[dataLng + 3] = 0xcc;
      txBuf[dataLng + 4] = 0xcc;
      msgTransmitt(txBuf, dataLng + 5);
    }