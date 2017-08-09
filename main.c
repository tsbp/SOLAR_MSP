//==============================================================================
#include "io430.h"
#include "LEDS.h"
#include "i2c.h"
#include "UART.h"
#include "LSM303.h"
#include "math.h"
//==============================================================================
u3AXIS_DATA accel;
u3AXIS_DATA compass;
//==============================================================================
void BH1715(unsigned char aOp, unsigned char aDev, unsigned char aOpCode, unsigned char *aBuf, unsigned int aLng)
{
  
//  if(aOp == LSM303_READ && aLng > 1) MSData[0] = (aReg | BIT7);
//  else                               MSData[0] = (aReg);
    
  if(aOp == LSM303_WRITE) 
  {
    MSData[0] = aOpCode;//*aBuf;
    NUM_BYTES_TX = 1;
    Setup_TX(aDev);
    i2cTransmit(); 
     while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
  }  
//  else
//    NUM_BYTES_TX = 1;
   //Transmit process
   
  
  
 
  
  if(aOp == LSM303_READ)
  {
    Setup_RX(aDev);
    i2cReceive(aLng/*6*/, aBuf);
    while (UCB0CTL1 & UCTXSTP);
    __no_operation();
  }
}
//==============================================================================
unsigned int mFilter(int * aBuf, unsigned int aLng)
{
  
//#define MEDIAN
  
#ifdef MEDIAN
  unsigned int Buf[20];
  for(unsigned int i = 0; i < aLng; i++) Buf[i] = aBuf[i];
  
  
  for(unsigned int k = 0; k < aLng - 1; k++)
    for(unsigned int i = k; i < aLng - 1; i++)
    {
      if(Buf[k] < Buf[i+1])
      {
        unsigned int tmp =  Buf[k];
        Buf[k] = Buf[i+1];
        Buf[i+1] = tmp;
      }
    }
  return Buf[aLng / 2];
#else 
  long sum = 0;
  for(unsigned int i = 0; i < aLng; i++)sum += aBuf[i];
  return sum / aLng; 
  
#endif
}
//==============================================================================
unsigned long op = 0;
unsigned char i2c_tmr = 0, i2c = 0;
unsigned int RxByteCtr;
unsigned int RxWord;
int accTmp[3][10];
int magTmp[3][10];
//==============================================================================
unsigned char tmp[10];
unsigned char mPres = 0;

unsigned int light;
//==============================================================================
void main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  BCSCTL1 = CALBC1_8MHZ;                    // Set DCO
  DCOCTL = CALDCO_8MHZ; 
  __delay_cycles(1000000L);
  //======= uart ================
  uartInit();
  //========= i2c ===============
  i2cInit();  
  TACTL = TASSEL_2 + MC_2 + ID_3;                  // SMCLK, contmode  
  TACCTL0 |= CCIE; 
  //=============================
  P1DIR &= ~BIT4; 
  ledStt = 0;
  __enable_interrupt();  
  //==============================================================
  LSM303Init();
  //==============================================================
  BH1715(1, 0x23, 0x01, 0, 1);
  BH1715(1, 0x23, 0x11, 0, 1);  
  //==============================================================
  
  while(1)
  {
    blinking(blink);
    leds(ledStt);
    if(txEn)
    {
      mPres = 1; 
      txEn = 0;
      txBuf.cmd = rxBuf[0];
      switch(rxBuf[0])
      {
          case 0:
            RxWord = rxBuf[1] | (rxBuf[2] << 8);
            
            
            if(RxWord > 50)  blink = BLINK_BACKWARD;//BLINK_UP;
            else             blink = BLINK_FORWARD; //BLINK_DOWN; 
            op =  100000L;
            
            msgTransmitt();            
            break;
            
          case 1:            
            msgTransmitt();   
            break;
      }
    }
    //=============== i2c =====================
    if(i2c) 
    {
      i2c = 0;
      BH1715(0, 0x23, 0x01, (unsigned char*)&light, 2);
      txBuf.light = ((unsigned char*)&light)[1] | 
                    ((unsigned char*)&light)[0] << 8;
      // shift
      for(unsigned int i = 0; i < 3; i++)
        for(unsigned int k = 10-1; k > 0; k--)
        {
          accTmp[i][k] = accTmp[i][k-1];
          magTmp[i][k] = magTmp[i][k-1];
        }
      
      lsm303(LSM303_READ,  LSM303A_I2C_ADDR, LSM303A_OUT_X_L, accel.byte, 6);
      accTmp[0][0] = accel.x;
      accTmp[1][0] = accel.y;
      accTmp[2][0] = accel.z;
      
      lsm303(LSM303_READ,  LSM303M_I2C_ADDR, LSM303M_OUT_X_H, tmp, 6);
     
      compass.x = (tmp[0] << 8) | tmp[1] ;
      compass.z = (tmp[2] << 8) | tmp[3] ;
      compass.y = (tmp[4] << 8) | tmp[5] ;
      
      magTmp[0][0] = compass.x;
      magTmp[1][0] = compass.y;
      magTmp[2][0] = compass.z;
      
      txBuf.accel.x = mFilter(accTmp[0], 10);
      txBuf.accel.y = mFilter(accTmp[1], 10);
      txBuf.accel.z = mFilter(accTmp[2], 10);
      
      txBuf.compass.x = mFilter(magTmp[0], 10);
      txBuf.compass.y = mFilter(magTmp[1], 10);
      txBuf.compass.z = mFilter(magTmp[2], 10);
      
      
      
      if(mPres) msgTransmitt(); 
    }
   
    //=============== op ======================
    if(op) op--;
    else   blink = BLINK_WAIT;
  }
}
//==============================================================================
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
   static unsigned int stuffing = 0, rec_counter=0;  // счетчик принятых байт  
   unsigned char rxByte = UCA0RXBUF;
   //============================
   if(rxByte == 0xc0)
   {     
     ledStt |= LED_RED;
     rec_counter = 0;     
   }
   else 
    {
      if ((rxByte == 0xdb) && !stuffing) stuffing = 1;
      else if (stuffing) 
      {
        if      (rxByte == 0xdc) rxByte = 0xc0;
        else if (rxByte == 0xdd) rxByte = 0xdb;
        stuffing = 0;
      }
      if(!stuffing)
      {        
         if(rec_counter < sizeof(rxBuf)) rxBuf[rec_counter] = rxByte;
         if(rxBuf[5] == 0xcc && rxBuf[6] == 0xcc)  txEn = 1;
         rec_counter++;
      } 
    }
}
//==============================================================================
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void)
{
  i2c = 1;
}


   