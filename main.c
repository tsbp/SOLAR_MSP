//==============================================================================
#include "io430.h"
#include "COMMON.h"
#include "i2c.h"
#include "UART.h"
#include "LSM303.h"
#include "BH1715.h"
#include "Calculations.h"
#include "NXP_CODE.h"
//==============================================================================
#define USE_LIGHT_SENSOR
//==============================================================================
u3AXIS_DATA accel, compass;

double Pitch, Roll, Yaw;
unsigned int YawDegrees, RollDegrees;
//==============================================================================
unsigned char tmp[6];
//unsigned int light;

//==============================================================================
#define DEGREES_OFFSET  (20)
//const long toDeg = 572958;
//==============================================================================
void rotation(void)
{
//  unsigned char inputs = getTermState();
//  if(inputs & 0x03) // vertical limit is reached
//  {
//    P2OUT &= ~REL_PWR1;   
//    P2OUT &= ~REL_DIR1;    
//    state.moveV = 0;    
//  }
//  if(inputs & 0x0c) // horizontal limit is reached
//  {
//    P2OUT &= ~REL_PWR2; 
//    P1OUT &= ~REL_DIR2;  
//    state.moveH = 0;    
//  }
  //=================================================== 
  if(!state.moving) // start moving
  {
    if(state.moveH)
    {
      if     (YawDegrees < (azimuth + DEGREES_OFFSET)) {move(HORIZONTAL, FORWARD, 30000L);}
      else if(YawDegrees > (azimuth - DEGREES_OFFSET)) {move(HORIZONTAL, BACKWARD, 30000L);}      
    }    
    else if(state.moveV)
    {
      if     (RollDegrees < (angle + DEGREES_OFFSET)) {move(VERTICAL, FORWARD, 30000L);}
      else if(RollDegrees > (angle - DEGREES_OFFSET)) {move(VERTICAL, BACKWARD, 30000L);}
    }    
  }
  else // keep moving 
  {
    if(state.moveH == 1)
    {
      if((blink == BLINK_FORWARD  && (YawDegrees > (azimuth - DEGREES_OFFSET)))|| 
         (blink == BLINK_BACKWARD && (YawDegrees < (azimuth + DEGREES_OFFSET))))
      {
        __no_operation();
        __no_operation();
        operation_duration = 0;
      }
    }
    else if(state.moveV)
    {
      if((blink == BLINK_UP   && (RollDegrees > (angle - DEGREES_OFFSET)))|| 
         (blink == BLINK_DOWN && (RollDegrees < (angle + DEGREES_OFFSET))))
      
      {
        __no_operation();
        operation_duration = 0;
      }
    }
  }
  
}
//==============================================================================
void main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  BCSCTL1 = CALBC1_8MHZ;                    // Set DCO
  DCOCTL = CALDCO_8MHZ; 
  __delay_cycles(1000000L);
  BCSCTL2 |= DIVS_2;
  //======= uart ================
  uartInit();
  //========= i2c ===============
  i2cInit();  
  //==============================================================
//  while(1) 
    LSM303Init();
    //==============================================================
#ifdef USE_LIGHT_SENSOR
  BH1715(1, 0x23, 0x01, 0, 1);
  BH1715(1, 0x23, 0x11, 0, 1);
#endif
  //=============================
  TACCR0 = 250;
  TACTL = TASSEL_2 + MC_1 + ID_3;                  // SMCLK, contmode  
  TACCTL0 |= CCIE; 
  //=============================
  P1DIR &= ~BIT4; 
  ledStt = 0;
  __enable_interrupt();  
  
    
  //======== relays =================================================
  P1OUT &= ~(BIT5);
  P2OUT &= ~(BIT0 | BIT1 | BIT2);
  P1DIR |=  (BIT5);
  P2DIR |=  (BIT0 | BIT1 | BIT2);
  //===== ins      ==================================================
  P2DIR &= ~(BIT4 | BIT5 | BIT6 | BIT7);
  P2SEL &= ~(BIT4 | BIT5 | BIT6 | BIT7);
  //==============================================================
  while(1)
  {
    if(state.cmdIn) cmdExecute(); 
    rotation();       
    //=============== i2c =====================
    if(state.i2c == 1) 
    {
      state.i2c = 0;
#ifdef USE_LIGHT_SENSOR
      BH1715(0, 0x23, 0x01, (unsigned char*)&light, 2);
      light = ((unsigned char*)&light)[1] | 
                    ((unsigned char*)&light)[0] << 8;
#endif
      
      lsm303(I2C_READ,  LSM303A_I2C_ADDR, LSM303A_OUT_X_L, accel.byte, 6);  
      lsm303(I2C_READ,  LSM303M_I2C_ADDR, LSM303M_OUT_X_H, tmp, 6);  
      compass.x = (tmp[0] << 8) | tmp[1] ;
      compass.z = (tmp[2] << 8) | tmp[3] ;
      compass.y = (tmp[4] << 8) | tmp[5] ;  
      
//      txBuf.data.a = compass.x;
//      txBuf.data.b = compass.y;
//      txBuf.data.c = compass.z;
     
      addValueToArray((int)(Roll  * 10000),  rollArray);
      addValueToArray((int)(Pitch * 10000), pitchArray);
      getAngles(&accel, &Pitch, &Roll);
      
      Yaw = heading(&compass, Pitch, Roll);
      addValueToArray((int)(Yaw*10000),  yawArray);
      
      _roll    = mFilter(rollArray,  FILTER_LENGHT);
      _pitch   = mFilter(pitchArray, FILTER_LENGHT);
      _heading = mFilter(yawArray,   FILTER_LENGHT);  
      
      long _tmp = (long) _heading;
      if(_tmp < 0) _tmp += 62832;
      YawDegrees  =  (int)_tmp;                  
      RollDegrees  = _roll;
    }
    //======= send data ==================
//    if(state.mPres == 1 && state.enTrans == 1) 
//    {
//      state.enTrans = 0;
//      
////      txBuf.angles.roll    = mFilter(rollArray,  FILTER_LENGHT);
////      txBuf.angles.pitch   = mFilter(pitchArray, FILTER_LENGHT);
////      txBuf.angles.heading = mFilter(yawArray,   FILTER_LENGHT);
//      
//      long tmp = (long) txBuf.angles.heading;
//      if(tmp < 0) tmp += 62832;
//      YawDegrees  =  (int)tmp;
//      
//      RollDegrees  = txBuf.angles.roll;
//      
//      
//      txBuf.data.c = getTermState(); // set state of ins
//      
//      //msgTransmitt(); 
//    }   
    
  }
}

//==============================================================================
#define I2C_INTERVAL     (10)
#define UART_INTERVAL    (10)
//==============================================================================
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void)
{ 
  static unsigned int i2c_cntr = I2C_INTERVAL;
  //=======================
  if(i2c_cntr) i2c_cntr--;
  else
  {
    state.i2c = 1;
    i2c_cntr = I2C_INTERVAL;
  }
  //=======================
  if(uart_cntr) uart_cntr--;
  else
  {
//    uart_cntr = UART_INTERVAL;
    if(state.enTrans == 1)
    {
      byteCntr = 0;
      state.cmdIn = 1;
      state.enTrans = 0;
    }
  }  
  //=============== op ======================
  if(operation_duration) operation_duration--;
  else   
  {
      blink = BLINK_WAIT;
      if(state.moveH)                 state.moveH = 0;
      if(!state.moveH && state.moveV) state.moveV = 0;
      STOP_MOTORS          
  }
  
  blinking(blink);
  leds(ledStt);
}


   