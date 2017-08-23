//==============================================================================
#include "io430.h"
#include "LEDS.h"
#include "i2c.h"
#include "UART.h"
#include "LSM303.h"
#include "BH1715.h"
#include "math.h"
//==============================================================================
u3AXIS_DATA accel;
u3AXIS_DATA compass;
#define FILTER_LENGHT   (50)
//==============================================================================
unsigned int mFilter(int * aBuf, unsigned int aLng)
{  
//#define MEDIAN
  
#ifdef MEDIAN
  unsigned int Buf[FILTER_LENGHT];
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
//int accTmp[3][FILTER_LENGHT];
//int magTmp[3][FILTER_LENGHT];

int rollArray[FILTER_LENGHT];
int pitchArray[FILTER_LENGHT];

double accxnorm;
double accynorm;
double Pitch;
double Roll; 
const double toDeg = 1800 / 3.14159265359;
//==============================================================================
unsigned char tmp[10];
unsigned char mPres = 1, enTrans = 0;

unsigned int light;
//==============================================================================
void getAngles(u3AXIS_DATA* aRaw)
{
  //===== roll, pitch ===========
  double cdf = 
    (double)aRaw->x * (double)aRaw->x + 
      (double)aRaw->y * (double)aRaw->y +
        (double)aRaw->z * (double)aRaw->z;      
  accxnorm = (double)aRaw->x / sqrt(cdf);     
  accynorm = (double)aRaw->y / sqrt(cdf);
  
  Pitch = asin(-accxnorm);
  Roll =  asin(accynorm/cos(Pitch));
  //====== yaw ======================
  
}
//==============================================================================
void addValueToArray(int aVal, int * aArr)
{
  for(unsigned int k = FILTER_LENGHT-1; k > 0; k--)
    aArr[k] = aArr[k-1];
  
  aArr[0] = aVal;
}
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
  //==============================================================
  LSM303Init();
  //=============================
  TACCR0 = 0xffff;
  TACTL = TASSEL_2 + MC_1 + ID_0;                  // SMCLK, contmode  
  TACCTL0 |= CCIE; 
  //=============================
  P1DIR &= ~BIT4; 
  ledStt = 0;
  __enable_interrupt();  
  
  //==============================================================
//  BH1715(1, 0x23, 0x01, 0, 1);
//  BH1715(1, 0x23, 0x11, 0, 1);  
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
//      BH1715(0, 0x23, 0x01, (unsigned char*)&light, 2);
//      txBuf.light = ((unsigned char*)&light)[1] | 
//                    ((unsigned char*)&light)[0] << 8;
      
      lsm303(I2C_READ,  LSM303A_I2C_ADDR, LSM303A_OUT_X_L, accel.byte, 6);      
      getAngles(&accel);
      
      addValueToArray((int)(Roll*10000),  rollArray);
      addValueToArray((int)(Pitch*10000), pitchArray);
      
      lsm303(I2C_READ,  LSM303M_I2C_ADDR, LSM303M_OUT_X_H, tmp, 6);
     
      compass.x = (tmp[0] << 8) | tmp[1] ;
      compass.z = (tmp[2] << 8) | tmp[3] ;
      compass.y = (tmp[4] << 8) | tmp[5] ;

    }
    //======= send data ==================
    if(mPres & enTrans) 
    {
      enTrans = 0;
      
      txBuf.accel.x = mFilter(rollArray, FILTER_LENGHT);
      txBuf.accel.y = mFilter(pitchArray, FILTER_LENGHT);
      
      txBuf.compass.x = compass.x;
      txBuf.compass.y = compass.y;
      txBuf.compass.z = compass.z;
      
      msgTransmitt(); 
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
  static int cntr = 10;
  if(cntr) cntr--;
  else
  {
    cntr = 20;
    enTrans = 1;
  }
  i2c = 1;
}


   